/**
   @brief libltc - en+decode linear timecode
   @file ltc.h
   @author Robin Gareus <robin@gareus.org>

   Copyright (C) 2006-2012 Robin Gareus <robin@gareus.org>
   Copyright (C) 2008-2009 Jan Weiß <jan@geheimwerk.de>

   inspired by SMPTE Decoder - Maarten de Boer <mdeboer@iua.upf.es>

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
#ifndef LTC_H
#define LTC_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h> /* size_t */

#ifndef DOXYGEN_IGNORE
/* libltc version */
#define LIBLTC_VERSION "0.6.0"
#define LIBLTC_VERSION_MAJOR  0
#define LIBLTC_VERSION_MINOR  6
#define LIBLTC_VERSION_MICRO  0

/* interface revision number
 * http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
 */
#define LIBLTC_CUR  5
#define LIBLTC_REV  0
#define LIBLTC_AGE  3
#endif

/**
 * default audio sample type: 8bit unsigned (mono)
 */
typedef unsigned char ltcsnd_sample_t;

/**
 * sample-count offset - 64bit wide
 */
typedef long long int ltc_off_t;

#define LTC_FRAME_BIT_COUNT	80

/**
 * Raw 80 bit SMPTE frame
 *
 * The datastream for each video frame of Longitudinal TimeCode consists of eighty bit-periods.
 *
 * At a frame-rate of 30 fps, the bit-rate corresponds to 30 [fps] * 80 [bits/f] = 2400 bits per second.
 * The frequency for a stream of zeros would be 1.2 kHz and for a stream of ones it would be 2.4 kHz.
 * \image html smptefmt.png
 * With all commonly used video-frame-rates and audio-sample-rates,  LTC timecode can be recorded
 * easily into a audio-track.
 *
 * In each frame, 26 of the eighty bits carry the SMPTE time in binary coded decimal (BCD).
 *
 * These Bits are FRAME-UNITS, FRAME-TENS, SECS-UNITS, SECS-TENS, MINS-UNITS, MINS-TENS, HOURS-UNITS and HOURS-TENS.
 * The BCD digits are loaded 'least significant bit first' (libltc takes care of the architecture specific alignment).
 *
 * 32 bits are assigned as eight groups of four USER-BITS (also sometimes called the "Binary Groups").
 * This capacity is generally used to carry extra info such as reel number and/or date.
 * The User Bits may be allocated howsoever one wishes as long as both Binary Group Flag Bits are cleared.
 *
 * The last 16 Bits make up the SYNC WORD. These bits indicate the frame boundary, the tape direction, and the bit-rate of the sync tone.
 * The values of these Bits are fixed as 0011 1111 1111 1101
 *
 * The Bi-Phase Mark Phase Correction Bit (Bit 27) may be set or cleared so that that every 80-bit word contains an even number of zeroes.
 * This means that the phase of the pulse train in every Sync Word will be the same.
 *
 * Bit 10 indicates drop-frame timecode.
 * The Colour Frame Flag col.frm is Bit 11; if the timecode intentionally synchronized to a colour TV field sequence, this bit is set.
 *
 * Bit 58 is not required for the BCD count for HOURS-TENS (which has a maximum value of two)
 * and has not been given any other special purpose so remains unassigned.
 * This Bit has been RESERVED for future assignment.
 *
 * Bits 43 and 59 are assigned as the Binary Group Flag Bits.
 * These Bits are used to indicate when a standard character set is used to format the User Bits data.
 * The Binary Group Flag Bits should be used only as shown in the truth table below.
 * The Unassigned entries in the table should not be used, as they may be allocated specific meanings in the future.
 *
 * <pre>
 *                                  Bit 43  Bit 59
 *  No User Bits format specified     0       0
 *  Eight-bit character set           1       0
 *  Unassigned (Reserved)             0       1
 *  Unassigned (Reserved)             1       1
 * </pre>
 *
 * further information: http://www.philrees.co.uk/articles/timecode.htm
 */
#if (defined __BIG_ENDIAN__ && !defined DOXYGEN_IGNORE)
// Big Endian version, bytes are "upside down"
struct LTCFrame {
	unsigned int user1:4;
	unsigned int frame_units:4;

	unsigned int user2:4;
	unsigned int col_frm:1;
	unsigned int dfbit:1;
	unsigned int frame_tens:2;

	unsigned int user3:4;
	unsigned int secs_units:4;

	unsigned int user4:4;
	unsigned int biphase_mark_phase_correction:1;
	unsigned int secs_tens:3;

	unsigned int user5:4;
	unsigned int mins_units:4;

	unsigned int user6:4;
	unsigned int binary_group_flag_bit1:1;
	unsigned int mins_tens:3;

	unsigned int user7:4;
	unsigned int hours_units:4;

	unsigned int user8:4;
	unsigned int binary_group_flag_bit2:1;
	unsigned int reserved:1;
	unsigned int hours_tens:2;

	unsigned int sync_word:16;
};
#else
/* Little Endian version -- and doxygen doc */
struct LTCFrame {
	unsigned int frame_units:4; ///< SMPTE framenumber BCD unit 0..9
	unsigned int user1:4;

	unsigned int frame_tens:2; ///< SMPTE framenumber BCD tens 0..3
	unsigned int dfbit:1; ///< indicated drop-frame timecode
	unsigned int col_frame:1; ///< colour-frame: timecode intentionally synchronized to a colour TV field sequence
	unsigned int user2:4;

	unsigned int secs_units:4; ///< SMPTE seconds BCD unit 0..9
	unsigned int user3:4;

	unsigned int secs_tens:3; ///< SMPTE seconds BCD tens 0..6
	unsigned int biphase_mark_phase_correction:1; ///< unused - see note on Bit 27 in description and \ref ltc_frame_set_parity .
	unsigned int user4:4;

	unsigned int mins_units:4; ///< SMPTE minutes BCD unit 0..9
	unsigned int user5:4;

	unsigned int mins_tens:3; ///< SMPTE minutes BCD tens 0..6
	unsigned int binary_group_flag_bit1:1; ///< indicate user-data char encoding, see table above
	unsigned int user6:4;

	unsigned int hours_units:4; ///< SMPTE hours BCD unit 0..9
	unsigned int user7:4;

	unsigned int hours_tens:2; ///< SMPTE hours BCD tens 0..2
	unsigned int reserved:1; ///< reserved -- don't use
	unsigned int binary_group_flag_bit2:1; ///< indicate user-data char encoding, see table above
	unsigned int user8:4;

	unsigned int sync_word:16;
};
#endif

/**
 * see LTCFrame
 */
typedef struct LTCFrame LTCFrame;

/**
 * Extended SMPTE frame - includes audio-sample position offsets
 */
struct LTCFrameExt {
	LTCFrame ltc; ///< the actual LTC frame. see \ref LTCFrame
	ltc_off_t off_start; ///< \anchor off_start the approximate sample in the stream corresponding to the start of the LTC frame.
	ltc_off_t off_end; ///< \anchor off_end the sample in the stream corresponding to the end of the LTC frame.
	int reverse; ///< if non-zero, a reverse played LTC frame was detected. Since the frame was reversed, it started at off_end and finishes as off_start (off_end > off_start). (Note: in reverse playback the (reversed) sync-word of the next/previous frame is detected, this offset is corrected).
	float biphase_tics[LTC_FRAME_BIT_COUNT]; ///< detailed timing info: phase of the LTC signal; the time between each bit in the LTC-frame in audio-frames. Summing all 80 values in the array will yield audio-frames/LTC-frame = (\ref off_end - \ref off_start + 1).
};

/**
 * see \ref LTCFrameExt
 */
typedef struct LTCFrameExt LTCFrameExt;

/**
 * Human readable time representation, decimal values.
 */
struct SMPTETimecode {
	char timezone[6];
	unsigned char years; ///< LTC-date uses 2-digit year 00.99
	unsigned char months; ///< valid months are 1..12
	unsigned char days; ///< day of month 1..31

	unsigned char hours; ///< hour 0..23
	unsigned char mins; ///< minute 0..60
	unsigned char secs; ///< second 0..60
	unsigned char frame; ///< sub-second frame 0..{FPS-1}
};

/**
 * see \ref SMPTETimecode
 */
typedef struct SMPTETimecode SMPTETimecode;


/**
 * opaque structure.
 * see: \ref ltc_decoder_create, \ref ltc_decoder_free
 */
typedef struct LTCDecoder LTCDecoder;

/**
 * opaque structure
 * see: \ref ltc_encoder_create, \ref ltc_encoder_free
 */
typedef struct LTCEncoder LTCEncoder;

/**
 * convert binary LTCFrame into SMPTETimecode struct
 * @param stime output
 * @param frame input
 * @param set_date if non-zero, the user-fields in LTCFrame will be parsed into the date variable of SMPTETimecode
 */
void ltc_frame_to_time(SMPTETimecode* stime, LTCFrame* frame, int set_date);

/**
 * convert SMPTETimecode struct into its binary LTC representation.
 * and set the Frame's parity bit accordingly (see \ref ltc_frame_set_parity)
 *
 * @param frame output - the frame to be set
 * @param stime input - timecode input
 * @param set_date if non-zero, the user-fields in LTCFrame will be set from the date in SMPTETimecode
 */
void ltc_time_to_frame(LTCFrame* frame, SMPTETimecode* stime, int set_date);

/**
 * reset all values of a LTC FRAME to zero, except for the sync-word (0x3FFD) at the end.
 * The sync word is set according to architecture (big/little endian).
 * also set the Frame's parity bit accordingly (see \ref ltc_frame_set_parity)
 * @param frame the LTCFrame to reset
 */
void ltc_frame_reset(LTCFrame* frame);

/**
 * increment the timecode by one Frame (1/framerate seconds)
 * and set the Frame's parity bit accordingly (see \ref ltc_frame_set_parity)
 *
 * @param frame the LTC-timecode to increment
 * @param fps integer framerate (for drop-frame-timecode set frame->dfbit and round-up the fps).
 * @param use_date - interpret user-data as date and increment date if timecode wraps after 24h.
 * (Note: leap-years are taken into account, but since the year is two-digit only, the 100,400yr rules are ignored.
 * "00" is assumed to be year 2000 which was a leap year.)
 * @return 1 if timecode was wrapped around after 23:59:59:ff, 0 otherwise
 */
int ltc_frame_increment(LTCFrame *frame, int fps, int use_date);

/**
 * decrement the timecode by one Frame (1/framerate seconds)
 * and set the Frame's parity bit accordingly (see \ref ltc_frame_set_parity)
 *
 * @param frame the LTC-timecode to decrement
 * @param fps integer framerate (for drop-frame-timecode set frame->dfbit and round-up the fps).
 * @param use_date - interpret user-data as date and decrement date if timecode wraps at 24h.
 * (Note: leap-years are taken into account, but since the year is two-digit only, the 100,400yr rules are ignored.
 * "00" is assumed to be year 2000 which was a leap year.)
 * @return 1 if timecode was wrapped around at 23:59:59:ff, 0 otherwise
 */
int ltc_frame_decrement(LTCFrame* frame, int fps, int use_date);

/**
 * Create a new LTC decoder.
 *
 * @param apv audio-frames per video frame. This is just used for initial settings, the speed is tracked dynamically. setting this in the right ballpark is needed to properly decode the first LTC frame in a sequence.
 * @param queue_size length of the internal queue to store decoded frames
 * to SMPTEDecoderWrite.
 * @return decoder handle or NULL if out-of-memory
 */
LTCDecoder * ltc_decoder_create(int apv, int queue_size);


/**
 * release memory of decoder-structure.
 * @param d decoder handle
 */
int ltc_decoder_free(LTCDecoder *d);

/**
 * Feed the LTC decoder with new audio samples.
 *
 * Parse raw audio for LTC timestamps. Once a complete LTC frame has been
 * decoded it is pushed into a queue (\ref ltc_decoder_read)
 *
 * @param d decoder handle
 * @param buf pointer to ltcsnd_sample_t - unsigned 8 bit mono audio data
 * @param size \anchor size number of samples to parse
 * @param posinfo (optional, recommended) sample-offset in the audio-stream. It is added to \ref off_start, \ref off_end in \ref LTCFrameExt and should be monotonic (ie incremented by \ref size for every call to ltc_decoder_write)
 */
void ltc_decoder_write(LTCDecoder *d,
		ltcsnd_sample_t *buf, size_t size,
		ltc_off_t posinfo);

/**
 * Decoded LTC frames are placed in a queue. This function retrieves
 * a frame from the queue, and stores it at LTCFrameExt*
 *
 * @param d decoder handle
 * @param frame the decoded LTC frame is copied there
 * @return 1 on success or 0 when no frames queued.
 */
int ltc_decoder_read(LTCDecoder *d, LTCFrameExt *frame);

/**
 * removed all frames from queue.
 * @param d decoder handle
 */
void ltc_decoder_queue_flush(LTCDecoder* d);

/**
 * count number of LTC frames currently in the queue
 * @param d decoder handle
 * @return number of queued frames
 */
int ltc_decoder_queue_length(LTCDecoder* d);



/**
 * Allocate and initialize LTC audio encoder.
 *
 * Note: if fps equals to 29.97 or 30000.0/1001.0, the LTCFrame's 'dfbit' bit is set to 1
 * to indicate drop-frame timecode.
 *
 * @param sample_rate audio sample rate (eg. 48000)
 * @param fps video-frames per second (e.g. 25.0)
 * @param use_date use LTC-user-data for date
 */
LTCEncoder * ltc_encoder_create(double sample_rate, double fps, int use_date);

/**
 * release encoder data structure
 * @param e encoder handle
 */
void ltc_encoder_free(LTCEncoder *e);

/**
 * set the encoder LTC-frame from given SMPTETimecode.
 * The next call to \ref ltc_encoder_encode_byte or
 * \ref ltc_encoder_encode_frame will encode this time to LTC audio-samples.
 *
 * Internally this call uses \ref ltc_time_to_frame because
 * the LTCEncoder operates on LTCframes only.
 *
 * @param e encoder handle
 * @param t timecode to set.
 */
void ltc_encoder_set_timecode(LTCEncoder *e, SMPTETimecode *t);

/**
 * query current encoder timecode.
 *
 * Note: the decoder store its internal state in an LTC-frame,
 * this function converts that LTC-Frame into SMPTETimecode on demand
 *
 * @param e encoder handle
 * @param t is set to current timecode
 */
void ltc_encoder_get_timecode(LTCEncoder *e, SMPTETimecode *t);

/**
 * moves the SMPTE to the next timecode frame.
 * uses \ref ltc_frame_increment() internally.
 */
int ltc_encoder_bump_timecode(LTCEncoder *e);

/**
 * low-level access to the internal LTCFrame data.
 *
 * Note: be careful to about f->dfbit, the encoder sets this [only] upon
 * initialization.
 *
 * @param e encoder handle
 * @param f LTC frame data to use
 */
void ltc_encoder_set_frame(LTCEncoder *e, LTCFrame *f);

/**
 * low-level access to the internal LTCFrame data
 *
 * @param e encoder handle
 * @param f return LTC frame data
 */
void ltc_encoder_get_frame(LTCEncoder *e, LTCFrame *f);

/**
 * copy the accumulated encoded audio to the given
 * sample-buffer and flush the buffer.
 *
 * @param e encoder handle
 * @param buf place to store the audio-samples, needs to be large enough
 * to hold \ref ltc_encoder_get_buffersize bytes
 * @return the number of bytes written to the memory area
 * pointed to by buf.
 */
int ltc_encoder_get_buffer(LTCEncoder *e, ltcsnd_sample_t *buf);


/**
 * get a pointer to the accumulated encoded audio-data.
 *
 * @param e encoder handle
 * @param size if set, the number of valid bytes in the buffer is stored there
 * @param flush call \ref ltc_encoder_buffer_flush - reset the buffer write-pointer
 * @return pointer to encoder-buffer
 */
ltcsnd_sample_t *ltc_encoder_get_bufptr(LTCEncoder *e, int *size, int flush);

/**
 * reset the write-pointer of the encoder-buffer
 * @param e encoder handle
 */
void ltc_encoder_buffer_flush(LTCEncoder *e);

/**
 * query the length of the internal buffer. It is allocated
 * to hold audio-frames for exactly one LTC frame for the given
 * sample-rate and frame-rate.  ie. (1 + sample-rate / fps) bytes
 *
 * @param e encoder handle
 * @return size of the allocated internal buffer.
 */
size_t ltc_encoder_get_buffersize(LTCEncoder *e);

/**
 * change the encoder settings without re-allocating any
 * library internal data structure (realtime safe).
 *
 * This call will fail if the internal buffer is too small
 * to hold one full LTC frame. Use \ref ltc_encoder_set_bufsize to
 * prepare an internal buffer large enough to accommodate all
 * sample_rate, fps combinations that you would like to re-init to.
 *
 * @param e encoder handle
 * @param sample_rate audio sample rate (eg. 48000)
 * @param fps video-frames per second (e.g. 25.0)
 * @param use_date use LTC-user-data for date
 */
int ltc_encoder_reinit(LTCEncoder *e, double sample_rate, double fps, int use_date);

/**
 * set a custom size for the internal buffer.
 *
 * This is needed if you are planning to call \ref ltc_encoder_reinit()
 * or if you want to keep more than one LTC frame's worth of data in
 * the library's internal buffer.
 *
 * The buffer-size is (1 + sample_rate / fps) bytes.
 * resizing the internal buffer will flush all existing data
 * in it - alike \ref ltc_encoder_buffer_flush.
 *
 * @param e encoder handle
 * @param sample_rate audio sample rate (eg. 48000)
 * @param fps video-frames per second (e.g. 25.0)
 * @return 0 on success, -1 if allocation fails (which makes the
 *   encoder unusable, call \ref ltc_encoder_free or realloc the buffer)
 */
int ltc_encoder_set_bufsize(LTCEncoder *e, double sample_rate, double fps);

/**
 * Generate LTC audio for given byte of the LTC-frame and
 * place it into the internal buffer.
 *
 * see \ref ltc_encoder_get_buffer and  \ref ltc_encoder_get_bufptr
 *
 * LTC has 10 bytes per frame: 0 <= bytecnt < 10
 * use SMPTESetTime(..) to set the current frame before Encoding.
 * see tests/encoder.c for an example.
 *
 * @param e encoder handle
 * @param byte byte of the LTC-frame to encode 0..9
 * @param speed vari-speed, <1.0 faster,  >1.0 slower ; must be > 0
 *
 * @return 0 on success, -1 if byte is invalud or buffer overflow (speed > 10.0)
 */
int ltc_encoder_encode_byte(LTCEncoder *e, int byte, double speed);

/**
 * encode a full LTC frame at fixed speed.
 * This is equivalent to calling \ref ltc_encoder_encode_byte 10 times for
 * bytes 0..9 with speed 1.0.
 *
 * Note: The buffer must be empty before calling this function. This is usually the case
 *
 * @param e encoder handle
 */
void ltc_encoder_encode_frame(LTCEncoder *e);

/**
 * Set the parity of the LTC frame.
 *
 * Bi-Phase Mark Phase Correction bit (bit 27) may be set or cleared so that
 * that every 80-bit word contains an even number of zeroes.
 * This means that the phase in every Sync Word will be the same.
 *
 * This is merely cosmetic; the motivation to keep the polarity of the waveform
 * constant is to make finding the Sync Word visibly (on a scope) easier.
 *
 * @param frame the LTC to analyze and set or clear the biphase_mark_phase_correction bit.
 */
void ltc_frame_set_parity(LTCFrame *frame);

#ifdef __cplusplus
}
#endif

#endif
