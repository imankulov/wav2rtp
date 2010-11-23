/*
 * $Id$
 * 
 * Copyright (c) 2010, R.Imankulov, Yu Jiang
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



wr_encoder_t encoder_map[] = {
    {"DUMMY", "Codec for testing and demo purposes", NULL, 111, 8000, 
            wr_dummy_encoder_get_input_buffer_size, wr_dummy_encoder_get_output_buffer_size, wr_dummy_encode, wr_dummy_encoder_init, wr_dummy_encoder_destroy}, 
    {"GSM", "GSM 06.10 full-rate codec", NULL, 3, 8000, 
            wr_gsm_encoder_get_input_buffer_size, wr_gsm_encoder_get_output_buffer_size, wr_gsm_encode, wr_gsm_encoder_init, wr_gsm_encoder_destroy}, 
    {"speex", "Speex narrowband mode codec", NULL, 96, 8000, 
            wr_speex_encoder_get_input_buffer_size, wr_speex_encoder_get_output_buffer_size, wr_speex_encode, wr_speex_encoder_init, wr_speex_encoder_destroy}, 
    {"PCMU", "ITU-T G.711 codec with u-law compression", NULL, 0, 8000, 
            wr_g711u_encoder_get_input_buffer_size, wr_g711u_encoder_get_output_buffer_size, wr_g711u_encode, wr_g711u_encoder_init, wr_g711u_encoder_destroy}, 
    {"PCMA", "ITU-T G.711 codec with a-law compression", NULL, 8, 8000, 
            wr_g711a_encoder_get_input_buffer_size, wr_g711a_encoder_get_output_buffer_size, wr_g711a_encode, wr_g711a_encoder_init, wr_g711a_encoder_destroy}, 
    {NULL, NULL, NULL, 0, 0,
            NULL, NULL, NULL, NULL, NULL}
};


wr_decoder_t decoder_map[] = {
    {"speex", "Speex narrowband mode codec", NULL, 96, 8000, 
            wr_speex_encoder_get_input_buffer_size, wr_speex_encoder_get_output_buffer_size, wr_speex_decode, wr_speex_decoder_init, wr_speex_decoder_destroy},
    {"PCMU", "ITU-T G.711 codec with u-law compression", NULL, 0, 8000, 
            wr_g711u_decoder_get_input_buffer_size, wr_g711u_decoder_get_output_buffer_size, wr_g711u_decode, wr_g711u_decoder_init, wr_g711u_decoder_destroy}, 
    {"PCMA", "ITU-T G.711 codec with a-law compression", NULL, 8, 8000, 
            wr_g711a_decoder_get_input_buffer_size, wr_g711a_decoder_get_output_buffer_size, wr_g711a_decode, wr_g711a_decoder_init, wr_g711a_decoder_destroy}, 

    {NULL, NULL, NULL, 0, 0,
            NULL, NULL, NULL, NULL, NULL}
};


wr_encoder_t * get_encoder_by_name(const char * name)
{
    int i=0;
    wr_encoder_t * pcodec;
    while((pcodec = &encoder_map[i])->name){
        if (strncmp(pcodec->name, name, WR_MAX_CODEC_NAME_SIZE) == 0 ){
            return pcodec;
        }
        i++;
    }
    return NULL;
}



wr_encoder_t * get_encoder_by_pt(int payload_type)
{
    int i=0;
    wr_encoder_t * pcodec;
    while((pcodec = &encoder_map[i])->name){
        if (pcodec->payload_type == payload_type){
            return pcodec;
        }
        i++;
    }
    return NULL;
}



wr_decoder_t * get_decoder_by_name(const char * name)
{
    int i=0;
    wr_decoder_t * pcodec;
    while((pcodec = &decoder_map[i])->name){
        if (strncmp(pcodec->name, name, WR_MAX_CODEC_NAME_SIZE) == 0 ){
            return pcodec;
        }
        i++;
    }
    return NULL;
}



wr_decoder_t * get_decoder_by_pt(int payload_type)
{
    int i=0;
    wr_decoder_t * pcodec;
    while((pcodec = &decoder_map[i])->name){
        if (pcodec->payload_type == payload_type){
            return pcodec;
        }
        i++;
    }
    return NULL;
}

