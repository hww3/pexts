#!/bin/sh

autoheader
aclocal
automake -a
autoconf --include=$1
