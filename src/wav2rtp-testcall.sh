#!/bin/bash

# $Id$
# 
# Copyright (c) 2007, R.Imankulov
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#  1. Redistributions of source code must retain the above copyright notice,
#  this list of conditions and the following disclaimer.
#
#  2. Redistributions in binary form must reproduce the above copyright notice,
#  this list of conditions and the following disclaimer in the documentation
#  and/or other materials provided with the distribution.
#
#  3. Neither the name of the R.Imankulov nor the names of its contributors may
#  be used to endorse or promote products derived from this software without
#  specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#


HELP_STRING="
This script runs wav2rtp and sipp one after another. 
Its behaviour is confugured with environement variables
listed below:

    WAV2RTP     path to wav2rtp program (default: search in \$PATH)
    SIPP        path to sipp program (default: search in \$PATH)
    SCENARIO    path to basic scenario.xml file (default: ./scenario.xml)
    WAV         path to input .wav file (default: ./test.wav)
    PCAP        path to output .pcap file (default: \$WAV with replace .wav to .pcap)
    HOST        hostname and port (in format ip.ad.dre.ss:port ) where to send RTP data
                (currently port is not used but should be pointed, default: 127.0.0.1:8000).
    CODEC_LIST  comma-separated list of codecs which will be used to encode .wav file to RTP
                (is not set by default thus must be defined explicitly)

Example: 

SIPP=~/svn-checkouts/sipp/trunk/sipp  CODEC_LIST='g711u,gsm,gsm' $0

"
# Search for wav2rtp
if [[ ! $WAV2RTP ]]; then 
    WAV2RTP=$(which wav2rtp)
fi

if [[ ! $WAV2RTP ]]; then 
    WAV2RTP=../wav2rtp
fi

if [[ ! -x $WAV2RTP ]]; then
    echo "$HELP_STRING"
    echo "wav2rtp executable is not found"
    exit 1
fi

# Search for sipp
if [[ ! $SIPP ]]; then 
    SIPP=$(which sipp)
fi

if [[ ! -x $SIPP ]]; then
    echo "$HELP_STRING"
    echo "sipp executable is not found"
    exit 1
fi

# Search for scenario
if [[ ! $SCENARIO ]]; then 
    SCENARIO=scenario.xml
fi

if [[ ! -f $SCENARIO ]]; then
    echo "$HELP_STRING"
    echo "scenario.xml is not found"
    exit 1
fi

# Search for wav file
if [[ ! $WAV ]]; then 
    WAV=test.wav
fi

if [[ ! -f $WAV ]]; then
    echo "$HELP_STRING"
    echo "input wavfile is not found"
    exit 1
fi

# Ouput pcap file definition
if [[ ! $PCAP ]]; then 
    PCAP=$(echo "$WAV" | sed -r  's/\.[^.]+$/.pcap/g')
fi

# Remote host
if [[ ! $HOST ]]; then
    HOST=127.0.0.1:8000
fi

# Codec list

if [[ ! $CODEC_LIST ]]; then
    echo "$HELP_STRING"
    echo "Define codec list!"
    exit 1
fi

sdp_data_packet_file=$(mktemp)
play_pcap_xml_file=$(mktemp)
scenario_file=$(mktemp)
trap "rm -f $sdp_data_packet_file $play_pcap_xml_file $scenario_file" 0 2 3 15

$WAV2RTP --host $HOST --from-file $WAV --codec-list $CODEC_LIST --output pcap --to-file $PCAP --print-sipp-scenario | awk "
/\*\*\*sdp_data_packet\*\*\*/{
    sdp_data_packet_found=1;
    play_pcap_xml_found=0;
    next;
}
/\*\*\*play_pcap_xml\*\*\*/{
    play_pcap_xml_found=1;
    sdp_data_packet_found=0;
    next;
}
{
    if (play_pcap_xml_found){
        print >> \"$play_pcap_xml_file\";
    }
    if (sdp_data_packet_found){
        print >> \"$sdp_data_packet_file\";
    }
}
"
cat $SCENARIO | sed -r "/\\*\\*\\*sdp_data_packet\\*\\*\\*/r $sdp_data_packet_file" \
              | sed -r "/\\*\\*\\*play_pcap_xml\\*\\*\\*/r $play_pcap_xml_file" \
              | sed -r "/\\*\\*\\*.*\\*\\*\\*/d" > $scenario_file

host_without_port=$(echo $HOST | awk -F: '{print $1}')
if [[ $(id -u) == 0 ]]; then
    $SIPP -m 1 -sf $scenario_file $host_without_port
else
    sudo $SIPP -m 1 -sf $scenario_file $host_without_port
fi
