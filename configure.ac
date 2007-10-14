#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(wav2rtp, 0.7pre)
AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([src/wav2rtp.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([gsm], [gsm_create])
AC_CHECK_LIB([ortp], [rtp_session_new])
AC_CHECK_LIB([sndfile], [sf_open])
AC_CHECK_LIB([speex], [speex_encoder_init])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h stdlib.h string.h strings.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
# AC_FUNC_MALLOC
AC_CHECK_FUNCS([bzero gettimeofday strchr])

# Additional definitions
if test "x${prefix}" = "xNONE"; then
    _confdir=$(eval echo $(echo "${sysconfdir}" | sed 's/prefix/ac_default_prefix/'))
else
    _confdir=$(eval echo "${sysconfdir}")
fi
AC_DEFINE_UNQUOTED([CONFDIR], "${_confdir}/wav2rtp", [wav2rtp configuration dir])

AC_CONFIG_FILES([Makefile conf/Makefile src/Makefile])
AC_OUTPUT
