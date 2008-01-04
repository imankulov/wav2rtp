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
#include "sort_filter.h"



int wr_rtp_timestamp_comparator(const void *a, const void *b)
{
    wr_rtp_packet_t * pa = (wr_rtp_packet_t *)a;
    wr_rtp_packet_t * pb = (wr_rtp_packet_t *)b;
    return timercmp(&pa->lowlevel_timestamp, &pb->lowlevel_timestamp, >);
}



wr_errorcode_t wr_sort_filter_notify(wr_rtp_filter_t * filter, wr_event_type_t event, wr_rtp_packet_t * packet)
{
    switch(event){

        case TRANSMISSION_START:  {
            wr_sort_filter_state_t * state = calloc(1, sizeof(*state));
            state->enabled = iniparser_getboolean(wr_options.output_options, "sort:enabled", 1);
            state->buffer_size = iniparser_getpositiveint(wr_options.output_options, "sort:buffer_size", 1);
            list_init(&state->buffer);
            list_attributes_comparator(&state->buffer, &wr_rtp_timestamp_comparator);
            filter->state = (void*)state;
            wr_rtp_filter_notify_observers(filter, event, packet);
            return WR_OK;
        }
        case NEW_PACKET: { 
            wr_sort_filter_state_t * state = (wr_sort_filter_state_t * ) (filter->state);
            if (!state->enabled){
                wr_rtp_filter_notify_observers(filter, event, packet);
                return WR_OK;
            }
            wr_rtp_packet_t * new_packet = calloc(1, sizeof(*new_packet));
            wr_rtp_packet_copy_with_data(new_packet, packet);
            list_append(&state->buffer, (void*)new_packet);
            if (list_size(&state->buffer) > state->buffer_size){
                wr_rtp_packet_t * first_packet;
                list_sort(&state->buffer, -1); 
                first_packet = list_extract_at(&state->buffer, 0);
                wr_rtp_filter_notify_observers(filter, event, first_packet);
                //wr_rtp_packet_destroy(first_packet);
                //free(first_packet);
            }
            return WR_OK;
        }
        case TRANSMISSION_END: {
            wr_sort_filter_state_t * state = (wr_sort_filter_state_t * ) (filter->state);
            list_sort(&state->buffer, -1); 
            while(list_size(&state->buffer)){
                wr_rtp_packet_t * packet;
                packet = list_extract_at(&state->buffer, 0);
                wr_rtp_filter_notify_observers(filter, NEW_PACKET, packet);
                wr_rtp_packet_destroy(packet);
                free(packet);
            }
            list_destroy(&state->buffer);
            free(state);
            wr_rtp_filter_notify_observers(filter, event, packet);
            return WR_OK;
        }
    }  
}
