#!/bin/sh

aclocal
automake -a -f -c
autoheader
autoconf --include=$1
