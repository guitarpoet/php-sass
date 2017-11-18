PHP_ARG_ENABLE(sass, whether to enable sass compiler support, [ --enable-sass Enable sass compiler support], yes, yes)
                               
if test "$PHP_SASS" != "no"; then 
	if test "$PHP_SASS" != "yes"; then
		LIBSASS_SEARCH_DIRS=$PHP_SASS
	else
		LIBSASS_SEARCH_DIRS="/usr/local/src /opt/local/src"
	fi

	for i in $LIBSASS_SEARCH_DIRS; do
		if test -f $i/libsass/include/sass.h; then
			LIBSASS_SRC=$i/libsass
		fi
	done

	LIBSASS_LIB_SEARCH_DIRS="/usr/local/lib /usr/lib /opt/local/lib"
	for i in $LIBSASS_LIB_SEARCH_DIRS; do
		if test -f $i/libsass.a; then
			LIBSASS_LD_DIR=$i
		fi
	done

	if test -z "$LIBSASS_SRC"; then
		AC_MSG_ERROR(Cannot find libsass codes!)
	fi

	if test -z "$LIBSASS_LD_DIR"; then
		AC_MSG_ERROR(Cannot find libsass.a at /usr/lib, /usr/local/lib or /opt/local/lib !)
	fi

	PHP_REQUIRE_CXX()
	PHP_ADD_LIBRARY_WITH_PATH(sass, $LIBSASS_LD_DIR, LIBSASS_SHARED_LIBADD)
    CPPFLAGS="-std=c++11"
	LDFLAGS="-lsass -lstdc++"
	PHP_ADD_INCLUDE($LIBSASS_SRC/include)
	PHP_ADD_INCLUDE($LIBSASS_SRC/src)
    PHP_NEW_EXTENSION(sass, src/sass.cpp src/sass_functions.cpp, $ext_shared)
fi
