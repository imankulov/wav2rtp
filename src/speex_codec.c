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
#include "speex_codec.h"
#include "options.h"
#include <stdlib.h>



wr_encoder_t * wr_speex_encoder_init(wr_encoder_t * pcodec)
{

    speex_encoder_state * state = malloc(sizeof(speex_encoder_state));
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

    pcodec->state = (void*)state;
    pcodec->payload_type = iniparser_getnonnegativeint(wr_options.codecs_options, "speex:payload_type", 96);
   
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

void wr_speex_encoder_destroy(wr_encoder_t * pcodec)
{
    speex_encoder_state * state = (speex_encoder_state*)(pcodec->state);
    speex_encoder_destroy(state->enc_state);
    free(pcodec->state);
}

int wr_speex_encoder_get_input_buffer_size(void * state)
{
    int frame_size = 0;
    speex_encoder_state * sstate = (speex_encoder_state *)state;
    speex_encoder_ctl(sstate->enc_state, SPEEX_GET_FRAME_SIZE, &frame_size);
    return frame_size;
}

int wr_speex_encoder_get_output_buffer_size(void * state)
{
    /* FIXME: This returns wittingly larger than needed size of the output buffer */
    /*
    int frame_size = 0;
    speex_encoder_state * s = (speex_encoder_state *)state;
    speex_encoder_ctl(s->enc_state, SPEEX_GET_FRAME_SIZE, &frame_size);
    return frame_size;
    */
    return 600;

}
int wr_speex_encode(void * state, const short * input, char * output)
{
        int speex_bytes = 0;
        speex_encoder_state * sstate = (speex_encoder_state *)state;
        speex_bits_reset(&(sstate->bits));
        speex_encode_int(sstate->enc_state, (short*)input, &(sstate->bits));
        speex_bytes = speex_bits_write(&(sstate->bits), output, speex_bits_nbytes(&(sstate->bits)));
        return speex_bytes; 
}


wr_decoder_t * wr_speex_decoder_init(wr_decoder_t * pcodec)
{
    speex_decoder_state * state = malloc(sizeof(speex_decoder_state));
    if (!state)
        return NULL;

    speex_bits_init(&(state->bits));
    state->dec_state = speex_decoder_init(&speex_nb_mode);

    pcodec->state = (void*)state;
    pcodec->payload_type = iniparser_getnonnegativeint(wr_options.codecs_options, "speex:payload_type", 96);
    return pcodec;
}


void wr_speex_decoder_destroy(wr_decoder_t * pcodec)
{
    speex_decoder_state * state = (speex_decoder_state*)(pcodec->state);
    speex_decoder_destroy(state->dec_state);
    free(pcodec->state);
}


int wr_speex_decoder_get_input_buffer_size(void * state)
{
    /* FIXME: This returns wittingly larger than needed size of the output buffer */
    /*
    int frame_size = 0;
    speex_decoder_state * s = (speex_decoder_state *)state;
    speex_decoder_ctl(s->dec_state, SPEEX_GET_FRAME_SIZE, &frame_size);
    return frame_size;
    */
    return 600;
}


int wr_speex_decoder_get_output_buffer_size(void * state)
{
    int frame_size = 0;
    speex_decoder_state * sstate = (speex_decoder_state *)state;
    speex_decoder_ctl(sstate->dec_state, SPEEX_GET_FRAME_SIZE, &frame_size);
    return frame_size;
}


int wr_speex_decode(void * state, const char * input, size_t input_size, short * output)
{
        int speex_bytes = 0;
        speex_decoder_state * sstate = (speex_decoder_state *)state;
        speex_bits_read_from(&(sstate->bits), (char*)input, input_size);
        speex_bytes = speex_decode_int(sstate->dec_state, &(sstate->bits), output); 
        return speex_bytes; 
}
