ACLOCAL_AMFLAGS = -I m4

if ENABLE_FACTORY
  USE_FACTORY = factory 
else
  USE_FACTORY =
endif

if ENABLE_P_PROCS_DYNAMIC
  USE_MODULES = dyn_modules
else
  USE_MODULES = 
endif

ALLOC = omalloc
 
PACKAGES=findexec $(ALLOC) $(USE_FACTORY) libpolys kernel numeric Singular IntegerProgramming $(USE_MODULES)

SUBDIRS=$(PACKAGES)

TESTDIRS=$(PACKAGES)


emacsdir=$(datadir)/emacs
dist_emacs_DATA =  emacs/BUGS \
    emacs/ChangeLog \
    emacs/cmpl.pl \
    emacs/COPYING \
    emacs/ESingular.emacs21 \
    emacs/lib.pl \
    emacs/Makefile.in \
    emacs/NEWS \
    emacs/singular.el \
    emacs/singular.el.decl \
    emacs/singular.xpm

# emacs/test.sing
# TODO: remove this!
#all-local:
#	[ -d ${builddir}/emacs ] || ln -s ${srcdir}/emacs ${builddir}


EXTRA_DIST = autogen.sh emacs


bin_SCRIPTS = libsingular-config
