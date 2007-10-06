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
#include "g711u_codec.h"
#include "contrib/g711.h"

t_codec * g711u_init_codec(t_codec * pcodec)
{
    wr_g711u_state * state = malloc(sizeof(wr_g711u_state));
    if (!state)
        return NULL;

    pcodec->state = (void*) state;
    pcodec->payload_type = 0;       // See RFC 1890
    pcodec->get_input_buffer_size = &wr_g711u_get_input_buffer_size;
    pcodec->get_output_buffer_size = &wr_g711u_get_output_buffer_size;
    pcodec->encode = &wr_g711u_encode;
    pcodec->destroy = &wr_g711u_destroy_codec;
    return pcodec;

}


void wr_g711u_destroy_codec(t_codec * pcodec)
{
    free(pcodec->state);
}

int wr_g711u_get_input_buffer_size(void * state)
{
    return 640;
}

int wr_g711u_get_output_buffer_size(void * state)
{
    return 640;
}

int wr_g711u_encode(void * state, const short * input, char * output) 
{
    int i = 0;
    for (i=0; i<640; i++){
        output[i] = (char)linear2ulaw((int)(input[i]));
    }    
    return 640;
} 
