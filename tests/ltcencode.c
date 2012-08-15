/**
   @brief self-test and example code for libltc LTCEncoder
   @file encoder.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ltc.h>

int main(int argc, char **argv) {
	FILE* file;
	double length = 2; // in seconds
	double fps = 25;
	int sampleRate = 48000;
	char *filename;

	int total = 0;
	ltcsnd_sample_t *buf;

	LTCEncoder *encoder;

	if (argc > 1) {
		filename = argv[1];
		if (argc > 2) {
			sscanf(argv[2], "%i", &sampleRate);
		}
		if (argc > 3) {
			sscanf(argv[3], "%lf", &fps);
		}
		if (argc > 4) {
			sscanf(argv[4], "%lf", &length);
		}
	} else {
		printf("Usage: %s <filename> [sample rate [frame rate [duration in ms]]]\n", argv[0]);
		return -1;
	}

	file = fopen(filename, "wb");
	if (!file) {
		return -1;
	}

	SMPTETimecode st;

	const char timezone[6] = "+0100";
	strcpy(st.timezone, timezone);
	st.years =  8;
	st.months = 12;
	st.days =   31;

	st.hours = 23;
	st.mins = 59;
	st.secs = 59;
	st.frame = 0;

	encoder = ltc_encoder_create(sampleRate, fps, 1);

#ifdef USE_LOCAL_BUFFER
	buf = calloc(ltc_encoder_get_buffersize(encoder), sizeof(ltcsnd_sample_t));
	if (!buf) {
		return -1;
	}
#endif

	ltc_encoder_set_timecode(encoder, &st);

	printf("sample rate = %i\n", sampleRate);
	printf("frames/sec = %.2f\n", fps);
	printf("secs to write = %.2f\n", length);

	int vframe_cnt = 0;
	int vframe_last = length * fps;

	while (vframe_cnt++ < vframe_last) {
		int byteCnt;

#if 1 /* encode and write each of the 80 LTC frame bits (10 bytes) */
		for (byteCnt = 0 ; byteCnt < 10 ; byteCnt++) {
			ltc_encoder_encode_byte(encoder, byteCnt, 1.0);

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

		ltc_encoder_bump_timecode(encoder);
	}

	printf("DONE: wrote %d samples\n", total);

	fclose(file);

	ltc_encoder_free(encoder);

#ifdef USE_LOCAL_BUFFER
	free(buf);
#endif

	return 0;
}
