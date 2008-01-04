bin_PROGRAMS = wav2rtp
#wav2rtp_SOURCES = wav2rtp.c wav2rtp.h options.c options.h \
#	speex_codec.c speex_codec.h dummy_codec.c dummy_codec.h  gsm_codec.c gsm_codec.h  g711u_codec.c g711u_codec.h \
#	pcap_output.c pcap_output.h \
#	misc.c misc.h \
#	network_emulator.c network_emulator.h \
#	contrib/g711.c contrib/g711.h contrib/in_cksum.c contrib/in_cksum.h error_types.h \
#	contrib/iniparser.c  contrib/iniparser.h \
#	contrib/simclist.c  contrib/simclist.h \
#	contrib/ranlib/com.c  contrib/ranlib/linpack.c  contrib/ranlib/ranlib.c  contrib/ranlib/ranlib.h
wav2rtp_SOURCES = wav2rtp.c wav2rtp.h options.c options.h \
	speex_codec.c speex_codec.h dummy_codec.c dummy_codec.h  gsm_codec.c gsm_codec.h  g711u_codec.c g711u_codec.h \
	contrib/g711.c contrib/g711.h contrib/in_cksum.c contrib/in_cksum.h error_types.h \
	contrib/iniparser.c  contrib/iniparser.h \
	contrib/simclist.c  contrib/simclist.h \
	contrib/ranlib/com.c  contrib/ranlib/linpack.c  contrib/ranlib/ranlib.c  contrib/ranlib/ranlib.h \
	rtpapi.c rtpapi.h \
	wavfile_filter.c wavfile_filter.h \
	pcap_filter.c pcap_filter.h \
	dummy_filter.c dummy_filter.h \
	independent_losses_filter.c independent_losses_filter.h \
	markov_losses_filter.c markov_losses_filter.h \
	uniform_delay_filter.c uniform_delay_filter.h \
	gamma_delay_filter.c gamma_delay_filter.h \
	log_filter.c log_filter.h \
	sipp_filter.c sipp_filter.h \
	sort_filter.c sort_filter.h \
	misc.c misc.h

bin_SCRIPTS = wav2rtp-testcall.sh
EXTRA_DIST = $(bin_SCRIPTS)
wav2rtp_LDADD = @LIBOBJS@
