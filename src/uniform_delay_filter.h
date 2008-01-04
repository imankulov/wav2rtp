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
#ifndef UNIFORM_DELAY_FILTER_H
#define UNIFORM_DELAY_FILTER_H
#include "rtpapi.h"

/** @defgroup uniform_delay uniform delay filter
 * This filter emulates independent delays of the packets using uniform 
 * distribution as model.
 * It uses section [uniform_delay] of the configuration file "output.ini"
 * There is two used options
 *
 *    min_delay = integer
 *    max_delay = integer
 *
 * Minimal and maximal delays are integer values in microseconds (10^-6 seconds).
 *  @{
 */


/** 
 * Structure to store internal state of the delay filter
 */
typedef struct __wr_uniform_delay_filter_state {
    int enabled;
    int min_delay;
    int max_delay;
} wr_uniform_delay_filter_state_t;

/**
 * Loss random data from input stream and pass result stream to its output.
 * This method is invoked when filter is notified.
 */
wr_errorcode_t wr_uniform_delay_filter_notify(wr_rtp_filter_t * filter, wr_event_type_t event, wr_rtp_packet_t * packet);
/** @} */

#endif
