#!/bin/sh

aclocal
automake -a -c
autoheader
autoconf --include=$1
