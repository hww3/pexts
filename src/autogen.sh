#!/bin/sh

if [ -z "$1" -o -z "$2" ]; then
    echo "Parameters absent. This file must be called from the toplevel source directory"
    echo "by the main autogen.sh script. I should never be called directly."
    exit 1
fi

autoconf --include=$1
