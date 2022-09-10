/*
   libltc - en+decode linear timecode

   Copyright (C) 2006-2022 Robin Gareus <robin@gareus.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.
   If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "encoder.h"

/**
 * add values to the output buffer
 */
static int addvalues(LTCEncoder *e, int n) {
	
	double tcf;
	ltcsnd_sample_t * wave;
	
	const ltcsnd_sample_t tgtval = e->state ? e->enc_hi : e->enc_lo;
	
	if (e->offset + n >= e->bufsize) {
#if 0
		fprintf(stderr, "libltc: buffer overflow: %d/%lu\n", (int) e->offset, (unsigned long) e->bufsize);
#endif
		return 1;
	}

	wave = &(e->buf[e->offset]);
	tcf =  e->filter_const;
	if (tcf > 0) {
		/* low-pass-filter
		 * LTC signal should have a rise time of 40 us +/- 10 us.
		 *
		 * rise-time means from <10% to >90% of the signal.
		 * in each call to addvalues() we start at 50%, so
		 * here we need half-of it. (0.000020 sec)
		 *
		 * e->cutoff = 1.0 -exp( -1.0 / (sample_rate * .000020 / exp(1.0)) );
		 */
		int i;
		int m;
		
		ltcsnd_sample_t val = SAMPLE_CENTER;
		m = (n+1)>>1;
		for (i = 0 ; i < m ; i++) {
			val = (ltcsnd_sample_t) ( val + tcf * (tgtval - val)); 
			wave[n-i-1] = wave[i] = val;
		}
	} else {
		/* perfect square wave */
		memset(wave, tgtval, n);
	}

	e->offset += n;
	return 0;
}

int encode_byte(LTCEncoder *e, int byte, double speed) {

	int err = 0;
	unsigned char c;
	unsigned char b;
	double spc;
	double sph;
	
	if (byte < 0 || byte > 9) return -1;
	if (speed ==0) return -1;

	c = ((unsigned char*)&e->f)[byte];
	b = (speed < 0)?128:1; // bit
	spc = e->samples_per_clock * fabs(speed);
	sph = e->samples_per_clock_2 * fabs(speed);

	do
	{
		int n;
		if ((c & b) == 0) {
			n = (int)(spc + e->sample_remainder);
			e->sample_remainder = spc + e->sample_remainder - n;
			e->state = !e->state;
			err |= addvalues(e, n);
		} else {
			n = (int)(sph + e->sample_remainder);
			e->sample_remainder = sph + e->sample_remainder - n;
			e->state = !e->state;
			err |= addvalues(e, n);

			n = (int)(sph + e->sample_remainder);
			e->sample_remainder = sph + e->sample_remainder - n;
			e->state = !e->state;
			err |= addvalues(e, n);
		}
		/* this is based on the assumption that with every compiler
		 * ((unsigned char) 128)<<1 == ((unsigned char 1)>>1) == 0
		 */
		if (speed < 0)
			b >>= 1;
		else
			b <<= 1;
	} while (b);

	return err;
}

int encode_transition(LTCEncoder *e) {
	if (e->offset + 1 >= e->bufsize) {
		return -1;
	}

	int off = e->offset;

	int n = e->bufsize - e->offset - 1;
	e->state = !e->state;
	int err = addvalues(e, n);

	if (e->filter_const <= 0) {
		e->offset = off + 1;
	}

	return err;
}
