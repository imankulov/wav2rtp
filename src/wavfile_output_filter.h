/*
 * $Id$
 * 
 * Copyright (c) 2007, R.Imankulov
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the R.Imankulov nor the names of its contributors may
 *  be used to endorse or promote products derived from this software without
 *  specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef WAVFIlE_OUTPUT_FILTER
#define WAVFILE_OUTPUT_FILTER
#include <sndfile.h>
#include "rtpapi.h"
/** @defgroup wavfile_output_filter wavfile output filter method definitions
 * This is the output filter which converts rtp packets to .wav format and store them into
 * file
 *  @{
 */


/** 
 * Structure to sotr internal state of the pcap output filter
 */
typedef struct __wr_wavfile_output_filter_state {
    SF_INFO file_info;
    SNDFILE * file; 
    struct timeval start_time;
    struct timeval end_time;      /**<  store the timestamp of the latest sample written to file */
} wr_wavfile_output_filter_state_t;


/**
 * Seek to position given in struct timeval. If it's needed extend file by zeroes
 */
wr_errorcode_t wr_wavfile_seek(wr_wavfile_output_filter_state_t *, const struct timeval * );

/**
 * Store data into file 
 * This method is invoked when filter is notified
 */
wr_errorcode_t wr_wavfile_output_filter_notify(wr_rtp_filter_t * filter, wr_event_type_t event, wr_rtp_packet_t * packet);
/** @} */

#endif
