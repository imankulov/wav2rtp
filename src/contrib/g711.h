#ifndef CONTRIB_G711_H
#define CONTRIB_G711_H

int linear2alaw(int     pcm_val);
int alaw2linear(int     a_val);
int linear2ulaw( int    pcm_val);
int ulaw2linear( int    u_val);

#endif
