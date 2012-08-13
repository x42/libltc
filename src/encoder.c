/* 
   libltcsmpte - en+decode linear SMPTE timecode

   Copyright (C) 2006 Robin Gareus <robin@gareus.org>

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

#include "ltcsmpte/ltcsmpte.h"
#include "encoder.h"

/**
 * add values to the output buffer
 */
void addvalues(SMPTEEncoder *e, int n) {
	curve_sample_t curve[256];
	curve_sample_t val = 0;
	curve_sample_t tgtval = e->state ? CURVE_MIN : CURVE_MAX;
	int i;
	int m = (n+1) / 2;
	for (i = 0 ; i < m ; i++) {
		// low pass filter to prevent aliasing
		val = (val + tgtval) / 2;
		curve[n-i-1] = curve[i] = val;
	}
	
	// we could use %zu without converting e->bufsize, but that requires C99
	if (e->offset+n >= e->bufsize) { fprintf(stderr, " buffer overflow: %i/%lu\n", e->offset, (unsigned long)e->bufsize); return; } 
	
#ifdef SAMPLE_AND_CURVE_ARE_DIFFERENT_TYPE
	for (i = 0; i < n ; i++) {
	#ifdef USE8BIT
		e->buf[e->offset++] = (curve[i]) + SAMPLE_CENTER;
	#else
		e->buf[e->offset++] = (sample_t)(curve[i]) + SAMPLE_CENTER;
	#endif
	}
#else
	memcpy( &(e->buf[e->offset]), curve, (n * sizeof(sample_t)) );
	e->offset += n;
#endif
}
