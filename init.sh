#!/bin/sh
# This is a script to make initial files for building
# the application with 
# ./configure && make && make install
#
# Run this script right after "svn co"
autoheader
aclocal
autoconf
automake --add-missing
