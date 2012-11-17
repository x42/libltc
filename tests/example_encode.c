/**
   @brief example code for libltc LTCEncoder
   @file example_encode.c
   @author Robin Gareus <robin@gareus.org>

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

/*
 * compile with
 *  gcc -o ltc-encoder example_encode.c -lltc -lm
 *
 * from the source-folder this can be compiled after building libltc:
 *  gcc -o ltc-encoder example_encode.c ../src/.libs/libltc.a -lm -I../src
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ltc.h>

/* define "USE_LOCAL_BUFFER" to
 * use a local buffer instead of a pointer to
 * libltc's internal buffer
 */
//#ifdef USE_LOCAL_BUFFER

int main(int argc, char **argv) {
	FILE* file;
	double length = 2; // in seconds
	double fps = 25;
	double sample_rate = 48000;
	char *filename;

	int vframe_cnt;
	int vframe_last;

	int total = 0;
	ltcsnd_sample_t *buf;

	LTCEncoder *encoder;
	SMPTETimecode st;

	/* start encoding at this timecode */
	const char timezone[6] = "+0100";
	strcpy(st.timezone, timezone);
	st.years =  8;
	st.months = 12;
	st.days =   31;

	st.hours = 23;
	st.mins = 59;
	st.secs = 59;
	st.frame = 0;

	/* parse commandline args */
	if (argc > 1) {
		filename = argv[1];
		if (argc > 2) {
			sample_rate = atof(argv[2]);
		}
		if (argc > 3) {
			fps = atof(argv[3]);
		}
		if (argc > 4) {
			length = atof(argv[4]);
		}
	} else {
		printf("ltc-encoder - test/example application to encode LTC to a file\n\n");
		printf("Usage: ltc-encoder <filename> [sample rate [frame rate [duration in s]]]\n\n");
		printf("default-values:\n");
		printf(" sample rate: 48000.0 [SPS], frame rate: 25.0 [fps], duration: 2.0 [sec]\n");
		printf("Report bugs to Robin Gareus <robin@gareus.org>\n");
		return 1;
	}

	/* open output file */
	file = fopen(filename, "wb");
	if (!file) {
		fprintf(stderr, "Error: can not open file '%s' for writing.\n", filename);
		return 1;
	}

	/* prepare encoder */
	encoder = ltc_encoder_create(sample_rate, fps,
			fps==25?LTC_TV_625_50:LTC_TV_525_60, LTC_USE_DATE);
	ltc_encoder_set_timecode(encoder, &st);

#ifdef USE_LOCAL_BUFFER
	buf = calloc(ltc_encoder_get_buffersize(encoder), sizeof(ltcsnd_sample_t));
	if (!buf) {
		return -1;
	}
#endif

	/* ready to go, print some info first */

	printf("sample rate: %.2f\n", sample_rate);
	printf("frames/sec: %.2f\n", fps);
	printf("secs to write: %.2f\n", length);
	printf("sample format: 8bit unsigned mono\n");

	vframe_cnt = 0;
	vframe_last = length * fps;

	while (vframe_cnt++ < vframe_last) {
#if 1 /* encode and write each of the 80 LTC frame bits (10 bytes) */
		int byte_cnt;
		for (byte_cnt = 0 ; byte_cnt < 10 ; byte_cnt++) {
			ltc_encoder_encode_byte(encoder, byte_cnt, 1.0);

#ifdef USE_LOCAL_BUFFER
			int len = ltc_encoder_get_buffer(encoder, buf);
#else
			int len;
			buf = ltc_encoder_get_bufptr(encoder, &len, 1);
#endif
			if (len > 0) {
				fwrite(buf, sizeof(ltcsnd_sample_t), len, file);
				total+=len;
			}
		}
#else /* encode a complete LTC frame in one step */
		ltc_encoder_encode_frame(encoder);

#ifdef USE_LOCAL_BUFFER
		int len = ltc_encoder_get_buffer(encoder, buf);
#else
		int len;
		buf = ltc_encoder_get_bufptr(encoder, &len, 1);
#endif

		if (len > 0) {
			fwrite(buf, sizeof(ltcsnd_sample_t), len, file);
			total+=len;
		}
#endif

		ltc_encoder_inc_timecode(encoder);
	}
	fclose(file);
	ltc_encoder_free(encoder);

	printf("Done: wrote %d samples to '%s'\n", total, filename);

#ifdef USE_LOCAL_BUFFER
	free(buf);
#endif

	return 0;
}
