/* in_cksum.h
 * Declaration of  Internet checksum routine.
 *
 * $Id: in_cksum.h 12117 2004-09-28 00:06:32Z guy $
 */

#ifndef CONTRIB_IN_CKSUM_H
#define CONTRIB_IN_CKSUM_H

typedef struct {
	const unsigned char *ptr;
	int	len;
} vec_t;

int in_cksum(const vec_t *vec, int veclen);

unsigned short in_cksum_shouldbe(unsigned short sum, unsigned short computed_sum);

#endif
