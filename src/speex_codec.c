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
#include "speex_codec.h"
#include "options.h"
#include <stdlib.h>



/**
 * Initialize object of type wr_codec_t to use with Speex.
 * Memory for pcodec object have to be already allocated
 * @return the same pinter to wr_codec_t or NULL in case when something goes wrong 
 */
wr_codec_t * wr_speex_init_codec(wr_codec_t * pcodec)
{

    speex_state * state = malloc(sizeof(speex_state));
    if (!state)
        return NULL;

    speex_bits_init(&(state->bits));
    state->enc_state = speex_encoder_init(&speex_nb_mode);
    
    state->quality = iniparser_getint(wr_options.codecs_options, "speex:quality", -1);
    state->complexity = iniparser_getint(wr_options.codecs_options, "speex:complexity", -1);
    state->bitrate = iniparser_getint(wr_options.codecs_options, "speex:bitrate", -1);
    state->abr_enabled = iniparser_getboolean(wr_options.codecs_options, "speex:abr_enabled", 0);
    state->vad_enabled = iniparser_getboolean(wr_options.codecs_options, "speex:vad_enabled", 0);
    state->dtx_enabled = iniparser_getboolean(wr_options.codecs_options, "speex:dtx_enabled", 0);
    state->vbr_enabled = iniparser_getboolean(wr_options.codecs_options, "speex:vbr_enabled", 0);
    state->vbr_quality = iniparser_getdouble(wr_options.codecs_options, "speex:vbr_quality", -1);
    #ifdef SPEEX_SET_VBR_MAX_BITRATE
    state->vbr_max_bitrate = iniparser_getint(wr_options.codecs_options, "speex:vbr_max_bitrate", -1);
    #endif

    pcodec->name = "speex";
    pcodec->description = "Speex NB codec";
    pcodec->sample_rate = 8000;

    pcodec->state = (void*)state;
    pcodec->payload_type = iniparser_getnonnegativeint(wr_options.codecs_options, "speex:payload_type", 96);
    pcodec->get_input_buffer_size = &wr_speex_get_input_buffer_size;
    pcodec->get_output_buffer_size = &wr_speex_get_output_buffer_size;
    pcodec->encode = &wr_speex_encode;
    pcodec->destroy = &wr_speex_destroy_codec;
   
    /* set up speex variables */
    speex_encoder_ctl(state->enc_state, SPEEX_SET_VAD, &(state->vad_enabled));
    speex_encoder_ctl(state->enc_state, SPEEX_SET_DTX, &(state->dtx_enabled));
    speex_encoder_ctl(state->enc_state, SPEEX_SET_ABR, &(state->abr_enabled));
    speex_encoder_ctl(state->enc_state, SPEEX_SET_VBR, &(state->vbr_enabled));
    if (state->vbr_quality >= 0 && state->vbr_quality <= 10){
        speex_encoder_ctl(state->enc_state, SPEEX_SET_VBR_QUALITY, &(state->vbr_quality));
    }else{
        state->vbr_quality = -1;
    }
    #ifdef SPEEX_SET_VBR_MAX_BITRATE
    if (state->vbr_max_bitrate > 0){
        speex_encoder_ctl(state->enc_state, SPEEX_SET_VBR_MAX_BITRATE, &(state->vbr_max_bitrate));
    }else{
        state->vbr_max_bitrate = -1;
    }
    #endif
    if (state->quality >= 0 && state->quality <= 10){
        speex_encoder_ctl(state->enc_state, SPEEX_SET_QUALITY, &(state->quality));
    }else{
        state->quality = -1;
    }
    if (state->complexity >= 0 && state->complexity <= 10){
        speex_encoder_ctl(state->enc_state, SPEEX_SET_COMPLEXITY, &(state->complexity));
    }else{
        state->complexity = -1;
    }
    return pcodec;
}

/**
 * Destroy object of type wr_codec_t for Speex
 */
void wr_speex_destroy_codec(wr_codec_t * pcodec)
{
    speex_state * state = (speex_state*)(pcodec->state);
    speex_encoder_destroy(state->enc_state);
    free(pcodec->state);
}

/**
 * Size of the input array for codec
 * This function return size of the input array which will be used on the next
 * encoding  iteration. Note that this value may be changed from one cycle step
 * to another. Note also that because every codec receive data in the format of
 * the array of shorts, real size of needed memory (passed to .alloc function)
 * will be: 
 *
 * speex_get_input_buffer_size(state) * sizeof(short)
 */
int wr_speex_get_input_buffer_size(void * state)
{
    int frame_size = 0;
    speex_state * sstate = (speex_state *)state;
    speex_encoder_ctl(sstate->enc_state, SPEEX_GET_FRAME_SIZE, &frame_size);
    return frame_size;
}

/**
 * Size of the output buffer 
 * This size is sufficient to write all data, encoded with speex_encode_array()
 * function. Real data size (which always is equal or less of this one) 
 * returned by speex_encode_array
 */
int wr_speex_get_output_buffer_size(void * state)
{
    speex_state * s = (speex_state *)state;
    /* FIXME: it's not a right way (need to use speex_bits_nbytes),
       but I don't know why  speex_bits_nbytes always returns 0 */
    int frame_size = 0;
    speex_encoder_ctl(s->enc_state, SPEEX_GET_FRAME_SIZE, &frame_size);
    return frame_size;
    /* return speex_bits_nbytes(&(s->bits)); */
}
/**
 * Do encoding of the one data frame
 */
int wr_speex_encode(void * state, const short * input, char * output)
{
        int speex_bytes = 0; //     Number of bytes which actually were be written out      
        speex_state * sstate = (speex_state *)state;
        speex_bits_reset(&(sstate->bits));
        speex_encode_int(sstate->enc_state, (short*)input, &(sstate->bits));
        speex_bytes = speex_bits_write(&(sstate->bits), output, speex_bits_nbytes(&(sstate->bits)));
        return speex_bytes; 
}
