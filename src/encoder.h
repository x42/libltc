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

#include "ltc.h"

struct LTCEncoder {
	double fps;
	double sample_rate;
	int use_date;

	size_t offset;
	size_t bufsize;
	ltcsnd_sample_t *buf;

	char state;

	double samples_per_clock;
	double samples_per_clock_2;
	double sample_remainder;

	LTCFrame f;
};

int encode_byte(LTCEncoder *e, int byte, double speed);
