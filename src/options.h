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
#ifndef __OPTIONS_H
#define __OPTIONS_H

#ifdef HAVE_CONFIG_H
    #include <config.h>
#endif

#include "contrib/iniparser.h"
#include "contrib/simclist.h"
#include "error_types.h"

/** Options passed from command-line arguments */
typedef struct __wr_options {

    char * filename;    
    list_t * codec_list;
    char * output_filename;
    dictionary * codecs_options; /**< Dictionary of codec options given from /etc/wav2rtp/codecs.conf */
    dictionary * output_options; /**< Dictionary of output options given from /etc/wav2rtp/output.conf */

} wr_options_t;

/** global application options */
wr_options_t wr_options;

/**
 * Load options from config command line and configuration files
 * if codecs_conf or output_conf filename is NULL,
 * globally defined filenames are used
 */
wr_errorcode_t get_options(const int argc, char * const argv[],
        const char * codecs_conf, const char * output_conf);


/**
 * Return a list of codecs by the given string wich contains its names
 * Memory for entire list will be allocated in this function, you have to free it with "free_codec_list"
 * @string: a comma separated list of codecs
 * @pcodec_list: a pointer to the object list_t
 * @return: 0 if allocation goes sucessfully or !=0
 */
wr_errorcode_t get_codec_list(char *, list_t **);
wr_errorcode_t define_option(const char *, dictionary *);
void free_codec_list(list_t *);
void print_usage(void);
#endif
