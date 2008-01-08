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
#include <sndfile.h> 
#include "rtpapi.h"
#include "codecapi.h"
#include "wavfile_filter.h"
#include "options.h"
#include "misc.h"


wr_errorcode_t wr_wavfile_filter_start(wr_rtp_filter_t * filter)
{

    SNDFILE * file;
    SF_INFO file_info;
    wr_encoder_t * codec;
    wr_rtp_packet_t rtp_packet;
    int sequence_number = 0;
    int rtp_timestamp = 0;
    struct timeval packet_start_timestamp;
    struct timeval packet_end_timestamp;

    gettimeofday(&packet_start_timestamp, NULL);
    timeval_copy(&packet_end_timestamp, &packet_start_timestamp);

    int rtp_in_frame = iniparser_getpositiveint(wr_options.output_options, "global:rtp_in_frame", 1);
    int frames_count = 0;

    /* open WAV file */
    file = sf_open(wr_options.filename, SFM_READ, &file_info);
    if (!file){
        wr_set_error("cannot open or render sound file");
        return WR_FATAL;
    }
    if (file_info.samplerate != 8000){
        wr_set_error("this tool works only with .wav files in 8kHz, rerecord your signal or resample it (with sox for example)\n");
        return WR_FATAL; 
    }

    list_iterator_start(wr_options.codec_list);
    if (list_iterator_hasnext(wr_options.codec_list)){
        codec = (wr_encoder_t*)list_iterator_next(wr_options.codec_list); 
        wr_rtp_packet_init(&rtp_packet, codec->payload_type, sequence_number, 1, rtp_timestamp, packet_start_timestamp);
    }else{
        wr_set_error("no codec is found");
        return WR_FATAL;
    }

    wr_rtp_filter_notify_observers(filter, TRANSMISSION_START, NULL);

    /* One cycle iteration encode one data frame */
    while(codec){
        int   input_buffer_size = (*codec->get_input_buffer_size)(codec->state);
        int   output_buffer_size = (*codec->get_output_buffer_size)(codec->state); 
        short input_buffer[input_buffer_size];
        char output_buffer[output_buffer_size];

        bzero(input_buffer, input_buffer_size);
        bzero(output_buffer, output_buffer_size);

        input_buffer_size = sf_read_short(file, input_buffer, input_buffer_size);
        if (!input_buffer_size){ /*EOF*/
            if (frames_count){
                wr_rtp_filter_notify_observers(filter, NEW_PACKET, &rtp_packet);
                wr_rtp_packet_destroy(&rtp_packet);
                frames_count = 0;
                sequence_number++;
                timeval_copy(&packet_start_timestamp, &packet_end_timestamp);
                wr_rtp_packet_init(&rtp_packet, codec->payload_type, sequence_number, 0, rtp_timestamp, packet_start_timestamp);
            }
            sf_seek(file, 0, SEEK_SET);
            if (list_iterator_hasnext(wr_options.codec_list)){
                codec = (wr_encoder_t*)list_iterator_next(wr_options.codec_list);
            }else{
                codec = NULL;
                list_iterator_stop(wr_options.codec_list);
            }
            continue;
        }
        output_buffer_size = (*codec->encode)(codec->state, input_buffer, output_buffer);
        wr_rtp_packet_add_frame(&rtp_packet, output_buffer, output_buffer_size,  1000 * input_buffer_size / file_info.samplerate);
        timeval_increment(&packet_end_timestamp, 1e6 * input_buffer_size / file_info.samplerate);
        rtp_timestamp += input_buffer_size;
        frames_count++;
        if (frames_count == rtp_in_frame){
            wr_rtp_filter_notify_observers(filter, NEW_PACKET, &rtp_packet);
            wr_rtp_packet_destroy(&rtp_packet);
            frames_count = 0;
            sequence_number++;
            timeval_copy(&packet_start_timestamp, &packet_end_timestamp);
            wr_rtp_packet_init(&rtp_packet, codec->payload_type, sequence_number, 0, rtp_timestamp, packet_start_timestamp);
        }
    }   
    wr_rtp_filter_notify_observers(filter, TRANSMISSION_END, NULL);
    sf_close(file);
}
