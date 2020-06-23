dnl adapted from rgdal configure.ac 2020-05-15 Michael Sumner
dnl

dnl Process this file with autoconf to produce a configure script.
AC_INIT(src/vapour.cpp)



# AC_PROG_CXX()

# find R home and set correct compiler + flags
: ${R_HOME=`R RHOME`}
if test -z "${R_HOME}"; then
  AC_MSG_ERROR([cannot determine R_HOME. Make sure you use R CMD INSTALL!])
fi
AC_MSG_NOTICE([R_HOME: ${R_HOME}])
RBIN="${R_HOME}/bin/R"

# pick all flags for testing from R
: ${CC=`"${RBIN}" CMD config CC`}
: ${CXX=`"${RBIN}" CMD config CXX`}
: ${CFLAGS=`"${RBIN}" CMD config CFLAGS`}
: ${CPPFLAGS=`"${RBIN}" CMD config CPPFLAGS`}
: ${LDFLAGS=`"${RBIN}" CMD config LDFLAGS`}
AC_MSG_NOTICE([CC: ${CC}])
AC_MSG_NOTICE([CXX: ${CXX}])

#CXX_11

R_VERSION=`echo 'o <- R.version; cat(paste0(o$major, ".", o$minor))' | ${RBIN} --vanilla --slave`
R_MINOR=`echo $R_VERSION | cut -f2 -d"."`
R_MAJOR=`echo $R_VERSION | cut -f1 -d"."`


if test ${R_MAJOR} -ge 4; then
 CXX11=`"${RBIN}" CMD config CXX11`
 CXX11STD=`"${RBIN}" CMD config CXX11STD`
else
 if test ${R_MAJOR} -eq 3 && test ${R_MINOR} -ge 4; then
  CXX11=`"${RBIN}" CMD config CXX11`
  CXX11STD=`"${RBIN}" CMD config CXX11STD`
 else
  CXX11=`"${RBIN}" CMD config CXX1X`
  CXX11STD=`"${RBIN}" CMD config CXX1XSTD`
 fi
fi

AC_MSG_NOTICE([CXX11 is: ${CXX11}, CXX11STD is: ${CXX11STD}])

CXX="${CXX11} ${CXX11STD}"
AC_MSG_NOTICE([CXX is: ${CXX}])

if test -n "${CXX11}"; then
HAVE_CXX11=1
else
HAVE_CXX11=0
fi

if test [${HAVE_CXX11} = 1] ; then
  AC_MSG_NOTICE([C++11 support available])
else
  AC_MSG_NOTICE([C++11 support not available])
fi


#GDAL

GDAL_CONFIG="gdal-config"

GDAL_CONFIG_SET="no"

AC_ARG_WITH([gdal-config],
    AC_HELP_STRING([--with-gdal-config=GDAL_CONFIG],
           [the location of gdal-config]),
           [gdal_config=$withval])
if test [ -n "$gdal_config" ] ; then
    GDAL_CONFIG_SET="yes"
    AC_SUBST([GDAL_CONFIG],["${gdal_config}"])
    AC_MSG_NOTICE(gdal-config set to $GDAL_CONFIG)
fi

if test ["$GDAL_CONFIG_SET" = "no"] ; then
  AC_PATH_PROG([GDAL_CONFIG], ["$GDAL_CONFIG"],["no"])
  if test ["$GDAL_CONFIG" = "no"] ; then
    AC_MSG_RESULT(no)
    AC_MSG_ERROR([gdal-config not found or not executable.])
  fi
else
  AC_MSG_CHECKING(gdal-config exists)
  if test -r "${GDAL_CONFIG}"; then
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
    AC_MSG_ERROR([gdal-config not found - configure argument error.])
  fi
  AC_MSG_CHECKING(gdal-config executable)
  if test -x "${GDAL_CONFIG}"; then
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
    AC_MSG_ERROR([gdal-config not executable.])
  fi
fi


AC_MSG_CHECKING(gdal-config usability)
if test `${GDAL_CONFIG} --version`;
then
  GDAL_CPPFLAGS=`${GDAL_CONFIG} --cflags`
  GDAL_LIBS=`${GDAL_CONFIG} --libs`
  GDAL_VERSION=`${GDAL_CONFIG} --version`
  GDAL_DEP_LIBS=`${GDAL_CONFIG} --dep-libs`
  GDAL_DATADIR=`${GDAL_CONFIG} --datadir`
  gdalok=yes
  AC_MSG_RESULT(yes)
else
      AC_MSG_RESULT(no)
	echo "Error: gdal-config not found"
	echo "The gdal-config script distributed with GDAL could not be found."
	echo "If you have not installed the GDAL libraries, you can"
	echo "download the source from  http://www.gdal.org/"
	echo "If you have installed the GDAL libraries, then make sure that"
	echo "gdal-config is in your path. Try typing gdal-config at a"
	echo "shell prompt and see if it runs. If not, use:"
	echo " --configure-args='--with-gdal-config=/usr/local/bin/gdal-config'"
	echo "with appropriate values for your installation."
	echo ""

	exit 1
fi


AC_MSG_NOTICE([GDAL: ${GDAL_VERSION}])
GDAL_MAJOR=`echo $GDAL_VERSION | cut -f1 -d"."`
GDAL_MINOR=`echo $GDAL_VERSION | cut -f2 -d"."`
GDAL_PATCH=`echo $GDAL_VERSION | cut -f3 -d"."`


AC_MSG_CHECKING([GDAL version >= 2.2.2])
if test ${GDAL_MAJOR} -gt 2 ; then
  AC_MSG_RESULT(yes)
else
  if test ${GDAL_MAJOR} = 2 && test ${GDAL_MINOR} -ge 2 && test ${GDAL_PATCH} -ge 2  ; then
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
    AC_MSG_ERROR([upgrade GDAL to 2.2.2 or later])
  fi
fi



INLIBS="${LIBS}"
INCPPFLAGS="${CPPFLAGS}"
INPKG_CPPFLAGS="${PKG_CPPFLAGS}"
INPKG_LIBS="${PKG_LIBS}"

AC_SUBST([PKG_CPPFLAGS], ["${INPKG_CPPFLAGS} ${GDAL_CPPFLAGS}"])
AC_SUBST([PKG_LIBS], ["${INPKG_LIBS} ${GDAL_LIBS}"])

# honor PKG_xx overrides
# for CPPFLAGS we will superfluously double R's flags
# since we'll set PKG_CPPFLAGS with this, but that shouldn't hurt
CPPFLAGS="${INCPPFLAGS} ${PKG_CPPFLAGS}"


NEED_DEPS=no
LIBS="${INLIBS} ${PKG_LIBS}"
[cat > gdal_test.cc <<_EOCONF
#include <gdal.h>
#ifdef __cplusplus
extern "C" {
#endif
int main() {
GDALAllRegister();
}
#ifdef __cplusplus
}
#endif
_EOCONF]

AC_MSG_CHECKING(GDAL: linking with --libs only)
${CXX} ${CPPFLAGS} -o gdal_test gdal_test.cc ${LIBS} 2> errors.txt
if test `echo $?` -ne 0 ; then
gdalok=no
AC_MSG_RESULT(no)
else
AC_MSG_RESULT(yes)
fi

if test "${gdalok}" = no; then
AC_MSG_CHECKING(GDAL: linking with --libs and --dep-libs)
LIBS="${LIBS} ${GDAL_DEP_LIBS}"
gdalok=yes
${CXX} ${CPPFLAGS} -o gdal_test gdal_test.cc ${LIBS} 2>> errors.txt
if test `echo $?` -ne 0 ; then
gdalok=no
fi
if test "${gdalok}" = yes; then
    NEED_DEPS=yes
    AC_MSG_RESULT(yes)
else
    AC_MSG_RESULT(no)
fi
fi

if test "${gdalok}" = no; then
   cat errors.txt
   AC_MSG_NOTICE([Install failure: compilation and/or linkage problems.])
   AC_MSG_ERROR([GDALAllRegister not found in libgdal.])
fi

rm -f gdal_test errors.txt gdal_test.cc

AC_MSG_CHECKING(GDAL: gdal-config data directory readable)
if test -d "${GDAL_DATADIR}" ; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
  AC_MSG_CHECKING(GDAL: GDAL_DATA data directory readable)
  if test -z ${GDAL_DATA} ; then
    AC_MSG_ERROR([GDAL data directory not found.])
  else
    if test -d "${GDAL_DATA}" ; then
      AC_MSG_RESULT(yes)
      AC_SUBST([GDAL_DATADIR], [${GDAL_DATA}])
    else
      AC_MSG_RESULT(no)
      AC_MSG_ERROR([GDAL data directory not found.])
    fi
  fi
fi

GDAL_GE_250="no"
GDAL_MAJ_VER=`echo $GDAL_VERSION | cut -d "." -f1`
GDAL_MOD_VER=`echo $GDAL_VERSION | cut -d "." -f2`
if test "${GDAL_MAJ_VER}" = 2 ; then
  if test "${GDAL_MOD_VER}" -ge 5 ; then
    GDAL_GE_250="yes"
  fi
else
  if test "${GDAL_MAJ_VER}" -ge 3 ; then
    GDAL_GE_250="yes"
  fi
fi

GDAL_DATA_TEST_FILE="${GDAL_DATADIR}/stateplane.csv"
AC_MSG_CHECKING(GDAL: ${GDAL_DATADIR}/stateplane.csv readable)
if test -r "${GDAL_DATA_TEST_FILE}" ; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
#  if test "${GDAL_GE_250}" = no; then
    AC_MSG_ERROR([stateplane.csv not found in GDAL data directory.])
#  fi
fi

# PROJ.4

# copied from sf/configure.ac

PROJ_CONFIG="pkg-config proj"

if `$PROJ_CONFIG --exists` ; then # pkg-config proj
  AC_MSG_NOTICE([pkg-config proj exists, will use it])
  proj_config_ok=yes
else # pkg-config proj
  AC_MSG_NOTICE([pkg-config proj not available])
  echo "  set PKG_CONFIG_PATH to the directory containing proj.pc"
  proj_config_ok=no
fi # pkg-config proj

PROJ_VERSION=""
PROJ6=""

if test "${proj_config_ok}" = yes; then # proj_config_ok
  PROJ_VERSION=`${PROJ_CONFIG} --modversion`
  AC_MSG_NOTICE([PROJ version: ${PROJ_VERSION}])

  PROJ_VERSION_DOT=`echo ${PROJ_VERSION} | tr -d "."`
  PROJV1=`echo "${PROJ_VERSION}" | cut -c 1`

  if test "${PROJV1}" -ge 6; then # PROJ >= 6
    PKG_CPPFLAGS="-DPROJ_H_API"
    PROJ6="yes"
  else # PROJ >= 6
    AC_MSG_NOTICE([PROJ version < 6.0.0])
    PKG_CPPFLAGS=""
    PROJ6="no"
  fi # PROJ >= 6

else # proj_config_ok

  AC_MSG_NOTICE([PROJ version not determined using pkg-config proj])

fi # proj_config_ok


AC_ARG_WITH([proj-include],
    AC_HELP_STRING([--with-proj-include=DIR],
                   [location of proj header files]),
    [proj_include_path=$withval])
if test [ -n "$proj_include_path" ] ; then # proj_include_path
   proj_include_path="-I${proj_include_path}"
   AC_SUBST([PKG_CPPFLAGS],["${PKG_CPPFLAGS} ${proj_include_path} ${INPKG_CPPFLAGS}"])
  if test "${proj_config_ok}" = yes; then # proj_config_ok
    AC_MSG_NOTICE([--with-proj-include= overrides pkg-config proj --cflags])
  fi # proj_config_ok
else # proj_include_path
  if test "${proj_config_ok}" = yes; then # proj_config_ok
    PROJ_INCLUDE_PATH=`${PROJ_CONFIG} --cflags`
    proj_include_path=${PROJ_INCLUDE_PATH}
    AC_SUBST([PKG_CPPFLAGS],["${PKG_CPPFLAGS} ${PROJ_INCLUDE_PATH} ${INPKG_CPPFLAGS}"])
  fi # proj_config_ok
fi # proj_include_path

# honor PKG_xx overrides
# for CPPFLAGS we will superfluously double R's flags
# since we'll set PKG_CPPFLAGS with this, but that shouldn't hurt
CPPFLAGS="${INCPPFLAGS} ${PKG_CPPFLAGS}"
AC_MSG_NOTICE([PROJ CPP flags: ${PKG_CPPFLAGS}])
PKG_LIBS=""
PROJ_LIB_PATH=""

# dnl ditto for a library path
AC_ARG_WITH([proj-lib],
    AC_HELP_STRING([--with-proj-lib=LIB_PATH],
               [the location of proj libraries]),
               [proj_lib_path=$withval])
if test [ -n "$proj_lib_path" ] ; then # proj_lib_path
    AC_SUBST([PKG_LIBS], ["-L${proj_lib_path} ${INPKG_LIBS}"])
  if test "${proj_config_ok}" = yes; then # proj_config_ok
    AC_MSG_NOTICE([--with-proj-lib= overrides pkg-config proj --libs])
  fi # proj_config_ok
else # proj_lib_path
  if test ${proj_config_ok} = "yes"; then # proj_config_ok
    PROJ_LIB_PATH=`${PROJ_CONFIG} --libs`
    AC_SUBST([PKG_LIBS], ["${PROJ_LIB_PATH} ${INPKG_LIBS}"])
  else # proj_config_ok
    AC_SUBST([PKG_LIBS], ["${INPKG_LIBS} -lproj"])
  fi # proj_config_ok
fi # proj_lib_path

LIBS="${PKG_LIBS} ${LIBS}"
AC_MSG_NOTICE([PROJ LIBS: ${PKG_LIBS}])


AC_ARG_WITH([proj_api],
    [  --with-proj_api[=ARG]        legacy proj_api.h while still available, or current proj.h (ARG="proj_api.h", "proj.h"(default))],,)

AC_MSG_CHECKING([PROJ header API:])

if test -z "$with_proj_api" ; then # with_proj_api
  PROJ_API=""
  if test "${PROJ6}" = "yes" ; then # PROJ6
    PROJ_API="proj.h"
  fi
  if test "${PROJ6}" = "no" ; then # PROJ6
    PROJ_API="proj_api.h"
  fi
else # with_proj_api
  PROJ_API="$with_proj_api"
fi # with_proj_api

api_string_ok="no"
if test -z "${PROJ_API}" ; then
  api_string_ok="yes"
fi
if test "${PROJ_API}" = "proj.h" ; then
  api_string_ok="yes"
fi
if test "${PROJ_API}" = "proj_api.h" ; then
  api_string_ok="yes"
fi

if test ${api_string_ok} = "no" ; then # PROJ_API
  AC_MSG_RESULT(no)
  AC_MSG_ERROR([PROJ API string ${PROJ_API} not recognized.])
else
  AC_MSG_RESULT(yes)
fi # PROJ_API

if test -z ${PROJ_API} ; then # PROJ_API = ""
  AC_MSG_NOTICE([API to be used as yet undetermined, searching ...])
  PROJ_VERSION_H=""
  LIBS="${INLIBS} ${PKG_LIBS}"
# proj.h case
[cat > proj_conf_test_proj.c <<_EOCONF
#include <stdio.h>
#include <stdlib.h>
#include <proj.h>

int main() {
    printf("%d.%d.%d\n", PROJ_VERSION_MAJOR, PROJ_VERSION_MINOR, PROJ_VERSION_PATCH);
    exit(0);
}
_EOCONF]

  if test `${CC} ${CFLAGS} ${PKGCPPFLAGS} -o proj_conf_test_proj proj_conf_test_proj.c ${PKG_LIBS} 2> /dev/null; echo $?` = 0 ; then
    PROJ_API="proj.h"
    PROJ6="yes"
    PROJ_VERSION_H=`./proj_conf_test_proj`
    PROJ_VERSION_DOT=`echo ${PROJ_VERSION_H} | tr -d "."`
rm -f proj_conf_test_proj.c proj_conf_test_proj
  else
[cat > proj_conf_test_proj_api.c <<_EOCONF
#include <stdio.h>
#include <proj_api.h>

int main() {
    printf("%d\n", PJ_VERSION);
    exit(0);
}
_EOCONF]
    if test `${CC} ${CFLAGS} ${PKG_CPPFLAGS} -o proj_conf_test_proj_api proj_conf_test_proj_api.c ${PKG_LIBS} 2> /dev/null; echo $?` = 0; then
      PROJ_API="proj_api.h"
      PROJ6="no"
      PROJ_VERSION_H=`./proj_conf_test_proj_api`
      PROJ_VERSION_DOT=`echo ${PROJ_VERSION_H} | tr -d "."`
rm -f proj_conf_test_proj_api.c proj_conf_test_proj_api
    fi
  fi
  if test -z "${PROJ_VERSION_H}"; then
    AC_MSG_ERROR([API to be used not found])
  else
    AC_MSG_NOTICE([Using API: ${PROJ_API}])
  fi
fi # PROJ_API = ""

dnl PROJ >= 6 contains C++ code, so AC_CHECK_LIB needs to be
dnl with the C++ runtime (and rgdal is going to use C++).
AC_LANG(C++)

if test "${PROJ_API}" =  "proj.h" ; then # proj.h

  if test ${PROJ6} = "yes"; then # PROJ6
    projhok=yes
    AC_CHECK_HEADER([proj.h],,projhok=no)
    if test "${projhok}" = no; then # libprojok
      AC_MSG_ERROR([proj.h not found in given locations.])
    fi # libprojok
    libprojok=yes
    AC_CHECK_LIB([proj], [proj_context_create],,libprojok=no)
    if test "${libprojok}" = no; then # libprojok
      AC_MSG_ERROR([proj_context_create not found in libproj.])
    fi # libprojok
  fi # PROJ6

fi # proj.h

if test "${PROJ_API}" =  "proj_api.h"; then # proj_api.h

LIBS="${INLIBS} ${PKG_LIBS}"
AC_MSG_CHECKING(proj_api.h presence and usability)
[cat > proj_conf_test1.c <<_EOCONF
#include <stdio.h>
#include <proj_api.h>

int main() {
    printf("%d\n", PJ_VERSION);
    exit(0);
}
_EOCONF]

  if test `${CC} ${CFLAGS} ${PKG_CPPFLAGS} -o proj_conf_test1 proj_conf_test1.c ${PKG_LIBS} 2> /dev/null; echo $?` = 0; then
    PROJ_VERSION_H=`./proj_conf_test1`
    AC_MSG_RESULT(yes)
  else
    if test `${CC} ${CFLAGS} ${PKG_CPPFLAGS} -DACCEPT_USE_OF_DEPRECATED_PROJ_API_H -o proj_conf_test1 proj_conf_test1.c ${PKG_LIBS} 2> /dev/null; echo $?` = 0; then
      PROJ6="yes"
      PKG_CPPFLAGS="${PKG_CPPFLAGS} -DACCEPT_USE_OF_DEPRECATED_PROJ_API_H"
      PROJ_VERSION_H=`./proj_conf_test1`
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no)
      AC_MSG_ERROR([proj_api.h not found in standard or given locations.])
    fi
  fi

rm -f proj_conf_test1.c proj_conf_test1

#AC_MSG_NOTICE(["|${PROJ_VERSION}| |${PROJ_VERSION_H}|"])

if test -n "${PROJ_VERSION}" ; then
  AC_MSG_CHECKING([PROJ version agreement])
  PROJ_VERSION_DOT=`echo ${PROJ_VERSION} | tr -d "."`
#echo "|${PROJ_VERSION}| |${PROJ_VERSION_DOT}| |${PROJ_VERSION_H}|"
  if test "${PROJ_VERSION_DOT}$" = "${PROJ_VERSION_H}$"; then
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
    AC_MSG_ERROR([proj_api.h and pkg-config give different versions])
  fi
fi

fi

if test -z "${PROJ_VERSION_DOT}"; then
  PROJ_VERSION_DOT=`echo ${PROJ_VERSION_H} | tr -d "."`
fi
NEW_PROJ_WITH_OLD_GDAL="no"
AC_MSG_CHECKING([Using GDAL < 3 with PROJ >= 6])
if test ${GDAL_MAJOR} -lt 3 && test ${PROJ_VERSION_DOT} -ge 600; then
    AC_MSG_RESULT(yes)
    if test ${PROJ_API} = "proj.h"; then
        PROJ_API = "proj_api.h"
        AC_MSG_WARN([GDAL >= 3 should be used with PROJ >= 6, now using --with-proj_api="proj_api.h" for deprecated API])
    else
        NEW_PROJ_WITH_OLD_GDAL="yes"
        AC_MSG_WARN([GDAL >= 3 should be used with PROJ >= 6])
    fi
  else
    AC_MSG_RESULT(no)
fi



if test "${PROJ6}" = "yes" && test "${PROJ_API}" =  "proj_api.h"; then
    PKG_CPPFLAGS="${PKG_CPPFLAGS} -DACCEPT_USE_OF_DEPRECATED_PROJ_API_H"
fi
#AC_MSG_NOTICE([CPP FLAGS: ${PKG_CPPFLAGS}])

AC_MSG_CHECKING([PROJ version >= 4.8.0])
if test ${PROJ_VERSION_DOT} -lt 480 ; then
  AC_MSG_RESULT(no)
  AC_MSG_ERROR([upgrade PROJ to 4.8.0 or later])
else
  AC_MSG_RESULT(yes)
fi

if test ${PROJ_VERSION_DOT} -lt 493; then

if test ${PROJ_VERSION_DOT} = 480 ; then
  ADDED_PKG_INCL_480=`pwd`"/inst/include"
  AC_SUBST([PKG_CPPFLAGS],["${PKG_CPPFLAGS} -I${ADDED_PKG_INCL_480}"])
  proj_include_path="${PKG_CPPFLAGS} -I${ADDED_PKG_INCL_480}"
  CPPFLAGS="${INCPPFLAGS} ${PKG_CPPFLAGS}"
fi

fi


# Optional local copy of GDAL datadir and PROJ_LIB

data_copy=no
AC_ARG_WITH([data-copy],
    AC_HELP_STRING([--with-data-copy=yes/no],
               [local copy of data directories in package, default no]),
               [data_copy=$withval])
if test "${data_copy}" = "yes" ; then
AC_MSG_NOTICE([Copy data for:])
  proj_lib0="${PROJ_LIB}"
  AC_ARG_WITH([proj-data],
    AC_HELP_STRING([--with-proj-data=DIR],
                   [location of PROJ.4 data directory]),
    [proj_lib1=$withval])
  if test -n "${proj_lib0}" ; then
    proj_lib="${proj_lib0}"
  else
    proj_lib="${proj_lib1}"
  fi
  if test -n "${proj_lib}" ; then
    if test -d "${proj_lib}" ; then
      cp -r "${proj_lib}" "${R_PACKAGE_DIR}"
      AC_MSG_NOTICE([  PROJ.4: ${proj_lib}])
    else
      AC_MSG_ERROR([PROJ.4 data files not found; set environment variable PROJ_LIB=DIR or --with-proj-data=DIR.])
    fi
  else
      AC_MSG_ERROR([PROJ.4 data files not found; set environment variable PROJ_LIB=DIR or --with-proj-data=DIR.])
  fi

  if test -d "${GDAL_DATADIR}" ; then
    cp -r "${GDAL_DATADIR}" "${R_PACKAGE_DIR}"
    AC_MSG_NOTICE([  GDAL: ${GDAL_DATADIR}])
  else
    AC_MSG_ERROR([GDAL data files not found.])
  fi
fi



#AC_MSG_NOTICE([CPP FLAGS: ${PKG_CPPFLAGS}])

# concluding substitution

AC_SUBST([PKG_CPPFLAGS], ["${INPKG_CPPFLAGS} ${GDAL_CPPFLAGS}"])
#AC_MSG_NOTICE([CPP FLAGS: ${PKG_CPPFLAGS}])
if test [ -n "$proj_include_path" ] ; then
#AC_MSG_NOTICE([INC_PATH: ${proj_include_path}])
AC_SUBST([PKG_CPPFLAGS], ["${PKG_CPPFLAGS} ${proj_include_path}"])
fi

if test ${PROJ6} = "yes" ; then
  if test ${PROJ_API} = "proj.h"; then
    AC_SUBST([PKG_CPPFLAGS], ["${PKG_CPPFLAGS} -DPROJ_H_API"])
  else
    AC_SUBST([PKG_CPPFLAGS], ["${PKG_CPPFLAGS} -DACCEPT_USE_OF_DEPRECATED_PROJ_API_H"])
  fi
fi

if test ${NEW_PROJ_WITH_OLD_GDAL} = "yes" ; then
  AC_SUBST([PKG_CPPFLAGS], ["${PKG_CPPFLAGS} -DNEW_PROJ_WITH_OLD_GDAL"])
fi

AC_SUBST([PKG_LIBS], ["${INPKG_LIBS} ${GDAL_LIBS}"])
if test "${NEED_DEPS}" = yes; then
   AC_SUBST([PKG_LIBS], ["${PKG_LIBS} ${GDAL_DEP_LIBS}"])
fi
if test [ -n "$proj_lib_path" ] ; then
AC_SUBST([PKG_LIBS], ["${PKG_LIBS} -L${proj_lib_path}"])
fi
if test [ -n "$PROJ_LIB_PATH" ] ; then
AC_SUBST([PKG_LIBS], ["${PKG_LIBS} ${PROJ_LIB_PATH}"])
else
AC_SUBST([PKG_LIBS], ["${PKG_LIBS} -lproj"])
fi
AC_MSG_NOTICE([Package CPP flags: ${PKG_CPPFLAGS}])
AC_MSG_NOTICE([Package LIBS: ${PKG_LIBS}])

AC_CONFIG_FILES(src/Makevars)
AC_OUTPUT
