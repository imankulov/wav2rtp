bin_PROGRAMS = wav2rtp
common_sources = rtpmap.c rtpmap.h options.c options.h codecapi.h \
	speex_codec.c speex_codec.h dummy_codec.c dummy_codec.h  gsm_codec.c gsm_codec.h  g711u_codec.c g711u_codec.h \
	contrib/g711.c contrib/g711.h contrib/in_cksum.c contrib/in_cksum.h error_types.h \
	contrib/iniparser.c  contrib/iniparser.h \
	contrib/simclist.c  contrib/simclist.h \
	contrib/ranlib/com.c  contrib/ranlib/linpack.c  contrib/ranlib/ranlib.c  contrib/ranlib/ranlib.h \
	rtpapi.c rtpapi.h \
	wavfile_filter.c wavfile_filter.h \
	pcap_filter.c pcap_filter.h \
	wavfile_output_filter.c wavfile_output_filter.h \
	dummy_filter.c dummy_filter.h \
	independent_losses_filter.c independent_losses_filter.h \
	markov_losses_filter.c markov_losses_filter.h \
	uniform_delay_filter.c uniform_delay_filter.h \
	gamma_delay_filter.c gamma_delay_filter.h \
	log_filter.c log_filter.h \
	sipp_filter.c sipp_filter.h \
	sort_filter.c sort_filter.h \
	g711a_codec.c g711a_codec.h \
	misc.c misc.h

wav2rtp_SOURCES = wav2rtp.c $(common_sources)

bin_SCRIPTS = wav2rtp-testcall.sh
EXTRA_DIST = $(bin_SCRIPTS)
wav2rtp_LDADD = @LIBOBJS@
TESTS = pcap_test
check_PROGRAMS = $(TESTS)
EXTRA_DIST += $(TESTS) 
CLEANFILES = testdata/empty_test.pcap testdata/one_packet_test.pcap
pcap_test_SOURCES = pcap_test.c $(common_sources)
