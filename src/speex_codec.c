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

#include <stdlib.h>



/**
 * Initialize object of type t_codec to use with Speex.
 * Memory for pcodec object have to be already allocated
 * @return the same pinter to t_codec or NULL in case when something goes wrong 
 */
t_codec * wr_speex_init_codec(t_codec * pcodec)
{

    speex_state * state = malloc(sizeof(speex_state));
    if (!state)
        return NULL;

    speex_bits_init(&(state->bits));
    state->enc_state = speex_encoder_init(&speex_nb_mode);

    pcodec->name = "speex";
    pcodec->description = "Speex narrowband mode codec";
    pcodec->sample_rate = 8000;

    pcodec->state = (void*)state;
    pcodec->payload_type = 96;
    pcodec->get_input_buffer_size = &wr_speex_get_input_buffer_size;
    pcodec->get_output_buffer_size = &wr_speex_get_output_buffer_size;
    pcodec->encode = &wr_speex_encode;
    pcodec->destroy = &wr_speex_destroy_codec;
    
    return pcodec;
}

/**
 * Destroy object of type t_codec for Speex
 */
void wr_speex_destroy_codec(t_codec * pcodec)
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
    speex_state * sstate = (speex_state *)state;
    return speex_bits_nbytes(&(sstate->bits));
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
