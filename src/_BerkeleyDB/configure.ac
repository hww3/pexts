AC_INIT(bdb.c)
AC_CONFIG_HEADER(bdb_config.h)
CAUDIUM_MODULE_INIT([BerkeleyDB])

OLD_LIBS=$LIBS
OLD_LDFLAGS=$LDFLAGS
OLD_CPPFLAGS=$CPPFLAGS

AC_ARG_WITH(BerkeleyDB,  [  --with-BerkeleyDB=path       Enable support for the Berkeley database],[],[withBerkeleyDB=yes])

if test x$with_BerkeleyDB = xno; then
  :
else
  AC_MSG_CHECKING(for BerkeleyDB)

  dirs="/usr/local/BerkeleyDB.3.1 /opt/BerkeleyDB.3.1 /usr/BerkeleyDB.3.1 /BerkeleyDB.3.1 /sw/BerkeleyDB.3.1 /usr/local/BerkeleyDB /opt/BerkeleyDB /usr/BerkeleyDB /BerkeleyDB /sw/BerkeleyDB  no"
  if test x$with_BerkeleyDB = xyes; then :; else
    # Bdb installation directory specified.
    dirs="$withBerkeleyDB $dirs"
  fi

  AC_CACHE_VAL(caudium_cv_bdb_dir, [
    for caudium_cv_bdb_dir in $dirs; do
      if test -d $caudium_cv_bdb_dir/.; then
        break
      else
        :
      fi
    done
  ])

  AC_MSG_RESULT($caudium_cv_bdb_dir)

  if test x$caudium_cv_bdb_dir = xno; then :; else
    LIBS="-R$caudium_cv_bdb_dir/lib/ -L$caudium_cv_bdb_dir/lib/"
    CFLAGS="-I$caudium_cv_bdb_dir/include/"
  fi
fi
AC_CHECK_HEADERS( db.h )
AC_CHECK_LIB(db-3, db_create)
if test "$ac_cv_lib_db_3_db_create" = "no"; then
  AC_CHECK_LIB(db-3.1, db_create)
  if test "$ac_cv_lib_db_3_1_db_create" = "no"; then
    AC_CHECK_LIB(db-3.2, db_create)
    if test "$ac_cv_lib_db_3_2_db_create" = "no"; then
      AC_CHECK_LIB(db, db_create)
    fi
  fi
fi 
AC_MSG_CHECKING(if Berkeley DB is new enough)
AC_CACHE_VAL(caudium_cv_bdb_new_enough, 
  AC_TRY_RUN([#include <db.h>
    int main(int argc, char **argv) {
    int M,m;
    db_version(&M,&m,NULL);
    if(M < 3 || (M == 3 && m < 1))
      exit(1);
    exit(0);
  }],[caudium_cv_bdb_new_enough=yes], [caudium_cv_bdb_new_enough=no],[caudium_cv_bdb_new_enough=yes])
)
if test "$caudium_cv_bdb_new_enough" = "yes"; then
  AC_DEFINE(HAVE_BDB, 1, [Define if BDB exists and is new enough])
fi
AC_MSG_RESULT($caudium_cv_bdb_new_enough)

AC_SUBST(makefile_rules)
AC_OUTPUT(Makefile)

dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
