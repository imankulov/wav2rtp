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
#include "raw_output.h"

#include "options.h"
#include <stdio.h>



t_output * wr_raw_init_output(t_output *  pout)
{
    wr_raw_state * state = malloc(sizeof(wr_raw_state));
    if (!state)
        return NULL;
    state->fd = fopen(wr_options.output_filename, "w");
    if (!state->fd){
        free(state);
        return NULL;
    }
    pout->state = (void*) state;
    pout->write = &wr_raw_write;
    pout->set_payload_type = &wr_raw_set_payload_type;
    pout->destroy = &wr_raw_destroy_output;
    return pout;
}

void wr_raw_set_payload_type(t_output  * pout, int payload_type)
{
    return; /* Do nothing */
}

/* XXX: Two last parameters never used */
int wr_raw_write(void * state, const char * buffer, int buffer_length, int buffer_length_in_ms, int buffer_delay_in_ms, int should_be_forget)
{
    wr_raw_state * s = (wr_raw_state * )state;    
    return fwrite(buffer, buffer_length, 1, s->fd);
}


void wr_raw_destroy_output(t_output* pout)
{
    wr_raw_state * s = (wr_raw_state * )(pout->state);
    fclose(s->fd); /* XXX: Here fclose() fail silently */
    free(s);
}
