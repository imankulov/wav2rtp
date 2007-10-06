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
#include <stdlib.h>
#include <stdio.h>
#include <pcap.h>

char errbuf[PCAP_ERRBUF_SIZE];


void dump_data(unsigned char * ptr, int len, const char title[])
{
    int i;

    printf("read data for  \"%s\" from [0x%x] (%d bytes)", title, (long)ptr, len);
    for (i=0; i<len; i++){
        if (i % 8 == 0)
            printf("  ");
        if (i % 16 == 0)
            printf("\n");
        printf("%02hhx ", ptr[i]);
    }
    printf("\n");
}


int main(int ac, char ** av)
{
    char file[] = "/home/roman/test.pcap";
    pcap_t *pcap;
    struct pcap_pkthdr *pkthdr = NULL;
    u_char *pktdata = NULL;

    pcap = pcap_open_offline(file, errbuf);
    if (!pcap){
        printf("Can't open PCAP file '%s'", file);
        return 1;
    }

    pkthdr = malloc(sizeof(*pkthdr));
    if (!pkthdr){
        printf("Can't allocate memory for pcap pkthdr");
        return 1;
    }
    pktdata = (u_char * )pcap_next(pcap, pkthdr);
    dump_data((unsigned char *)pkthdr, sizeof(*pkthdr), "Packet header");
    dump_data((unsigned char *)pktdata, pkthdr->len, "packet data");
/* 
    while ((pktdata = (u_char *) pcap_next (pcap, pkthdr)) != NULL){
        
    }
*/
}
