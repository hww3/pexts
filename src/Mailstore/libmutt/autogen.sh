#!/bin/sh

aclocal
automake -a
autoheader
autoconf --include=$1
