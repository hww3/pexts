#!/bin/sh
cd `dirname $0`
mydir=`pwd`

aclocal -I aclocal
for a in `find src tools -name "autogen.sh" |sort`; do
  dir=`dirname $a`
  echo "Running autogen in '$dir'"
  cd $dir >/dev/null 2>&1
  ./autogen.sh $mydir
  cd $mydir >/dev/null 2>&1
done

autoheader >/dev/null 2>&1
autoconf >/dev/null 2>&1
