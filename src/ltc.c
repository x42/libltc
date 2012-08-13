/* 
   libltcsmpte - en+decode linear SMPTE timecode

   Copyright (C) 2006 Robin Gareus <robin@gareus.org>
   Copyright (C) 2008-2009 Jan <jan@geheimwerk.de>

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ltc.h"
#include "decoder.h"
#include "encoder.h"

void addvalues(SMPTEEncoder *e, int n);

/* -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * Decoder 
 */

SMPTEDecoder* SMPTEDecoderCreate(int sampleRate, int queueLen, int correctJitter) {
	SMPTEDecoder* d = (SMPTEDecoder*) calloc(1, sizeof(SMPTEDecoder));
	d->sampleRate = sampleRate;
	d->qLen = queueLen;
	d->queue = (SMPTEFrameExt*) calloc(d->qLen, sizeof(SMPTEFrameExt));
	d->biphaseToBinaryState = 1;
	d->soundToBiphasePeriod = d->sampleRate / 25 / 80;
#ifdef SAMPLE_IS_UNSIGNED
	d->soundToBiphaseLimit = (d->soundToBiphasePeriod * 14) >> 4;
#else
	d->soundToBiphaseLimit = (d->soundToBiphasePeriod * 14) / 16;
#endif
	d->correctJitter = correctJitter;
	
	d->samplesToSeconds = 1 / ((double)d->sampleRate * sizeof(sample_t));

	return d;
}

int SMPTEFreeDecoder(SMPTEDecoder *d) {
	if (!d) return 1;
	if (d->queue) free(d->queue);
	free(d);
	
	return 0;
}

double SMPTEDecoderSamplesToSeconds(SMPTEDecoder* d, long int sampleCount) {
	return (double)sampleCount * d->samplesToSeconds;
}

int SMPTEDecoderWrite(SMPTEDecoder *d, sample_t *buf, int size, long int posinfo) {
	// The offset values below mark the last sample belonging to the respective value.
	unsigned char code[1024]; // decoded biphase values, only one bit per byte is used // TODO: make boolean 1 bit
	unsigned long offs[1024]; // positions in the sample buffer (buf) where each of the values in code was detected
	unsigned char bits[1024]; // bits decoded from code, only one bit per byte is used // TODO: make boolean 1 bit 
	unsigned long offb[1024]; // positions in the sample buffer (buf) where each of the values in bits was detected

	//TODO check if size <= 1024; dynamic alloc buffers in Decoder struct.

	size = audio_to_biphase(d, buf, code, offs, size);
	WRITE_DECODER_BIPHASE_DIAGNOSTICS(d, code, offs, size, posinfo);
	size = biphase_decode(d, code, offs, bits, offb, size);
	WRITE_DECODER_BITS_DIAGNOSTICS(d, bits, offb, size, posinfo);
	return ltc_decode(d, bits, offb, posinfo, size);
}

int SMPTEDecoderRead(SMPTEDecoder* decoder, SMPTEFrameExt* frame) {
	if (!frame) return 0;
	if (decoder->qReadPos != decoder->qWritePos) {
		memcpy(frame, &decoder->queue[decoder->qReadPos], sizeof(SMPTEFrameExt));
		decoder->qReadPos++;
		if (decoder->qReadPos == decoder->qLen)
			decoder->qReadPos = 0;
		return 1;
	}
	return 0;
}

int SMPTEDecoderReadLast(SMPTEDecoder* decoder, SMPTEFrameExt* frame) {
	int rv = 0;
	while (decoder->qReadPos != decoder->qWritePos) {
		memcpy(frame, &decoder->queue[decoder->qReadPos], sizeof(SMPTEFrameExt));
		decoder->qReadPos++;
		if (decoder->qReadPos == decoder->qLen)
			decoder->qReadPos = 0;
		rv = 1;
	}
	return(rv);
}

/* -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * Encoder 
 */

SMPTEEncoder* SMPTEEncoderCreate(int sampleRate, double fps) {
	double baud = fps * 80.0; 
	SMPTEEncoder* encoder = (SMPTEEncoder*)( calloc(1, sizeof(SMPTEEncoder)) );
	encoder->nsamples = 0;
	encoder->samplesPerClock = ((float)sampleRate / (float)baud);
	encoder->samplesPerHalveClock = encoder->samplesPerClock / 2.0;
	encoder->remainder = 0.5;
	SMPTEFrameReset(&encoder->f);
	encoder->bufsize = 4096; // FIXME
	encoder->buf = calloc(encoder->bufsize, sizeof(sample_t));
	
	return encoder;
}

int SMPTEFreeEncoder(SMPTEEncoder *e) {
	if (!e) return 1;

	if (e->buf) free(e->buf);
	free(e);

	return (0);
}

int SMPTEEncode(SMPTEEncoder *e, int byteCnt) {
/* TODO
	assert(frame >= 0);
	assert(byteCnti >= 0);
	assert(byteCnt < 10);
*/			
	
	unsigned char c = ((unsigned char*)&e->f)[byteCnt];
	unsigned char b = 1;

	do
	{	
		int n;
		if ((c & b) == 0) {
			n = (int)(e->samplesPerClock + e->remainder);
			e->remainder = (e->samplesPerClock + e->remainder) - (float)n;
			e->nsamples += n;
			e->state = !e->state;
			addvalues(e, n);
		}
		else {
			n = (int)(e->samplesPerHalveClock + e->remainder);
			e->remainder = (e->samplesPerHalveClock + e->remainder) - (float)n;
			e->nsamples += n;
			e->state = !e->state;
			addvalues(e, n);

			n = (int)(e->samplesPerHalveClock + e->remainder);
			e->remainder = (e->samplesPerHalveClock + e->remainder) - (float)n;
			e->nsamples += n;
			e->state = !e->state;
			addvalues(e, n);
		}
		b <<= 1;
	} while (b);
	
	return 0;
}

int SMPTESetNsamples(SMPTEEncoder *e, int val) {
	e->nsamples = val;
	return(e->nsamples);
}

int SMPTEGetNsamples(SMPTEEncoder *e) {
	return(e->nsamples);
}

int SMPTEGetTime(SMPTEEncoder *e, SMPTETime *t) {
	if (!t) return 0;
	return SMPTEFrameToTime(&e->f, t);
}

int SMPTESetTime(SMPTEEncoder *e, SMPTETime *t) {
	// assert bytes
	return SMPTETimeToFrame(t, &e->f);
}

int SMPTEEncIncrease(SMPTEEncoder *e, int fps) {
	return SMPTEFrameIncrease(&e->f, fps);
}

size_t SMPTEGetBuffersize(SMPTEEncoder *e) {
	return(e->bufsize);
}

int SMPTEGetBuffer(SMPTEEncoder *e, sample_t *buf) {
	int len = e->offset;
	memcpy( buf, e->buf, (len * sizeof(sample_t)) );
	e->offset = 0;
	return(len);
}
