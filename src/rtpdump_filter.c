/*
 * $Id$
 *
 * Copyright (c) 2021, Orgad Shaneh
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
 *  3. Neither the name of the Orgad Shaneh nor the names of its contributors may
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
#ifdef _WIN32
#include "wincompat.h"
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

#include "rtpdump_filter.h"
#include "options.h"

typedef struct st_rtpdump_info
{
    uint16_t length;
    uint16_t plen;
    uint32_t rec_time; /**< milliseconds since start of recording */
} rtpdump_info_t;

/**
 * Structure to store internal state of the rtpdump output filter
 */
typedef struct __wr_rtpdump_filter_state {
    FILE * file;
    struct timeval start_timestamp;
} wr_rtpdump_filter_state_t;

static wr_errorcode_t __write_rtpdump_header(wr_rtpdump_filter_state_t *state)
{
    struct timeval start_timestamp;
    struct in_addr ip_src;
    uint16_t port, padding = 0;

    size_t len = fprintf(state->file, "#!rtpplay1.0 %s/%u\n",
                         iniparser_getstring(wr_options.output_options, "global:dst_ip", "127.0.0.2"),
                         iniparser_getnonnegativeint(wr_options.output_options, "global:dst_port", 8002));
    if (len < 1)
        return WR_FATAL;
    ip_src.s_addr = inet_addr(iniparser_getstring(wr_options.output_options, "global:src_ip", "127.0.0.1"));
    port = htons((short)iniparser_getnonnegativeint(wr_options.output_options, "global:src_port", 8001));
    gettimeofday(&start_timestamp, NULL);
    start_timestamp.tv_sec = htonl(start_timestamp.tv_sec);
    start_timestamp.tv_usec = htonl(start_timestamp.tv_usec);

    if (fwrite(&start_timestamp, sizeof(timestamp), 1, state->file) == 0)
        return WR_FATAL;
    if (fwrite(&ip_src.s_addr, 4, 1, state->file) == 0)
        return WR_FATAL;
    if (fwrite(&port, 2, 1, state->file) == 0)
        return WR_FATAL;
    if (fwrite(&padding, 2, 1, state->file) == 0)
        return WR_FATAL;
    return WR_OK;
}

wr_errorcode_t wr_rtpdump_filter_notify(wr_rtp_filter_t *filter, wr_event_type_t event, wr_rtp_packet_t *packet)
{
    switch (event)
    {
    case TRANSMISSION_START:
    {
        wr_rtpdump_filter_state_t *state = calloc(1, sizeof(wr_rtpdump_filter_state_t));
        state->file = fopen(wr_options.output_filename, "wb");
        if (!state->file)
        {
            free(state);
            filter->state = NULL;
            wr_set_error("Cannot open output file");
            return WR_FATAL;
        }
        if (__write_rtpdump_header(state) != WR_OK)
        {
            fclose(state->file);
            free(state);
            wr_set_error("Cannot write rtpdump header to the file");
            return WR_FATAL;
        }
        filter->state = (void *)state;
        return WR_OK;
    }

    case NEW_PACKET:
    {
        wr_errorcode_t retval;
        rtpdump_info_t rtpdump_packet;
        wr_rtp_header_t rtp_header;
        wr_rtpdump_filter_state_t *state = (wr_rtpdump_filter_state_t *)filter->state;
        rtpdump_packet.plen = sizeof(rtp_header);
        list_iterator_start(&(packet->data_frames));
        while (list_iterator_hasnext(&(packet->data_frames)))
        {
            wr_data_frame_t *current_data = list_iterator_next(&(packet->data_frames));
            rtpdump_packet.plen += current_data->size;
        }
        list_iterator_stop(&(packet->data_frames));
        wr_rtp_header_init(&rtp_header, packet);
        rtpdump_packet.length = htons(rtpdump_packet.plen + sizeof(rtpdump_packet));
        rtpdump_packet.plen = htons(rtpdump_packet.plen);
        rtpdump_packet.rec_time = htonl(packet->rtp_timestamp / 8);
        {
            int retval;
            if (!filter->state)
            {
                wr_set_error("internal state of the output filter was not initialized");
                return WR_FATAL;
            }
            retval = fwrite(&rtpdump_packet, sizeof(rtpdump_packet), 1, state->file);
            if (retval != 1)
            {
                wr_set_error("cannot write packet header");
                return WR_FATAL;
            }
            retval = fwrite(&rtp_header, sizeof(rtp_header), 1, state->file);
            if (retval != 1)
            {
                wr_set_error("cannot write RTP header");
                return WR_FATAL;
            }
            list_iterator_start(&(packet->data_frames));
            while (list_iterator_hasnext(&(packet->data_frames)))
            {
                wr_data_frame_t *current_data = list_iterator_next(&(packet->data_frames));
                fwrite(current_data->data, current_data->size, 1, state->file);
            }
            list_iterator_stop(&(packet->data_frames));
        }
    }
        return WR_OK;

    case TRANSMISSION_END:
        if (filter->state)
        {
            wr_rtpdump_filter_state_t *state = (wr_rtpdump_filter_state_t *)filter->state;
            fclose(state->file);
            free(filter->state);
            return WR_OK;
        }
        else
        {
            wr_set_error("cannot close file");
            return WR_FATAL;
        }
    }
}
