#include "newRageSoundReader_MP3.h"
#include <io.h>"

newRageSoundReader_MP3::newRageSoundReader_MP3()
{
	SampleRate=0;
	Codec=NULL;
	Context=NULL;
	decoded_frame=NULL;
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
	avcodec::av_register_all(); 
	uint8_t buffer[20480 + 32];
	if (Codec)
		avcodec::av_free(Codec);
	if (Context)
		avcodec::avcodec_close(Context);
	if (decoded_frame)
		avcodec::av_frame_free(&decoded_frame);

	Codec = avcodec::avcodec_find_decoder(avcodec::AV_CODEC_ID_MP3);
	if (!Codec) {
		SetError("Error finding decoder");
		return OPEN_UNKNOWN_FILE_FORMAT;
	}
	Context = avcodec::avcodec_alloc_context3(Codec);
	if (!Context) {
		SetError("Error allocating context");
		return OPEN_UNKNOWN_FILE_FORMAT;
	}
	/* open it */
	if (avcodec::avcodec_open2(Context, Codec, NULL) < 0) {
		SetError("Error opening decoder");
		return OPEN_UNKNOWN_FILE_FORMAT;;
	}

	avcodec::AVPacket avpkt;
	avcodec::av_init_packet(&avpkt);
	decoded_frame = NULL;
	m_pFile = pFile;
	avpkt.data = buffer;
	avpkt.size = m_pFile->Read(buffer, 20480);
	if(decoded_frame)
		av_frame_free(&decoded_frame);
	decoded_frame = avcodec::av_frame_alloc();
	while (avpkt.size > 0) {
		if (!decoded_frame) {
			if (!(decoded_frame = avcodec::av_frame_alloc()))
				SetError("Error allocating memory for frame"); {
				return OPEN_UNKNOWN_FILE_FORMAT;
			}
		}
		int got_frame = 0;
		int len = avcodec_decode_audio4(Context, decoded_frame, &got_frame, &avpkt);
		if (got_frame) {
			return OPEN_OK;
		}
		if (len < 0) {
			SetError("Error decoding");
			return OPEN_UNKNOWN_FILE_FORMAT;
		}
		avpkt.size -= len;
		avpkt.data += len;
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
			if (len > 0)
				avpkt.size += len;
		}
	}
	SetError("Couldn't read a frame");
	return OPEN_UNKNOWN_FILE_FORMAT;
}

int newRageSoundReader_MP3::GetLength() const
{
	return 0;
}

newRageSoundReader_MP3 *newRageSoundReader_MP3::Copy() const 
{
	newRageSoundReader_MP3 *ret = new newRageSoundReader_MP3;
	RageFileBasic *pFile = m_pFile->Copy();
	pFile->Seek(0);
	ret->Open(pFile);
	return ret;
}

int newRageSoundReader_MP3::SetPosition(int iFrame)
{
	return 2;
}

int newRageSoundReader_MP3::Read(float *pBuf, int iFrames)
{
	return 0;
}
int newRageSoundReader_MP3::GetNextSourceFrame()
{
	return 0;
}