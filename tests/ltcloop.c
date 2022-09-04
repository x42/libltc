/**
   @brief self-test encode and decode LTC
   @file ltcloop.c
   @author Robin Gareus <robin@gareus.org>

   Copyright (C) 2006-2022 Robin Gareus <robin@gareus.org>
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
	double fps = 25;
	double samplerate = 48000;
	int    vframe_cnt = 0;
	int    vframe_end = 1;

	LTCEncoder* encoder = ltc_encoder_create (samplerate, fps, 0, 0);
	ltc_encoder_set_filter(encoder, 0);
	ltc_encoder_set_volume(encoder, -3.0);

	int frame_size = ltc_encoder_get_buffersize (encoder) * sizeof (ltcsnd_sample_t);

	ltcsnd_sample_t* buf = malloc ((1 + vframe_end) * frame_size);

	/* Encode */
	int off = 0;
	for (vframe_cnt = 0; vframe_cnt < vframe_end; ++ vframe_cnt) {
		ltc_encoder_encode_frame (encoder);
		off += ltc_encoder_copy_buffer (encoder, &buf[off]);
		ltc_encoder_inc_timecode (encoder);
	}

	/* finish frame */
	ltc_encoder_end_encode (encoder);
	off += ltc_encoder_copy_buffer (encoder, &buf[off]);
	ltc_encoder_free(encoder);

#if 0
	printf ("Encoded: %d [bytes]\n", off);
	ltcsnd_sample_t last = 0;
	for (int i = 0; i < off; ++i) {
		if (buf[i] != last) {
			printf ("%3d : %d\n", i, buf[i]);
			last = buf[i];
		}
	}
#endif

	/* Decode */
	LTCDecoder* decoder = ltc_decoder_create(samplerate / fps, vframe_end);
	LTCFrameExt frame;
	ltc_decoder_write (decoder, buf, off, 0);

	while (ltc_decoder_read (decoder, &frame)) {
		SMPTETimecode stime;
		ltc_frame_to_time (&stime, &frame.ltc, 0);
		--vframe_cnt;

#if 0
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
#endif
	}

	ltc_decoder_free (decoder);
	free (buf);

	return vframe_cnt == 0 ? 0 : -1;
}
