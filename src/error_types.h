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
#ifndef __ERROR_TYPES_H
#define __ERROR_TYPES_H

/** @defgroup error_types Error types 
 *  In this section are defined various errors types and functions to work with.
 *  @{
 */

/** DEPRECATED! */
#define CANNOT_RENDER_WAVFILE (1)
#define CODEC_NOT_RECOGNIZED (2)
#define CANNOT_INITIALIZE_CODEC (4)
#define CANNOT_INITIALIZE_OUTPUT (8)
#define OUTPUT_NOT_RECOGNIZED (16)
#define CANNOT_PARSE_CONFIG (32)

/**
 * Errorcodes which observer's "notify" method returns
 */
typedef enum __wr_errorcode {
    WR_OK,
    WR_WARN, 
    WR_FATAL, 
    WR_STOP,
} wr_errorcode_t;


/** String which contains error descritions when something goes wrong  */
char wr_error[2048];

/**
 * Set error description
 */
#define wr_set_error(x) bzero(wr_error,1024);strncpy(wr_error,x,1023);

/**
 * Print error description
 */
#define wr_print_error() printf("%s\n",wr_error);

#endif
