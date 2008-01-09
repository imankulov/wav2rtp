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
#include "options.h"
#include "rtpmap.h"
#include "wavfile_output_filter.h"



wr_errorcode_t wr_wavfile_seek(wr_wavfile_output_filter_state_t * state, const struct timeval * tv)
{
    if (timercmp(tv, &state->end_time, >)){
        struct timeval tv_offset;
        int offset;
        short * data;

        timersub(tv, &state->end_time, &tv_offset); 
        offset = (int)((tv_offset.tv_sec * 1e6 + tv_offset.tv_usec) * state->file_info.samplerate / 1e6);
        data = calloc(offset, sizeof(short));
        bzero(data, sizeof(offset * sizeof(short)));
        sf_seek(state->file, 0, SEEK_END);
        sf_write_short(state->file, data, offset);
        memcpy(&state->end_time, tv, sizeof(struct timeval));
        free(data);
    } else {
        struct timeval tv_offset;
        int offset;

        timersub(tv, &state->start_time, &tv_offset); 
        offset = (int)((tv_offset.tv_sec * 1e6 + tv_offset.tv_usec) * state->file_info.samplerate / 1e6);
        sf_seek(state->file, offset, SEEK_SET);
        
    }
    return WR_OK;
}



wr_errorcode_t wr_wavfile_output_filter_notify(wr_rtp_filter_t * filter, wr_event_type_t event, wr_rtp_packet_t * packet)
{

    switch(event) {
        case TRANSMISSION_START:
            {
                wr_wavfile_output_filter_state_t * state = calloc(1, sizeof(*state));
                state->file_info.samplerate = 8000; /* samples per second */
                state->file_info.channels = 1;
                state->file_info.format = SF_FORMAT_WAV|SF_FORMAT_PCM_16;
                state->file = sf_open(iniparser_getstring(wr_options.output_options, "wavfile_output:filename", "output.wav"), SFM_WRITE, &state->file_info);
                filter->state = (void*)state;
            }
            return WR_OK;

        case NEW_PACKET:
            {
                wr_wavfile_output_filter_state_t * state = (wr_wavfile_output_filter_state_t * ) (filter->state);
                wr_decoder_t * decoder = get_decoder_by_pt(packet->payload_type);
                struct timeval tv_offset;
                int offset = 0;


                if (!timerisset(&state->start_time)){
                    memcpy(&state->start_time, &packet->lowlevel_timestamp, sizeof(struct timeval)); 
                } 
                if (!timerisset(&state->end_time)){
                    memcpy(&state->end_time, &packet->lowlevel_timestamp, sizeof(struct timeval)); 
                } 

                if (!decoder){
                    wr_set_error("cannot found RTP decoder");
                    return WR_FATAL;
                }
                if (!wr_decoder_is_initialized(decoder))
                    decoder->init(decoder);

                wr_wavfile_seek(state, &packet->lowlevel_timestamp);
                list_iterator_start(&packet->data_frames);
                while(list_iterator_hasnext(&packet->data_frames)){                    
                    wr_data_frame_t * frame = (wr_data_frame_t * ) list_iterator_next(&packet->data_frames);
                    int output_size = decoder->get_output_buffer_size(decoder->state);
                    short * output  =  calloc(output_size, sizeof(short));
                    decoder->decode(decoder->state, frame->data, frame->size, output);
                    sf_write_short(state->file, output, output_size);
                    timeval_increment(&state->end_time, frame->length_in_ms * 1000);
                    free(output);
                }
                list_iterator_stop(&packet->data_frames);
            }
            return WR_OK;

        case TRANSMISSION_END:
            {            
                wr_wavfile_output_filter_state_t * state = (wr_wavfile_output_filter_state_t * ) (filter->state);
                sf_close(state->file);
            }
            return WR_OK;
    }
}
