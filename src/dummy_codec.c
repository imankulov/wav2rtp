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


#include "dummy_codec.h"
#include "options.h"

wr_encoder_t * wr_dummy_encoder_init(wr_encoder_t * pcodec)
{
    
    dummy_state * state = malloc(sizeof(dummy_state));
    if (!state)
        return NULL;
    state->retval = (char)iniparser_getnonnegativeint(wr_options.codecs_options, "dummy:retval", 0);
    state->input_buffer_size = iniparser_getpositiveint(wr_options.codecs_options, "dummy:input_buffer_size", 640);
    state->output_buffer_size = iniparser_getpositiveint(wr_options.codecs_options, "dummy:output_buffer_size", 64);

    pcodec->state = (void*) state;
    pcodec->payload_type = iniparser_getnonnegativeint(wr_options.codecs_options, "dummy:payload_type", 0);
    return pcodec;

}
void wr_dummy_encoder_destroy(wr_encoder_t * pcodec)
{
    free(pcodec->state);
}

int wr_dummy_encoder_get_input_buffer_size(void * state)
{
    dummy_state * dstate = (dummy_state *)state;
    return dstate->input_buffer_size;
}
int wr_dummy_encoder_get_output_buffer_size(void * state)
{
    dummy_state * dstate = (dummy_state *)state;
    return dstate->output_buffer_size;
}
int wr_dummy_encode(void * state, const short * input, char * output) 
{
    int i;
    dummy_state * dstate = (dummy_state *)state;
    for (i = 0; i < dstate->output_buffer_size; i++){
        output[i] = dstate->retval;
    }
    return dstate->output_buffer_size;
} 
