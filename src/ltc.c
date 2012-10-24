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

#include "ltc.h"
#include "decoder.h"
#include "encoder.h"

/* -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * Decoder
 */

LTCDecoder* ltc_decoder_create(int apv, int queue_len) {
	LTCDecoder* d = (LTCDecoder*) calloc(1, sizeof(LTCDecoder));
	if (!d) return NULL;

	d->queue_len = queue_len;
	d->queue = (LTCFrameExt*) calloc(d->queue_len, sizeof(LTCFrameExt));
	if (!d->queue) {
		free(d);
		return NULL;
	}
	d->biphase_state = 1;
	d->snd_to_biphase_period = apv / 80;
	d->snd_to_biphase_lmt = (d->snd_to_biphase_period * 14) / 4;

	d->snd_to_biphase_min = SAMPLE_CENTER;
	d->snd_to_biphase_max = SAMPLE_CENTER;
	d->frame_start_prev = -1;
	d->biphase_tic = 0;

	return d;
}

int ltc_decoder_free(LTCDecoder *d) {
	if (!d) return 1;
	if (d->queue) free(d->queue);
	free(d);

	return 0;
}

void ltc_decoder_write_float(LTCDecoder *d, float *buf, size_t size, ltc_off_t posinfo) {
	int i;
	for (i=0; i<size; i++) {
		ltcsnd_sample_t s = 128 + (buf[i] * 127.0);
		decode_ltc(d, &s, 1, posinfo + (ltc_off_t)i);
	}
}

void ltc_decoder_write_s16(LTCDecoder *d, short *buf, size_t size, ltc_off_t posinfo) {
	int i;
	for (i=0; i<size; i++) {
		ltcsnd_sample_t s = 128 + (buf[i] >> 8);
		decode_ltc(d, &s, 1, posinfo + (ltc_off_t)i);
	}
}

void ltc_decoder_write(LTCDecoder *d, ltcsnd_sample_t *buf, size_t size, ltc_off_t posinfo) {
	decode_ltc(d, buf, size, posinfo);
}

int ltc_decoder_read(LTCDecoder* d, LTCFrameExt* frame) {
	if (!frame) return -1;
	if (d->queue_read_off != d->queue_write_off) {
		memcpy(frame, &d->queue[d->queue_read_off], sizeof(LTCFrameExt));
		d->queue_read_off++;
		if (d->queue_read_off == d->queue_len)
			d->queue_read_off = 0;
		return 1;
	}
	return 0;
}

void ltc_decoder_queue_flush(LTCDecoder* d) {
	while (d->queue_read_off != d->queue_write_off) {
		d->queue_read_off++;
		if (d->queue_read_off == d->queue_len)
			d->queue_read_off = 0;
	}
}

int ltc_decoder_queue_length(LTCDecoder* d) {
	return (d->queue_write_off - d->queue_read_off + d->queue_len) % d->queue_len;
}

/* -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * Encoder
 */

LTCEncoder* ltc_encoder_create(double sample_rate, double fps, int use_date) {
	LTCEncoder* e = (LTCEncoder*) calloc(1, sizeof(LTCEncoder));
	if (!e)
		return NULL;

	e->bufsize = 1 + sample_rate / fps;
	e->buf = (ltcsnd_sample_t*) calloc(e->bufsize, sizeof(ltcsnd_sample_t));
	if (!e->buf) {
		free(e);
		return NULL;
	}

	e->sample_rate = sample_rate;
	e->fps = fps;
	e->use_date = use_date;
	e->samples_per_clock = sample_rate / (fps * 80.0);
	e->samples_per_clock_2 = e->samples_per_clock / 2.0;
	e->sample_remainder = 0.5;
	ltc_frame_reset(&e->f);

	if (fps==29.97 || fps == 30000.0/1001.0)
		e->f.dfbit = 1;
	return e;
}

void ltc_encoder_free(LTCEncoder *e) {
	if (!e) return;
	if (e->buf) free(e->buf);
	free(e);
}

int ltc_encoder_reinit(LTCEncoder *e, double sample_rate, double fps, int use_date) {

	size_t bufsize = 1 + sample_rate / fps;
	if (bufsize > e->bufsize) {
		return -1;
	}

	e->sample_rate = sample_rate;
	e->fps = fps;
	e->use_date = use_date;
	e->samples_per_clock = sample_rate / (fps * 80.0);
	e->samples_per_clock_2 = e->samples_per_clock / 2.0;
	e->sample_remainder = 0.5;
	ltc_frame_reset(&e->f);

	if (fps==29.97 || fps == 30000.0/1001.0)
		e->f.dfbit = 1;
	return 0;
}

int ltc_encoder_set_bufsize(LTCEncoder *e, double sample_rate, double fps) {
	free (e->buf);
	e->offset = 0;
	e->bufsize = 1 + sample_rate / fps;
	e->buf = (ltcsnd_sample_t*) calloc(e->bufsize, sizeof(ltcsnd_sample_t));
	if (!e->buf) {
		return -1;
	}
	return 0;
}

int ltc_encoder_encode_byte(LTCEncoder *e, int byte, double speed) {
	return encode_byte(e, byte, speed);
}

void ltc_encoder_encode_frame(LTCEncoder *e) {
	int byte;
	for (byte = 0 ; byte < 10 ; byte++) {
		encode_byte(e, byte, 1.0);
	}
}

void ltc_encoder_get_timecode(LTCEncoder *e, SMPTETimecode *t) {
	ltc_frame_to_time(t, &e->f, e->use_date);
}

void ltc_encoder_set_timecode(LTCEncoder *e, SMPTETimecode *t) {
	ltc_time_to_frame(&e->f, t, e->use_date);
}

void ltc_encoder_get_frame(LTCEncoder *e, LTCFrame *f) {
	memcpy(f, &e->f, sizeof(LTCFrame));
}

void ltc_encoder_set_frame(LTCEncoder *e, LTCFrame *f) {
	memcpy(&e->f, f, sizeof(LTCFrame));
}

int ltc_encoder_inc_timecode(LTCEncoder *e) {
	return ltc_frame_increment (&e->f, rint(e->fps), e->use_date);
}

int ltc_encoder_dec_timecode(LTCEncoder *e) {
	return ltc_frame_decrement (&e->f, rint(e->fps), e->use_date);
}

size_t ltc_encoder_get_buffersize(LTCEncoder *e) {
	return(e->bufsize);
}

void ltc_encoder_buffer_flush(LTCEncoder *e) {
	e->offset = 0;
}

ltcsnd_sample_t *ltc_encoder_get_bufptr(LTCEncoder *e, int *size, int flush) {
	if (size) *size = e->offset;
	if (flush) e->offset = 0;
	return e->buf;
}

int ltc_encoder_get_buffer(LTCEncoder *e, ltcsnd_sample_t *buf) {
	const int len = e->offset;
	memcpy(buf, e->buf, len * sizeof(ltcsnd_sample_t) );
	e->offset = 0;
	return(len);
}

void ltc_frame_set_parity(LTCFrame *frame) {
	int i;
	unsigned char p = 0;
	frame->biphase_mark_phase_correction = 0;
	for (i=0; i < LTC_FRAME_BIT_COUNT / 8; ++i){
		p = p ^ (((unsigned char*)frame)[i]);
	}
#define PRY(BIT) ((p>>BIT)&1)
	frame->biphase_mark_phase_correction =
		PRY(0)^PRY(1)^PRY(2)^PRY(3)^PRY(4)^PRY(5)^PRY(6)^PRY(7);
}
