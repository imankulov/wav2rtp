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
#ifndef __G711A_CODEC_H
#define __G711A_CODEC_H

#include "codecapi.h"


/**
 * Common (encoder and decoder) G.711a state (don't use it, use instead #wr_g711a_encoder_state and
 * #wr_g711a_decoder_state)
 */
typedef struct {
    int buffer_size;
} wr_g711a_state;

/** G.711a encoder internal state*/
typedef wr_g711a_state wr_g711a_encoder_state;
/** G.711a decoder internal state*/
typedef wr_g711a_state wr_g711a_decoder_state;


wr_encoder_t * wr_g711a_encoder_init(wr_encoder_t * pcodec);
void wr_g711a_encoder_destroy(wr_encoder_t * pcodec);
int wr_g711a_encoder_get_input_buffer_size(void * state);
int wr_g711a_encoder_get_output_buffer_size(void * state);
int wr_g711a_encode(void * state, const short * input, char * output); 

wr_decoder_t * wr_g711a_decoder_init(wr_decoder_t * pcodec);
void wr_g711a_decoder_destroy(wr_decoder_t * pcodec);
int wr_g711a_decoder_get_input_buffer_size(void * state);
int wr_g711a_decoder_get_output_buffer_size(void * state);
int wr_g711a_decode(void * state, const char * input, size_t, short * output); 

#endif
