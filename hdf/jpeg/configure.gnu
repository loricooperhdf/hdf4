#!/bin/sh

# Guess values for system-dependent variables and create Makefiles.
# Generated automatically using autoconf version 2.1 
# Copyright (C) 1992, 1993, 1994 Free Software Foundation, Inc.
#
# This configure script is free software; the Free Software Foundation
# gives unlimited permission to copy, distribute and modify it.

# Defaults:
ac_help=
ac_default_prefix=/usr/local
# Any additions from configure.in:
ac_help="$ac_help
  --enable-maxmem[=N]     enable use of temp files, set max mem usage to N MB"
ac_help="$ac_help
"

# Initialize some variables set by options.
# The variables have the same names as the options, with
# dashes changed to underlines.
build=NONE
cache_file=./config.cache
exec_prefix=NONE
host=NONE
no_create=
nonopt=NONE
no_recursion=
prefix=NONE
program_prefix=NONE
program_suffix=NONE
program_transform_name=s,x,x,
silent=
site=
srcdir=
target=NONE
verbose=
x_includes=NONE
x_libraries=NONE

# Initialize some other variables.
subdirs=

ac_prev=
for ac_option
do

  # If the previous option needs an argument, assign it.
  if test -n "$ac_prev"; then
    eval "$ac_prev=\$ac_option"
    ac_prev=
    continue
  fi

  case "$ac_option" in
  -*=*) ac_optarg=`echo "$ac_option" | sed 's/[-_a-zA-Z0-9]*=//'` ;;
  *) ac_optarg= ;;
  esac

  # Accept the important Cygnus configure options, so we can diagnose typos.

  case "$ac_option" in

  -build | --build | --buil | --bui | --bu | --b)
    ac_prev=build ;;
  -build=* | --build=* | --buil=* | --bui=* | --bu=* | --b=*)
    build="$ac_optarg" ;;

  -cache-file | --cache-file | --cache-fil | --cache-fi \
  | --cache-f | --cache- | --cache | --cach | --cac | --ca | --c)
    ac_prev=cache_file ;;
  -cache-file=* | --cache-file=* | --cache-fil=* | --cache-fi=* \
  | --cache-f=* | --cache-=* | --cache=* | --cach=* | --cac=* | --ca=* | --c=*)
    cache_file="$ac_optarg" ;;

  -disable-* | --disable-*)
    ac_feature=`echo $ac_option|sed -e 's/-*disable-//'`
    # Reject names that are not valid shell variable names.
    if test -n "`echo $ac_feature| sed 's/[-a-zA-Z0-9_]//g'`"; then
      { echo "configure: error: $ac_feature: invalid feature name" 1>&2; exit 1; }
    fi
    ac_feature=`echo $ac_feature| sed 's/-/_/g'`
    eval "enable_${ac_feature}=no" ;;

  -enable-* | --enable-*)
    ac_feature=`echo $ac_option|sed -e 's/-*enable-//' -e 's/=.*//'`
    # Reject names that are not valid shell variable names.
    if test -n "`echo $ac_feature| sed 's/[-_a-zA-Z0-9]//g'`"; then
      { echo "configure: error: $ac_feature: invalid feature name" 1>&2; exit 1; }
    fi
    ac_feature=`echo $ac_feature| sed 's/-/_/g'`
    case "$ac_option" in
      *=*) ;;
      *) ac_optarg=yes ;;
    esac
    eval "enable_${ac_feature}='$ac_optarg'" ;;

  -exec-prefix | --exec_prefix | --exec-prefix | --exec-prefi \
  | --exec-pref | --exec-pre | --exec-pr | --exec-p | --exec- \
  | --exec | --exe | --ex)
    ac_prev=exec_prefix ;;
  -exec-prefix=* | --exec_prefix=* | --exec-prefix=* | --exec-prefi=* \
  | --exec-pref=* | --exec-pre=* | --exec-pr=* | --exec-p=* | --exec-=* \
  | --exec=* | --exe=* | --ex=*)
    exec_prefix="$ac_optarg" ;;

  -gas | --gas | --ga | --g)
    # Obsolete; use --with-gas.
    with_gas=yes ;;

  -help | --help | --hel | --he)
    # Omit some internal or obsolete options to make the list less imposing.
    # This message is too long to be a string in the A/UX 3.1 sh.
    cat << EOF
Usage: configure [options] [host]
Options: [defaults in brackets after descriptions]
Configuration:
  --cache-file=FILE       cache test results in FILE
  --help                  print this message
  --no-create             do not create output files
  --quiet, --silent       do not print \`checking...' messages
  --version               print the version of autoconf that created configure
Directory and file names:
  --prefix=PREFIX         install architecture-independent files in PREFIX
                          [$ac_default_prefix]
  --exec-prefix=PREFIX    install architecture-dependent files in PREFIX
                          [same as prefix]
  --srcdir=DIR            find the sources in DIR [configure dir or ..]
  --program-prefix=PREFIX prepend PREFIX to installed program names
  --program-suffix=SUFFIX append SUFFIX to installed program names
  --program-transform-name=PROGRAM run sed PROGRAM on installed program names
Host type:
  --build=BUILD           configure for building on BUILD [BUILD=HOST]
  --host=HOST             configure for HOST [guessed]
  --target=TARGET         configure for TARGET [TARGET=HOST]
Features and packages:
  --disable-FEATURE       do not include FEATURE (same as --enable-FEATURE=no)
  --enable-FEATURE[=ARG]  include FEATURE [ARG=yes]
  --with-PACKAGE[=ARG]    use PACKAGE [ARG=yes]
  --without-PACKAGE       do not use PACKAGE (same as --with-PACKAGE=no)
  --x-includes=DIR        X include files are in DIR
  --x-libraries=DIR       X library files are in DIR
--enable and --with options recognized:$ac_help
EOF
    exit 0 ;;

  -host | --host | --hos | --ho)
    ac_prev=host ;;
  -host=* | --host=* | --hos=* | --ho=*)
    host="$ac_optarg" ;;

  -nfp | --nfp | --nf)
    # Obsolete; use --without-fp.
    with_fp=no ;;

  -no-create | --no-create | --no-creat | --no-crea | --no-cre \
  | --no-cr | --no-c)
    no_create=yes ;;

  -no-recursion | --no-recursion | --no-recursio | --no-recursi \
  | --no-recurs | --no-recur | --no-recu | --no-rec | --no-re | --no-r)
    no_recursion=yes ;;

  -prefix | --prefix | --prefi | --pref | --pre | --pr | --p)
    ac_prev=prefix ;;
  -prefix=* | --prefix=* | --prefi=* | --pref=* | --pre=* | --pr=* | --p=*)
    prefix="$ac_optarg" ;;

  -program-prefix | --program-prefix | --program-prefi | --program-pref \
  | --program-pre | --program-pr | --program-p)
    ac_prev=program_prefix ;;
  -program-prefix=* | --program-prefix=* | --program-prefi=* \
  | --program-pref=* | --program-pre=* | --program-pr=* | --program-p=*)
    program_prefix="$ac_optarg" ;;

  -program-suffix | --program-suffix | --program-suffi | --program-suff \
  | --program-suf | --program-su | --program-s)
    ac_prev=program_suffix ;;
  -program-suffix=* | --program-suffix=* | --program-suffi=* \
  | --program-suff=* | --program-suf=* | --program-su=* | --program-s=*)
    program_suffix="$ac_optarg" ;;

  -program-transform-name | --program-transform-name \
  | --program-transform-nam | --program-transform-na \
  | --program-transform-n | --program-transform- \
  | --program-transform | --program-transfor \
  | --program-transfo | --program-transf \
  | --program-trans | --program-tran \
  | --progr-tra | --program-tr | --program-t)
    ac_prev=program_transform_name ;;
  -program-transform-name=* | --program-transform-name=* \
  | --program-transform-nam=* | --program-transform-na=* \
  | --program-transform-n=* | --program-transform-=* \
  | --program-transform=* | --program-transfor=* \
  | --program-transfo=* | --program-transf=* \
  | --program-trans=* | --program-tran=* \
  | --progr-tra=* | --program-tr=* | --program-t=*)
    program_transform_name="$ac_optarg" ;;

  -q | -quiet | --quiet | --quie | --qui | --qu | --q \
  | -silent | --silent | --silen | --sile | --sil)
    silent=yes ;;

  -site | --site | --sit)
    ac_prev=site ;;
  -site=* | --site=* | --sit=*)
    site="$ac_optarg" ;;

  -srcdir | --srcdir | --srcdi | --srcd | --src | --sr)
    ac_prev=srcdir ;;
  -srcdir=* | --srcdir=* | --srcdi=* | --srcd=* | --src=* | --sr=*)
    srcdir="$ac_optarg" ;;

  -target | --target | --targe | --targ | --tar | --ta | --t)
    ac_prev=target ;;
  -target=* | --target=* | --targe=* | --targ=* | --tar=* | --ta=* | --t=*)
    target="$ac_optarg" ;;

  -v | -verbose | --verbose | --verbos | --verbo | --verb)
    verbose=yes ;;

  -version | --version | --versio | --versi | --vers)
    echo "configure generated by autoconf version 2.1"
    exit 0 ;;

  -with-* | --with-*)
    ac_package=`echo $ac_option|sed -e 's/-*with-//' -e 's/=.*//'`
    # Reject names that are not valid shell variable names.
    if test -n "`echo $ac_package| sed 's/[-_a-zA-Z0-9]//g'`"; then
      { echo "configure: error: $ac_package: invalid package name" 1>&2; exit 1; }
    fi
    ac_package=`echo $ac_package| sed 's/-/_/g'`
    case "$ac_option" in
      *=*) ;;
      *) ac_optarg=yes ;;
    esac
    eval "with_${ac_package}='$ac_optarg'" ;;

  -without-* | --without-*)
    ac_package=`echo $ac_option|sed -e 's/-*without-//'`
    # Reject names that are not valid shell variable names.
    if test -n "`echo $ac_package| sed 's/[-a-zA-Z0-9_]//g'`"; then
      { echo "configure: error: $ac_package: invalid package name" 1>&2; exit 1; }
    fi
    ac_package=`echo $ac_package| sed 's/-/_/g'`
    eval "with_${ac_package}=no" ;;

  --x)
    # Obsolete; use --with-x.
    with_x=yes ;;

  -x-includes | --x-includes | --x-include | --x-includ | --x-inclu \
  | --x-incl | --x-inc | --x-in | --x-i)
    ac_prev=x_includes ;;
  -x-includes=* | --x-includes=* | --x-include=* | --x-includ=* | --x-inclu=* \
  | --x-incl=* | --x-inc=* | --x-in=* | --x-i=*)
    x_includes="$ac_optarg" ;;

  -x-libraries | --x-libraries | --x-librarie | --x-librari \
  | --x-librar | --x-libra | --x-libr | --x-lib | --x-li | --x-l)
    ac_prev=x_libraries ;;
  -x-libraries=* | --x-libraries=* | --x-librarie=* | --x-librari=* \
  | --x-librar=* | --x-libra=* | --x-libr=* | --x-lib=* | --x-li=* | --x-l=*)
    x_libraries="$ac_optarg" ;;

  -*) { echo "configure: error: $ac_option: invalid option; use --help to show usage" 1>&2; exit 1; }
    ;;

  *=*)
    varname=`echo "$ac_option"|sed -e 's/=.*//'`
    # Reject names that aren't valid shell variable names.
    if test -n "`echo $varname| sed 's/[a-zA-Z0-9_]//g'`"; then
      { echo "configure: error: $varname: invalid shell variable name" 1>&2; exit 1; }
    fi
    val="`echo "$ac_option"|sed 's/[^=]*=//'`"
    test -n "$verbose" && echo "	setting shell variable $varname to $val"
    eval "$varname='$val'"
    eval "export $varname" ;;

  *) 
    if test -n "`echo $ac_option| sed 's/[-a-z0-9.]//g'`"; then
      echo "configure: warning: $ac_option: invalid host type" 1>&2
    fi
    if test "x$nonopt" != xNONE; then
      { echo "configure: error: can only configure for one host and one target at a time" 1>&2; exit 1; }
    fi
    nonopt="$ac_option"
    ;;

  esac
done

if test -n "$ac_prev"; then
  { echo "configure: error: missing argument to --`echo $ac_prev | sed 's/_/-/g'`" 1>&2; exit 1; }
fi

trap 'rm -fr conftest* confdefs* core $ac_clean_files; exit 1' 1 2 15

# File descriptor usage:
# 0 unused; standard input
# 1 file creation
# 2 errors and warnings
# 3 unused; some systems may open it to /dev/tty
# 4 checking for... messages and results
# 5 compiler messages saved in config.log
if test "$silent" = yes; then
  exec 4>/dev/null
else
  exec 4>&1
fi
exec 5>./config.log

echo "\
This file contains any messages produced by compilers while
running configure, to aid debugging if configure makes a mistake.
" 1>&5

# Strip out --no-create and --no-recursion so they do not pile up.
# Also quote any args containing shell metacharacters.
ac_configure_args=
for ac_arg
do
  case "$ac_arg" in
  -no-create | --no-create | --no-creat | --no-crea | --no-cre \
  | --no-cr | --no-c) ;;
  -no-recursion | --no-recursion | --no-recursio | --no-recursi \
  | --no-recurs | --no-recur | --no-recu | --no-rec | --no-re | --no-r) ;;
  *" "*|*"	"*|*[\[\]\~\#\$\^\&\*\(\)\{\}\\\|\;\<\>\?]*)
  ac_configure_args="$ac_configure_args '$ac_arg'" ;;
  *) ac_configure_args="$ac_configure_args $ac_arg" ;;
  esac
done

# NLS nuisances.
# Only set LANG and LC_ALL to C if already set.
# These must not be set unconditionally because not all systems understand
# e.g. LANG=C (notably SCO).
if test "${LC_ALL+set}" = set; then LC_ALL=C; export LC_ALL; fi
if test "${LANG+set}"   = set; then LANG=C;   export LANG;   fi

# confdefs.h avoids OS command line length limits that DEFS can exceed.
rm -rf conftest* confdefs.h
# AIX cpp loses on an empty file, so make sure it contains at least a newline.
echo > confdefs.h

# A filename unique to this package, relative to the directory that
# configure is in, which we can look for to find out if srcdir is correct.
ac_unique_file=jcmaster.c

# Find the source files, if location was not specified.
if test -z "$srcdir"; then
  ac_srcdir_defaulted=yes
  # Try the directory containing this script, then its parent.
  ac_prog=$0
  ac_confdir=`echo $ac_prog|sed 's%/[^/][^/]*$%%'`
  test "x$ac_confdir" = "x$ac_prog" && ac_confdir=.
  srcdir=$ac_confdir
  if test ! -r $srcdir/$ac_unique_file; then
    srcdir=..
  fi
else
  ac_srcdir_defaulted=no
fi
if test ! -r $srcdir/$ac_unique_file; then
  if test "$ac_srcdir_defaulted" = yes; then
    { echo "configure: error: can not find sources in $ac_confdir or .." 1>&2; exit 1; }
  else
    { echo "configure: error: can not find sources in $srcdir" 1>&2; exit 1; }
  fi
fi
srcdir=`echo "${srcdir}" | sed 's%\([^/]\)/*$%\1%'`

# Prefer explicitly selected file to automatically selected ones.
if test -z "$CONFIG_SITE"; then
  if test "x$prefix" != xNONE; then
    CONFIG_SITE="$prefix/share/config.site $prefix/etc/config.site"
  else
    CONFIG_SITE="$ac_default_prefix/share/config.site $ac_default_prefix/etc/config.site"
  fi
fi
for ac_site_file in $CONFIG_SITE; do
  if test -r "$ac_site_file"; then
    echo "loading site script $ac_site_file"
    . "$ac_site_file"
  fi
done


ac_ext=c
# CFLAGS is not in ac_cpp because -g, -O, etc. are not valid cpp options.
ac_cpp='$CPP $CPPFLAGS'
ac_compile='${CC-cc} $CFLAGS $CPPFLAGS conftest.$ac_ext -c 1>&5 2>&5'
ac_link='${CC-cc} $CFLAGS $CPPFLAGS $LDFLAGS conftest.$ac_ext -o conftest $LIBS 1>&5 2>&5'

if (echo "testing\c"; echo 1,2,3) | grep c >/dev/null; then
  # Stardent Vistra SVR4 grep lacks -e, says ghazi@caip.rutgers.edu.
  if (echo -n testing; echo 1,2,3) | sed s/-n/xn/ | grep xn >/dev/null; then
    ac_n= ac_c='
' ac_t='	'
  else
    ac_n=-n ac_c= ac_t=
  fi
else
  ac_n= ac_c='\c' ac_t=
fi



# Extract the first word of "gcc", so it can be a program name with args.
set dummy gcc; ac_word=$2
echo $ac_n "checking for $ac_word""... $ac_c" 1>&4
if eval "test \"`echo '${'ac_cv_prog_CC'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&4
else
  if test -n "$CC"; then
  ac_cv_prog_CC="$CC" # Let the user override the test.
else
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
  for ac_dir in $PATH; do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      ac_cv_prog_CC="gcc"
      break
    fi
  done
  IFS="$ac_save_ifs"
  test -z "$ac_cv_prog_CC" && ac_cv_prog_CC="cc"
fi
fi
CC="$ac_cv_prog_CC"
if test -n "$CC"; then
  echo "$ac_t""$CC" 1>&4
else
  echo "$ac_t""no" 1>&4
fi


echo $ac_n "checking whether we are using GNU C""... $ac_c" 1>&4
if eval "test \"`echo '${'ac_cv_prog_gcc'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&4
else
  cat > conftest.c <<EOF
#ifdef __GNUC__
  yes;
#endif
EOF
if ${CC-cc} -E conftest.c 2>&5 | egrep yes >/dev/null 2>&1; then
  ac_cv_prog_gcc=yes
else
  ac_cv_prog_gcc=no
fi
fi
echo "$ac_t""$ac_cv_prog_gcc" 1>&4
if test $ac_cv_prog_gcc = yes; then
  GCC=yes
else
  GCC=
fi
if test "${CFLAGS+set}" != set; then
  echo $ac_n "checking whether ${CC-cc} accepts -O""... $ac_c" 1>&4
if eval "test \"`echo '${'ac_cv_prog_cc_o'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&4
else
  echo 'int f(){ return 0; }' > conftest.c
if test -z "`${CC-cc} -O -c conftest.c 2>&1`"; then
  ac_cv_prog_cc_o=yes
else
  ac_cv_prog_cc_o=no
fi
rm -f conftest*

fi
  echo "$ac_t""$ac_cv_prog_cc_o" 1>&4
  if test $ac_cv_prog_cc_o = yes; then
    CFLAGS="-O"
  else
    CFLAGS=""
  fi
fi

echo $ac_n "checking how to run the C preprocessor""... $ac_c" 1>&4
# On Suns, sometimes $CPP names a directory.
if test -n "$CPP" && test -d "$CPP"; then
  CPP=
fi
if test -z "$CPP"; then
if eval "test \"`echo '${'ac_cv_prog_CPP'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&4
else
    # This must be in double quotes, not single quotes, because CPP may get
  # substituted into the Makefile and "${CC-cc}" will confuse make.
  CPP="${CC-cc} -E"
  # On the NeXT, cc -E runs the code through the compiler's parser,
  # not just through cpp.
  cat > conftest.$ac_ext <<EOF
#line 497 "configure"
#include "confdefs.h"
#include <assert.h>
Syntax Error
EOF
eval "$ac_cpp conftest.$ac_ext >/dev/null 2>conftest.out"
ac_err=`grep -v '^ *+' conftest.out`
if test -z "$ac_err"; then
  :
else
  echo "$ac_err" >&5
  rm -rf conftest*
  CPP="${CC-cc} -E -traditional-cpp"
  cat > conftest.$ac_ext <<EOF
#line 511 "configure"
#include "confdefs.h"
#include <assert.h>
Syntax Error
EOF
eval "$ac_cpp conftest.$ac_ext >/dev/null 2>conftest.out"
ac_err=`grep -v '^ *+' conftest.out`
if test -z "$ac_err"; then
  :
else
  echo "$ac_err" >&5
  rm -rf conftest*
  CPP=/lib/cpp
fi
rm -f conftest*
fi
rm -f conftest*
  ac_cv_prog_CPP="$CPP"
fi
fi
CPP="$ac_cv_prog_CPP"
echo "$ac_t""$CPP" 1>&4

# If we cannot run a trivial program, we must be cross compiling.
echo $ac_n "checking whether cross-compiling""... $ac_c" 1>&4
if eval "test \"`echo '${'ac_cv_c_cross'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&4
else
  if test "$cross_compiling" = yes; then
  ac_cv_cross=yes
else
cat > conftest.$ac_ext <<EOF
#line 543 "configure"
#include "confdefs.h"
main(){return(0);}
EOF
eval $ac_link
if test -s conftest && (./conftest; exit) 2>/dev/null; then
  ac_cv_c_cross=no
else
  ac_cv_c_cross=yes
fi
fi
rm -fr conftest*
fi
cross_compiling=$ac_cv_c_cross
echo "$ac_t""$ac_cv_c_cross" 1>&4

echo $ac_n "checking for function prototypes""... $ac_c" 1>&4
if eval "test \"`echo '${'ijg_cv_have_prototypes'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&4
else
  cat > conftest.$ac_ext <<EOF
#line 564 "configure"
#include "confdefs.h"

int testfunction (int arg1, int * arg2); /* check prototypes */
struct methods_struct {		/* check method-pointer declarations */
  int (*error_exit) (char *msgtext);
  int (*trace_message) (char *msgtext);
  int (*another_method) (void);
};
int testfunction (int arg1, int * arg2) /* check definitions */
{ return arg2[arg1]; }
int test2function (void)	/* check void arg list */
{ return 0; }

int main() { return 0; }
int t() {
 
; return 0; }
EOF
if eval $ac_compile; then
  rm -rf conftest*
  ijg_cv_have_prototypes=yes
else
  rm -rf conftest*
  ijg_cv_have_prototypes=no
fi
rm -f conftest*

fi
echo "$ac_t""$ijg_cv_have_prototypes" 1>&4
if test $ijg_cv_have_prototypes = yes; then
  cat >> confdefs.h <<\EOF
#define HAVE_PROTOTYPES 
EOF

else
  echo Your compiler does not seem to know about function prototypes.
  echo Perhaps it needs a special switch to enable ANSI C mode.
  echo If so, we recommend running configure like this:
  echo "   ./configure  CC='cc -switch'"
  echo where -switch is the proper switch.
fi
ac_safe=`echo "stddef.h" | tr './\055' '___'`
echo $ac_n "checking for stddef.h""... $ac_c" 1>&4
if eval "test \"`echo '${'ac_cv_header_$ac_safe'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&4
else
  cat > conftest.$ac_ext <<EOF
#line 612 "configure"
#include "confdefs.h"
#include <stddef.h>
EOF
eval "$ac_cpp conftest.$ac_ext >/dev/null 2>conftest.out"
ac_err=`grep -v '^ *+' conftest.out`
if test -z "$ac_err"; then
  rm -rf conftest*
  eval "ac_cv_header_$ac_safe=yes"
else
  echo "$ac_err" >&5
  rm -rf conftest*
  eval "ac_cv_header_$ac_safe=no"
fi
rm -f conftest*
fi
if eval "test \"`echo '$ac_cv_header_'$ac_safe`\" = yes"; then
  echo "$ac_t""yes" 1>&4
  cat >> confdefs.h <<\EOF
#define HAVE_STDDEF_H 
EOF

else
  echo "$ac_t""no" 1>&4
fi

ac_safe=`echo "stdlib.h" | tr './\055' '___'`
echo $ac_n "checking for stdlib.h""... $ac_c" 1>&4
if eval "test \"`echo '${'ac_cv_header_$ac_safe'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&4
else
  cat > conftest.$ac_ext <<EOF
#line 644 "configure"
#include "confdefs.h"
#include <stdlib.h>
EOF
eval "$ac_cpp conftest.$ac_ext >/dev/null 2>conftest.out"
ac_err=`grep -v '^ *+' conftest.out`
if test -z "$ac_err"; then
  rm -rf conftest*
  eval "ac_cv_header_$ac_safe=yes"
else
  echo "$ac_err" >&5
  rm -rf conftest*
  eval "ac_cv_header_$ac_safe=no"
fi
rm -f conftest*
fi
if eval "test \"`echo '$ac_cv_header_'$ac_safe`\" = yes"; then
  echo "$ac_t""yes" 1>&4
  cat >> confdefs.h <<\EOF
#define HAVE_STDLIB_H 
EOF

else
  echo "$ac_t""no" 1>&4
fi

ac_safe=`echo "string.h" | tr './\055' '___'`
echo $ac_n "checking for string.h""... $ac_c" 1>&4
if eval "test \"`echo '${'ac_cv_header_$ac_safe'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&4
else
  cat > conftest.$ac_ext <<EOF
#line 676 "configure"
#include "confdefs.h"
#include <string.h>
EOF
eval "$ac_cpp conftest.$ac_ext >/dev/null 2>conftest.out"
ac_err=`grep -v '^ *+' conftest.out`
if test -z "$ac_err"; then
  rm -rf conftest*
  eval "ac_cv_header_$ac_safe=yes"
else
  echo "$ac_err" >&5
  rm -rf conftest*
  eval "ac_cv_header_$ac_safe=no"
fi
rm -f conftest*
fi
if eval "test \"`echo '$ac_cv_header_'$ac_safe`\" = yes"; then
  echo "$ac_t""yes" 1>&4
  :
else
  echo "$ac_t""no" 1>&4
cat >> confdefs.h <<\EOF
#define NEED_BSD_STRINGS 
EOF

fi

echo $ac_n "checking for size_t""... $ac_c" 1>&4
cat > conftest.$ac_ext <<EOF
#line 705 "configure"
#include "confdefs.h"

#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <stdio.h>
#ifdef NEED_BSD_STRINGS
#include <strings.h>
#else
#include <string.h>
#endif
typedef size_t my_size_t;

int main() { return 0; }
int t() {
 my_size_t foovar; 
; return 0; }
EOF
if eval $ac_compile; then
  rm -rf conftest*
  ijg_size_t_ok=yes
else
  rm -rf conftest*
  ijg_size_t_ok="not ANSI, perhaps it is in sys/types.h"
fi
rm -f conftest*

echo "$ac_t""$ijg_size_t_ok" 1>&4
if test "$ijg_size_t_ok" != yes; then
ac_safe=`echo "sys/types.h" | tr './\055' '___'`
echo $ac_n "checking for sys/types.h""... $ac_c" 1>&4
if eval "test \"`echo '${'ac_cv_header_$ac_safe'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&4
else
  cat > conftest.$ac_ext <<EOF
#line 744 "configure"
#include "confdefs.h"
#include <sys/types.h>
EOF
eval "$ac_cpp conftest.$ac_ext >/dev/null 2>conftest.out"
ac_err=`grep -v '^ *+' conftest.out`
if test -z "$ac_err"; then
  rm -rf conftest*
  eval "ac_cv_header_$ac_safe=yes"
else
  echo "$ac_err" >&5
  rm -rf conftest*
  eval "ac_cv_header_$ac_safe=no"
fi
rm -f conftest*
fi
if eval "test \"`echo '$ac_cv_header_'$ac_safe`\" = yes"; then
  echo "$ac_t""yes" 1>&4
  cat >> confdefs.h <<\EOF
#define NEED_SYS_TYPES_H 
EOF

cat > conftest.$ac_ext <<EOF
#line 767 "configure"
#include "confdefs.h"
#include <sys/types.h>
EOF
if (eval "$ac_cpp conftest.$ac_ext") 2>&5 |
  egrep "size_t" >/dev/null 2>&1; then
  rm -rf conftest*
  ijg_size_t_ok="size_t is in sys/types.h"
else
  rm -rf conftest*
  ijg_size_t_ok=no
fi
rm -f conftest*

else
  echo "$ac_t""no" 1>&4
ijg_size_t_ok=no
fi

echo "$ac_t""$ijg_size_t_ok" 1>&4
if test "$ijg_size_t_ok" = no; then
  echo Type size_t is not defined in any of the usual places.
  echo Try putting '"typedef unsigned int size_t;"' in jconfig.h.
fi
fi
echo $ac_n "checking for type unsigned char""... $ac_c" 1>&4
cat > conftest.$ac_ext <<EOF
#line 794 "configure"
#include "confdefs.h"

int main() { return 0; }
int t() {
 unsigned char un_char; 
; return 0; }
EOF
if eval $ac_compile; then
  rm -rf conftest*
  echo "$ac_t""yes" 1>&4
cat >> confdefs.h <<\EOF
#define HAVE_UNSIGNED_CHAR 
EOF

else
  rm -rf conftest*
  echo "$ac_t""no" 1>&4
fi
rm -f conftest*
echo $ac_n "checking for type unsigned short""... $ac_c" 1>&4
cat > conftest.$ac_ext <<EOF
#line 816 "configure"
#include "confdefs.h"

int main() { return 0; }
int t() {
 unsigned short un_short; 
; return 0; }
EOF
if eval $ac_compile; then
  rm -rf conftest*
  echo "$ac_t""yes" 1>&4
cat >> confdefs.h <<\EOF
#define HAVE_UNSIGNED_SHORT 
EOF

else
  rm -rf conftest*
  echo "$ac_t""no" 1>&4
fi
rm -f conftest*
echo $ac_n "checking for type void""... $ac_c" 1>&4
cat > conftest.$ac_ext <<EOF
#line 838 "configure"
#include "confdefs.h"

/* Caution: a C++ compiler will insist on valid prototypes */
typedef void * void_ptr;	/* check void * */
#ifdef HAVE_PROTOTYPES		/* check ptr to function returning void */
typedef void (*void_func) (int a, int b);
#else
typedef void (*void_func) ();
#endif

#ifdef HAVE_PROTOTYPES		/* check void function result */
void test3function (void_ptr arg1, void_func arg2)
#else
void test3function (arg1, arg2)
     void_ptr arg1;
     void_func arg2;
#endif
{
  char * locptr = (char *) arg1; /* check casting to and from void * */
  arg1 = (void *) locptr;
  (*arg2) (1, 2);		/* check call of fcn returning void */
}

int main() { return 0; }
int t() {
 
; return 0; }
EOF
if eval $ac_compile; then
  rm -rf conftest*
  echo "$ac_t""yes" 1>&4
else
  rm -rf conftest*
  echo "$ac_t""no" 1>&4
cat >> confdefs.h <<\EOF
#define void char
EOF

fi
rm -f conftest*
echo $ac_n "checking for working const""... $ac_c" 1>&4
if eval "test \"`echo '${'ac_cv_c_const'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&4
else
  cat > conftest.$ac_ext <<EOF
#line 884 "configure"
#include "confdefs.h"

int main() { return 0; }
int t() {

/* Ultrix mips cc rejects this.  */
typedef int charset[2]; const charset x;
/* SunOS 4.1.1 cc rejects this.  */
char const *const *ccp;
char **p;
/* NEC SVR4.0.2 mips cc rejects this.  */
struct point {int x, y;};
static struct point const zero;
/* AIX XL C 1.02.0.0 rejects this.
   It does not let you subtract one const X* pointer from another in an arm
   of an if-expression whose if-part is not a constant expression */
const char *g = "string";
ccp = &g + (g ? g-g : 0);
/* HPUX 7.0 cc rejects these. */
++ccp;
p = (char**) ccp;
ccp = (char const *const *) p;
{ /* SCO 3.2v4 cc rejects this.  */
  char *t;
  char const *s = 0 ? (char *) 0 : (char const *) 0;

  *t++ = 0;
}
{ /* Someone thinks the Sun supposedly-ANSI compiler will reject this.  */
  int x[] = {25, 17};
  const int *foo = &x[0];
  ++foo;
}
{ /* Sun SC1.0 ANSI compiler rejects this -- but not the above. */
  typedef const int *iptr;
  iptr p = 0;
  ++p;
}
{ /* AIX XL C 1.02.0.0 rejects this saying
     "k.c", line 2.27: 1506-025 (S) Operand must be a modifiable lvalue. */
  struct s { int j; const int *ap[3]; };
  struct s *b; b->j = 5;
}
{ /* ULTRIX-32 V3.1 (Rev 9) vcc rejects this */
  const int foo = 10;
}

; return 0; }
EOF
if eval $ac_compile; then
  rm -rf conftest*
  ac_cv_c_const=yes
else
  rm -rf conftest*
  ac_cv_c_const=no
fi
rm -f conftest*

fi
echo "$ac_t""$ac_cv_c_const" 1>&4
if test $ac_cv_c_const = no; then
  cat >> confdefs.h <<\EOF
#define const 
EOF

fi

echo $ac_n "checking for broken incomplete types""... $ac_c" 1>&4
cat > conftest.$ac_ext <<EOF
#line 1011 "configure"
#include "confdefs.h"
 typedef struct undefined_structure * undef_struct_ptr; 
int main() { return 0; }
int t() {

; return 0; }
EOF
if eval $ac_compile; then
  rm -rf conftest*
  echo "$ac_t""ok" 1>&4
else
  rm -rf conftest*
  echo "$ac_t""broken" 1>&4
cat >> confdefs.h <<\EOF
#define INCOMPLETE_TYPES_BROKEN 
EOF

fi
rm -f conftest*
echo $ac_n "checking for short external names""... $ac_c" 1>&4
cat > conftest.$ac_ext <<EOF
#line 1033 "configure"
#include "confdefs.h"

int possibly_duplicate_function () { return 0; }
int possibly_dupli_function () { return 1; }

int main() { return 0; }
int t() {
 
; return 0; }
EOF
if eval $ac_link; then
  rm -rf conftest*
  echo "$ac_t""ok" 1>&4
else
  rm -rf conftest*
  echo "$ac_t""short" 1>&4
cat >> confdefs.h <<\EOF
#define NEED_SHORT_EXTERNAL_NAMES 
EOF

fi
rm -f conftest*

echo $ac_n "checking to see if char is signed""... $ac_c" 1>&4
if test "$cross_compiling" = yes; then
  echo Assuming that char is signed on target machine.
echo If it is unsigned, this will be a little bit inefficient.

else
cat > conftest.$ac_ext <<EOF
#line 1064 "configure"
#include "confdefs.h"

#ifdef HAVE_PROTOTYPES
int is_char_signed (int arg)
#else
int is_char_signed (arg)
     int arg;
#endif
{
  if (arg == 189) {		/* expected result for unsigned char */
    return 0;			/* type char is unsigned */
  }
  else if (arg != -67) {	/* expected result for signed char */
    printf("Hmm, it seems 'char' is not eight bits wide on your machine.\n");
    printf("I fear the JPEG software will not work at all.\n\n");
  }
  return 1;			/* assume char is signed otherwise */
}
char signed_char_check = (char) (-67);
main() {
  exit(is_char_signed((int) signed_char_check));
}
EOF
eval $ac_link
if test -s conftest && (./conftest; exit) 2>/dev/null; then
  echo "$ac_t""no" 1>&4
cat >> confdefs.h <<\EOF
#define CHAR_IS_UNSIGNED 
EOF

else
  echo "$ac_t""yes" 1>&4
fi
fi
rm -fr conftest*
echo $ac_n "checking to see if right shift is signed""... $ac_c" 1>&4
if test "$cross_compiling" = yes; then
  echo "$ac_t""Assuming that right shift is signed on target machine." 1>&4
else
cat > conftest.$ac_ext <<EOF
#line 1105 "configure"
#include "confdefs.h"

#ifdef HAVE_PROTOTYPES
int is_shifting_signed (long arg)
#else
int is_shifting_signed (arg)
     long arg;
#endif
/* See whether right-shift on a long is signed or not. */
{
  long res = arg >> 4;

  if (res == -0x7F7E80CL) {	/* expected result for signed shift */
    return 1;			/* right shift is signed */
  }
  /* see if unsigned-shift hack will fix it. */
  /* we can't just test exact value since it depends on width of long... */
  res |= (~0L) << (32-4);
  if (res == -0x7F7E80CL) {	/* expected result now? */
    return 0;			/* right shift is unsigned */
  }
  printf("Right shift isn't acting as I expect it to.\n");
  printf("I fear the JPEG software will not work at all.\n\n");
  return 0;			/* try it with unsigned anyway */
}
main() {
  exit(is_shifting_signed(-0x7F7E80B1L));
}
EOF
eval $ac_link
if test -s conftest && (./conftest; exit) 2>/dev/null; then
  echo "$ac_t""no" 1>&4
cat >> confdefs.h <<\EOF
#define RIGHT_SHIFT_IS_UNSIGNED 
EOF

else
  echo "$ac_t""yes" 1>&4
fi
fi
rm -fr conftest*
echo $ac_n "checking to see if fopen accepts b spec""... $ac_c" 1>&4
if test "$cross_compiling" = yes; then
  echo "$ac_t""Assuming that it does." 1>&4
else
cat > conftest.$ac_ext <<EOF
#line 1152 "configure"
#include "confdefs.h"

#include <stdio.h>
main() {
  if (fopen("conftestdata", "wb") != NULL)
    exit(0);
  exit(1);
}
EOF
eval $ac_link
if test -s conftest && (./conftest; exit) 2>/dev/null; then
  echo "$ac_t""yes" 1>&4
else
  echo "$ac_t""no" 1>&4
cat >> confdefs.h <<\EOF
#define DONT_USE_B_MODE 
EOF

fi
fi
rm -fr conftest*

# Find a good install program.  We prefer a C program (faster),
# so one script is as good as another.  But avoid the broken or
# incompatible versions:
# SysV /etc/install, /usr/sbin/install
# SunOS /usr/etc/install
# IRIX /sbin/install
# AIX /bin/install
# AFS /usr/afsws/bin/install, which mishandles nonexistent args
# SVR4 /usr/ucb/install, which tries to use the nonexistent group "staff"
# ./install, which can be erroneously created by make from ./install.sh.
echo $ac_n "checking for a BSD compatible install""... $ac_c" 1>&4
if test -z "$INSTALL"; then
if eval "test \"`echo '${'ijg_cv_path_install'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&4
else
    IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
  for ac_dir in $PATH; do
    case "$ac_dir" in
    ''|.|/etc|/usr/sbin|/usr/etc|/sbin|/usr/afsws/bin|/usr/ucb) ;;
    *)
      # OSF1 and SCO ODT 3.0 have their own names for install.
      for ac_prog in ginstall installbsd scoinst install; do
        if test -f $ac_dir/$ac_prog; then
	  if test $ac_prog = install &&
            grep dspmsg $ac_dir/$ac_prog >/dev/null 2>&1; then
	    # AIX install.  It has an incompatible calling convention.
	    # OSF/1 installbsd also uses dspmsg, but is usable.
	    :
	  else
	    ijg_cv_path_install="$ac_dir/$ac_prog -c"
	    break 2
	  fi
	fi
      done
      ;;
    esac
  done
  IFS="$ac_save_ifs"
  # As a last resort, use cp
  test -z "$ijg_cv_path_install" && ijg_cv_path_install="cp"
fi
  INSTALL="$ijg_cv_path_install"
fi
echo "$ac_t""$INSTALL" 1>&4

# Use test -z because SunOS4 sh mishandles braces in ${var-val}.
# It thinks the first close brace ends the variable substitution.
test -z "$INSTALL_PROGRAM" && INSTALL_PROGRAM='$(INSTALL)'

if test -z "$INSTALL_DATA"; then
  if test "$INSTALL" = cp; then
    INSTALL_DATA='$(INSTALL)'
  else
    INSTALL_DATA='$(INSTALL) -m 644'
  fi
fi

# Extract the first word of "ranlib", so it can be a program name with args.
set dummy ranlib; ac_word=$2
echo $ac_n "checking for $ac_word""... $ac_c" 1>&4
if eval "test \"`echo '${'ac_cv_prog_RANLIB'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&4
else
  if test -n "$RANLIB"; then
  ac_cv_prog_RANLIB="$RANLIB" # Let the user override the test.
else
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
  for ac_dir in $PATH; do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      ac_cv_prog_RANLIB="ranlib"
      break
    fi
  done
  IFS="$ac_save_ifs"
  test -z "$ac_cv_prog_RANLIB" && ac_cv_prog_RANLIB=":"
fi
fi
RANLIB="$ac_cv_prog_RANLIB"
if test -n "$RANLIB"; then
  echo "$ac_t""$RANLIB" 1>&4
else
  echo "$ac_t""no" 1>&4
fi

MEMORYMGR="jmemnobs.o"
MAXMEM="no"
# Check whether --enable-maxmem or --disable-maxmem was given.
enableval="$enable_maxmem"
if test -n "$enableval"; then
  MAXMEM="$enableval"
fi

# support --with-maxmem for backwards compatibility with IJG V5.
# Check whether --with-maxmem or --without-maxmem was given.
withval="$with_maxmem"
if test -n "$withval"; then
  MAXMEM="$withval"
fi

if test "x$MAXMEM" = xyes; then
  MAXMEM=1
fi
if test "x$MAXMEM" != xno; then
  if test -n "`echo $MAXMEM | sed 's/[0-9]//g'`"; then
    { echo "configure: error: non-numeric argument to --enable-maxmem" 1>&2; exit 1; }
  fi
  DEFAULTMAXMEM=`expr $MAXMEM \* 1048576`
cat >> confdefs.h <<EOF
#define DEFAULT_MAX_MEM ${DEFAULTMAXMEM}
EOF

echo $ac_n "checking for 'tmpfile()'""... $ac_c" 1>&4
cat > conftest.$ac_ext <<EOF
#line 1289 "configure"
#include "confdefs.h"
#include <stdio.h>
int main() { return 0; }
int t() {
 FILE * tfile = tmpfile(); 
; return 0; }
EOF
if eval $ac_link; then
  rm -rf conftest*
  echo "$ac_t""yes" 1>&4
MEMORYMGR="jmemansi.o"
else
  rm -rf conftest*
  echo "$ac_t""no" 1>&4
MEMORYMGR="jmemname.o"
cat >> confdefs.h <<\EOF
#define NEED_SIGNAL_CATCHER 
EOF
echo $ac_n "checking for 'mktemp()'""... $ac_c" 1>&4
cat > conftest.$ac_ext <<EOF
#line 1310 "configure"
#include "confdefs.h"

int main() { return 0; }
int t() {
 char fname[80]; mktemp(fname); 
; return 0; }
EOF
if eval $ac_link; then
  rm -rf conftest*
  echo "$ac_t""yes" 1>&4
else
  rm -rf conftest*
  echo "$ac_t""no" 1>&4
cat >> confdefs.h <<\EOF
#define NO_MKTEMP 
EOF

fi
rm -f conftest*

fi
rm -f conftest*

fi

# Prepare to massage makefile.cfg correctly.
if test $ijg_cv_have_prototypes = yes; then
  ANSI2KNR=""
  ISANSICOM="# "
else
  ANSI2KNR="ansi2knr"
  ISANSICOM=""
fi
# ansi2knr needs -DBSD if string.h is missing
if test $ac_cv_header_string_h = no; then
  ANSI2KNRFLAGS="-DBSD"
else
  ANSI2KNRFLAGS=""
fi
# Set up -I directives
if test "x$srcdir" = x.; then
  INCLUDEFLAGS='-I$(srcdir)'
else
  INCLUDEFLAGS='-I. -I$(srcdir)'
fi
trap '' 1 2 15

trap 'rm -fr conftest* confdefs* core $ac_clean_files; exit 1' 1 2 15

test "x$prefix" = xNONE && prefix=$ac_default_prefix
# Let make expand exec_prefix.
test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'

# Any assignment to VPATH causes Sun make to only execute
# the first set of double-colon rules, so remove it if not needed.
# If there is a colon in the path, we need to keep it.
if test "x$srcdir" = x.; then
  ac_vpsub='/^[ 	]*VPATH[ 	]*=[^:]*$/d'
fi

trap 'rm -f $CONFIG_STATUS conftest*; exit 1' 1 2 15

DEFS=-DHAVE_CONFIG_H

# Without the "./", some shells look in PATH for config.status.
: ${CONFIG_STATUS=./config.status}

echo creating $CONFIG_STATUS
rm -f $CONFIG_STATUS
cat > $CONFIG_STATUS <<EOF
#!/bin/sh
# Generated automatically by configure.
# Run this file to recreate the current configuration.
# This directory was configured as follows,
# on host `(hostname || uname -n) 2>/dev/null | sed 1q`:
#
# $0 $ac_configure_args
#
# Compiler output produced by configure, useful for debugging
# configure, is in ./config.log if it exists.

ac_cs_usage="Usage: $CONFIG_STATUS [--recheck] [--version] [--help]"
for ac_option
do
  case "\$ac_option" in
  -recheck | --recheck | --rechec | --reche | --rech | --rec | --re | --r)
    echo "running \${CONFIG_SHELL-/bin/sh} $0 $ac_configure_args --no-create --no-recursion"
    exec \${CONFIG_SHELL-/bin/sh} $0 $ac_configure_args --no-create --no-recursion ;;
  -version | --version | --versio | --versi | --vers | --ver | --ve | --v)
    echo "$CONFIG_STATUS generated by autoconf version 2.1"
    exit 0 ;;
  -help | --help | --hel | --he | --h)
    echo "\$ac_cs_usage"; exit 0 ;;
  *) echo "\$ac_cs_usage"; exit 1 ;;
  esac
done

ac_given_srcdir=$srcdir

trap 'rm -fr Makefile:makefile.cfg jconfig.h:jconfig.cfg conftest*; exit 1' 1 2 15

# Protect against being on the right side of a sed subst in config.status. 
sed 's/%@/@@/; s/@%/@@/; s/%g$/@g/; /@g$/s/[\\\\&%]/\\\\&/g; 
 s/@@/%@/; s/@@/@%/; s/@g$/%g/' > conftest.subs <<\CEOF
$ac_vpsub
$extrasub
s%@CFLAGS@%$CFLAGS%g
s%@CPPFLAGS@%$CPPFLAGS%g
s%@CXXFLAGS@%$CXXFLAGS%g
s%@DEFS@%$DEFS%g
s%@LDFLAGS@%$LDFLAGS%g
s%@LIBS@%$LIBS%g
s%@exec_prefix@%$exec_prefix%g
s%@prefix@%$prefix%g
s%@program_transform_name@%$program_transform_name%g
s%@CC@%$CC%g
s%@CPP@%$CPP%g
s%@INSTALL@%$INSTALL%g
s%@INSTALL_PROGRAM@%$INSTALL_PROGRAM%g
s%@INSTALL_DATA@%$INSTALL_DATA%g
s%@RANLIB@%$RANLIB%g
s%@MEMORYMGR@%$MEMORYMGR%g
s%@ANSI2KNR@%$ANSI2KNR%g
s%@ISANSICOM@%$ISANSICOM%g
s%@ANSI2KNRFLAGS@%$ANSI2KNRFLAGS%g
s%@INCLUDEFLAGS@%$INCLUDEFLAGS%g

CEOF
EOF
cat >> $CONFIG_STATUS <<EOF

CONFIG_FILES=\${CONFIG_FILES-"Makefile:makefile.cfg"}
EOF
cat >> $CONFIG_STATUS <<\EOF
for ac_file in .. $CONFIG_FILES; do if test "x$ac_file" != x..; then
  # Support "outfile[:infile]", defaulting infile="outfile.in".
  case "$ac_file" in
  *:*) ac_file_in=`echo "$ac_file"|sed 's%.*:%%'`
       ac_file=`echo "$ac_file"|sed 's%:.*%%'` ;;
  *) ac_file_in="${ac_file}.in" ;;
  esac

  # Adjust relative srcdir, etc. for subdirectories.

  # Remove last slash and all that follows it.  Not all systems have dirname.
  ac_dir=`echo $ac_file|sed 's%/[^/][^/]*$%%'`
  if test "$ac_dir" != "$ac_file" && test "$ac_dir" != .; then
    # The file is in a subdirectory.
    test ! -d "$ac_dir" && mkdir "$ac_dir"
    ac_dir_suffix="/$ac_dir"
    # A "../" for each directory in $ac_dir_suffix.
    ac_dots=`echo $ac_dir_suffix|sed 's%/[^/]*%../%g'`
  else
    ac_dir_suffix= ac_dots=
  fi

  case "$ac_given_srcdir" in
  .)  srcdir=.
      if test -z "$ac_dots"; then top_srcdir=.
      else top_srcdir=`echo $ac_dots|sed 's%/$%%'`; fi ;;
  /*) srcdir="$ac_given_srcdir$ac_dir_suffix"; top_srcdir="$ac_given_srcdir" ;;
  *) # Relative path.
    srcdir="$ac_dots$ac_given_srcdir$ac_dir_suffix"
    top_srcdir="$ac_dots$ac_given_srcdir" ;;
  esac

  echo creating "$ac_file"
  rm -f "$ac_file"
  configure_input="Generated automatically from `echo $ac_file_in|sed 's%.*/%%'` by configure."
  case "$ac_file" in
  *Makefile*) ac_comsub="1i\\
# $configure_input" ;;
  *) ac_comsub= ;;
  esac
  sed -e "$ac_comsub
s%@configure_input@%$configure_input%g
s%@srcdir@%$srcdir%g
s%@top_srcdir@%$top_srcdir%g
" -f conftest.subs $ac_given_srcdir/$ac_file_in > $ac_file
fi; done
rm -f conftest.subs

# These sed commands are passed to sed as "A NAME B NAME C VALUE D", where
# NAME is the cpp macro being defined and VALUE is the value it is being given.
#
# ac_d sets the value in "#define NAME VALUE" lines.
ac_dA='s%^\([ 	]*\)#\([ 	]*define[ 	][ 	]*\)'
ac_dB='\([ 	][ 	]*\)[^ 	]*%\1#\2'
ac_dC='\3'
ac_dD='%g'
# ac_u turns "#undef NAME" with trailing blanks into "#define NAME VALUE".
ac_uA='s%^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)'
ac_uB='\([ 	]\)%\1#\2define\3'
ac_uC=' '
ac_uD='\4%g'
# ac_e turns "#undef NAME" without trailing blanks into "#define NAME VALUE".
ac_eA='s%^\([ 	]*\)#\([ 	]*\)undef\([ 	][ 	]*\)'
ac_eB='$%\1#\2define\3'
ac_eC=' '
ac_eD='%g'

CONFIG_HEADERS=${CONFIG_HEADERS-"jconfig.h:jconfig.cfg"}
for ac_file in .. $CONFIG_HEADERS; do if test "x$ac_file" != x..; then
  # Support "outfile[:infile]", defaulting infile="outfile.in".
  case "$ac_file" in
  *:*) ac_file_in=`echo "$ac_file"|sed 's%.*:%%'`
       ac_file=`echo "$ac_file"|sed 's%:.*%%'` ;;
  *) ac_file_in="${ac_file}.in" ;;
  esac

  echo creating $ac_file

  rm -f conftest.frag conftest.in conftest.out
  cp $ac_given_srcdir/$ac_file_in conftest.in

EOF

# Transform confdefs.h into a sed script conftest.vals that substitutes
# the proper values into config.h.in to produce config.h.  And first:
# Protect against being on the right side of a sed subst in config.status. 
# Protect against being in an unquoted here document in config.status.
rm -f conftest.vals
cat > conftest.hdr <<\EOF
s/[\\&%]/\\&/g
s%[\\$`]%\\&%g
s%#define \([A-Za-z_][A-Za-z0-9_]*\) \(.*\)%${ac_dA}\1${ac_dB}\1${ac_dC}\2${ac_dD}%gp
s%ac_d%ac_u%gp
s%ac_u%ac_e%gp
EOF
sed -n -f conftest.hdr confdefs.h > conftest.vals
rm -f conftest.hdr

# This sed command replaces #undef with comments.  This is necessary, for
# example, in the case of _POSIX_SOURCE, which is predefined and required
# on some systems where configure will not decide to define it.
cat >> conftest.vals <<\EOF
EOF

# Break up conftest.vals because some shells have a limit on
# the size of here documents, and old seds have small limits too.
# Maximum number of lines to put in a single here document.
ac_max_here_lines=12

rm -f conftest.tail
while :
do
  ac_lines=`grep -c . conftest.vals`
  # grep -c gives empty output for an empty file on some AIX systems.
  if test -z "$ac_lines" || test "$ac_lines" -eq 0; then break; fi
  # Write a limited-size here document to conftest.frag.
  echo '  cat > conftest.frag <<CEOF' >> $CONFIG_STATUS
  sed ${ac_max_here_lines}q conftest.vals >> $CONFIG_STATUS
  echo 'CEOF
  sed -f conftest.frag conftest.in > conftest.out
  rm -f conftest.in
  mv conftest.out conftest.in
' >> $CONFIG_STATUS
  sed 1,${ac_max_here_lines}d conftest.vals > conftest.tail
  rm -f conftest.vals
  mv conftest.tail conftest.vals
done
rm -f conftest.vals

cat >> $CONFIG_STATUS <<\EOF
  rm -f conftest.frag conftest.h
  echo "/* $ac_file.  Generated automatically by configure.  */" > conftest.h
  cat conftest.in >> conftest.h
  rm -f conftest.in
  if cmp -s $ac_file conftest.h 2>/dev/null; then
    echo "$ac_file is unchanged"
    rm -f conftest.h
  else
    rm -f $ac_file
    mv conftest.h $ac_file
  fi
fi; done



exit 0
EOF
chmod +x $CONFIG_STATUS
rm -fr confdefs* $ac_clean_files
test "$no_create" = yes || ${CONFIG_SHELL-/bin/sh} $CONFIG_STATUS

