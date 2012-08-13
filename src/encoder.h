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

struct SMPTEEncoder {
	int sampleRate;
	int nsamples;
	int offset;
	size_t bufsize;
	sample_t *buf;

	int state;
	float samplesPerClock;
	float samplesPerHalveClock;
	float remainder;

	SMPTEFrame f;
};

