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
#ifndef __WAV2RTP_H
#define __WAV2RTP_H 1

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


/** Codec abstraction object */
typedef struct __codec{
    
    /* Members */
    char * name;        /**< name of the codec as in SDP packet */
    char * description; /**< human-readable description of the codec */
    void * state;       /**< internal state of the codec, represented for its own struct type for each type of codec */
    int payload_type;   /**< default payload type for this codec */
    int sample_rate;    /**< sample-rate, used at least in SDP description of this codec */ 

    /* Methods */
    int (*get_input_buffer_size)(void *);
    int (*get_output_buffer_size)(void *);
    int (*encode)(void *, const short * , char *);
    void (*destroy)(struct __codec * );

} t_codec;


/** One way list of codecs */
typedef struct __codec_list {
    struct __codec_list * next;
    struct __codec * codec;
} t_codec_list;


/** Output abstraction object (for pcap, RTP output or smth. else) */
typedef struct __output {

    /* Members */    

    void * state; /**< this store internal state of the output object */


    /* Methods */
    int (* write)(void * state, const char * buffer, int buffer_length, int buffer_length_in_ms, int buffer_delay_in_ms, int should_be_forget); /**< write out data portion into its own datastream (file, network socket ...)  */
    void (*set_payload_type)(struct __output *, int ); /**< if codec payload type changes during data writing, this method have to be called */
    void (*destroy)(struct __output * );    /**< Pointer to function which destroy this  object */
} t_output;
#endif
