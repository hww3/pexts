#!/bin/sh

cd `dirname $0`
mydir=`pwd`
tmpfile_with=$mydir/.tmp_stuff.with
tmpfile_subdirs=$mydir/.tmp_stuff.subdirs
module_dirs=$mydir/.module_dirs
config_files="$mydir/configure.ac.in $mydir/src/configure.ac.in"

#
# Catch some signals for cleanup
#
trap 'rm -f $tmpfile_with $tmpfile_subdirs; exit 0' 0 1 2 13 15

clean_dirs()
{
  rm -rf `find ./ -name "autom4te.cache" -type d -print`
  rm -f `find ./ -name "configure" -type f -print`
}

gather_modules()
{
  read MODLINE MODDEFAULT
  while [ -n "$MODLINE" ]; do
   cat <<EOF | tr -d '\n' >> $tmpfile_with
AC_ARG_WITH(\\$MODLINE, þ
  AC_HELP_STRING(\\[--with-\\$MODLINE\\],\\[compile with the \\$MODLINE module\\]), \\[þ
    if test "x\\\$withval" != "xno"; thenþ
       ENABLE_MODULES="\\\$ENABLE_MODULES $MODLINE"; export ENABLE_MODULESþ
       MODULE_${MODLINE}_ENABLED=yes; export MODULE_${MODLINE}_ENABLEDþ
    fi\\], \\[þ
    if test "x$MODDEFAULT" = "xdefault"; thenþ
       ENABLE_MODULES="\\\$ENABLE_MODULES $MODLINE"; export ENABLE_MODULESþ
       MODULE_${MODLINE}_ENABLED=yes; export MODULE_${MODLINE}_ENABLEDþ
    fiþ
  \\])þ
þ
EOF
	MODULES="$MODULES $MODLINE"
	read MODLINE MODDEFAULT
    done
#    cat $tmpfile.tmp | tr '\n' '\\\\n' > $tmpfile
#    rm -f $tmpfile
}

gather_subdirs()
{
  read MODLINE MODDEFAULT
  while [ -n "$MODLINE" ]; do
    cat <<EOF | tr -d '\n' >> $tmpfile_subdirs
if test "x\\\$MODULE_${MODLINE}_ENABLED" = "xyes"; thenþ
  AC_CONFIG_SUBDIRS($MODLINE)þ
fiþ
EOF
    read MODLINE MODDEFAULT
  done
}

create_config()
{
    if [ ! -f $1 ]; then
	    echo "Config template $1 does not exist. Aborting."
	    exit 1
    fi    

    sed -e "s#@enabled_modules_pre@#`cat $tmpfile_subdirs`#g" \
        -e "s#@module_with_pre@#`cat $tmpfile_with`#g" < $1 \
	| tr '\376' '\n' > `dirname $1`/configure.ac
}

echo "Cleaning the directories"
clean_dirs

rm -f $module_dirs
echo "Generating the list of modules"

for d in `find "./src/." -name "*" -type d -prune -print`; do
    if [ -f $d/.pexts_module ]; then
      pexts_module_val=`cat $d/.pexts_module | head -n 1 | tr -d '\t\n'`
      module_val=""
      case $pexts_module_val in
        enabled|disabled|default) module_val="$pexts_module_val" ;;
	      *) echo "Invalid value in $d/.pexts_module - it should be 'enabled', 'disabled' or 'default' to, "
	         echo "respectively, activate, deactivate or activate to be compiled by default the module."
	         echo
	         ;;
      esac
      if [ -z "$module_val" ]; then
        continue
      fi
      if [ "x$pexts_module_val" = "xdisabled" ]; then
        echo " Module `basename $d` is disabled."
        continue
      fi
      echo "`basename $d` $module_val" >> $module_dirs
    fi
done

#
# Generate the --with parameters plus help
#
MODULES=""
if [ -f $module_dirs ]; then
    MODULE_NAMES="`cat $module_dirs | cut -d ' ' -f 1 | tr '\n' ' '`"
    rm -f $tmpfile_with
    gather_modules < $module_dirs
    gather_subdirs < $module_dirs
    if [ -z "$MODULES" ]; then
	    echo "No modules were enabled. Aborting."
	    exit 1
    fi
    for cf in $config_files; do
      create_config $cf
    done
fi

rm -f $tmpfile

aclocal
for a in . $MODULE_NAMES; do
  dir=src/$a
  echo "Running autogen in '$dir'"
  cd $dir >/dev/null 2>&1
  ./autogen.sh $mydir $module_dirs
  if [ -f .pexts_autogen_extra ]; then
    for b in `cat .pexts_autogen_extra`; do
      if [ ! -d $b ]; then
        continue
      fi
      cd $b >/dev/null 2>&1
      echo " Running autogen in '$dir/$b'"
      ./autogen.sh $mydir $module_dirs
    done
  fi
  cd $mydir >/dev/null 2>&1
done

autoheader >/dev/null 2>&1
autoconf >/dev/null 2>&1

exit 0
