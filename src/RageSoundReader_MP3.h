/* RageSoundReader_MP3 - An interface to read MP3s via MAD. */

#ifndef RAGE_SOUND_READER_MP3_H
#define RAGE_SOUND_READER_MP3_H

#include "RageSoundReader_FileReader.h"
#include "RageFile.h"


namespace avcodec
{
	extern "C"
	{
		#include <libavformat/avformat.h>
		#include <libswscale/swscale.h>
		#include <libavutil/pixdesc.h>
		#include <libavutil/opt.h>
		#include <libavcodec/avcodec.h>
		#include <libavutil/channel_layout.h>
		#include <libavutil/common.h>
		#include <libavutil/imgutils.h>
		#include <libavutil/mathematics.h>
		#include <libavutil/samplefmt.h>
	}
};

#define INBUF_SIZE 4096
#define AUDIO_INBUF_SIZE 8192
#define AUDIO_REFILL_THRESH 4096

struct madlib_t;

typedef unsigned long id3_length_t;

signed long id3_tag_query( const unsigned char *data, id3_length_t length );
void fill_frame_index_cache( madlib_t *mad );

class RageSoundReader_MP3: public RageSoundReader_FileReader
{
public:
	OpenResult Open( RageFileBasic *pFile );
	void Close();
	int GetLength() const { return GetLengthConst(false); }
	int GetLength_Fast() const { return GetLengthConst(true); }
	int SetPosition( int iSample );
	int Read( float *pBuf, int iFrames );
	unsigned GetNumChannels() const { return Channels; }
	int GetSampleRate() const { return SampleRate; }
	int GetNextSourceFrame() const;
	bool SetProperty( const RString &sProperty, float fValue );

	RageSoundReader_MP3();
	~RageSoundReader_MP3();
	RageSoundReader_MP3( const RageSoundReader_MP3 & ); /* not defined; don't use */
	RageSoundReader_MP3 *Copy() const;

private:
	int SampleRate;
	int Channels;
	bool m_bAccurateSync;

	madlib_t *mad;

	//Codec to decode with
    avcodec::AVCodec *codec;
	//Codec context
    avcodec::AVCodecContext *Context;
	//Packet used to decode frames
    avcodec::AVPacket avpkt;
	//Last decoded frame
    avcodec::AVFrame *decoded_frame;
	int length;
	//Inner buffer to read the file
	uint8_t inbuf[AUDIO_INBUF_SIZE + 32];
	int nbDecodedFrames;
	float decoded_data[AUDIO_INBUF_SIZE + 32];
	//Decode the whole file
	int DoFFMPEGDecode();
	//Decode one frame
	int DoFFMPEGFrameDecode();;


	bool MADLIB_rewind();
	int SetPosition_toc( int iSample, bool Xing );
	int SetPosition_hard( int iSample );
	int SetPosition_estimate( int iSample );

	int fill_buffer();
	int do_mad_frame_decode( bool headers_only=false );
	int resync();
	void synth_output();
	int seek_stream_to_byte( int byte );
	bool handle_first_frame();
	int GetLengthInternal( bool fast );
	int GetLengthConst( bool fast ) const;
};

#endif

/*
 * Copyright (c) 2001-2004 Chris Danford, Glenn Maynard
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

