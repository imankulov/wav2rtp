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
#include <string.h>
#include <sndfile.h> 

#include "options.h"
#include "error_types.h"

#include "rtpapi.h"
#include "wavfile_filter.h"
#include "dummy_filter.h"
#include "sort_filter.h"
#include "pcap_filter.h"
#include "wavfile_output_filter.h"
#include "independent_losses_filter.h"
#include "markov_losses_filter.h"
#include "uniform_delay_filter.h"
#include "gamma_delay_filter.h"
#include "log_filter.h"
#include "sipp_filter.h"

#include "speex_codec.h"
#include "dummy_codec.h"
#include "gsm_codec.h"
#include "g711u_codec.h"




int main(int argc, char ** argv)
{


    wr_rtp_filter_t wavfile_filter;

    wr_rtp_filter_t gamma_delay_filter;
    wr_rtp_filter_t uniform_delay_filter;

    wr_rtp_filter_t markov_losses_filter;
    wr_rtp_filter_t independent_losses_filter;

    wr_rtp_filter_t log_filter;
    wr_rtp_filter_t pcap_filter;
    wr_rtp_filter_t wavfile_output_filter;
    wr_rtp_filter_t sipp_filter;
    wr_rtp_filter_t sort_filter;

    wr_errorcode_t retval;

    /* parse options */
    retval = get_options(argc, argv, NULL, NULL);
    if (retval == WR_FATAL){
        fprintf(stderr, "FATAL ERROR: %s\n", wr_error);
        return retval;
    }else if (retval == WR_STOP){
        return 0;
    }

    /* initialize random number generator */
    {
        unsigned seed;
        struct timeval tv;
        gettimeofday(&tv, NULL);
        memcpy(&seed, &tv, sizeof(seed));
        srandom(seed);
        setall(random(), random());
    }

    wr_rtp_filter_create(&wavfile_filter, "input wav file filter", &wr_do_nothing_on_notify);

    wr_rtp_filter_create(&gamma_delay_filter, "gamma delay intermediate filter", &wr_gamma_delay_filter_notify);
    wr_rtp_filter_create(&uniform_delay_filter, "uniform_delay intermediate filter", &wr_uniform_delay_filter_notify);

    wr_rtp_filter_create(&markov_losses_filter, "markov_losses intermediate filter", &wr_markov_losses_filter_notify);
    wr_rtp_filter_create(&independent_losses_filter, "independent_losses intermediate filter", &wr_independent_losses_filter_notify);

    wr_rtp_filter_create(&log_filter, "log intermediate filter", &wr_log_filter_notify);
    wr_rtp_filter_create(&pcap_filter, "pcap output filter", &wr_pcap_filter_notify);
    wr_rtp_filter_create(&log_filter, "log filter", &wr_log_filter_notify);
    wr_rtp_filter_create(&sipp_filter, "sipp filter", &wr_sipp_filter_notify);
    wr_rtp_filter_create(&sort_filter, "sort filter", &wr_sort_filter_notify);
    wr_rtp_filter_create(&wavfile_output_filter, "sort filter", &wr_wavfile_output_filter_notify);
   
    wr_rtp_filter_append_observer(&wavfile_filter, &gamma_delay_filter);
    wr_rtp_filter_append_observer(&gamma_delay_filter, &uniform_delay_filter);
    wr_rtp_filter_append_observer(&uniform_delay_filter, &sort_filter);
    wr_rtp_filter_append_observer(&sort_filter, &markov_losses_filter);
    wr_rtp_filter_append_observer(&markov_losses_filter, &independent_losses_filter);

    wr_rtp_filter_append_observer(&independent_losses_filter, &log_filter);
    wr_rtp_filter_append_observer(&independent_losses_filter, &pcap_filter);
    wr_rtp_filter_append_observer(&independent_losses_filter, &wavfile_output_filter);
    wr_rtp_filter_append_observer(&independent_losses_filter, &sipp_filter);
   
    wr_wavfile_filter_start(&wavfile_filter);

    return 0; 
}
