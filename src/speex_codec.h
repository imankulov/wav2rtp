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
#ifndef __SPEEX_CODEC_H
#define __SPEEX_CODEC_H

#include <speex/speex.h>
#include "codecapi.h"

/** Speex encoder  object */
typedef struct {

    /* Speex state */
    SpeexBits bits;     /**< speex bit-packing struct  */
    void * enc_state;   /**< speex encoder state */

    /* Speex options */
    int quality;        /**< speex quality: 0<=q<=10 */
    int complexity;     /**< speex encoder complexity */
    int bitrate;        /**< bitrate */
    int abr_enabled;    /**< average bitrate */
    int vad_enabled;    /**< voice activity detection */
    int dtx_enabled;    /**< discontinious transmission */
    int vbr_enabled;    /**< boolean value, true if VBR (vraiable bit-rate) should be enabled */
    float vbr_quality;  /**< VBR quality: 0<=q<=10 */
    #ifdef SPEEX_SET_VBR_MAX_BITRATE
    int vbr_max_bitrate;/**< max bitrate with VBR enabled */
    #endif
    
} speex_encoder_state;


/** Speex encoder  object */
typedef struct {
    SpeexBits bits;     /**< speex bit-packing struct  */
    void * dec_state;   /**< speex decoder state */
} speex_decoder_state;


/**
 * Initialize object of type wr_encoder_t to use with Speex.
 * Memory for pcodec object have to be already allocated
 * @return the same pinter to wr_encoder_t or NULL in case when something goes wrong 
 */
wr_encoder_t *  wr_speex_encoder_init(wr_encoder_t * );


/**
 * Destroy object of type wr_encoder_t for Speex
 */
void wr_speex_encoder_destroy(wr_encoder_t *);


/**
 * Size of the input array for codec.
 *
 * This function returns the size of the input array which will be used on the next
 * encoding  iteration. Note that this value may be changed from one cycle step
 * to another. Note also that because every codec receive data in the format of
 * the array of shorts, real size of needed memory (passed to .alloc function)
 * will be: 
 *
 * speex_get_input_buffer_size(state) * sizeof(short)
 */
int wr_speex_encoder_get_input_buffer_size(void * state);


/**
 * Size of the output buffer 
 * This size is sufficient to write all data, encoded with speex_encode_array()
 * function. Real data size (which always is equal or less of this one) 
 * returned by speex_encode_array
 */
int wr_speex_encoder_get_output_buffer_size(void * state);


/**
 * Do encoding of the one data frame
 */
int wr_speex_encode(void * state, const short * input, char * output); 

wr_decoder_t *  wr_speex_decoder_init(wr_decoder_t * );
void wr_speex_decoder_destroy(wr_decoder_t *);
int wr_speex_decoder_get_input_buffer_size(void * state);
int wr_speex_decoder_get_output_buffer_size(void * state);
int wr_speex_decode(void * state, const char * input, size_t input_size, short * output); 

#endif
