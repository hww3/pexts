#!/bin/sh

autoheader
aclocal
automake -a
autoconf -B $1
