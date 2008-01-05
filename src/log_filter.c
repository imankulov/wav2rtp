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
#include "log_filter.h"


wr_errorcode_t wr_log_filter_notify(wr_rtp_filter_t * filter, wr_event_type_t event, wr_rtp_packet_t * packet)
{
    switch(event){

        case TRANSMISSION_START:  {
            wr_log_filter_state_t * state = calloc(1, sizeof(*state));
            state->enabled = iniparser_getboolean(wr_options.output_options, "log:enabled", 1);
            filter->state = (void*)state;
            wr_rtp_filter_notify_observers(filter, event, packet);
            return WR_OK;
        }
        case NEW_PACKET: {
            char diff[256];
            wr_log_filter_state_t * state = (wr_log_filter_state_t * ) (filter->state);
            if (!state->enabled){
                wr_rtp_filter_notify_observers(filter, event, packet);
                return WR_OK;
            }
            bzero(diff, sizeof(diff));
            /* count diff between current and previous timestamps */
            if (!timerisset(&state->prev_timestamp)){
                strncpy(diff, "--.------", 255);
            } else {
                char diffsign;
                struct timeval timediff;
                if (timercmp(&packet->lowlevel_timestamp, &state->prev_timestamp, >=)){
                    timersub(&packet->lowlevel_timestamp, &state->prev_timestamp, &timediff);
                    diffsign = '+';
                } else {
                    timersub(&state->prev_timestamp, &packet->lowlevel_timestamp, &timediff);
                    diffsign = '-';
                }
                snprintf(diff, 256, "%c%ld.%06ld", diffsign, timediff.tv_sec, timediff.tv_usec);
            }
            
            printf("%ld.%06ld\t%s\t%d\t%d\t%d\n", 
                packet->lowlevel_timestamp.tv_sec, 
                packet->lowlevel_timestamp.tv_usec, 
                diff, 
                packet->sequence_number,
                packet->rtp_timestamp, 
                packet->payload_type
            );
            memcpy(&state->prev_timestamp, &packet->lowlevel_timestamp, sizeof(struct timeval));
            wr_rtp_filter_notify_observers(filter, event, packet);
            return WR_OK;
        }
        case TRANSMISSION_END: {
            free(filter->state);
            wr_rtp_filter_notify_observers(filter, event, packet);
            return WR_OK;
        }
    }  
}
