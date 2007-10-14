bin_PROGRAMS = wav2rtp
wav2rtp_SOURCES = wav2rtp.c wav2rtp.h options.c options.h \
	speex_codec.c speex_codec.h dummy_codec.c dummy_codec.h  gsm_codec.c gsm_codec.h  g711u_codec.c g711u_codec.h \
	rtp_output.c rtp_output.h  raw_output.c raw_output.h  pcap_output.c pcap_output.h \
	contrib/g711.c contrib/g711.h contrib/in_cksum.c contrib/in_cksum.h error_types.h \
	contrib/iniparser.c  contrib/iniparser.h
bin_SCRIPTS = wav2rtp-testcall.sh
wav2rtp_LDADD = @LIBOBJS@
