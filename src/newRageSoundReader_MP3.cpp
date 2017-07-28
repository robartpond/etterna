#include "newRageSoundReader_MP3.h"
#include <io.h>"

newRageSoundReader_MP3::newRageSoundReader_MP3()
{
	codec=NULL;
	codecCtx = NULL;
	decodedFrame=NULL;
	length = 0;
	numChannels = 0;
	sampleRate = 1;
}


newRageSoundReader_MP3::~newRageSoundReader_MP3()
{
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
RageSoundReader_FileReader::OpenResult newRageSoundReader_MP3::Open(RageFileBasic *pFile)
{
	RegisterProtocols();
	m_pFile = pFile;

	if (codec)
		avcodec::av_free(codec);
	if (codecCtx)
		avcodec::avcodec_close(codecCtx);
	if (decodedFrame)
		avcodec::av_frame_free(&decodedFrame);

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
	
	if (avcodec::avcodec_open2(codecCtx, codec, NULL) < 0) {
		SetError("Error opening decoder");
		return OPEN_UNKNOWN_FILE_FORMAT;
	}
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
	int numSamples = decodedFrame->nb_samples;
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
	return -1;
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
	if (decodedFrame) {
		samplesToRead -= WriteSamples(buf, samplesToRead);
	}
	if (samplesToRead <= 0)
		return iFrames;
	while (samplesToRead > 0) {
		switch (ReadAFrame()) {
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

//Return: -1=> Error already set. >=0 => BytesWritten
int newRageSoundReader_MP3::WriteSamples(void *pBuf, int samplesToRead)
{
	char *buf = (char*)(pBuf);
	int dataSize = avcodec::av_get_bytes_per_sample(codecCtx->sample_fmt);
	int i, ch;
	if (dataSize < 0) {
		SetError("Error while reading (No data)");
		return -1;
	}
	unsigned int firstSample = curSample;
	unsigned int firstChannel = curChannel;
	unsigned int samplesWritten = 0;
	if((numChannels-curChannel) * (numSamples-curSample) <= samplesToRead)
		for (; curChannel < numChannels; curChannel++) {
			for (; curSample < numSamples; curSample++) {
				memcpy(buf + samplesWritten, decodedFrame->data[ch] + dataSize*curSample, dataSize);
				samplesWritten++;
			}
			curSample = 0;
		}
	else
		for (; curChannel < numChannels; curChannel++) {
			if (numSamples - curSample <= samplesToRead) {
				for (; curSample < numSamples; curSample++) {
					memcpy(buf + samplesWritten, decodedFrame->data[ch] + dataSize*curSample, dataSize);
					samplesWritten++;
				}
				curSample = 0;
			}
			else {
				for (; curSample < samplesToRead; curSample++) {
					memcpy(buf + samplesWritten, decodedFrame->data[ch] + dataSize*curSample, dataSize);
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
	uint8_t buffer[20480 + 32];
	unsigned int bytesRead = 0;
	avcodec::AVPacket avpkt;
	avcodec::av_init_packet(&avpkt);
	avpkt.data = buffer;
	avpkt.size = m_pFile->Read(buffer, 20480);
	bytesRead += avpkt.size;
	if (decodedFrame)
		avcodec::av_frame_free(&decodedFrame);
	decodedFrame = avcodec::av_frame_alloc();
	while (avpkt.size > 0) {
		if (!decodedFrame) {
			if (!(decodedFrame = avcodec::av_frame_alloc()))
				SetError("Error allocating memory for frame"); {
				return -1;
			}
		}
		int gotFrame = 0;
		int len = avcodec_decode_audio4(codecCtx, decodedFrame, &gotFrame, &avpkt);
		avpkt.size -= len;
		avpkt.data += len;
		if (gotFrame) {
			//Go back to the beginning of the next frame
			curFrame++;
			curChannel = 0;
			curSample = 0;
			m_pFile->Seek(m_pFile->Tell() - avpkt.size);
			return bytesRead-avpkt.size;
		}
		if (len < 0) {
			SetError("Error decoding");
			return -1;
		}
		avpkt.dts =
			avpkt.pts = AV_NOPTS_VALUE;
		if (avpkt.size < 4096) {
			/* Refill the input buffer, to avoid trying to decode
			* incomplete frames. Instead of this, one could also use
			* a parser, or use a proper container format through
			* libavformat. */
			memmove(buffer, avpkt.data, avpkt.size);
			avpkt.data = buffer;
			uint8_t aux[1024];
			len = m_pFile->Read(avpkt.data + avpkt.size, 20480 - avpkt.size);
			if (len > 0) {
				avpkt.size += len;
				bytesRead += len;
			} else {
				return -2;
			}

		}
	}
}

int newRageSoundReader_MP3::GetNextSourceFrame()
{
	return -1;
}