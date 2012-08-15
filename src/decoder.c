/*
   libltc - en+decode linear timecode

   Copyright (C) 2005 Maarten de Boer <mdeboer@iua.upf.es>
   Copyright (C) 2006-2012 Robin Gareus <robin@gareus.org>
   Copyright (C) 2008-2009 Jan <jan@geheimwerk.de>

   Binary constant generator macro for endianess conversion
   by Tom Torfs - donated to the public domain

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

/** turn a numeric literal into a hex constant
 *  (avoids problems with leading zeroes)
 *  8-bit constants max value 0x11111111, always fits in unsigned long
 */
#define HEX__(n) 0x##n##LU

/**
 * 8-bit conversion function
 */
#define B8__(x) ((x&0x0000000FLU)?1:0)	\
	+((x&0x000000F0LU)?2:0)	 \
	+((x&0x00000F00LU)?4:0)	 \
	+((x&0x0000F000LU)?8:0)	 \
	+((x&0x000F0000LU)?16:0) \
	+((x&0x00F00000LU)?32:0) \
	+((x&0x0F000000LU)?64:0) \
	+((x&0xF0000000LU)?128:0)

/** for upto 8-bit binary constants */
#define B8(d) ((unsigned char)B8__(HEX__(d)))

/** for upto 16-bit binary constants, MSB first */
#define B16(dmsb,dlsb) (((unsigned short)B8(dmsb)<<8) + B8(dlsb))

/** for upto 32-bit binary constants, MSB first */
#define B32(dmsb,db2,db3,dlsb) (((unsigned long)B8(dmsb)<<24) \
	+ ((unsigned long)B8(db2)<<16) \
	+ ((unsigned long)B8(db3)<<8)  \
	+ B8(dlsb))

/** turn a numeric literal into a hex constant
 *(avoids problems with leading zeroes)
 * 8-bit constants max value 0x11111111, always fits in unsigned long
 */
#define HEX__(n) 0x##n##LU

/** 8-bit conversion function */
#define B8__(x) ((x&0x0000000FLU)?1:0)	\
	+((x&0x000000F0LU)?2:0)  \
	+((x&0x00000F00LU)?4:0)  \
	+((x&0x0000F000LU)?8:0)  \
	+((x&0x000F0000LU)?16:0) \
	+((x&0x00F00000LU)?32:0) \
	+((x&0x0F000000LU)?64:0) \
	+((x&0xF0000000LU)?128:0)


/** for upto 8-bit binary constants */
#define B8(d) ((unsigned char)B8__(HEX__(d)))

/** for upto 16-bit binary constants, MSB first */
#define B16(dmsb,dlsb) (((unsigned short)B8(dmsb)<<8) + B8(dlsb))

/** for upto 32-bit binary constants, MSB first */
#define B32(dmsb,db2,db3,dlsb) (((unsigned long)B8(dmsb)<<24)	 \
	+ ((unsigned long)B8(db2)<<16) \
	+ ((unsigned long)B8(db3)<<8)  \
	+ B8(dlsb))

/* Example usage:
 * B8(01010101) = 85
 * B16(10101010,01010101) = 43605
 * B32(10000000,11111111,10101010,01010101) = 2164238933
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "decoder.h"

static void parse_ltc(LTCDecoder *d, unsigned char bit, int offset, long int posinfo) {
	int bitNum, bitSet, bytePos;

	if (d->decodeBitCnt == 0) {
		memset(&d->decodeFrame, 0, sizeof(LTCFrame));

		if (d->poffset < 0) {
			d->decodeFrameStartPos = posinfo - d->soundToBiphasePeriod; // or d->soundToBiphaseLimit
		} else {
			d->decodeFrameStartPos = d->poffset;
		}
	}
	d->poffset = offset + posinfo;

	if (d->decodeBitCnt >= LTC_FRAME_BIT_COUNT) {
		/* shift bits backwards */
		int k = 0;
		const int maxBytePos = LTC_FRAME_BIT_COUNT >> 3;

		for (k=0; k< maxBytePos; k++) {
			const unsigned char bi = ((unsigned char*)&d->decodeFrame)[k];
			unsigned char bo = 0;
			bo |= (bi & B8(10000000) ) ? B8(01000000) : 0;
			bo |= (bi & B8(01000000) ) ? B8(00100000) : 0;
			bo |= (bi & B8(00100000) ) ? B8(00010000) : 0;
			bo |= (bi & B8(00010000) ) ? B8(00001000) : 0;
			bo |= (bi & B8(00001000) ) ? B8(00000100) : 0;
			bo |= (bi & B8(00000100) ) ? B8(00000010) : 0;
			bo |= (bi & B8(00000010) ) ? B8(00000001) : 0;
			if (k+1 < maxBytePos) {
				bo |= ( (((unsigned char*)&d->decodeFrame)[k+1]) & B8(00000001) ) ? B8(10000000): B8(00000000);
			}
			((unsigned char*)&d->decodeFrame)[k] = bo;
		}

		d->decodeFrameStartPos += ceil(d->soundToBiphasePeriod);
		d->decodeBitCnt--;
	}

	d->decodeSyncWord <<= 1;
	if (bit) {

		d->decodeSyncWord |= B16(00000000,00000001);

		if (d->decodeBitCnt < LTC_FRAME_BIT_COUNT) {
			// Isolating the lowest three bits: the location of this bit in the current byte
			bitNum = (d->decodeBitCnt & B8(00000111));
			// Using the bit number to define which of the eight bits to set
			bitSet = (B8(00000001) << bitNum);
			// Isolating the higher bits: the number of the byte/char the target bit is contained in
			bytePos = d->decodeBitCnt >> 3;

			(((unsigned char*)&d->decodeFrame)[bytePos]) |= bitSet;
		}

	}
	d->decodeBitCnt++;

	if (d->decodeSyncWord == B16(00111111,11111101) /*LTC Sync Word 0x3ffd*/) {
		if (d->decodeBitCnt == LTC_FRAME_BIT_COUNT) {

			memcpy( &d->queue[d->qWritePos].ltc,
				&d->decodeFrame,
				sizeof(LTCFrame));

			d->queue[d->qWritePos].off_start = d->decodeFrameStartPos;
			d->queue[d->qWritePos].off_end = posinfo + offset - 1;

			d->qWritePos++;

			if (d->qWritePos == d->qLen)
				d->qWritePos = 0;
		}
		d->decodeBitCnt = 0;
	}
}

static inline void biphase_decode2(LTCDecoder *d, int offset, long int pos) {
	if (d->soundToBiphaseState == d->biphaseToBinaryPrev) {
		d->biphaseToBinaryState = 1;
		parse_ltc(d, 0, offset, pos);
	} else {
		d->biphaseToBinaryState = 1 - d->biphaseToBinaryState;
		if (d->biphaseToBinaryState == 1) {
			parse_ltc(d, 1, offset, pos);
		}
	}
	d->biphaseToBinaryPrev = d->soundToBiphaseState;
}

void decode_ltc(LTCDecoder *d, ltcsnd_sample_t *sound, int size, long int posinfo) {
	int i;

	for (i = 0 ; i < size ; i++) {
		ltcsnd_sample_t max_threshold, min_threshold;

		/* track minimum and maximum values */
		d->soundToBiphaseMin = SAMPLE_CENTER - (((SAMPLE_CENTER - d->soundToBiphaseMin) * 15) / 16);
		d->soundToBiphaseMax = SAMPLE_CENTER + (((d->soundToBiphaseMax - SAMPLE_CENTER) * 15) / 16);

		if (sound[i] < d->soundToBiphaseMin)
			d->soundToBiphaseMin = sound[i];
		if (sound[i] > d->soundToBiphaseMax)
			d->soundToBiphaseMax = sound[i];

		/* set the thresholds for hi/lo state tracking */
		min_threshold = SAMPLE_CENTER - (((SAMPLE_CENTER - d->soundToBiphaseMin) * 8) / 16);
		max_threshold = SAMPLE_CENTER + (((d->soundToBiphaseMax - SAMPLE_CENTER) * 8) / 16);

		if ( /* Check for a biphase state change */
			   (  d->soundToBiphaseState && (sound[i] > max_threshold) )
			|| ( !d->soundToBiphaseState && (sound[i] < min_threshold) )
		   ) {
			/* If the sample count has risen above the biphase length limit */
			if (d->soundToBiphaseCnt > d->soundToBiphaseLimit) {
				/* single state change within a biphase priod. decode to a 0 */
				biphase_decode2(d, i, posinfo);
				biphase_decode2(d, i, posinfo);

			} else {
				/* "short" state change covering half a period
				 * together with the next or previous state change decode to a 1
				 */
				d->soundToBiphaseCnt *= 2;
				biphase_decode2(d, i, posinfo);

			}

			/* track speed variations
			 * As this is only executed at a state change,
			 * d->soundToBiphaseCnt is an accurate representation of the current period length.
			 */
			d->soundToBiphasePeriod = (d->soundToBiphasePeriod*3 + d->soundToBiphaseCnt) / 4.0;

			/* This limit specifies when a state-change is
			 * considered biphase-clock or 2*biphase-clock.
			 * The relation with period has been determined
			 * through trial-and-error */
			d->soundToBiphaseLimit = (d->soundToBiphasePeriod * 13) / 16;

			d->soundToBiphaseCnt = 0;
			d->soundToBiphaseState = !d->soundToBiphaseState;
		}
		d->soundToBiphaseCnt++;
	}
}
