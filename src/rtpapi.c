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
#include "rtpapi.h"
#include "contrib/simclist.h"



int wr_rtp_packet_init(wr_rtp_packet_t * rtp_packet, int payload_type, int sequence_number, int markbit, uint32_t rtp_timestamp, struct timeval lowlevel_timestamp)
{
    rtp_packet->payload_type = payload_type;
    rtp_packet->sequence_number = sequence_number;
    rtp_packet->rtp_timestamp = rtp_timestamp;
    rtp_packet->lowlevel_timestamp.tv_sec = lowlevel_timestamp.tv_sec;
    rtp_packet->lowlevel_timestamp.tv_usec = lowlevel_timestamp.tv_usec;
    rtp_packet->markbit = markbit;
    list_init(&rtp_packet->data_frames);
}



int wr_rtp_packet_copy(wr_rtp_packet_t * new_packet, wr_rtp_packet_t * old_packet)
{
    memcpy(new_packet, old_packet, sizeof(wr_rtp_packet_t));
    return 0;
}



int wr_rtp_packet_copy_with_data(wr_rtp_packet_t * new_packet, wr_rtp_packet_t * old_packet)
{
    wr_rtp_packet_copy(new_packet, old_packet);
    list_init(&new_packet->data_frames);
    list_iterator_start(&old_packet->data_frames);
    while(list_iterator_hasnext(&old_packet->data_frames)){
        wr_data_frame_t * frame = (wr_data_frame_t * ) list_iterator_next(&old_packet->data_frames);
        wr_rtp_packet_add_frame(new_packet, frame->data, frame->size, frame->length_in_ms);
    }
    list_iterator_stop(&old_packet->data_frames);
    return 0;
}



void wr_rtp_packet_destroy(wr_rtp_packet_t * rtp_packet)
{
    list_iterator_start(&rtp_packet->data_frames);
    while(list_iterator_hasnext(&rtp_packet->data_frames)){
        wr_data_frame_t * frame = (wr_data_frame_t * ) list_iterator_next(&rtp_packet->data_frames);
        free(frame->data);
        free(frame);
    }
    list_iterator_stop(&rtp_packet->data_frames);
    list_destroy(&rtp_packet->data_frames);
}



wr_errorcode_t wr_rtp_packet_add_frame(wr_rtp_packet_t * packet, uint8_t * data, size_t size, int length_in_ms)
{
    /* XXX: no checks for calloc unsucessfull call */
    wr_data_frame_t * frame = calloc(1, sizeof(wr_data_frame_t));    
    frame->data = calloc(size, sizeof(uint8_t));
    memcpy(frame->data, data, size * sizeof(uint8_t));
    frame->size = size;
    frame->length_in_ms = length_in_ms;
    list_append(&packet->data_frames, frame);
    return WR_OK; 
}



int wr_rtp_packet_delete_frame(wr_rtp_packet_t * packet, int position)
{
    /* XXX: Not yet implemented */
}

void wr_rtp_filter_create(wr_rtp_filter_t * filter, char *name, 
        wr_errorcode_t (*notify)(wr_rtp_filter_t * filter, wr_event_type_t event, wr_rtp_packet_t * packet) 
    )
{
    bzero(filter, sizeof(wr_rtp_filter_t));
    strncpy(filter->name, name, sizeof(filter->name)-1);
    filter->notify = notify;
}



void wr_rtp_filter_append_observer(wr_rtp_filter_t * filter, wr_rtp_filter_t * observer)
{
    int i;
    for (i=0; i<MAX_OBSERVERS-1; i++){
        wr_rtp_filter_t * last_observer = filter->observers[i];
        if (!last_observer){
            filter->observers[i] = observer;
            break;
        }
    }
}



void wr_rtp_filter_notify_observers(wr_rtp_filter_t * filter, wr_event_type_t event, wr_rtp_packet_t * packet)
{
    int i;
    for (i=0; i<MAX_OBSERVERS; i++){
        wr_errorcode_t retval;
        wr_rtp_filter_t * observer = filter->observers[i];
        if (!observer) break;
        retval = (*observer->notify)(observer, event, packet);
        switch(retval){
            case WR_FATAL: 
                fprintf(stderr, "%s\t%s: %s\n", "FATAL", filter->name, wr_error); 
                break;
            case WR_WARN:
                fprintf(stderr, "%s\t%s: %s\n",  "WARNING", filter->name, wr_error); 
                break;
            default:
                break;
        }
    }
}



wr_errorcode_t wr_do_nothing_on_notify(wr_rtp_filter_t * filter, wr_event_type_t event, wr_rtp_packet_t * packet)
{
    /* Do nothing ;-) */
}
