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
#ifndef PCAP_OUTPUT_H
#define PCAP_OUTPUT_H

#include "rtpapi.h"
#include "error_types.h"
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ether.h> /* ether_aton_r */
#include <arpa/inet.h>
#include <ortp/rtp.h>
#include <pcap.h>

#define TCPDUMP_MAGIC (0xa1b2c3d4)
/* #define DLT_EN10MB    (1) */

/**
 * Structure which describe internal state of the pcap-output object
 */
typedef struct __wr_pcap_state {
    FILE * fd;                    /**< output file descriptor */
    int seq_number;
    int timestamp;
    struct timeval ts;
    list_t packet_buffer;         /**< temporary storage for wr_pcap_packet before writing it to the file */

    struct ether_header e_header; /**< this store common information about ethernet header of every packet */
    struct iphdr ip_header;       /**< this store common information about IP header of every packet */
    struct udphdr udp_header;     /**< this store common information about UDP header of every packet */
    rtp_header_t rtp_header;      /**< this store common information about RTP header of every packet */

} wr_pcap_state_t;

/**
 * Storage for the ready-to-write object.
 * This object sis stored into temporary buffer to correctly treat delay variances
 * (we need to be sure that early packets written into .pcap file first)
 */
typedef struct __wr_pcap_packet {
   
    struct pcap_pkthdr ph;        /**< pcap packet header */ 
    struct ether_header e_header; /**< this store common information about ethernet header of every packet */
    struct iphdr ip_header;       /**< this store common information about IP header of every packet */
    struct udphdr udp_header;     /**< this store common information about UDP header of every packet */
    rtp_header_t rtp_header;      /**< this store common information about RTP header of every packet */
    list_t * data_frames;           /**< list of data frames */ 
     
} wr_pcap_packet_t;


/**
 * Initialize pcap output object
 * @param pout pointer to the object to initialize
 * @return the same pointer if all OK or NULL in case of some error
 */
wr_output_t * wr_pcap_init_output(wr_output_t *);

int wr_pcap_write(void*, list_t*, wr_codec_t *, wr_network_emulator_t *);

void wr_pcap_destroy_output(wr_output_t*);

/**
 * Compare pcap data packets by arrival date
 */
int wr_pcap_comparator(const void *a, const void *b);

/**
 * Write out all passed data from buffer in file
 */
int wr_flush_pcap_buffer(list_t * buffer, struct timeval ts, FILE * fd, int flush_all);

/**
 *  Common function which increment timeval
 *  @param i period to increment in microseconds
 */
void timeval_increment(struct timeval * tv, int i);


/**
 * Compare two timeval structures.
 * @return 0 if a = b, -1 if a<b , 1 if a>b
 */
int timeval_comparator(struct timeval a, struct timeval b);
#endif 
