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
#include "rtpmap.h"



wr_codec_t rtpmap[] = {
    {"DUMMY", "Codec for testing and demo purposes", NULL, 111, 8000, 
            dummy_get_input_buffer_size, dummy_get_output_buffer_size, dummy_encode, dummy_init_codec, dummy_destroy_codec}, 
    {"GSM", "GSM 06.10 full-rate codec", NULL, 3, 8000, 
            wr_gsm_get_input_buffer_size, wr_gsm_get_output_buffer_size, wr_gsm_encode, gsm_init_codec, wr_gsm_destroy_codec}, 
    {"speex", "Speex narrowband mode codec", NULL, 96, 8000, 
            wr_speex_get_input_buffer_size, wr_speex_get_output_buffer_size, wr_speex_encode, wr_speex_init_codec, wr_speex_destroy_codec}, 
    {"PCMU", "ITU-T G.711 codec with u-law compression", NULL, 0, 8000, 
            wr_g711u_get_input_buffer_size, wr_g711u_get_output_buffer_size, wr_g711u_encode, g711u_init_codec, wr_g711u_destroy_codec}, 

    {NULL, NULL, NULL, 0, 0,
            NULL, NULL, NULL, NULL, NULL}
};



wr_codec_t * get_codec_by_name(const char * name)
{
    int i=0;
    wr_codec_t * pcodec;
    while((pcodec = &rtpmap[i])->name){
        if (strncmp(pcodec->name, name, WR_MAX_CODEC_NAME_SIZE) == 0 ){
            return pcodec;
        }
        i++;
    }
    return NULL;
}



wr_codec_t * get_codec_by_pt(int payload_type)
{
    int i=0;
    wr_codec_t * pcodec;
    while((pcodec = &rtpmap[i])->name){
        if (pcodec->payload_type == payload_type){
            return pcodec;
        }
        i++;
    }
    return NULL;
}

