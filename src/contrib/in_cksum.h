/* in_cksum.h
 * Declaration of  Internet checksum routine.
 *
 * $Id: in_cksum.h 12117 2004-09-28 00:06:32Z guy $
 */

#ifndef CONTRIB_IN_CKSUM_H
#define CONTRIB_IN_CKSUM_H 1


#ifndef guint8
    #define guint8 unsigned char
#endif

#ifndef guint16
    #define guint16 unsigned short
#endif

#ifndef guint32
    #define guint32 unsigned long
#endif

typedef struct {
	const guint8 *ptr;
	int	len;
} vec_t;

int in_cksum(const vec_t *vec, int veclen);

guint16 in_cksum_shouldbe(guint16 sum, guint16 computed_sum);

#endif
