#!/bin/bash


PACKAGE_DIR_PATH=$DA_TOP/oss/source/$1/$2/$1-$2

echo $PACKAGE_DIR_PATH

do_configure() {
	# WARNING: gross hack follows:
	# An autotools built package generally needs these scripts, however only
	# automake or libtoolize actually install the current versions of them.
	# This is a problem in builds that do not use libtool or automake, in the case
	# where we -need- the latest version of these scripts.  e.g. running a build
	# for a package whose autotools are old, on an x86_64 machine, which the old
	# config.sub does not support.  Work around this by installing them manually
	# regardless.

	old=$(pwd)
	cd $PACKAGE_DIR_PATH

	if [ -e configure.in -o -e configure.ac ]; then

		if [ -e configure ]; then
			rm -f configure
		fi ## we assume configure.ac or configure.in existed

		ACLOCAL="aclocal --system-acdir=$AUTOTOOL_ROOT/usr/share/aclocal"
		if [ x"default" = xdefault ]; then
			acpaths=
			for i in `find . -maxdepth 2 -name \*.m4|grep -v 'aclocal.m4'| \
				grep -v 'acinclude.m4' | grep -v 'aclocal-copy' | sed -e 's,\(.*/\).*$,\1,'|sort -u`; do
				acpaths="$acpaths -I $i"
			done
		else
			acpaths="default"
		fi

		#acpaths='./'
		AUTOV=`automake --version | sed -e '1{s/.* //;s/\.[0-9]\+$//};q'`
		$AUTOTOOL_ROOT/usr/bin/automake --version
		echo "AUTOV is $AUTOV"
		if [ -d ${BUILD_TOOLDIR}/aclocal-$AUTOV ]; then
			ACLOCAL="$ACLOCAL --automake-acdir=$AUTOTOOL_ROOT/usr/share/aclocal-$AUTOV"
		fi

		# autoreconf is too shy to overwrite aclocal.m4 if it doesn't look
		# like it was auto-generated.  Work around this by blowing it away
		# by hand, unless the package specifically asked not to run aclocal.
		if ! echo --exclude=autopoint | grep -q "aclocal"; then
			rm -f aclocal.m4
		fi

		if [ -e configure.in ]; then
			CONFIGURE_AC=configure.in
		else
			CONFIGURE_AC=configure.ac
		fi

		if grep "^[[:space:]]*AM_GLIB_GNU_GETTEXT" $CONFIGURE_AC >/dev/null; then
			if grep "sed.*POTFILES" $CONFIGURE_AC >/dev/null; then
				: do nothing -- we still have an old unmodified configure.ac
	    		else
				echo "no" | glib-gettextize --force --copy
			fi
		else if grep "^[[:space:]]*AM_GNU_GETTEXT" $CONFIGURE_AC >/dev/null; then
			# We'd call gettextize here if it wasn't so broken...
				cp /usr/share/gettext/config.rpath config.rpath
				if [ -d po ]; then
					cp -f /usr/share/gettext/po/Makefile.in.in po/
					if [ ! -e po/remove-potcdate.sin ]; then
						cp /usr/share/gettext/po/remove-potcdate.sin po/
					fi
				fi
				for i in gettext.m4 iconv.m4 lib-ld.m4 lib-link.m4 lib-prefix.m4 nls.m4 po.m4 progtest.m4; do
					for j in `find . -name $i | grep -v aclocal-copy`; do
						rm $j
					done
				done
			fi
		fi
		mkdir -p m4
		echo "$?"
		if grep "^[[:space:]]*[AI][CT]_PROG_INTLTOOL" $CONFIGURE_AC >/dev/null; then
			intltoolize --copy --force --automake
		fi
		ACLOCAL="$ACLOCAL" autoreconf  --verbose --install --force --exclude=autopoint $acpaths  
	else
		echo "no configure.ac"	
	fi
	cd $old

}

do_configure

