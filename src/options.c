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
#include "options.h"
#include "rtpmap.h"

#ifdef HAVE_CONFIG_H
    #include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#endif

wr_options_t wr_options;

static void print_usage(const char * confdir)
{
    int i = 0;
    printf( "\n"
            "This tool used to convert data from .wav to rtp packets which can be sent over network interface or stored into pcap or rtpdump file\n"
            "\n"
            "USAGE: wav2rtp [-f|--from-file] file.wav [-t|--to-file] file.pcap [-c|--codec] codec [ other options ... ]\n"
            "  -v, --version            \tPrint to stdout version of this tool\n"
            "  -f, --from-file          \tFilename from which sound (speech) data will be readed\n"
            "  -t, --to-file            \tOutput file\n"
            "  -m, --format             \tOutput Format (pcap or rtpdump)\n"
            "  -c, --codec-list         \tComma separated list of codecs (without spaces), which will be used to encode .wav file\n"
            "  -o, --output-option      \tOutput option which redefine %s/output.conf. Recorded in form \"section:key=value\"\n"
            "  -O, --codecs-option       \tCodec option which redefine %s/codecs.conf. Recorded in the form \"section:key=value\"\n"
            "\n"
            "Codecs options (such as payload type and other) may be defined in the config file: %s/codecs.conf\n"
            "\n"
            "EXAMPLE: \n"
            "  wav2rtp -f test.wav -t test.pcap -c PCMU,GSM\n"
            "\n"
            "This reads file \"test.wav\" TWO TIMES, encodes it first with G.711 then with GSM 06.10 and stores data in pcap file \"test.pcap\"\n"
            "\n"
            "  wav2rtp -f test.wav -t test.pcap -c PCMU,GSM -o log:enabled=true\n"
            "\n"
            "This case is the same as previous plus logging to stdout will be available\n"
            "\n"
            "  wav2rtp -f test.wav -t test.dump -c PCMA -m rtpdump\n"
            "\n"
            "This reads file \"test.wav\", encodes it with G.711 and stores data in rtpdump file \"test.dump\"\n"
            "\n",
            confdir, confdir, confdir
    );
    printf("CODEC LIST:\n");
    while(encoder_map[i].name){
        printf("%s\t%s\n", encoder_map[i].name, encoder_map[i].description); i++;
    }
    printf("\n");
}

static wr_output_format __output_format_arg(const char *format)
{
    if (!strcasecmp(format, "pcap"))
        return WR_OUTPUT_PCAP;
    if (!strcasecmp(format, "rtpdump"))
        return WR_OUTPUT_RTPDUMP;
    return WR_OUTPUT_UNKNOWN;
}

wr_errorcode_t get_options(const int argc, char * const argv[],
        const char * codecs_conf, const char * output_conf)
{
    int c;
    extern char *optarg;
    extern int optind, optopt;
    int hlp = 0, cset = 0, fset=0, version=0;
    char * chr; 
    char * codec_list = NULL;
    const char * confdir = CONFDIR;
    const char * default_codecs = CONFDIR "/codecs.conf";
    const char * default_output = CONFDIR "/output.conf";
    int need_define_codec_list = 0;
    static struct option long_options[] = {
        {"help", 0, NULL, 'h', }, 
        {"version", 0, NULL, 'h', }, 
        {"from-file", 1, NULL, 'f', }, 
        {"codec-list", 1, NULL, 'c', }, 
        {"output-option", 1, NULL, 'o', },
        {"codecs-option", 1, NULL, 'O', },
        {"to-file", 1, NULL, 't', }, 
        {"format", 1, NULL, 'm', },
        {0, 0, 0, 0},
    };
#ifdef _WIN32
    char conf_directory[MAX_PATH];
    char bin_directory[MAX_PATH];
    char default_codecs_rel[MAX_PATH];
    char default_output_rel[MAX_PATH];
    char * last_slash;
    GetModuleFileNameA(NULL, conf_directory, sizeof(conf_directory));
    last_slash = strrchr(conf_directory, '\\');
    *last_slash = 0;
    strcpy(bin_directory, conf_directory);
    last_slash = strrchr(conf_directory, '\\');
    strcpy(last_slash + 1, "etc\\wav2rtp");
    if (access(conf_directory, 0) == 0)
        confdir = conf_directory;
    else
        confdir = bin_directory;
    snprintf(default_codecs_rel, MAX_PATH, "%s\\%s", confdir, "codecs.conf");
    snprintf(default_output_rel, MAX_PATH, "%s\\%s", confdir, "output.conf");
    default_codecs = default_codecs_rel;
    default_output = default_output_rel;
#endif

    memset(&wr_options, 0, sizeof(wr_options_t));

    wr_options.codecs_options = iniparser_new(
            codecs_conf ? codecs_conf : default_codecs);
    if (!wr_options.codecs_options){
        char message[1024];
        snprintf(message, sizeof(message), "Cannot load or parse file with codec options "
                "(default location is %s)", default_codecs);
        wr_set_error(message);
        return WR_FATAL;
    }

    wr_options.output_options = iniparser_new(
            output_conf ? output_conf : default_output);
    if (!wr_options.output_options){
        char message[1024];
        snprintf(message, sizeof(message), "Cannot load or parse file with output options "
                "(default location is %s)", default_output);
        wr_set_error(message);
        return WR_FATAL;
    }

    while(1){
        int option_index = 0;
        c = getopt_long(argc, argv, "hvf:c:o:O:t:m:", long_options, &option_index);
        if (c == -1)
            break;
        switch(c){
            case 'h':
                hlp++;
                break;
            case 'v':
                printf("%s\n", VERSION);
                version++;
                break;
            case 'f':
                wr_options.filename = optarg;
                fset ++;
                break;
            case 'c':
                /* Codecs have to be initialized after all */
                codec_list = strdup(optarg);
                need_define_codec_list = 1;
                break;
            case 't':
                wr_options.output_filename = optarg;
                break;
            case 'm':
                wr_options.output_format = __output_format_arg(optarg);
                break;
            case 'o':
                if (define_option(optarg, wr_options.output_options) != WR_OK)
                    hlp++;
                break;
            case 'O':
                if (define_option(optarg, wr_options.codecs_options) != WR_OK)
                    hlp++;
                break;
            default:
                hlp++;
                break;
       
        }

    }
    if (version){
        return WR_STOP;
    }
    if (need_define_codec_list){
        if (get_codec_list(codec_list, &wr_options.codec_list) == 0)
            cset ++;
        free(codec_list);
    }
    if (hlp || !(cset && fset)){
        print_usage(confdir);
        wr_set_error("not enough of input arguments");
        return WR_FATAL;
    }
    if (!wr_options.output_filename){
        wr_set_error("output filename is not set");
        return WR_FATAL;
    }
    if (strncmp(wr_options.output_filename, wr_options.filename, 1024) == 0){
        wr_set_error("output filename is the same that input filename");
        return WR_FATAL;
    }
    if (wr_options.output_format == WR_OUTPUT_UNKNOWN){
        wr_set_error("output format is unsupported. Supported formats are pcap and rtpdump");
        return WR_FATAL;
    }
    return WR_OK;
}


wr_errorcode_t get_codec_list(char * string, list_t ** pcodec_list)
{
    list_t * codec_list;   
    {
        codec_list = malloc(sizeof(list_t));
        list_init(codec_list);
    }
    {
        char str[1024];
        char * token = NULL;
        char * lasts;

        if (strlen(string) > 1023){
            wr_set_error("size of list of codec is larger than 1023 symbols"); 
            return WR_FATAL;        
        }       
        memset(str, 0, sizeof(str));
        strncpy(str, string, sizeof(str)-2);

        token = strtok_r(str, ",", &lasts);
        while(token){
            wr_encoder_t * pcodec = get_encoder_by_name(token);
            if (!pcodec || !(*pcodec->init)(pcodec) ){
                wr_set_error("Cannot found codec with given name");
                list_destroy(codec_list);
                free_codec_list(codec_list);
                return WR_FATAL;
            }
            list_append(codec_list, pcodec);
            /* Next iteration */
            token = strtok_r(NULL, ",", &lasts);
        }  
        (*pcodec_list) = codec_list;
        return WR_OK;
    }
}



void free_codec_list(list_t * list)
{
    free(list);
}


wr_errorcode_t define_option(const char * option, dictionary * d)
{
    char * opt = strdup(option);
    char * c = opt;
    while(*c){
        if (*c == '='){
            *c = '\0';
            c++;
            iniparser_setstr(d, opt, c);
            free(opt);
            return WR_OK;
        }
        c++;
    }
    free(opt);
    wr_set_error("Delimiter \"=\" not found in option");
    return WR_FATAL;
}
