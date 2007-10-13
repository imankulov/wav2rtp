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
#include <stdio.h>
#include <strings.h>
#include <time.h>
#include <sys/time.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <ortp/rtp.h>

#include "contrib/in_cksum.h"
#include <pcap.h>

#include "pcap_output.h"
#include "options.h"
extern long timezone;



t_output * wr_pcap_init_output(t_output *  pout)
{
    wr_pcap_state * state = malloc(sizeof(wr_pcap_state));
    struct pcap_file_header fh;
    int len = 0;

    if (!state)
        return NULL;
    /* state->payload_type = codec.payload_type; */
    state->payload_type = 0;
    state->seq_number = 0x1ACA; /* TODO: This should be random to make known-plaintext attacks on encryption more difficult (see RFC 3550) */
    state->timestamp = 0; /*TODO: this should be random */
    state->first_packet = 1;
    gettimeofday(&(state->ts), NULL);
    state->fd = fopen(wr_options.output_filename, "w");
    if (!state->fd){
        free(state);
        return NULL;
    }
    
    /* Write a header */
    bzero(&fh, sizeof(fh));
    fh.magic = TCPDUMP_MAGIC;
    fh.version_major = PCAP_VERSION_MAJOR;
    fh.version_minor = PCAP_VERSION_MINOR;
    fh.thiszone = timezone;
    fh.sigfigs = 0;
    fh.snaplen = 0xFFFFFFFF;
    fh.linktype = DLT_EN10MB; 
    
    len = fwrite(&fh, sizeof(fh), 1, state->fd);
    if (len < 1){
        fclose(state->fd);
        free(state);
        return NULL;
    }
    pout->state = (void*) state;
    pout->write = &wr_pcap_write;
    pout->set_payload_type = wr_pcap_set_paload_type;
    pout->destroy = &wr_pcap_destroy_output;

    return pout;
}


void wr_pcap_set_paload_type(t_output * pout, int payload_type)
{
    wr_pcap_state * s = (wr_pcap_state * )(pout->state);
    s->payload_type = payload_type;
}


int wr_pcap_write(void * state, const char * buffer, int buffer_length, int buffer_length_in_ms, int buffer_delay_in_ms, int should_be_forget)
{
    wr_pcap_state * s = (wr_pcap_state * )state;

    struct pcap_pkthdr ph;        

    struct ether_header e_header = {
        ether_dhost: { 0x00, 0x14, 0x85, 0x1A, 0xE3, 0xA2 }, 
        ether_shost: { 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD }, 
        ether_type:  htons(0x0800),  /* Ethertype IP */
    };

    struct iphdr ip_header = {
        version: 4, 
        ihl: 5,
        tos: 0x00,
        tot_len: 0x0000,    /* This have to be changed */
        id: 0x0000,
        frag_off: 0x0000,             
        ttl: 64, 
        protocol: SOL_UDP,  /* UDP */
        check: 0x0000,      /* This have to be changed */
        saddr: IP(127,0,0,1),
        daddr: IP(127,0,0,2),
    };
    vec_t iphdr_vec[] = {   /* Just to count an IP checksum */
        {
            ptr: (guint8*) &ip_header, 
            len: sizeof(ip_header),
        },
    };
    struct udphdr udp_header = {
        source: htons(5061),
        dest: htons(5060),
        len: sizeof(buffer),
        check: 0x0000,      /* This may be changed, RFC 768 allows this field to be blank */
    };
    rtp_header_t rtp_header = {
        version: 2,
        padbit: 0, 
        extbit: 0, 
        cc: 0, 
        markbit: s->first_packet, 
        paytype: s->payload_type, 
        seq_number: htons(s->seq_number++),
        timestamp: htonl(s->timestamp),
        ssrc: 0x12011A0C, /* This should be random */
        /* csrc - this have to be null! */
    };
    s->timestamp += buffer_length_in_ms * 8;
    int rtp_header_length = sizeof(rtp_header) - sizeof(rtp_header.csrc);

    s->first_packet = 0;

    /* Update fields */
    ip_header.tot_len = sizeof(ip_header) + sizeof(udp_header) + rtp_header_length + buffer_length;
    ip_header.check = in_cksum(iphdr_vec, 1);
    
    bzero(&ph, sizeof(ph));
    memcpy(&(ph.ts), &(s->ts), sizeof(struct timeval));
    timeval_increment(&(s->ts), (buffer_length_in_ms + buffer_delay_in_ms)*1000);
    ph.caplen = sizeof(e_header) + sizeof(ip_header) + sizeof(udp_header) + rtp_header_length + buffer_length;
    ph.len = ph.caplen;
    
    if (!should_be_forget){
        fwrite(&ph, sizeof(ph), 1, s->fd);
        fwrite(&e_header, sizeof(e_header), 1, s->fd);
        fwrite(&ip_header, sizeof(ip_header), 1, s->fd);
        fwrite(&udp_header, sizeof(udp_header), 1, s->fd);
        fwrite(&rtp_header, rtp_header_length, 1, s->fd);
        return fwrite(buffer, buffer_length, 1, s->fd);
    }else{
        return 0;
    }
}


void wr_pcap_destroy_output(t_output* pout)
{
    wr_pcap_state * s = (wr_pcap_state * )(pout->state);
    fclose(s->fd); /* XXX: Here fclose() fail silently */
    free(s);
}


void timeval_increment(struct timeval * tv, int ms)
{
    tv->tv_usec += (ms % 1000000);
    tv->tv_sec += (ms / 1000000);
}
