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
#include "contrib/ranlib/ranlib.h"
#include "gamma_delay_filter.h"



wr_errorcode_t wr_gamma_delay_filter_notify(wr_rtp_filter_t * filter, wr_event_type_t event, wr_rtp_packet_t * packet)
{
    switch(event){

        case TRANSMISSION_START:  {
            wr_gamma_delay_filter_state_t * state = calloc(1, sizeof(*state));
            state->enabled = iniparser_getboolean(wr_options.output_options, "gamma_delay:enabled", 1);
            state->shape = iniparser_getpositiveint(wr_options.output_options, "gamma_delay:shape", 0);
            state->scale = iniparser_getpositiveint(wr_options.output_options, "gamma_delay:scale", 0);
            filter->state = (void*)state;
            wr_rtp_filter_notify_observers(filter, event, packet);
            return WR_OK;
        }
        case NEW_PACKET: {
            wr_gamma_delay_filter_state_t * state = (wr_gamma_delay_filter_state_t * ) (filter->state);
            int delay;
            if (!state->enabled){
                wr_rtp_filter_notify_observers(filter, event, packet);
                return WR_OK;
            }
            delay =  (int)gengam(1/(float)state->scale, state->shape);
            wr_rtp_packet_t new_packet;
            wr_rtp_packet_copy(&new_packet, packet);
            timeval_increment(&new_packet.lowlevel_timestamp, delay);
            wr_rtp_filter_notify_observers(filter, event, &new_packet);
            return WR_OK;
        }
        case TRANSMISSION_END: {
            free(filter->state);
            wr_rtp_filter_notify_observers(filter, event, packet);
            return WR_OK;
        }
    }  
}
