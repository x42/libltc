/*
   libltc - en+decode linear timecode

   Copyright (C) 2006-2012 Robin Gareus <robin@gareus.org>

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
#include <math.h>

#include "encoder.h"

/**
 * add values to the output buffer
 */
static int addvalues(LTCEncoder *e, int n) {
	const ltcsnd_sample_t tgtval = e->state ? 38 : 218; // -3dbFS

	if (e->offset + n >= e->bufsize) {
		fprintf(stderr, "libltc: buffer overflow: %i/%lu\n", e->offset, (unsigned long) e->bufsize);
		return 1;
	}

	ltcsnd_sample_t * const wave = &(e->buf[e->offset]);

	/* LTC signal should have a rise time of 25 us +/- 5 us. */

#if 0 /* fixed low-pass filter */
	/* this implementation is "close-enough"
	 * it is independent from the sample-rate and produces
	 * [typically] longer rise-times than the spec,
	 * but is more in line with modern sound-cards.
	 * that overshoot and distort easily.
	 */
	int i;
	ltcsnd_sample_t val = 128;
	int m = (n+1)>>1;
	for (i = 0 ; i < m ; i++) {
		val = (val>>1) + (tgtval>>1);
		wave[n-i-1] = wave[i] = val;
	}
#elif 1 /* LPF */
	/* rise-time means from <10% to >90% of the signal.
	 * in each call to addvalues() we start at 50%, so
	 * here we need half-of it.
	 */
#if 0
	const double tc = e->sample_rate * .0000125 / 2.71828; // audio-samples / exp(1)
	const double cutoff = (tc<2)? 0.5 : (1.0/tc); // prevent overshoot & distortion
#else
	const double cutoff =  1-exp( -1.0 / (e->sample_rate * .0000125 / exp(1)) );
#endif

	int i;
	ltcsnd_sample_t val = 128;
	int m = (n+1)>>1;
	for (i = 0 ; i < m ; i++) {
		val = val + cutoff * (tgtval - val);
		wave[n-i-1] = wave[i] = val;
	}
#else /* perfect square wave */
	memset(wave, tgtval, n);
#endif

	e->offset += n;
	return 0;
}

int encode_byte(LTCEncoder *e, int byte, double speed) {
	if (byte < 0 || byte > 9) return -1;
	if (speed ==0) return -1;

	int err = 0;
	const unsigned char c = ((unsigned char*)&e->f)[byte];
	unsigned char b = (speed < 0)?128:1; // bit
	const double spc = e->samples_per_clock * fabs(speed);
	const double sph = e->samples_per_clock_2 * fabs(speed);

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
		/* this is based on the assumtion that with every compiler
		 * ((unsigned char) 128)<<1 == ((unsigned char 1)>>1) == 0
		 */
		if (speed < 0)
			b >>= 1;
		else
			b <<= 1;
	} while (b);

	return err;
}
