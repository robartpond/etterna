#include "newRageSoundReader_MP3.h"
#include <io.h>

newRageSoundReader_MP3::newRageSoundReader_MP3()
{
	codec=NULL;
	codecCtx = NULL;
	decodedFrame=NULL;
	IOCtx = NULL;
	formatCtx = NULL;
	length = 0;
	numChannels = 0;
	sampleRate = 1;
}


newRageSoundReader_MP3::~newRageSoundReader_MP3()
{
	//Free everything if it isn't already
	if (codecCtx)
		avcodec::avcodec_close(codecCtx);
	if (formatCtx)
		avcodec::avformat_close_input(&formatCtx);  // AVFormatContext is released by avformat_close_input
	if (IOCtx)
		avcodec::av_free(IOCtx);             // AVIOContext is released by av_free
	if (codec)
		avcodec::av_free(codec);
	if (decodedFrame)
		avcodec::av_frame_free(&decodedFrame);
}

void RegisterProtocols()
{
	static bool Done = false;
	if (Done)
		return;
	Done = true;

	avcodec::avcodec_register_all();
	avcodec::av_register_all();
}
int ReadFunc(void* ptr, uint8_t* buf, int buf_size)
{
	RageFileBasic *pFile = reinterpret_cast<RageFileBasic *>(ptr);
	return pFile->Read(buf, buf_size);
}
// whence: SEEK_SET, SEEK_CUR, SEEK_END (like fseek) and AVSEEK_SIZE
int64_t SeekFunc(void* ptr, int64_t pos, int whence)
{
	RageFileBasic *pFile = reinterpret_cast<RageFileBasic *>(ptr);
	if (whence == AVSEEK_SIZE)
		return -1;
	return pFile->Seek(static_cast<int>(pos), whence);
}
RageSoundReader_FileReader::OpenResult newRageSoundReader_MP3::Open(RageFileBasic *pFile)
{
	RegisterProtocols();
	m_pFile = pFile;

	//Free everything if it isn't already
	if (formatCtx)
		avcodec::avformat_close_input(&formatCtx);  // AVFormatContext is released by avformat_close_input
	if (IOCtx)
		avcodec::av_free(IOCtx);             // AVIOContext is released by av_free
	if (codecCtx)
		avcodec::avcodec_close(codecCtx);
	if (codec)
		avcodec::av_free(codec);
	if (decodedFrame)
		avcodec::av_frame_free(&decodedFrame);

	IOCtx = avcodec::avio_alloc_context(buffer, 20480 + 64,  // internal Buffer and its size
		0,                  // bWriteable (1=true,0=false) 
		pFile,          // user data ; will be passed to our callback functions
		ReadFunc,
		0,                  // Write callback function (not used in this example) 
		SeekFunc);

	formatCtx = avcodec::avformat_alloc_context();
	formatCtx->pb = IOCtx;


	// Determining the input format:
	int readBytes = 0;
	readBytes += m_pFile->Read(buffer, 20480 + 64);
	m_pFile->Seek(0, SEEK_SET);
	avcodec::AVProbeData probeData;
	probeData.buf = buffer;
	probeData.buf_size = readBytes;
	probeData.filename = "";

	// Determine the input-format:
	formatCtx->iformat = av_probe_input_format(&probeData, 1);
	formatCtx->flags = AVFMT_FLAG_CUSTOM_IO;
	if (avcodec::avformat_open_input(&formatCtx, "", 0, 0) != 0) {
		SetError("Error opening file");
		return OPEN_UNKNOWN_FILE_FORMAT;
	}

	// Retrieve stream information
	if (avcodec::avformat_find_stream_info(formatCtx, NULL)<0) {
		SetError("Couldn't find stream information");
		return OPEN_UNKNOWN_FILE_FORMAT;
	}

	// Find the first audio stream
	audioStream = -1;
	int i = 0;
	int nbStreams = formatCtx->nb_streams;
	for (i = 0; i<nbStreams; i++) {
		if (formatCtx->streams[i]->codec->codec_type == avcodec::AVMEDIA_TYPE_AUDIO) {
			audioStream = i;
			break;
		}
	}
	if (audioStream == -1) {
		SetError("Didn't find a audio stream");
		return OPEN_UNKNOWN_FILE_FORMAT;
	}
	// Get a pointer to the codec context for the audio stream
	codecCtx = formatCtx->streams[audioStream]->codec;

	// Find the decoder for the audio stream
	codec = avcodec::avcodec_find_decoder(codecCtx->codec_id);
	if (codec == NULL) {
		SetError("Codec not found\n");
		return OPEN_UNKNOWN_FILE_FORMAT;
	}

	/*
	codec = avcodec::avcodec_find_decoder(avcodec::AV_CODEC_ID_MP3);
	if (!codec) {
		SetError("Error finding decoder");
		return OPEN_UNKNOWN_FILE_FORMAT;
	}

	codecCtx = avcodec::avcodec_alloc_context3(codec);
	if (!codecCtx) {
		SetError("Error allocating context");
		return OPEN_UNKNOWN_FILE_FORMAT;
	}
	*/
	if (avcodec::avcodec_open2(codecCtx, codec, NULL) < 0) {
		SetError("Error opening decoder");
		return OPEN_UNKNOWN_FILE_FORMAT;
	}
	curFrame = 0;
	curChannel = 0;
	curSample = 0;
	switch (ReadAFrame()) {
	case -1:
		return OPEN_UNKNOWN_FILE_FORMAT;
		break;
	case -2:
		SetError("Couldn't read a frame");
		return OPEN_UNKNOWN_FILE_FORMAT;
		break;
	};
	numChannels = codecCtx->channels;
	numSamples = decodedFrame->nb_samples;
	bitrate = codecCtx->bit_rate;
	sampleRate = codecCtx->sample_rate;
	//Aproximation to the length?
	length = m_pFile->GetFileSize() / bitrate;
	return OPEN_OK;
}


newRageSoundReader_MP3 *newRageSoundReader_MP3::Copy() const 
{
	newRageSoundReader_MP3 *ret = new newRageSoundReader_MP3;
	RageFileBasic *pFile = m_pFile->Copy();
	pFile->Seek(0);
	ret->Open(pFile);
	return ret;
}

//0=> EOF. -1=> Error . >=0 => Properly SetPosition
int newRageSoundReader_MP3::SetPosition(int iFrame)
{
	return avcodec::av_seek_frame(formatCtx, audioStream, iFrame, AVSEEK_FLAG_ANY);
}

//I think this is supposed to read samples, not frames
//Each sample being a float to be stored in the buffer
//So we need to read and decode a frame(Each frame has 1 or more channels and each has samples) if needed
//Then read samples from the frame until the frame finishes or we read enough samples
//Then either return the samples read or read and decode another frame
int newRageSoundReader_MP3::Read(float *pBuf, int iFrames)
{
	char* buf = (char*)(pBuf);//Increases by 1 byte
	unsigned int samplesToRead = iFrames;
	if (decodedFrame!=NULL) {
		samplesToRead -= WriteSamples(buf, samplesToRead);
	}
	if (samplesToRead <= 0)
		return iFrames;
	while (samplesToRead > 0) {
		int a = ReadAFrame();
		switch (a) {
		case -1:
			return ERROR;
			break;
		case -2:
			SetError("EOF");
			return END_OF_FILE;
			break;
		};
		samplesToRead -= WriteSamples(buf+iFrames-samplesToRead, samplesToRead);
	}
	return iFrames-samplesToRead;
}

//Return: -1=> Error already set. >=0 => SamplesWritten
int newRageSoundReader_MP3::WriteSamples(void *pBuf, int samplesToRead)
{
	char *buf = (char*)(pBuf);
	int dataSize = avcodec::av_get_bytes_per_sample(codecCtx->sample_fmt);

	if (dataSize < 0) {
		SetError("Error while reading (No data)");
		return -1;
	}
	if (!decodedFrame) {
		int a = ReadAFrame();
		switch (a) {
		case -1:
			return ERROR;
			break;
		case -2:
			SetError("EOF");
			return END_OF_FILE;
			break;
		};
	}
	unsigned int firstSample = curSample;
	unsigned int firstChannel = curChannel;
	unsigned int samplesWritten = 0;
	numSamples = decodedFrame->nb_samples;
	if((numChannels-curChannel) * (numSamples-curSample) <= samplesToRead)
		for (; curChannel < numChannels; curChannel++) {
			for (; curSample < numSamples; curSample++) {
				memcpy(buf + samplesWritten, decodedFrame->data[curChannel] + dataSize*curSample, dataSize);
				samplesWritten++;
			}
			curSample = 0;
		}
	else
		for (; curChannel < numChannels; curChannel++) {
			if (numSamples - curSample <= samplesToRead) {
				for (; curSample < numSamples; curSample++) {
					memcpy(buf + samplesWritten, decodedFrame->data[curChannel] + dataSize*curSample, dataSize);
					samplesWritten++;
				}
				curSample = 0;
			}
			else {
				for (; curSample < samplesToRead; curSample++) {
					memcpy(buf + samplesWritten, decodedFrame->data[curChannel] + dataSize*curSample, dataSize);
					samplesWritten++;
				}
				return samplesWritten;
			}
		}
	//Free the frame since we've read it all
	avcodec::av_frame_free(&decodedFrame);
	return samplesWritten;

}

//Return: -1 => Error already set. -2 => EOF. >=0 => bytesRead
int newRageSoundReader_MP3::ReadAFrame()
{
	avcodec::AVPacket avpkt;
	avcodec::av_init_packet(&avpkt);
	avpkt.data = buffer;
	if (decodedFrame)
		avcodec::av_frame_free(&decodedFrame);
	if (!(decodedFrame = avcodec::av_frame_alloc())) {
		SetError("Error allocating memory for frame");
		return -1;
	}
	while (avcodec::av_read_frame(formatCtx, &avpkt) >= 0) {
	//while (avpkt.size > 0) {
		if (avpkt.stream_index == audioStream) {
			int size = avpkt.size;
			while (avpkt.size > 0) {
				int gotFrame = 0;
				int len = avcodec_decode_audio4(codecCtx, decodedFrame, &gotFrame, &avpkt);
				if (len == -1) {
					SetError("Error while decoding\n");
					return -1;
				}
				//avpkt.size -= len;
				//avpkt.data += len;
				if (gotFrame) {
					//Go back to the beginning of the next frame
					curFrame++;
					curChannel = 0;
					curSample = 0;
					//m_pFile->Seek(m_pFile->Tell() - avpkt.size);
					//return bytesRead-avpkt.size;
					int read = avpkt.size;
					av_free_packet(&avpkt);
					return read;
				}
			}
			break;
			/*
			avpkt.dts =
				avpkt.pts = AV_NOPTS_VALUE;
			if (avpkt.size < 4096) {
				// Refill the input buffer, to avoid trying to decode
				// incomplete frames. Instead of this, one could also use
				// a parser, or use a proper container format through
				// libavformat. 
				memmove(buffer, avpkt.data, avpkt.size);
				avpkt.data = buffer;
				len = m_pFile->Read(avpkt.data + avpkt.size, 20480 - avpkt.size);
				if (len > 0) {
					avpkt.size += len;
					bytesRead += len;
				} else {
					return -2;
				}
			}
			*/
		}
	}
	av_free_packet(&avpkt);
	return -2;
}

int newRageSoundReader_MP3::GetNextSourceFrame()
{
	return -1;
}