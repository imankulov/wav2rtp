#include <stdio.h>
#include <string.h>
#include "pcap_filter.h"
#define CHECK(f) { err=(f); if (err) {wr_print_error(); return 1;}  }


wr_errorcode_t files_are_equals(const char *filename1, const char *filename2,
        const size_t bufsize)
{
    FILE *fd1, *fd2;
    char buf1[bufsize];
    char buf2[bufsize];
    size_t sz1, sz2;
    fd1 = fopen(filename1, "r");
    if (!fd1){
        wr_set_error("first file is not found");
        return WR_FATAL;
    }
    fd2 = fopen(filename2, "r");
    if (!fd2){
        wr_set_error("second file is not found");
        return WR_FATAL;
    }
    sz1 = fread(buf1, 1, bufsize, fd1);
    sz2 = fread(buf2, 1, bufsize, fd2);
    if (sz1 != sz2){
        wr_set_error("size of two files are not match");
        return WR_FATAL;
    }
    if (memcmp(buf1, buf2, sz1) != 0){
        wr_set_error("files are differ");
        return WR_FATAL;
    }
    return WR_OK;
}


int main(int argc, char ** argv)
{
    wr_rtp_filter_t pcap_filter;
    wr_errorcode_t err;

    /* init app with default options */
    char * av[] = {
        "pcap_test", "--codec-list", "PCMU", "--to-file", "dontcare.pcap", "--from-file", "dontcare.wav"
    };
    CHECK( get_options(7, av, "../conf/wav2rtp/codecs.conf", "../conf/wav2rtp/output.conf") );

    wr_rtp_filter_create(&pcap_filter, "test pcap filter", &wr_pcap_filter_notify);

    /* Some simple sanity checks */
    {
        if (sizeof(wr_rtp_header_t) != (2+2+4+4+4*16)){
            printf("Wrong size of the RTP header: %d\n", sizeof(wr_rtp_header_t));
            return WR_FATAL;
        }

    }

    /* Create empty test pcap file */
    {
        wr_options.output_filename = "testdata/empty_test.pcap";
        CHECK( wr_pcap_filter_notify(&pcap_filter, TRANSMISSION_START, NULL) );
        CHECK( wr_pcap_filter_notify(&pcap_filter, TRANSMISSION_END, NULL) );
        CHECK( files_are_equals("testdata/empty_test.pcap", "testdata/empty_test.pcap.ref", 1024));
    }
    /* Create pcap file with just one packet */
    {
        wr_rtp_packet_t p;
        struct timeval tv = {0,0};
        uint8_t data[] = {0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD};
        wr_options.output_filename = "testdata/one_packet_test.pcap";

        CHECK( wr_rtp_packet_init(&p, 0, 0, 0, 0, tv) );
        CHECK( wr_rtp_packet_add_frame(&p, data, 8, 1000) );
        CHECK( wr_pcap_filter_notify(&pcap_filter, TRANSMISSION_START, NULL));
        CHECK( wr_pcap_filter_notify(&pcap_filter, NEW_PACKET, &p));
        CHECK( wr_pcap_filter_notify(&pcap_filter, TRANSMISSION_END, NULL));
        CHECK( files_are_equals("testdata/one_packet_test.pcap", "testdata/one_packet_test.pcap.ref", 1024));
    }
    return WR_OK;
}
