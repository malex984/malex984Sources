ACLOCAL_AMFLAGS = -I ../../m4

MYINCLUDES =  -I${top_srcdir} -I${top_builddir} \
-I${top_srcdir}/libpolys -I${top_builddir}/libpolys \
$(FACTORY_INCLUDES) $(RESOURCES_INCLUDES) $(OMALLOC_INCLUDES) \
$(FLINT_CFLAGS) $(NTL_CFLAGS) $(GMP_CFLAGS)

if SI_BUILTIN_DEMO
  noinst_LTLIBRARIES=demo.la
  P_PROCS_CPPFLAGS_COMMON = -DSTATIC_VERSION 
  P_PROCS_MODULE_LDFLAGS = -module
else
  module_LTLIBRARIES=demo.la
  moduledir = $(libexecdir)/singular/MOD
  P_PROCS_CPPFLAGS_COMMON = -DDYNAMIC_VERSION 
# Add under Mac OS X: -flat_namespace -weak_reference_mismatches weak -undefined dynamic_lookup
  P_PROCS_MODULE_LDFLAGS = -module -export-dynamic -avoid-version -flat_namespace -weak_reference_mismatches weak -undefined dynamic_lookup
endif

SOURCES = mod_main.cc
demo_la_SOURCES   = $(SOURCES)
demo_la_CPPFLAGS  = ${MYINCLUDES} ${P_PROCS_CPPFLAGS_COMMON}
demo_la_LDFLAGS   = ${AM_LDFLAGS} ${P_PROCS_MODULE_LDFLAGS}

AM_COLOR_TESTS=always

TESTS_ENVIRONMENT  = SINGULARPATH='${abs_top_builddir}/Singular/LIB:${abs_top_srcdir}/Singular/LIB:${abs_top_builddir}/libpolys/polys/.libs:${abs_top_builddir}/factory/gftables:${abs_builddir}/.libs:${abs_srcdir}'
TESTS_ENVIRONMENT += SINGULAR_ROOT_DIR='${abs_top_builddir}'
TESTS_ENVIRONMENT += SINGULAR_BIN_DIR='${abs_top_builddir}/Singular'

TESTS=test.sh

EXTRA_DIST = $(TESTS) 

