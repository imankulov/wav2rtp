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


int print_sdp()
{
    printf("m=audio 8000 RTP/AVP 97\n");
    printf("a=rtpmap:97 speex/8000\n");
    printf("a=fmtp:97 mode=4;mode=any\n");
}


int main(int argc, char ** argv)
{
    t_codec_list * current_codec_list; 
    t_output output;


	RtpSession *session = NULL;
	uint32_t user_ts = 0; // TODO: This should be random

    /* Get options from command line */
    if (get_options(argc, argv)){
        return 1;
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
    current_codec_list = wr_options.codec_list;
    while(current_codec_list){
        SNDFILE * file;
        SF_INFO file_info;
        t_codec codec = *(current_codec_list->codec);

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

        /* Update payload type to output */
        (*output.set_payload_type)(&output, codec.payload_type);

        while(1){
            int input_buffer_size = (*codec.get_input_buffer_size)(codec.state);
            int output_buffer_size = (*codec.get_input_buffer_size)(codec.state);
            short input_buffer[input_buffer_size];
            char output_buffer[output_buffer_size];

            bzero(input_buffer, input_buffer_size * sizeof(short));
            input_buffer_size = sf_read_short(file, input_buffer, input_buffer_size);

            if (!input_buffer_size){/* EOF */
                sf_close(file);
                break;
            }

            /* encode data */
            bzero(output_buffer, output_buffer_size);
            output_buffer_size = (*codec.encode)(codec.state, input_buffer, output_buffer);
     
            /* write out  encoded data */
            (*output.write)(output.state, output_buffer, output_buffer_size, 1000 * input_buffer_size / file_info.samplerate);
        }

        /* Next iteration */
        current_codec_list = current_codec_list->next;
    }
    (*output.destroy)(&output);
    free_codec_list(wr_options.codec_list);
    return 0; 
}
