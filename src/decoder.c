/* 
   libltcsmpte - en+decode linear SMPTE timecode

   Copyright (C) 2005 Maarten de Boer <mdeboer@iua.upf.es>
   Copyright (C) 2006 Robin Gareus <robin@gareus.org>
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

#include "ltc.h"
#include "decoder.h"

int audio_to_biphase(SMPTEDecoder *d, sample_t *sound, unsigned char *biphase, unsigned long *offset, int size) {
	int i;
	int j = 0;
#ifdef SAMPLE_IS_INTEGER
	int max_threshold, min_threshold;
#else
	sample_t max_threshold, min_threshold;
#endif
	
	for (i = 0 ; i < size ; i++) {
		/* track minimum and maximum values */
#ifdef SAMPLE_IS_UNSIGNED
		d->soundToBiphaseMin = SAMPLE_CENTER - (((SAMPLE_CENTER - d->soundToBiphaseMin) * 15) / 16);
		d->soundToBiphaseMax = SAMPLE_CENTER + (((d->soundToBiphaseMax - SAMPLE_CENTER) * 15) / 16);
#else
		d->soundToBiphaseMin = d->soundToBiphaseMin * 15 / 16;
		d->soundToBiphaseMax = d->soundToBiphaseMax * 15 / 16;
#endif
		
		if (sound[i] < d->soundToBiphaseMin) 
			d->soundToBiphaseMin = sound[i];
		if (sound[i] > d->soundToBiphaseMax)
			d->soundToBiphaseMax = sound[i];
		
		/* set the thresholds for hi/lo state tracking */
#ifdef SAMPLE_IS_UNSIGNED
		min_threshold = SAMPLE_CENTER - (((SAMPLE_CENTER - d->soundToBiphaseMin) * 8) / 16);
		max_threshold = SAMPLE_CENTER + (((d->soundToBiphaseMax - SAMPLE_CENTER) * 8) / 16);
#else
		min_threshold =  d->soundToBiphaseMin * 8 / 16;
		max_threshold =  d->soundToBiphaseMax * 8 / 16;
#endif
		
		if (
			   (  d->soundToBiphaseState && (sound[i] > max_threshold) ) // Check for a biphase state change
			|| ( !d->soundToBiphaseState && (sound[i] < min_threshold) ) // 
		   ) {
			// There is a state change at sound[i]
			
			// If the sample count has risen above the biphase length limit  
			if (d->soundToBiphaseCnt > d->soundToBiphaseLimit) {
				// single state change within a biphase priod => will decode to a 0
				offset[j] = i;
				biphase[j++] = d->soundToBiphaseState;
				offset[j] = i;
				biphase[j++] = d->soundToBiphaseState;
			} else {
				// "short" state change covering half a period 
				// => together with the next or previous state change, will decode to a 1
				offset[j] = i;
				biphase[j++] = d->soundToBiphaseState;
				d->soundToBiphaseCnt *= 2;
			}

			/* track speed variations */
			// As this is only executed at a state change,  
			// d->soundToBiphaseCnt is an accurate representation of the current period length.
			d->soundToBiphasePeriod = (d->soundToBiphasePeriod*3 + d->soundToBiphaseCnt) / 4;

			/* This limit specifies when a state-change is
			 * considered biphase-clock or 2*biphase-clock.
			 * The relation with period has been determined
			 * through trial-and-error */
			d->soundToBiphaseLimit = (d->soundToBiphasePeriod * 14) / 16;				
			
			d->soundToBiphaseCnt = 0;
			d->soundToBiphaseState = !d->soundToBiphaseState;
		}
		
		d->soundToBiphaseCnt++;
		
#ifdef DIAGNOSTICS_OUTPUT
		c = -1;
		d->diagnosticsData[++c] = (diagnostics_t)d->diagnosticsPos;
		d->diagnosticsData[++c] = (diagnostics_t)sound[i]-SAMPLE_CENTER;
		d->diagnosticsData[++c] = (diagnostics_t)d->soundToBiphaseMin-SAMPLE_CENTER;
		d->diagnosticsData[++c] = (diagnostics_t)d->soundToBiphaseMax-SAMPLE_CENTER;
		d->diagnosticsData[++c] = (diagnostics_t)min_threshold-SAMPLE_CENTER;
		d->diagnosticsData[++c] = (diagnostics_t)max_threshold-SAMPLE_CENTER;
		d->diagnosticsData[++c] = (diagnostics_t)-(d->soundToBiphaseState*20)+10; 
		d->diagnosticsData[++c] = (diagnostics_t)d->soundToBiphaseCnt;
		d->diagnosticsData[++c] = (diagnostics_t)d->soundToBiphaseLimit;
		d->diagnosticsData[++c] = (diagnostics_t)d->soundToBiphasePeriod;
		
		fwrite(&d->diagnosticsData, sizeof(diagnostics_t), DIAGNOSTICS_DATA_SIZE, d->diagnosticsFile);
		
		d->diagnosticsPos++;
#endif
		
	}
	return j;
}

int biphase_decode(SMPTEDecoder *d, unsigned char *biphase, unsigned long *offset_in, unsigned char *bits, unsigned long *offset_out, int size) {
	int i;
	int j = 0;
	
	for (i = 0 ; i < size ; i++) {
		offset_out[j] = offset_in[i];
		if (biphase[i] == d->biphaseToBinaryPrev) {
			d->biphaseToBinaryState = 1;
			bits[j++] = 0;
		} else {
			d->biphaseToBinaryState = 1 - d->biphaseToBinaryState;
			if (d->biphaseToBinaryState == 1) 
				bits[j++] = 1;
		}
		d->biphaseToBinaryPrev = biphase[i];
	}
	return j;	
}

int ltc_decode(SMPTEDecoder *d, unsigned char *bits, unsigned long *offset, long int posinfo, int size) {
	int i;
	int bitNum, bitSet, bytePos;
		
	for (i = 0 ; i < size ; i++) {
		if (d->decodeBitCnt == 0) {
			memset(&d->decodeFrame, 0, sizeof(SMPTEFrame));
			
			if (i > 0) {
				d->decodeFrameStartPos = posinfo + offset[i-1];
			}
			else {
				d->decodeFrameStartPos = posinfo + offset[i] - d->soundToBiphasePeriod; // d->soundToBiphasePeriod is an approximation 
			}

		}

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

			d->decodeFrameStartPos += d->soundToBiphasePeriod;
			d->decodeBitCnt--;
		}

		d->decodeSyncWord <<= 1;
		if (bits[i]) {
			
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
				
				memcpy( &d->queue[d->qWritePos].base,
					&d->decodeFrame,
					sizeof(SMPTEFrame));

				if (d->correctJitter)
					d->queue[d->qWritePos].delayed = size-i;
				else
					d->queue[d->qWritePos].delayed = 0;

				// TODO: compare: sampleRate*80*soundToBiphasePeriod*delayed
				d->queue[d->qWritePos].startpos = d->decodeFrameStartPos;
				d->queue[d->qWritePos].endpos = posinfo + offset[i];
				
				d->qWritePos++;

				if (d->qWritePos == d->qLen)
					d->qWritePos = 0;
			}
			d->decodeBitCnt = 0;
		}
	}
	return 1;
}
