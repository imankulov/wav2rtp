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
#include "gsm_codec.h"
#include "options.h"


t_codec * gsm_init_codec(t_codec * pcodec)
{
    wr_gsm_state * state = malloc(sizeof(wr_gsm_state));
    if (!state)
        return NULL;

    state->handle = gsm_create();
    if (!state->handle){
        free(state);
        return NULL;
    }

    pcodec->name = "GSM";
    pcodec->sample_rate = 8000;
    pcodec->description = "GSM 06.10 Full-Rate codec";
    pcodec->state = (void*) state;
    pcodec->payload_type = iniparser_getnonnegativeint(wr_options.codecs_options, "gsm:payload_type", 3);
    pcodec->get_input_buffer_size = &wr_gsm_get_input_buffer_size;
    pcodec->get_output_buffer_size = &wr_gsm_get_output_buffer_size;
    pcodec->encode = &wr_gsm_encode;
    pcodec->destroy = &wr_gsm_destroy_codec;
    return pcodec;

}


void wr_gsm_destroy_codec(t_codec * pcodec)
{
    wr_gsm_state * state =  (wr_gsm_state * )(pcodec->state);
    gsm_destroy(state->handle);
    free(pcodec->state);
}

int wr_gsm_get_input_buffer_size(void * state)
{
    return 160;
}

int wr_gsm_get_output_buffer_size(void * state)
{
    return 33;
}

int wr_gsm_encode(void * state, const short * input, char * output) 
{
    wr_gsm_state * sstate = (wr_gsm_state *)state;
    gsm_encode(sstate->handle, (gsm_signal*)input, (gsm_byte*)output);
    return 33;
} 
