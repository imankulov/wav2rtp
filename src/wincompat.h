#ifndef __WINCOMPAT_H__
#define __WINCOMPAT_H__

#ifdef _WIN32

#include <winsock2.h>
#include <stdint.h>

#define ETH_ALEN	6
#define ETHER_ADDR_LEN ETH_ALEN

struct ether_addr
{
  uint8_t ether_addr_octet[ETH_ALEN];
};

struct ether_header
{
  uint8_t  ether_dhost[ETH_ALEN];
  uint8_t  ether_shost[ETH_ALEN];
  uint16_t ether_type;
};

#define SOL_UDP            17      /* sockopt level for UDP */

struct ip {
	uint8_t ip_hl:4,		/* header length */
		     ip_v:4;		/* version */
	uint8_t   ip_tos;		/* type of service */
	uint16_t ip_len;		/* total length */
	uint16_t ip_id;		/* identification */
	uint16_t ip_off;		/* fragment offset field */
	uint8_t  ip_ttl;		/* time to live */
	uint8_t  ip_p;			/* protocol */
	uint16_t ip_sum;		/* checksum */
	struct  in_addr ip_src,ip_dst;  /* source and dest address */
};

struct udphdr
{
  uint16_t uh_sport;	/* source port */
  uint16_t uh_dport;	/* destination port */
  uint16_t uh_ulen;		/* udp length */
  uint16_t uh_sum;		/* udp checksum */
};

struct ether_addr *ether_aton (const char *str);

#endif

#endif
