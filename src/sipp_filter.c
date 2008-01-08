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
#include "sipp_filter.h"
#include "codecapi.h"




void __print_sipp_scenario(int duration)
{

    printf("***sdp_data_packet***\n"); 
    printf(
        "v=0\n"
        "o=user1 53655765 2353687637 IN IP[local_ip_type] [local_ip]\n"
        "s=-\n"
        "c=IN IP[local_ip_type] [local_ip]\n"
        "t=0 0\n"
        "m=audio [auto_media_port] RTP/AVP ");

    /* Print payload types without duplicates */
    list_iterator_start(wr_options.codec_list);
    while(list_iterator_hasnext(wr_options.codec_list)){
        wr_encoder_t * current_codec = (wr_encoder_t *)list_iterator_next(wr_options.codec_list);
        printf("%d ", current_codec->payload_type);
        /* TODO:  check for duplicates */
    }
    printf("\n");
    list_iterator_stop(wr_options.codec_list);

    list_iterator_start(wr_options.codec_list);
    while(list_iterator_hasnext(wr_options.codec_list)){
        wr_encoder_t * current_codec = (wr_encoder_t *)list_iterator_next(wr_options.codec_list);
        printf("a=rtpmap:%d %s/%d\n", current_codec->payload_type, current_codec->name, current_codec->sample_rate);
    }
    list_iterator_stop(wr_options.codec_list);
    /* speex hack */
    /*
    list_iterator_start(wr_options.codec_list);
    while(list_iterator_hasnext(wr_options.codec_list)){
        wr_encoder_t * current_codec = (wr_encoder_t *)list_iterator_next(wr_options.codec_list);
        if (strncmp(current_codec->name, "speex", 6)==0){
            speex_state * state = (speex_state *)current_codec->state;
            printf("a=fmtp:%d mode=%d;mode=%d\n", current_codec->payload_type, state->quality, state->quality);
        }
    }
    list_iterator_stop(wr_options.codec_list);
    */
    printf("\n");

    if (wr_options.output_filename){
        printf("***play_pcap_xml***\n");
        printf(
            "<nop>\n"
            "   <action>\n"
            "       <exec play_pcap_audio=\"%s\"/>\n"
            "   </action>\n"
            "</nop>\n"
            "<pause milliseconds=\"%d\"/>\n", wr_options.output_filename, duration);
    }
}



wr_errorcode_t wr_sipp_filter_notify(wr_rtp_filter_t * filter, wr_event_type_t event, wr_rtp_packet_t * packet)
{
    switch(event){

        case TRANSMISSION_START:  {
            wr_sipp_filter_state_t * state = calloc(1, sizeof(*state));
            state->enabled = iniparser_getboolean(wr_options.output_options, "sipp:enabled", 1);
            filter->state = (void*)state;
            wr_rtp_filter_notify_observers(filter, event, packet);
            return WR_OK;
        }
        case NEW_PACKET: {
            double rand;
            wr_sipp_filter_state_t * state = (wr_sipp_filter_state_t * ) (filter->state);
            if (!state->enabled){
                wr_rtp_filter_notify_observers(filter, event, packet);
                return WR_OK;
            }
            if (!timerisset(&state->first_timestamp)){
                memcpy(&state->first_timestamp, &packet->lowlevel_timestamp, sizeof(struct timeval));
            }
            memcpy(&state->last_timestamp, &packet->lowlevel_timestamp, sizeof(struct timeval));
            list_iterator_start(&packet->data_frames);
            state->last_duration = 0;
            while (list_iterator_hasnext(&packet->data_frames)) {
                wr_data_frame_t * data = (wr_data_frame_t*)list_iterator_next(&packet->data_frames);
                state->last_duration += data->length_in_ms;
            }
            list_iterator_stop(&packet->data_frames);
            wr_rtp_filter_notify_observers(filter, event, packet);
            return WR_OK;
        }
        case TRANSMISSION_END: {
            wr_sipp_filter_state_t * state = (wr_sipp_filter_state_t * ) (filter->state);
            if (state->enabled){
                struct timeval diff;
                int duration = state->last_duration;
                timersub(&state->last_timestamp, &state->first_timestamp, &diff);
                duration += (diff.tv_sec * 1000 + diff.tv_usec / 1000);
                __print_sipp_scenario(duration);
            }
            free(state);
            wr_rtp_filter_notify_observers(filter, event, packet);
            return WR_OK;
        }
    }  
}
