#!/bin/sh

autoheader
autoconf -B $1
automake
