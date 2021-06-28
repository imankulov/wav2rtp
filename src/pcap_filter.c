/*
 * $Id$
 * 
 * Copyright (c) 20010, R.Imankulov, Yu Jiang
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
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#if defined (__linux__)
#include <netinet/ether.h> /* ether_aton_r */
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#endif
#include <arpa/inet.h>
#include <pcap.h>

#include "contrib/in_cksum.h"
#include "pcap_filter.h"
#include "options.h"

#ifdef __linux__
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1
#endif
#endif

wr_errorcode_t __init_ether_header(struct ether_header * e)
{
    struct ether_addr *tmp_addr;

    memset(e, 0, sizeof(*e));
    e->ether_type =  htons(0x0800);  /* ethertype IP */

    if (!(tmp_addr = ether_aton(iniparser_getstring(wr_options.output_options,  "global:dst_mac", "DE:AD:BE:EF:DE:AD")))){
        wr_set_error("Cannot parse destination ethernet address from config");
        return WR_FATAL;
    }
    memcpy(e->ether_dhost, tmp_addr->ether_addr_octet, 6);

    if (!(tmp_addr = ether_aton(iniparser_getstring(wr_options.output_options,  "global:src_mac", "AA:BB:CC:DD:EE:FF")))){
        wr_set_error("Cannot parse source ethernet address from config");
        return WR_FATAL;
    }
    memcpy(e->ether_shost, tmp_addr->ether_addr_octet, 6);
    return WR_OK;
}


wr_errorcode_t __init_ip_header(struct ip * ip_header)
{
    memset(ip_header, 0, sizeof(*ip_header));
    ip_header->ip_v = 4;
    ip_header->ip_hl = 5;
    ip_header->ip_tos = 0x00;
    ip_header->ip_id = 0x0000;
    ip_header->ip_off = 0x0000;             
    ip_header->ip_ttl = 64; 
    ip_header->ip_p = IPPROTO_UDP;  /* UDP */

    ip_header->ip_src.s_addr = inet_addr(iniparser_getstring(wr_options.output_options, "global:src_ip", "127.0.0.1"));    
    if (ip_header->ip_src.s_addr == -1){
        wr_set_error("Cannot parse source IP address from config");
        return WR_FATAL;
    }
    ip_header->ip_dst.s_addr = inet_addr(iniparser_getstring(wr_options.output_options, "global:dst_ip", "127.0.0.2"));
    if (ip_header->ip_dst.s_addr == -1){
        wr_set_error("Cannot parse destination IP address from config");
        return WR_FATAL;
    }
    return WR_OK;
}


wr_errorcode_t __init_udp_header(struct udphdr * udp_header)
{
    memset(udp_header, 0, sizeof(*udp_header));
#ifdef __linux__
    udp_header->source = htons((short)iniparser_getnonnegativeint(wr_options.output_options,  "global:src_port", 8001));
    udp_header->dest = htons((short)iniparser_getnonnegativeint(wr_options.output_options, "global:dst_port", 8002));
    udp_header->check = 0;
#else
    udp_header->uh_sport = htons((short)iniparser_getnonnegativeint(wr_options.output_options,  "global:src_port", 8001));
    udp_header->uh_dport = htons((short)iniparser_getnonnegativeint(wr_options.output_options, "global:dst_port", 8002));
    udp_header->uh_sum = 0;
#endif
    return WR_OK;
}

wr_errorcode_t __init_rtp_header(wr_rtp_header_t * rtp_header)
{
    memset(rtp_header, 0, sizeof(*rtp_header));
    rtp_header->version = 2;
    rtp_header->padbit = 0;
    rtp_header->extbit = 0;
    rtp_header->cc = 0;
    rtp_header->ssrc = 0x12011A0C; /* XXX: This should be random */
    return WR_OK;
}

wr_errorcode_t wr_pcap_filter_notify(wr_rtp_filter_t * filter, wr_event_type_t event, wr_rtp_packet_t * packet)
{

    switch(event) {
        case TRANSMISSION_START:
            {
                struct pcap_file_header fh;
                size_t len;
                wr_pcap_filter_state_t * state = calloc(1, sizeof(wr_pcap_filter_state_t)); 
                state->file = fopen(wr_options.output_filename, "w");
                if (!state->file){
                    free(state);
                    filter->state = NULL;
                    wr_set_error("Cannot open output file");
                    return WR_FATAL;
                }
                /* Write a header */
                memset(&fh, 0, sizeof(fh));
                fh.magic = TCPDUMP_MAGIC;
                fh.version_major = PCAP_VERSION_MAJOR;
                fh.version_minor = PCAP_VERSION_MINOR;
                fh.thiszone = timezone;
                fh.sigfigs = 0;
                fh.snaplen = 0x0000FFFF;
                fh.linktype = DLT_EN10MB; 
                len = fwrite(&fh, sizeof(fh), 1, state->file);
                if (len < 1){
                    fclose(state->file);
                    free(state);
                    wr_set_error("Cannot write pcap header to the file");
                    return WR_FATAL;
                }
                filter->state = (void*)state;
                return WR_OK;
            }
            break;

        case NEW_PACKET:
            {
                wr_errorcode_t retval;
                struct wr_pcap_pkthdr ph;
                struct ether_header e_header;
                struct ip ip_header;
                struct udphdr udp_header;
                wr_rtp_header_t rtp_header;
                vec_t iphdr_vec[] = { /* to count an IP checksum */
                    {
                        .ptr = (unsigned char *)&ip_header,
                        .len = sizeof(ip_header),
                    },
                };
                if ((retval=__init_ether_header(&e_header)) != WR_OK){
                    return retval;
                }
                if ((retval=__init_ip_header(&ip_header)) != WR_OK){
                    return retval;
                }
                if ((retval=__init_udp_header(&udp_header)) != WR_OK){
                    return retval;
                }
                if ((retval=__init_rtp_header(&rtp_header)) != WR_OK){
                    return retval;
                }
                rtp_header.markbit = packet->markbit;
                rtp_header.paytype = packet->payload_type; 
                rtp_header.seq_number = htons(packet->sequence_number);
                rtp_header.timestamp = htonl(packet->rtp_timestamp);

                ip_header.ip_len = sizeof(ip_header)  + 
                                   sizeof(udp_header) +                                   
                                   sizeof(rtp_header) - sizeof(rtp_header.csrc);
                ph.caplen = sizeof(e_header) + ip_header.ip_len;
#ifdef __linux__
                udp_header.len = sizeof(udp_header) + sizeof(rtp_header) - sizeof(rtp_header.csrc);
#else
                udp_header.uh_ulen = sizeof(udp_header) + sizeof(rtp_header) - sizeof(rtp_header.csrc);
#endif
                list_iterator_start(&(packet->data_frames));
                while(list_iterator_hasnext(&(packet->data_frames))){
                    wr_data_frame_t * current_data = list_iterator_next(&(packet->data_frames));
                    ip_header.ip_len += current_data->size;
#ifdef __linux__
                    udp_header.len += current_data->size;
#else
                    udp_header.uh_ulen += current_data->size;
#endif
                    ph.caplen += current_data->size;
                }
                list_iterator_stop(&(packet->data_frames));

                ip_header.ip_len = htons(ip_header.ip_len);
#ifdef __linux__
                udp_header.len = htons(udp_header.len);
#else
                udp_header.uh_ulen = htons(udp_header.uh_ulen);
#endif

                ip_header.ip_sum = 0;
                ip_header.ip_sum = in_cksum(iphdr_vec, 1);
                wr_pcap_timeval_copy(&(ph.ts), &(packet->lowlevel_timestamp));
                ph.len = ph.caplen;

                {
                    int retval;
                    wr_pcap_filter_state_t * state = (wr_pcap_filter_state_t * ) filter->state;
                    if (!filter->state){
                        wr_set_error("internal state of the output filter was not initialized");
                        return WR_FATAL;
                    }
                    retval = fwrite(&ph, sizeof(ph), 1, state->file);
                    retval += fwrite(&e_header, sizeof(e_header), 1, state->file);
                    retval += fwrite(&ip_header, sizeof(ip_header), 1, state->file);
                    retval += fwrite(&udp_header, sizeof(udp_header), 1, state->file);
                    retval += fwrite(&rtp_header, sizeof(rtp_header) - sizeof(rtp_header.csrc), 1, state->file);
                    if (retval != 5){
                        wr_set_error("cannot write packet header");
                        return WR_FATAL;
                    }
                    list_iterator_start(&(packet->data_frames));
                    while (list_iterator_hasnext(&(packet->data_frames))){
                        wr_data_frame_t * current_data = list_iterator_next(&(packet->data_frames));
                        fwrite(current_data->data, current_data->size, 1, state->file);
                    }
                    list_iterator_stop(&(packet->data_frames));
                }
            }
            return WR_OK;

        case TRANSMISSION_END:
            if (filter->state){
                wr_pcap_filter_state_t * state = (wr_pcap_filter_state_t * ) filter->state;
                fclose(state->file);
                free(filter->state);
                return WR_OK;
            } else {
                wr_set_error("cannot close file"); 
                return WR_FATAL;
            }
    }
}
