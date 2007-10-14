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
#include <ortp/ortp.h>


#include "wav2rtp.h"

#include "options.h"
#include "error_types.h"

#include "speex_codec.h"
#include "dummy_codec.h"
#include "gsm_codec.h"
#include "g711u_codec.h"

#include "rtp_output.h"
#include "raw_output.h"
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

    t_codec_list * current_codec = wr_options.codec_list;

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
    while(current_codec){
        /* forward check for duplicates */
        t_codec_list * tmp = current_codec->next;
        int duplicate_found = 0;
        while(tmp){
            if (tmp->codec->payload_type == current_codec->codec->payload_type){
                duplicate_found = 1;
                break;
            }
            tmp = tmp->next; 
        }
        if(!duplicate_found){
            printf("%d ", current_codec->codec->payload_type);
        } 
        current_codec = current_codec->next;
    }
    printf("\n");
    current_codec = wr_options.codec_list;
    while(current_codec){
        /* once again forward check for duplicates */
        t_codec_list * tmp = current_codec->next;
        int duplicate_found = 0;
        while(tmp){
            if (tmp->codec->payload_type == current_codec->codec->payload_type){
                duplicate_found = 1;
                break;
            }
            tmp = tmp->next; 
        }
        if (!duplicate_found){
            printf("a=rtpmap:%d %s/%d\n", current_codec->codec->payload_type, current_codec->codec->name, current_codec->codec->sample_rate);
        }
        current_codec = current_codec->next;
    }
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
    int retval = 0;

    t_codec_list * current_codec_list; 
    t_output output;    

    SNDFILE * file;
    SF_INFO file_info;

	uint32_t user_ts = 0; /* TODO: This should be random */

    /* Get options from command line */
    if (retval = get_options(argc, argv)){
        wr_print_error();
        return retval;
    }
    
    /* output initialization */
    if (strncmp(wr_options.output_type, "rtp", 4) == 0){
        if (!wr_rtp_init_output(&output)){
            perror("Cannot initalize datastructure to send data via RTP");
            free_codec_list(wr_options.codec_list);
            return CANNOT_INITIALIZE_OUTPUT;
        }
    }else if(strncmp(wr_options.output_type, "raw", 4) == 0){
        if (!wr_options.output_filename){
            printf("Output filename is not set");
            free_codec_list(wr_options.codec_list);
            return CANNOT_INITIALIZE_OUTPUT;
        }
        if (strncmp(wr_options.output_filename, wr_options.filename, 1024) == 0){
            printf("Output filename is the same that input filename");
            free_codec_list(wr_options.codec_list);
            return CANNOT_INITIALIZE_OUTPUT;
        }       
        if(!wr_raw_init_output(&output)){
            perror("Cannot write datastructure into raw file");
            free_codec_list(wr_options.codec_list);
            return CANNOT_INITIALIZE_OUTPUT;
        }
    }else if(strncmp(wr_options.output_type, "pcap", 5) == 0){
        if (!wr_options.output_filename){
            perror("Output filename is not set");
            free_codec_list(wr_options.codec_list);
            return CANNOT_INITIALIZE_OUTPUT;
        }
        if (strncmp(wr_options.output_filename, wr_options.filename, 1024) == 0){
            printf("Output filename is the same that input filename");
            free_codec_list(wr_options.codec_list);
            return CANNOT_INITIALIZE_OUTPUT;
        }       
        if(!wr_pcap_init_output(&output)){
            perror("Cannot write datastructure into pcap file");
            free_codec_list(wr_options.codec_list);
            return CANNOT_INITIALIZE_OUTPUT;
        }
    }else{
        printf("Output type not recognized");
        free_codec_list(wr_options.codec_list);
        return OUTPUT_NOT_RECOGNIZED;
    }

    /* Read data and encode it with selected codec */
    
    /* Open WAV file */
    bzero(&file_info, sizeof(file_info));
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
    current_codec_list = wr_options.codec_list;

    while(current_codec_list){
        t_codec codec = *(current_codec_list->codec);

        /* Update payload type to output */
        (*output.set_payload_type)(&output, codec.payload_type);

        while(1){
            int input_buffer_size = (*codec.get_input_buffer_size)(codec.state);
            int output_buffer_size = (*codec.get_input_buffer_size)(codec.state);
            short input_buffer[input_buffer_size];
            char output_buffer[output_buffer_size];

            bzero(input_buffer, input_buffer_size * sizeof(short));
            input_buffer_size = sf_read_short(file, input_buffer, input_buffer_size);

            duration += input_buffer_size * 1000 / file_info.samplerate;

            if (!input_buffer_size){/* EOF */
                sf_seek(file, 0, SEEK_SET);
                break;
            }

            /* encode data */
            bzero(output_buffer, output_buffer_size);
            output_buffer_size = (*codec.encode)(codec.state, input_buffer, output_buffer);
     
            /* write out  encoded data */
            (*output.write)(output.state, output_buffer, output_buffer_size, 1000 * input_buffer_size / file_info.samplerate, 0, 0);
        }

        /* Next iteration */
        current_codec_list = current_codec_list->next;
    }    
    sf_close(file);
    (*output.destroy)(&output);
    free_codec_list(wr_options.codec_list);
    print_sipp_scenario(duration);
    return 0; 
}
