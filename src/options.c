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
#include "options.h"
#include "error_types.h"

#include <stdio.h>
#include <string.h>
#include <getopt.h>

void print_usage()
{
    printf( "\n"
            "This tool used to convert data from .wav to rtp packets which can be sent over network interface or stored into pcap file\n"
            "\n"
            "USAGE: wav2rtp [-H|--host] hostname:port [-f|--from-file] filename.wav [-c|--codec] codec [-o|--output] output_type [ other options ... ]\n"
            "  -H, --host               \tHostname and port to send rtp packets\n"
            "  -f, --from-file          \tFilename from which sound (speech) data will be readed\n"
            "  -c, --codec-list         \tComma separated list of codec types (without spaces), which will be used to encode .wav file. You can choose one of: speex, dummy, gsm, g711u\n"
            "  -o, --output             \tOutput type, you can send data to RTP stream (\"rtp\"), write it to raw file (\"raw\") or write it in the file in pcap (\"pcap\") format\n"
            "\n"
            "Other options.\n"
            "\n"
            "  -t, --to-file            \tOutput file. Used when output type is \"raw\" or \"pcap\"\n"
            "  -s, --print-sipp-scenario\tPrint two essential parts of SIPp scenario: SDP data packet and XML-string which force to play pcap audio.\n"
            "\n"
            "Codecs options (such as payload type and other) may be defined in the config file: " CONFDIR "/codec.conf\n"
            "\n"
            "EXAMPLE: \n"
            "  wav2rtp -H 192.168.0.1:8000 -f test.wav -c g711u,gsm -o pcap -t test.pcap\n"
            "\n"
            "This read file \"test.wav\" TWO TIMES, encode it first with G.711 then with GSM 06.10 and store data in pcap file \"test.pcap\"\n"
            "\n"
    );
}


int get_options(const int argc, char * const argv[])
{
    int c;
    extern char *optarg;
    extern int optind, optopt;
    int hlp = 0, hset = 0, cset = 0, fset=0, oset=0;
    char * chr; 
    static struct option long_options[] = {
        {"help", 0, NULL, 'h', }, 
        {"host", 1, NULL, 'H', }, 
        {"from-file", 1, NULL, 'f', }, 
        {"codec-list", 1, NULL, 'c', }, 
        {"output", 1, NULL, 'o', }, 
        {"to-file", 1, NULL, 't', }, 
        {"print-sipp-scenario", 0, NULL, 's', }, 
        {0, 0, 0, 0},
    };

    bzero(&wr_options, sizeof(t_options));    
    wr_options.codecs_options = iniparser_new(CONFDIR "/codecs.conf");
    if (!wr_options.codecs_options){
        wr_set_error("Cannot load or parse file with codec options: " CONFDIR "/codecs.conf");
        return CANNOT_PARSE_CONFIG; 
    }
    while(1){
        int option_index = 0;
        c = getopt_long(argc, argv, "hH:f:c:o:t:", long_options, &option_index);
        if (c == -1)
            break;
        switch(c){
            case 'h':
                hlp++;
                break;
            case 'H': 
                if ((chr=strchr(optarg, ':')) == NULL){
                    hlp++;
                }else{
                    chr[0] = '\0';
                    
                    wr_options.inet_addr = optarg;
                    wr_options.remote_port = atoi(&(chr[1]));
                    
                    if (wr_options.remote_port <= 0 || !strlen(wr_options.inet_addr)){
                        hlp++;
                    } else {
                        hset ++ ;
                    }
                }                
                break;
            case 'f':
                wr_options.filename = optarg;
                fset ++;
                break;
            case 'c':
                if (get_codec_list(optarg, &wr_options.codec_list) == 0)
                    cset ++;
                break;
            case 'o':
                wr_options.output_type = optarg;
                oset ++;
                break;
            case 't':
                wr_options.output_filename = optarg;
                break;
            case 's':
                wr_options.print_sipp_scenario = 1;
                break;
            default:
                hlp++;
                break;
       
        }

    }
    if (hlp || !(hset && cset && fset && oset)){
        print_usage();
        return 1;
    }
    return 0;
}


/**
 * Return an object t_codec_list
 * Memory for entire list will be allocated in this function, you have to free it with "free_codec_list"
 * @string: a comma separated list of codecs
 * @pplist: a pointer to the first object t_codec_list or pointer to NULL if nothing is found;
 * @return: 0 if allocation goes sucessfully or !=0
 */
int get_codec_list(char * string, t_codec_list ** pplist)
{

    t_codec_list * start_list = NULL;
    t_codec_list * current_list = NULL;
    char str[1024];
    char * token = NULL;
    char * lasts;

    if (strlen(string) > 1023){
        wr_set_error("size of list of codec is larger than 1023 symbols"); 
        return CANNOT_INITIALIZE_CODEC;        
    }       
    bzero(str, sizeof(str));
    strncpy(str, string, 1023);

    token = strtok_r(str, ",", &lasts);
    while(token){
        t_codec codec;
        t_codec * pcodec;
        t_codec_list * p_codec_list;

        /* Try to initialize codec*/
        if (strncmp(token, "speex", 6) == 0){
            if (!wr_speex_init_codec(&codec)){
                wr_set_error("Cannot initialize codec speex");
                goto cannot_initialize_codec;
            }
        }else if (strncmp(token, "gsm", 4) == 0){
            if (!gsm_init_codec(&codec)){
                wr_set_error("Cannot initialize codec GSM 06.10");
                goto cannot_initialize_codec;
            }
        }else if (strncmp(token, "g711u", 6) == 0){
            if (!g711u_init_codec(&codec)){
                wr_set_error("Cannot initialize codec G.711 (u-Law)");
                goto cannot_initialize_codec;
            }
        }else if (strncmp(token, "dummy", 6) == 0){
            if (!dummy_init_codec(&codec)){
                wr_set_error("Cannot initialize codec dummy");
                goto cannot_initialize_codec;
            }
        }else{
            printf("Codec not recognized");
            goto cannot_initialize_codec;
        }

        /* Create t_codec and t_codec_list elements */
        pcodec = (t_codec * )malloc(sizeof(t_codec));
        memcpy(pcodec, &codec, sizeof(t_codec));      
        p_codec_list = (t_codec_list * )malloc(sizeof(t_codec_list));
        bzero(p_codec_list, sizeof(t_codec_list));
        p_codec_list->next = NULL;
        p_codec_list->codec = pcodec;

        if(!start_list){
            start_list = p_codec_list;
            current_list = p_codec_list;         
        } else {
            current_list->next = p_codec_list;
            current_list = p_codec_list;
        }

        /* Next iteration */
        token = strtok_r(NULL, ",", &lasts);
    }
    (*pplist) = start_list;
    return 0;

cannot_initialize_codec:
    free_codec_list(start_list);
    return CANNOT_INITIALIZE_CODEC;

}

/** 
 * Free memory previously allocated with get_codec_list
 */
void free_codec_list(t_codec_list * list)
{
    if (!list)
        return;

    /* TODO: Not yet implemented */
    return; 
}



