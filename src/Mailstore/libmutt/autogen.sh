#!/bin/sh

libtoolize -c -f
aclocal
autoheader
automake -a -c
autoconf --include=$1
