/*
   libltc - en+decode linear timecode

   Copyright (C) 2006-2012 Robin Gareus <robin@gareus.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser Public License as published by
   the Free Software Foundation; either version 3 or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include "ltc.h"
#define LTC_FRAME_BIT_COUNT	80
#define SAMPLE_CENTER 128 // unsigned 8 bit.

struct LTCDecoder {
	LTCFrameExt* queue;
	int qLen;
	int qReadPos;
	int qWritePos;

	unsigned char biphaseToBinaryState;
	unsigned char biphaseToBinaryPrev;
	unsigned char soundToBiphaseState;
	int soundToBiphaseCnt;		///< counts the samples in the current period
	int soundToBiphaseLimit;	///< specifies when a state-change is considered biphase-clock or 2*biphase-clock
	double soundToBiphasePeriod;	///< track length of a period - used to set soundToBiphaseLimit

	ltcsnd_sample_t soundToBiphaseMin;
	ltcsnd_sample_t soundToBiphaseMax;

	unsigned short decodeSyncWord;
	LTCFrame decodeFrame;
	int decodeBitCnt;

	long int decodeFrameStartPos;
	int poffset;
};


void decode_ltc(LTCDecoder *d, ltcsnd_sample_t *sound, int size, long int posinfo);
