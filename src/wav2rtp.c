/**
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
#include <string.h>
#include <sndfile.h> 

#include "wav2rtp.h"

#include "options.h"
#include "error_types.h"

#include "speex_codec.h"
#include "dummy_codec.h"
#include "gsm_codec.h"
#include "g711u_codec.h"

#include "network_emulator.h"

#include "pcap_output.h"

/** XXX: unused */
int print_sdp()
{
    printf("m=audio 8000 RTP/AVP 97\n");
    printf("a=rtpmap:97 speex/8000\n");
    printf("a=fmtp:97 mode=4;mode=any\n");
}

/**
 * Print to stdout essentials parts of thie SIPp XML scenario
 * Uses speech duration (in ms) and global variable  wr_options
 */

void print_sipp_scenario(int duration)
{

    if (!wr_options.print_sipp_scenario)
        return; 

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
        wr_codec_t * current_codec = (wr_codec_t *)list_iterator_next(wr_options.codec_list);
        printf("%d ", current_codec->payload_type);
        /* TODO:  check for duplicates */
    }
    printf("\n");
    list_iterator_stop(wr_options.codec_list);

    list_iterator_start(wr_options.codec_list);
    while(list_iterator_hasnext(wr_options.codec_list)){
        wr_codec_t * current_codec = (wr_codec_t *)list_iterator_next(wr_options.codec_list);
        printf("a=rtpmap:%d %s/%d\n", current_codec->payload_type, current_codec->name, current_codec->sample_rate);
    }
    list_iterator_stop(wr_options.codec_list);
    /* speex hack */
    /*
    list_iterator_start(wr_options.codec_list);
    while(list_iterator_hasnext(wr_options.codec_list)){
        wr_codec_t * current_codec = (wr_codec_t *)list_iterator_next(wr_options.codec_list);
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


int main(int argc, char ** argv)
{

    int duration = 0;   /* total output duration */
    int get_options_retval = 0;

    wr_output_t output;
    wr_network_emulator_t netem;
    wr_data_frame_t * current_frame = NULL;
    wr_codec_t * codec;
    int frames_count = 0;
    list_t * data_frames = NULL;

    SNDFILE * file;
    SF_INFO file_info;

    /* get options from command line */
    if (get_options_retval = get_options(argc, argv)){
        wr_print_error();
        return get_options_retval;
    }

    /* network emulator initialization */
    wr_network_emulator_init(&netem);
    
    /* output initialization. there is only one output type: pcap files */
    if (!wr_options.output_filename){
        fprintf(stderr, "Output filename is not set");
        free_codec_list(wr_options.codec_list);
        return CANNOT_INITIALIZE_OUTPUT;
    }
    if (strncmp(wr_options.output_filename, wr_options.filename, 1024) == 0){
        fprintf(stderr, "Output filename is the same that input filename");
        free_codec_list(wr_options.codec_list);
        return CANNOT_INITIALIZE_OUTPUT;
    }       
    if(!wr_pcap_init_output(&output)){
        wr_print_error();
        free_codec_list(wr_options.codec_list);
        return CANNOT_INITIALIZE_OUTPUT;
    }
    
    /* open WAV file */
    file = sf_open(wr_options.filename, SFM_READ, &file_info);
    if (!file){
        printf("Cannot open or render sound file");
        return CANNOT_RENDER_WAVFILE;
    }
    if (file_info.samplerate != 8000){
        printf("Sorry, this tool currently works only with .wav files in 8kHz. Rerecord your signal or resample it (with sox, for example)\n");
        sf_close(file);
        return CANNOT_RENDER_WAVFILE; 
    }

    list_iterator_start(wr_options.codec_list);
    if (list_iterator_hasnext(wr_options.codec_list)){
        codec = (wr_codec_t*)list_iterator_next(wr_options.codec_list); 
    }else{
        codec = NULL;
    }
    while(codec){
        
        int input_buffer_size = (*codec->get_input_buffer_size)(codec->state);
        int output_buffer_size = (*codec->get_output_buffer_size)(codec->state);        
        short input_buffer[input_buffer_size];
        int tmp_frame_size = 0;

        bzero(input_buffer, input_buffer_size * sizeof(short));
        input_buffer_size = sf_read_short(file, input_buffer, input_buffer_size);
        if (!input_buffer_size){/* EOF */            
            if (data_frames){
                (*output.write)(output.state, data_frames, codec, &netem);
                data_frames = NULL;
            }
            sf_seek(file, 0, SEEK_SET);
            if (list_iterator_hasnext(wr_options.codec_list)){
                codec = (wr_codec_t*)list_iterator_next(wr_options.codec_list);
            }else{
                codec = NULL;
                list_iterator_stop(wr_options.codec_list);
            }
            continue;
        }
        duration += input_buffer_size * 1000 / file_info.samplerate;
        current_frame = (wr_data_frame_t *)malloc(sizeof(wr_data_frame_t));
        if (!current_frame){
            wr_set_error("Error in memory allocation");
            return CANNOT_RENDER_WAVFILE;
        }
        bzero(current_frame, sizeof(wr_data_frame_t));
        if (!data_frames){
            data_frames = (list_t *)malloc(sizeof(list_t));
            if (!data_frames){
                wr_set_error("Error in memory allocation");
            }
            bzero(data_frames, sizeof(*data_frames));
            list_init(data_frames);
            frames_count = 0;
        }else{
            frames_count ++;
        }
        list_append(data_frames, current_frame);
        tmp_frame_size = output_buffer_size;
        current_frame->length_in_ms = 1000 * input_buffer_size / file_info.samplerate;
        current_frame->data = malloc(output_buffer_size);
        if (!current_frame->data){
            wr_set_error("Error in memory allocation for datastructure");
            return CANNOT_RENDER_WAVFILE;
        }
        bzero(current_frame->data, output_buffer_size);
        current_frame->size = (*codec->encode)(codec->state, input_buffer, current_frame->data);
        if (current_frame->size > tmp_frame_size){
            printf("ooops: encoding process fill more memory than allocated to it\n");
        }
        if (frames_count == iniparser_getpositiveint(wr_options.output_options, "global:rtp_in_frame", 1) - 1 ){
            (*output.write)(output.state, data_frames, codec, &netem);
            data_frames = NULL;
        }
    }   
    print_sipp_scenario(duration);
    sf_close(file);
    (*output.destroy)(&output);
    free_codec_list(wr_options.codec_list);
    return 0; 
}
