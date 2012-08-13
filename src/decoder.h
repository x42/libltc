/* 
   libltcsmpte - en+decode linear SMPTE timecode

   Copyright (C) 2006 Robin Gareus <robin@gareus.org>

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

struct SMPTEDecoder {
	int sampleRate;

	SMPTEFrameExt* queue;
	int qLen;
	int qReadPos;
	int qWritePos;

	unsigned char biphaseToBinaryState;
	unsigned char biphaseToBinaryPrev;
	unsigned char soundToBiphaseState;
	int soundToBiphaseCnt;		// counts the samples in the current period, resets every period
	int soundToBiphasePeriod;	// length of a period (tracks speed variations)
	int soundToBiphaseLimit;	// specifies when a state-change is considered biphase-clock or 2*biphase-clock
	sample_t soundToBiphaseMin;
	sample_t soundToBiphaseMax;
	unsigned short decodeSyncWord;
	SMPTEFrame decodeFrame;
	int decodeBitCnt;
	long int decodeFrameStartPos;
	
	int correctJitter;
	
	double samplesToSeconds;
};


int audio_to_biphase(SMPTEDecoder *d, sample_t *sound, unsigned char *biphase, unsigned long *offset, int size);
int biphase_decode(SMPTEDecoder *d, unsigned char *biphase, unsigned long *offset_in, unsigned char *bits,  unsigned long *offset_out, int size);
int ltc_decode(SMPTEDecoder *d, unsigned char *bits, unsigned long *offset, long int posinfo, int size);

