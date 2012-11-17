/**
   @brief self-test (and example code) for libltc LTCEncoder
   @file ltcencoder.c
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
	double sampleRate = 48000;
	char *filename;

	int total = 0;

	int vframe_cnt;
	int vframe_last;

	LTCEncoder *encoder;
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

	if (argc > 1) {
		filename = argv[1];
		if (argc > 2) {
			sampleRate = atof(argv[2]);
		}
		if (argc > 3) {
			fps = atof(argv[3]);
		}
		if (argc > 4) {
			length = atof(argv[4]);
		}
	} else {
		printf("ltcencode - test/example application to encode LTC to a file\n\n");
		printf("Usage: ltcencode <filename> [sample rate [frame rate [duration in s]]]\n\n");
		printf("default-values:\n");
		printf(" sample rate: 48000.0 [SPS], frame rate: 25.0 [fps], duration: 2.0 [sec]\n");
		printf("Report bugs to Robin Gareus <robin@gareus.org>\n");
		return 1;
	}

	file = fopen(filename, "wb");
	if (!file) {
		fprintf(stderr, "Error: can not open file '%s' for writing.\n", filename);
		return 1;
	}

	encoder = ltc_encoder_create(1, 1, 0, LTC_USE_DATE);
	ltc_encoder_set_bufsize(encoder, sampleRate, fps);
	ltc_encoder_reinit(encoder, sampleRate, fps,
			fps==25?LTC_TV_625_50:LTC_TV_525_60, LTC_USE_DATE);

	ltc_encoder_set_filter(encoder, 0);
	ltc_encoder_set_filter(encoder, 25.0);
	ltc_encoder_set_volume(encoder, -18.0);

	ltc_encoder_set_timecode(encoder, &st);

	printf("sample rate: %.2f\n", sampleRate);
	printf("frames/sec: %.2f\n", fps);
	printf("secs to write: %.2f\n", length);
	printf("sample format: 8bit unsigned mono\n");

	vframe_cnt = 0;
	vframe_last = length * fps;

	while (vframe_cnt++ < vframe_last) {
		int len;
		ltcsnd_sample_t *buf;

		ltc_encoder_encode_frame(encoder);

		buf = ltc_encoder_get_bufptr(encoder, &len, 1);

		if (len > 0) {
			fwrite(buf, sizeof(ltcsnd_sample_t), len, file);
			total+=len;
		}

		ltc_encoder_inc_timecode(encoder);
	}
	fclose(file);
	ltc_encoder_free(encoder);

	printf("Done: wrote %d samples to '%s'\n", total, filename);

	return 0;
}
