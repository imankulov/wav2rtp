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
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDINGR
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef MARKOV_LOSSES_FILTER_H
#define MARKOV_LOSSES_FILTER_H
#include "rtpapi.h"

/** @defgroup markov_losses markov losses filter
 * This filter emulates markov random losses.
 * Markov losses means the next state solely depends on the present state.
 * It uses section [markov_losses] of the configuration file "output.ini"
 * There is two used options
 * loss_0_1 = float from 0 to 1, loss probability on conditions that prevoius packet was NOT lost
 * loss_1_1 = float from 0 to 1, loss probability on conditions that prevoius packet was lost
 *
 *  @{
 */


/** 
 * Structure to store  internal state of the pcap output filter
 */
typedef struct __wr_markov_losses_filter_state {
    int enabled;
    int  prev_lost;
    double loss_0_1;
    double loss_1_1;
} wr_markov_losses_filter_state_t;

/**
 * Loss random data from input stream and pass result stream to its output.
 * This method is invoked when filter is notified.
 */
wr_errorcode_t wr_markov_losses_filter_notify(wr_rtp_filter_t * filter, wr_event_type_t event, wr_rtp_packet_t * packet);
/** @} */

#endif
