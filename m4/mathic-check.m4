AC_DEFUN([LB_CHECK_MATHICGB],
[


dnl ----- The TBB dependency
AC_ARG_WITH([tbb], AS_HELP_STRING(
    [--with-tbb], [use TBB, which is required for multithreading. The value
      detect, which is the default, enables TBB if it can be found and
      otherwise prints a warning and continues the build without
      multithreading support. TBB is not available for Cygwin (last checked
      March 2013).]
))
AS_IF([test "x$with_tbb" == "x"], [with_tbb="detect"])
AS_IF(
  [test "x$with_tbb" == "xdetect"],
  [PKG_CHECK_MODULES([TBB], [tbb], [with_tbb="yes"], [with_tbb="no";
    AC_MSG_WARN([TBB not detected. Compiling without multithreading and without precise timing.])
  ])],
  [test "x$with_tbb" == "xyes"], [PKG_CHECK_MODULES([TBB], [tbb])],    
  [test "x$with_tbb" == "xno"], [],
  [AC_MSG_ERROR([invalid value $with_tbb for with_tbb.])]
)
AS_IF([test "x$with_tbb" == "xno"], [TBB_CFLAGS="-DMATHICGB_NO_TBB"])

dnl ----- The librt dependency
dnl On Linux TBB calls clock_gettime, which requires librt, but librt is not
dnl linked in automatically. So we need to check for that.

dnl the first AC_LINK_IFELSE causes tests for lots of C++ related things,
dnl and these print out messages. So to avoid those messages appearing
dnl after "if librt..." and before the result for that test, we do an
dnl empty AC_LINK_IFELSE. Probably there is a better way.
AC_LINK_IFELSE([AC_LANG_SOURCE([[]])], [], [])

dnl We check if -lrt is necessary. We need librt if we are building with TBB,
dnl if linking and compiling works when linking with librt but it doesn't
dnl without linking with librt.
AS_IF([test "x$with_tbb" == "xyes"],
  [AC_MSG_CHECKING([if librt is needed to support TBB on this platform]);
  oldLIBS=$LIBS;
  LIBS="$TBB_LIBS -lrt $LIBS";
  oldCFLAGS=$CFLAGS;
  CFLAGS="$CFLAGS $TBB_CFLAGS";
  AC_LINK_IFELSE(
    [AC_LANG_PROGRAM(
      [[#]include [<]tbb/tbb.h[>]], [[tbb::tick_count::now();]]
    )],
    [LIBS=$oldLibs; AC_LINK_IFELSE(
      [AC_LANG_PROGRAM(
        [[#]include [<]tbb/tbb.h[>]], [[tbb::tick_count::now();]]
      )],
      [AC_MSG_RESULT([no])],
      [AC_MSG_RESULT([yes]); RT_LIBS="-lrt"]
    )],
    [AC_MSG_RESULT([no])]
  )];
  LIBS=$oldLIBS;
  CFLAGS=$oldCFLAGS;
)

#
 AC_ARG_WITH(mathicgb,
   [AS_HELP_STRING( 
     [--with-mathicgb=yes|no], 
     [Use the MathicGB library. Default is no.] 
   )],
   [], 
   [with_mathicgb="no"] 
 )
 #    
 # MathicGB
 dnl Checking these pre-requisites and adding them to libs is necessary
 dnl for some reason, at least on Cygwin.
 AS_IF( [test "x$with_mathicgb" = xyes], 
 [
  AC_LANG_PUSH([C++])
  AC_CHECK_LIB(memtailor, libmemtailorIsPresent, [],
    [AC_MSG_ERROR([Cannot find libmemtailor, which is required for MathicGB.])])
  AC_CHECK_LIB(mathic, libmathicIsPresent, [], 
    [AC_MSG_ERROR([Cannot find libmathic, which is required for MathicGB.])])
  AC_CHECK_LIB(mathicgb, libmathicgbIsPresent, [],
    [AC_MSG_ERROR([Cannot find the MathicGB library.])])
  AC_CHECK_HEADER([mathicgb.h])
  AC_LANG_POP([C++])
  AC_DEFINE(HAVE_MATHICGB,1,[Define if mathicgb is to be used])
#  AC_SUBST(HAVE_MATHICGB_VALUE, 1)  
 ])
 #
])
