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
#include <sys/time.h>
#include "markov_losses_filter.h"


wr_errorcode_t wr_markov_losses_filter_notify(wr_rtp_filter_t * filter, wr_event_type_t event, wr_rtp_packet_t * packet)
{
    switch(event){

        case TRANSMISSION_START:  {
            wr_markov_losses_filter_state_t * state = calloc(1, sizeof(*state));

            state->enabled = iniparser_getboolean(wr_options.output_options, "markov_losses:enabled", 1);
            state->loss_1_1 = iniparser_getdouble(wr_options.output_options, "markov_losses:loss_1_1", 0);
            if (state->loss_1_1 < 0 )  state->loss_1_1 = 0;
            if (state->loss_1_1 > 1 )  state->loss_1_1 = 1; 

            state->loss_0_1 = iniparser_getdouble(wr_options.output_options, "markov_losses:loss_0_1", 0);
            if (state->loss_0_1 < 0 )  state->loss_0_1 = 0;
            if (state->loss_0_1 > 1 )  state->loss_0_1 = 1; 

            filter->state = (void*)state;
            wr_rtp_filter_notify_observers(filter, event, packet);
            return WR_OK;
        }
        case NEW_PACKET: {
            double rand;
            wr_markov_losses_filter_state_t * state = (wr_markov_losses_filter_state_t * ) (filter->state);
            if (!state->enabled){
                wr_rtp_filter_notify_observers(filter, event, packet);
                return WR_OK;
            }
            double threshold =  (state->prev_lost) ? state->loss_1_1 : state->loss_0_1;
            int lost;
            rand = (double) random() / RAND_MAX;
            state->prev_lost = (rand < threshold) ? 1 : 0;
            if (!state->prev_lost){
                wr_rtp_filter_notify_observers(filter, event, packet);
            }
            return WR_OK;
        }
        case TRANSMISSION_END: {
            wr_markov_losses_filter_state_t * state = (wr_markov_losses_filter_state_t * ) (filter->state);
            free(state);
            wr_rtp_filter_notify_observers(filter, event, packet);
            return WR_OK;
        }
    }  
}
