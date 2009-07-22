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
#ifndef PCAP_FILTER
#define PCAP_FILTER
#include "rtpapi.h"
/** @defgroup pcap_filter pcap output filter method definitions
 * This is the most essential output filter - pcap filter which convert rtp packets to pcap format and store them into
 * file
 *  @{
 */


#define TCPDUMP_MAGIC (0xa1b2c3d4)
/** 
 * Structure to store internal state of the pcap output filter
 */
typedef struct __wr_pcap_filter_state {
    FILE * file; 
} wr_pcap_filter_state_t;

/**
 * Store data into file 
 * This method is invoked when filter is notified
 */
wr_errorcode_t wr_pcap_filter_notify(wr_rtp_filter_t * filter, wr_event_type_t event, wr_rtp_packet_t * packet);


/**
 * Pcap timeval
 */
struct wr_pcap_timeval {
    int32_t tv_sec;       /*< seconds */
    int32_t tv_usec;      /*< microseconds */
};


/**
 * Pcap packet header.
 */
struct wr_pcap_pkthdr {
    struct wr_pcap_timeval ts; 	/*< time stamp */
    uint32_t caplen;     	/*< length of portion present */
    uint32_t len;        	/*< length this packet (off wire) */
};

#define wr_pcap_timeval_copy(pcap_tv, tv) \
	{ (pcap_tv)->tv_sec=(tv)->tv_sec; (pcap_tv)->tv_usec=(tv)->tv_usec; }
/** @} */

#endif
