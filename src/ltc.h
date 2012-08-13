/** 
   @brief libltcsmpte - en+decode linear SMPTE timecode
   @file ltcsmpte.h
   @author Robin Gareus <robin@gareus.org>

   Copyright (C) 2006 Robin Gareus <robin@gareus.org>
   Copyright (C) 2008-2009 Jan <jan@geheimwerk.de>

   inspired by SMPTE Decoder - Maarten de Boer <mdeboer@iua.upf.es>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/
#ifndef LTCSMPTE_H
#define LTCSMPTE_H 1

#ifdef __cplusplus
extern "C" {
#endif
#ifndef DOXYGEN_IGNORE
// libltcsmpte version
#define LIBLTC_VERSION "0.0.1"
#define LIBLTC_VERSION_MAJOR  0
#define LIBLTC_VERSION_MINOR  0
#define LIBLTC_VERSION_MICRO  1

//interface revision number
//http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
#define LIBLTC_CUR  0
#define LIBLTC_REV  0
#define LIBLTC_AGE  0
#endif

#include <sys/types.h>

#define FPRNT_TIME "%lf"
#define TIME_DELIM	"\t"

#define USE8BIT
typedef unsigned char sample_t;
typedef short curve_sample_t;
#define SAMPLE_AND_CURVE_ARE_DIFFERENT_TYPE
#define SAMPLE_CENTER	128 // unsigned 8 bit.
#define CURVE_MIN 	-127	
#define CURVE_MAX 	127
#define SAMPLE_IS_UNSIGNED
#define SAMPLE_IS_INTEGER


#define LTC_FRAME_BIT_COUNT	80
/**
 * Raw 80 bit SMPTE frame 
 */
#ifdef __BIG_ENDIAN__
// Big Endian version, bytes are "upside down"
typedef struct SMPTEFrame {
	unsigned int user1:4;
	unsigned int frameUnits:4;
	
	unsigned int user2:4;
	unsigned int colFrm:1;
	unsigned int dfbit:1;
	unsigned int frameTens:2;
	
	unsigned int user3:4;
	unsigned int secsUnits:4;
	
	unsigned int user4:4;
	unsigned int biphaseMarkPhaseCorrection:1;
	unsigned int secsTens:3;
	
	unsigned int user5:4;
	unsigned int minsUnits:4;
	
	unsigned int user6:4;
	unsigned int binaryGroupFlagBit1:1;
	unsigned int minsTens:3;
	
	unsigned int user7:4;
	unsigned int hoursUnits:4;
	
	unsigned int user8:4;
	unsigned int binaryGroupFlagBit2:1;
	unsigned int reserved:1;
	unsigned int hoursTens:2;
	
	unsigned int syncWord:16;
} SMPTEFrame;

#else
// Little Endian version (default)
typedef struct SMPTEFrame {
	unsigned int frameUnits:4;
	unsigned int user1:4;
	
	unsigned int frameTens:2;
	unsigned int dfbit:1;
	unsigned int colFrm:1;
	unsigned int user2:4;
	
	unsigned int secsUnits:4;
	unsigned int user3:4;
	
	unsigned int secsTens:3;
	unsigned int biphaseMarkPhaseCorrection:1;
	unsigned int user4:4;
	
	unsigned int minsUnits:4;
	unsigned int user5:4;
	
	unsigned int minsTens:3;
	unsigned int binaryGroupFlagBit1:1;
	unsigned int user6:4;
	
	unsigned int hoursUnits:4;
	unsigned int user7:4;
	
	unsigned int hoursTens:2;
	unsigned int reserved:1;
	unsigned int binaryGroupFlagBit2:1;
	unsigned int user8:4;
	
	unsigned int syncWord:16;
} SMPTEFrame;

#endif

/**
 * Human readable time representation
 */
typedef struct SMPTETime {
// these are only set when compiled with ENABLE_DATE
	char timezone[6];
	unsigned char years;
	unsigned char months;
	unsigned char days;
// 
	unsigned char hours;
	unsigned char mins;
	unsigned char secs;
	unsigned char frame;
} SMPTETime;




/**
 * Extended SMPTE frame 
 * The maximum error for startpos is 1/80 of a frame. Usually it is 0;
 */
typedef struct SMPTEFrameExt {
	SMPTEFrame base; ///< the SMPTE decoded from the audio
	int delayed; ///< detected jitter in LTC-framerate/80 unit(s) - bit count in LTC frame.
	long int startpos; ///< the approximate sample in the stream corresponding to the start of the LTC SMPTE frame. 
	long int endpos; ///< the sample in the stream corresponding to the end of the LTC SMPTE frame.
} SMPTEFrameExt;


/**
 * opaque structure.
 * see: SMPTEDecoderCreate, SMPTEFreeDecoder
 */
typedef struct SMPTEDecoder SMPTEDecoder;

/**
 * opaque structure
 * see: SMPTEEncoderCreate, SMPTEFreeEncoder
 */
typedef struct SMPTEEncoder SMPTEEncoder;


/**
 * convert binary SMPTEFrame into SMPTETime struct 
 */
int SMPTEFrameToTime(SMPTEFrame* frame, SMPTETime* stime);

/**
 * convert SMPTETime struct into it's binary SMPTE representation.
 */
int SMPTETimeToFrame(SMPTETime* stime, SMPTEFrame* frame);

/**
 * set all values of a SMPTE FRAME to zero except for the sync-word (0x3FFD) at the end.
 * This will also clear the dfbit. 
 */
int SMPTEFrameReset(SMPTEFrame* frame);

/**
 * increments the SMPTE by one SMPTE-Frame (1/framerate seconds)
 */
int SMPTEFrameIncrease(SMPTEFrame *frame, int framesPerSec);


/**
 * Create a new decoder. Pass sample rate, number of smpte frames per 
 * seconds, and the size of the internal queue where decoded frames are
 * stored. Set correctJitter flag, to correct jitter resulting from
 * audio fragment size when decoding from a realtime audiostream. This
 * works only correctly when buffers of exactly fragment size are passed
 * to SMPTEDecoderWrite. (as discussed on LAD [msgID])
 */
SMPTEDecoder * SMPTEDecoderCreate(int sampleRate, int queueSize, int correctJitter);


/**
 * release memory of Decoder structure.
 */
int SMPTEFreeDecoder(SMPTEDecoder *d);

/**
 * Reset the decoder error tracking internals
 */
int SMPTEDecoderErrorReset(SMPTEDecoder *decoder);

/**
 * Feed the SMPTE decoder with new samples. 
 *
 * parse raw audio for LTC timestamps. If found, store them in the 
 * Decoder queue (see SMPTEDecoderRead)
 * always returns 1 ;-)
 * d: the decoder 
 * buf: pointer to sample_t (defaults to unsigned 8 bit) mono audio data
 * size: number of bytes to parse
 * posinfo: (optional) byte offset in stream to set LTC location offset
 */
int SMPTEDecoderWrite(SMPTEDecoder *decoder, sample_t *buf, int size, long int posinfo);

/**
 * All decoded SMPTE frames are placed in a queue. This function gets 
 * a frame from the queue, and stores it in SMPTEFrameExt* frame.
 * Returns 1 on success, 0 when no frames where on the queue, and
 * and errorcode otherwise.
 */
int SMPTEDecoderRead(SMPTEDecoder *decoder, SMPTEFrameExt *frame);

/** 
 * flush unread LTCs in queue and return the last timestamp.
 * (note that the last timestamp may not be the latest if the queue has
 * overflown!)
 */
int SMPTEDecoderReadLast(SMPTEDecoder* decoder, SMPTEFrameExt* frame);

/** 
 * Convert the index or position of a sample to 
 * its position in time (seconds) relative to the first sample. 
 */
double SMPTEDecoderSamplesToSeconds(SMPTEDecoder* d, long int sampleCount);

/** 
 * Allocate and initialize LTC encoder
 * @param sampleRate: audio sample rate (eg. 48000)
 */
SMPTEEncoder * SMPTEEncoderCreate(int sampleRate, double fps);

/** 
 * release encoder data structure
 */
int SMPTEFreeEncoder(SMPTEEncoder *e);

/**
 * set internal Audio-sample counter. 
 * SMPTEEncode() increments this counter when it encodes samples.
 */
int SMPTESetNsamples(SMPTEEncoder *e, int val);

/**
 * returns the current values of the Audio-sample counter.
 * ie. the number of encoded samples.
 */
int SMPTEGetNsamples(SMPTEEncoder *e);

/**
 * moves the SMPTE to the next frame.
 * it uses SMPTEFrameIncrease().
 */
int SMPTEEncIncrease(SMPTEEncoder *e, int fps);

/**
 * sets the current encoder SMPTE frame to SMPTETime.
 */
int SMPTESetTime(SMPTEEncoder *e, SMPTETime *t);

/** 
 * set t from current Encoder timecode
 */
int SMPTEGetTime(SMPTEEncoder *e, SMPTETime *t);

/**
 * returns and flushes the accumulated Encoded Audio.
 * returns the number of bytes written to the memory area
 * pointed to by buf.
 *
 * no overflow check is perfomed! use DTMFGetBuffersize().
 */
int SMPTEGetBuffer(SMPTEEncoder *e, sample_t *buf);

/**
 * returns the size of the accumulated encoded Audio in bytes.
 */
size_t SMPTEGetBuffersize(SMPTEEncoder *e);

/**
 * Generate LTC audio for byte "byteCnt" of the current frame into the buffer of Encoder e.
 * LTC has 10 bytes per frame: 0 <= bytecnt < 10 
 * use SMPTESetTime(..) to set the current frame before Encoding.
 * see tests/encoder.c for an example.
 */
int SMPTEEncode(SMPTEEncoder *e, int byteCnt);

#ifdef __cplusplus
}
#endif

#endif
