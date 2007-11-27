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
#include <netinet/ether.h> /* ether_aton_r */
#include <arpa/inet.h>

#include "contrib/in_cksum.h"
#include "misc.h"
#include "pcap_output.h"
#include "options.h"
extern long timezone;



int wr_pcap_comparator(const void *a, const void *b)
{
    wr_pcap_packet_t * pa = (wr_pcap_packet_t *)a;
    wr_pcap_packet_t * pb = (wr_pcap_packet_t *)b;
    return timeval_comparator(pa->ph.ts, pb->ph.ts);
}



int wr_flush_pcap_buffer(list_t * buffer, struct timeval ts, FILE * fd, int flush_all)
{
    list_sort(buffer, -1);

    while(list_size(buffer) > 0){
        wr_pcap_packet_t * pcap_packet = (wr_pcap_packet_t*) list_get_at(buffer, 0);
        if(!pcap_packet){
            printf("ooops: work with list failed\n");
            break;
        }
        if (timeval_comparator(ts, pcap_packet->ph.ts) < 0 && !flush_all){
            break;
        }

        fwrite(&(pcap_packet->ph), sizeof(pcap_packet->ph), 1, fd);
        fwrite(&(pcap_packet->e_header), sizeof(pcap_packet->e_header), 1, fd);
        fwrite(&(pcap_packet->ip_header), sizeof(pcap_packet->ip_header), 1, fd);
        fwrite(&(pcap_packet->udp_header), sizeof(pcap_packet->udp_header), 1, fd);
        fwrite(&(pcap_packet->rtp_header), sizeof(pcap_packet->rtp_header) - sizeof(pcap_packet->rtp_header.csrc) , 1, fd);        
        list_iterator_start(pcap_packet->data_frames);
        while(list_iterator_hasnext(pcap_packet->data_frames)){
            wr_data_frame_t * data_frame = list_iterator_next(pcap_packet->data_frames);
            fwrite(data_frame->data, data_frame->size, 1, fd);
        }
        list_iterator_stop(pcap_packet->data_frames);
        list_destroy(pcap_packet->data_frames);
        list_delete_at(buffer, 0);
    }
}



wr_output_t * wr_pcap_init_output(wr_output_t *  pout)
{
    wr_pcap_state_t * state = malloc(sizeof(wr_pcap_state_t));
    struct pcap_file_header fh;
    struct ether_addr tmp_addr;

    int len = 0;
    if (!state)
        return NULL;
    state->seq_number = 0x1ACA; /* TODO: This should be random to make known-plaintext attacks on encryption more difficult (see RFC 3550) */
    state->timestamp = 0;       /*TODO: this should be random */

    bzero(state, sizeof(wr_pcap_state_t));
    list_init(&(state->packet_buffer));
    list_attributes_comparator(&(state->packet_buffer), &wr_pcap_comparator);

    bzero(&(state->e_header), sizeof(state->e_header));
    state->e_header.ether_type =  htons(0x0800);  /* ethertype IP */

    if (!ether_aton_r(iniparser_getstring(wr_options.output_options,  "global:dst_mac", "DE:AD:BE:EF:DE:AD"), &tmp_addr)){
        wr_set_error("Cannot parse destination ethernet address from config");
        return NULL;
    }
    if (!memcpy(state->e_header.ether_dhost, tmp_addr.ether_addr_octet, 6)){
        wr_set_error("Memory allocation error");
        return NULL;
    }
    if (!ether_aton_r(iniparser_getstring(wr_options.output_options,  "global:src_mac", "AA:BB:CC:DD:EE:FF"), &tmp_addr)){
        wr_set_error("Cannot parse source ethernet address from config");
        return NULL;
    }
    if (!memcpy(state->e_header.ether_shost, tmp_addr.ether_addr_octet, 6)){
        wr_set_error("Memory allocation error");
        return NULL;
    }

    bzero(&(state->ip_header), sizeof(state->ip_header));
    state->ip_header.version = 4;
    state->ip_header.ihl = 5;
    state->ip_header.tos = 0x00;
    state->ip_header.id = 0x0000;
    state->ip_header.frag_off = 0x0000;             
    state->ip_header.ttl = 64; 
    state->ip_header.protocol = SOL_UDP;  /* UDP */

    state->ip_header.saddr = inet_addr(iniparser_getstring(wr_options.output_options, "global:src_ip", "127.0.0.1"));    
    if (state->ip_header.saddr == -1){
        wr_set_error("Cannot parse source IP address from config");
        return NULL;
    }
    state->ip_header.daddr = inet_addr(iniparser_getstring(wr_options.output_options, "global:dst_ip", "127.0.0.2"));
    if (state->ip_header.daddr == -1){
        wr_set_error("Cannot parse destination IP address from config");
        return NULL;
    }

    bzero(&(state->udp_header), sizeof(state->udp_header));
    state->udp_header.source = htons((short)iniparser_getnonnegativeint(wr_options.output_options,  "global:src_port", 8001));
    state->udp_header.dest = htons((short)iniparser_getnonnegativeint(wr_options.output_options, "global:dst_port", 8002));
    state->udp_header.check = 0;
    bzero(&(state->rtp_header), sizeof(state->rtp_header));
    state->rtp_header.version = 2;
    state->rtp_header.padbit = 0;
    state->rtp_header.extbit = 0;
    state->rtp_header.cc = 0;
    state->rtp_header.markbit = 1;
    state->rtp_header.paytype = 0; 
    state->rtp_header.seq_number = 0;
    state->rtp_header.ssrc = 0x12011A0C, /* XXX: This should be random */
    gettimeofday(&(state->ts), NULL);
    state->fd = fopen(wr_options.output_filename, "w");
    if (!state->fd){
        free(state);
        wr_set_error("Cannot open output file");
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
    pout->destroy = &wr_pcap_destroy_output;

    return pout;
}

int i=0;

int wr_pcap_write(void * state, list_t * data_frames, wr_codec_t * codec, wr_network_emulator_t * netem)
{
    wr_packet_state_t packet_state;
    wr_pcap_state_t * s = (wr_pcap_state_t *)state;
    packet_state.data_frames = data_frames;


    wr_network_emulator_next(netem, &packet_state);
    if (packet_state.lost) {
        s->rtp_header.markbit = 0;
        s->seq_number++;
        if (!list_iterator_start(data_frames)){
            printf("ooops: error while starting iterator");
        }
        while(list_iterator_hasnext(data_frames)){
            wr_data_frame_t * current_data = list_iterator_next(data_frames);
            s->timestamp += current_data->length_in_ms * 8;  // TODO: 8 kHz hardcoded 
            timeval_increment(&(s->ts), (current_data->length_in_ms)*1000);
        }
        list_iterator_stop(data_frames);

    } else {
        struct pcap_pkthdr ph;
        wr_pcap_packet_t * pcap_packet;
        wr_pcap_state_t * s = (wr_pcap_state_t *)state;
        wr_data_frame_t *  current_data;
        vec_t iphdr_vec[] = { /* to count an IP checksum */
            {
                ptr: (guint8*) &(s->ip_header),
                len: sizeof(s->ip_header),
            },
        };
        bzero(&ph, sizeof(ph));
        s->ip_header.tot_len = sizeof(s->ip_header)  + 
                               sizeof(s->udp_header) +
                               sizeof(s->rtp_header) - sizeof(s->rtp_header.csrc);
        ph.caplen = sizeof(s->e_header) + s->ip_header.tot_len;
        s->udp_header.len = sizeof(s->udp_header) + sizeof(s->rtp_header) - sizeof(s->rtp_header.csrc);
        
        list_iterator_start(data_frames);
        while(list_iterator_hasnext(data_frames)){
            current_data = list_iterator_next(data_frames);
            s->ip_header.tot_len +=  current_data->size;
            s->udp_header.len += current_data->size;
            ph.caplen += current_data->size;
            if (ph.caplen >  0xFFFFFFFF){
                fprintf(stderr, "warning: packet is too large, bypassing\n");
                return 0;
            }
        }
        list_iterator_stop(data_frames);

        /* printf("PH len: %d, IP len: %d, UDP len: %d\n", ph.caplen, s->ip_header.tot_len, s->udp_header.len); */
        s->ip_header.tot_len = htons(s->ip_header.tot_len);
        s->udp_header.len = htons(s->udp_header.len);
        s->ip_header.check = 0;
        s->ip_header.check = in_cksum(iphdr_vec, 1);
        s->rtp_header.paytype = codec->payload_type;
        s->rtp_header.seq_number =  htons(s->seq_number++);
        s->rtp_header.timestamp =  htonl(s->timestamp);
        memcpy(&(ph.ts), &(s->ts), sizeof(struct timeval));
        timeval_increment(&(ph.ts), packet_state.delay);
        ph.len = ph.caplen;
        pcap_packet = malloc(sizeof(wr_pcap_packet_t));
        if (!pcap_packet){
            wr_set_error("Cannot allocate memory");
            return 0;
        }
        bzero(pcap_packet, sizeof(wr_pcap_packet_t));
        pcap_packet->data_frames = data_frames;
        memcpy(&(pcap_packet->ph), &ph, sizeof(ph));
        memcpy(&(pcap_packet->e_header), &(s->e_header), sizeof(s->e_header));
        memcpy(&(pcap_packet->ip_header), &(s->ip_header), sizeof(s->ip_header));
        memcpy(&(pcap_packet->udp_header), &(s->udp_header), sizeof(s->udp_header));
        memcpy(&(pcap_packet->rtp_header), &(s->rtp_header), sizeof(s->rtp_header) - sizeof(s->rtp_header.csrc));
        if (!list_iterator_start(data_frames)){
            printf("ooops: error while starting iterator");
        }
        while(list_iterator_hasnext(data_frames)){
            current_data = list_iterator_next(data_frames);
            s->timestamp += current_data->length_in_ms * 8;  // TODO: 8 kHz hardcoded 
            timeval_increment(&(s->ts), (current_data->length_in_ms)*1000);
        }
        list_iterator_stop(data_frames);
        list_append(&(s->packet_buffer), pcap_packet);
        /* wr_flush_pcap_buffer(&(s->packet_buffer), s->ts, s->fd, 0);  FIXME: ERROR IS SMTH. HERE */
        s->rtp_header.markbit = 0;
        return 1;
    }
}


void wr_pcap_destroy_output(wr_output_t* pout)
{
    wr_pcap_state_t * s = (wr_pcap_state_t * )(pout->state);
    wr_flush_pcap_buffer(&(s->packet_buffer), s->ts, s->fd, 1);
    fclose(s->fd); /* XXX: Here fclose() fail silently */
    free(s);
}


void timeval_increment(struct timeval * tv, int ms)
{
    tv->tv_usec += (ms % 1000000);    
    tv->tv_sec += (ms / 1000000);

    tv->tv_sec += tv->tv_usec / 1000000;
    tv->tv_usec = tv->tv_usec % 1000000;
}

int timeval_comparator(struct timeval a, struct timeval b)
{
    if (a.tv_sec  <  b.tv_sec  )
        return -1;
    if (a.tv_sec  >  b.tv_sec  )
        return 1;
    if (a.tv_usec  <  b.tv_usec  )
        return -1;
    if (a.tv_usec  >  b.tv_usec  )
        return 1;
    return 0;
}
