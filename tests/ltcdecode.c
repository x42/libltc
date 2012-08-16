/**
   @brief self-test and example code for libltc LTCDecoder
   @file ltcdecode.c
   @author Robin Gareus <robin@gareus.org>

   Copyright (C) 2003  Maarten de Boer <mdeboer@iua.upf.es>
   Copyright (C) 2006-2012 Robin Gareus <robin@gareus.org>
   Copyright (C) 2008-2009 Jan <jan@geheimwerk.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdio.h>
#include <math.h>
#include <ltc.h>

#define BUFFER_SIZE (1024)

/**
 * simple example and test decoder
 */
int main(int argc, char **argv) {
	int apv = 1920;
	ltcsnd_sample_t sound[BUFFER_SIZE];
	size_t n;
	long int total;
	FILE* f;
	char* filename;

	LTCDecoder *decoder;
	LTCFrameExt frame;

	// fIXME
	if (argc > 1) {
		filename = argv[1];
		if (argc > 2) {
			sscanf(argv[2], "%i", &apv);
		}
	} else {
		printf("Usage: %s <filename> [audio-frames-per-video-frame]\n", argv[0]);
		return -1;
	}

	f = fopen(filename, "r");

	if (!f) {
		fprintf(stderr, "error opening '%s'\n", filename);
		return -1;
	}
	fprintf(stderr, "* reading from: %s\n", filename);

	total = 0;

	decoder = ltc_decoder_create(apv, 32);

	do {
		n = fread(sound, sizeof(ltcsnd_sample_t), BUFFER_SIZE, f);
		ltc_decoder_write(decoder, sound, n, total);

		while (ltc_decoder_read(decoder, &frame)) {
			SMPTETimecode stime;

			ltc_frame_to_time(&stime, &frame.ltc, 1);

			printf("%04d-%02d-%02d %s ",
				((stime.years < 67) ? 2000+stime.years : 1900+stime.years),
				stime.months,
				stime.days,
				stime.timezone
				);

			printf("%02d:%02d:%02d%c%02d | %8lld %8lld%s\n",
					stime.hours,
					stime.mins,
					stime.secs,
					(frame.ltc.dfbit) ? '.' : ':',
					stime.frame,
					frame.off_start,
					frame.off_end,
					frame.reverse ? "  R" : ""
					);
		}

		total += n;

	} while (n);

	fclose(f);
	ltc_decoder_free(decoder);

	return 0;
}
