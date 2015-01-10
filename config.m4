PHP_ARG_ENABLE(sass, whether to enable sass compiler support, [ --enable-sass Enable sass compiler support], yes, yes)
                               
if test "$PHP_SASS" != "no"; then 
	if test "$PHP_SASS" != "yes"; then
		LIBSASS_SEARCH_DIRS=$PHP_SASS
	else
		LIBSASS_SEARCH_DIRS="/usr/local/include /usr/include /opt/local/include /usr/local/src /opt/local/src"
	fi

	for i in $LIBSASS_SEARCH_DIRS; do
		if test -f $i/libsass/sass.h; then
			LIBSASS_INCDIR=$i
		fi
	done

	LIBSASS_LIB_SEARCH_DIRS="/usr/local/lib /usr/lib /opt/local/lib"
	for i in $LIBSASS_LIB_SEARCH_DIRS; do
		if test -f $i/libsass.a; then
			LIBSASS_LD_DIR=$i
		fi
	done

	if test -z "$LIBSASS_INCDIR"; then
		AC_MSG_ERROR(Cannot find libsass!)
	fi

	if test -z "$LIBSASS_LD_DIR"; then
		AC_MSG_ERROR(Cannot find libsass.a!)
	fi

	PHP_ADD_LIBRARY_WITH_PATH(sass, $LIBSASS_LD_DIR, LIBSASS_SHARED_LIBADD)
	LDFLAGS="-lsass -lstdc++"
	PHP_ADD_INCLUDE($LIBSASS_INCDIR)
    PHP_NEW_EXTENSION(sass, src/sass.c, $ext_shared)
fi
