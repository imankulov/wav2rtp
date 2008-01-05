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
Its behaviour is confugured with environement variables and options for wav2rtp.

Environment variables are listed below:

    WAV2RTP         path to wav2rtp program (default: search in \$PATH)
    SIPP            path to sipp program (default: search in \$PATH)
    SCENARIO        path to basic scenario.xml file (default: ./scenario.xml)
    HOST            destination IP-address (default is 127.0.0.1)
    THISHOST        source (this host) IP-address (default is 127.0.0.1)
    CLIENT_USERNAME name of the service on the client-side (default: \"service\")

Example: 

SCENARIO=../doc/examples/scenario.xml  $0 -c speex -f test.wav -t test.pcap \\
    -o independent_losses:enabled=true -o independent_losses:loss_rate=0.05

"
# Search for wav2rtp
WAV2RTP=${WAV2RTP:-$(which wav2rtp)}
WAV2RTP=${WAV2RTP:-./wav2rtp}
if [[ ! -x $WAV2RTP ]]; then
    echo "$HELP_STRING"
    echo "wav2rtp executable is not found"
    exit 1
fi

# Search for sipp
SIPP=${SIPP:-$(which sipp)}
if [[ ! -x $SIPP ]]; then
    echo "$HELP_STRING"
    echo "sipp executable is not found"
    exit 1
fi

# Search for scenario
SCENARIO=${SCENARIO:-scenario.xml}
if [[ ! -f $SCENARIO ]]; then
    echo "$HELP_STRING"
    echo "scenario.xml is not found"
    exit 1
fi

# Define client name
CLIENT_USERNAME=${CLIENT_USERNAME:-service}

# Remote host
HOST=${HOST:-127.0.0.1}

# Local host
THISHOST=${THISHOST:-127.0.0.1}

sdp_data_packet_file=$(mktemp)
play_pcap_xml_file=$(mktemp)
scenario_file=$(mktemp)
trap "rm -f $sdp_data_packet_file $play_pcap_xml_file $scenario_file" 0 2 3 15


echo $WAV2RTP $@  -o "sipp:enabled=true"
$WAV2RTP $@ -o "sipp:enabled=true" | awk "
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

if [[ $(id -u) == 0 ]]; then
    $SIPP -m 1 -sf $scenario_file $HOST -s $CLIENT_USERNAME -i $THISHOST
else
    sudo $SIPP -m 1 -sf $scenario_file $HOST -s $CLIENT_USERNAME -i $THISHOST
fi
exit 0
