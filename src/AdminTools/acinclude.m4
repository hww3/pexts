define([AP_SHADOW_GID],
[
#### Try to find the shadow group id ####
AC_MSG_CHECKING(for shadow group ID)
if test ! -f /etc/shadow; then
    AC_MSG_RESULT(no /etc/shadow on this system)
else
    host_ok="no"
    case $host_os in
	*linux*) host_ok="yes";;       
    esac
    if test "$host_ok" = "yes"; then
	shadow_line="`ls -ln /etc/shadow`"
	shadow_gr_r="`echo $shadow_line | cut -c 5`"
	if test -z $shadow_id; then
	    shadow_id="`echo $shadow_line | tr -s ' ' | cut -d ' ' -f 4`"
	fi
	AC_DEFINE_UNQUOTED(SHADOW_GID, $shadow_id, Shadow group ID used on this system)
	if test "$shadow_gr_r" != "r"; then
	    AC_MSG_WARN(/etc/shadow not group-readable)
	else
	    AC_MSG_RESULT($shadow_id)
	fi
    else
	AC_MSG_RESULT(not a Linux machine)
    fi
fi
])
