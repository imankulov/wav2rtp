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
#include "rtp_output.h"
#include "options.h"

t_output * wr_rtp_init_output(t_output * pout)
{

    wr_rtp_state * state = malloc(sizeof(wr_rtp_state)); 
    if (!state)
        return NULL;
        
    // Session initialization
	ortp_init();
	ortp_scheduler_init();
	ortp_set_log_level_mask(ORTP_ERROR);
   	state->session=rtp_session_new(RTP_SESSION_SENDONLY);	
    state->user_ts = 0; /* TODO: This should be random */

	rtp_session_set_scheduling_mode(state->session,1);
	rtp_session_set_blocking_mode(state->session,1);
	rtp_session_set_connected_mode(state->session,TRUE);
	rtp_session_set_remote_addr(state->session, wr_options.inet_addr, wr_options.remote_port);
	rtp_session_set_send_payload_type(state->session, 0);
    pout->state = (void*) state;
    pout->write = &wr_rtp_write;
    pout->set_payload_type = &wr_rtp_set_paload_type;
    pout->destroy = &wr_rtp_destroy_output;
    return pout;
}

void wr_rtp_set_paload_type(t_output * pout, int payload_type)
{
    wr_rtp_state * s = (wr_rtp_state * )(pout->state);
	rtp_session_set_send_payload_type(s->session, payload_type);
}


/* XXX: Two last parameters never used */
int wr_rtp_write(void * state, const char * buffer, int buffer_length, int buffer_length_in_ms, int buffer_delay_in_ms, int should_be_forget)
{
    wr_rtp_state * sstate = (wr_rtp_state * )(state);

    rtp_session_send_with_ts(sstate->session, (uint8_t*)buffer, buffer_length, sstate->user_ts);
    rtp_session_make_time_distorsion(sstate->session, buffer_length_in_ms);
    sstate->user_ts += 160;
    printf(".");
    fflush(stdout);
    return buffer_length;
}

void wr_rtp_destroy_output(t_output * pout)
{
    wr_rtp_state * s = (wr_rtp_state *)(pout->state);
    rtp_session_destroy(s->session);
    free(s);
}
