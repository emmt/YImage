#
# Makefile -
#
#	Makefile for YImage plugin.
#

# where are the sources? (automatically filled in by configure script)
srcdir=.

# these values filled in by: yorick -batch make.i
Y_MAKEDIR=
Y_EXE=
Y_EXE_PKGS=
Y_EXE_HOME=
Y_EXE_SITE=
Y_HOME_PKG=

# ----------------------------------------------------- optimization flags

# options for make command line, e.g.-   make COPT=-g TGT=exe
COPT=$(COPT_DEFAULT)
TGT=$(DEFAULT_TGT)

# ------------------------------------------------ macros for this package

PKG_NAME=yimage
PKG_I=$(srcdir)/image.i

#OBJS=img_morph.o img_segment.o img_noise.o img_linear.o \
#     ocr_cost.o itempool.o itemstack.o yanpr.o
OBJS = img_copy.o img_cost.o img_linear.o img_morph.o img_noise.o \
       img_segment.o img_yorick.o img_detect.o \
       itempool.o itemstack.o watershed.o
INCS = $(srcdir)/img.h $(srcdir)/c_pseudo_template.h

# change to give the executable a name other than yorick
PKG_EXENAME=yorick

# PKG_DEPLIBS=-Lsomedir -lsomelib   for dependencies of this package
PKG_DEPLIBS=
# set compiler (or rarely loader) flags specific to this package
PKG_CFLAGS= -DYORICK
#PKG_CFLAGS= -DDEBUG -DIMG_DLL -DIMG_DLL_EXPORTS -fvisibility=hidden
PKG_LDFLAGS=

# list of additional package names you want in PKG_EXENAME
# (typically Y_EXE_PKGS should be first here)
EXTRA_PKGS=$(Y_EXE_PKGS)

# list of additional files for clean
PKG_CLEAN=img_version.h fnlist

# autoload file for this package, if any
PKG_I_START=$(srcdir)/image-start.i
# non-pkg.i include files for this package, if any
PKG_I_EXTRA=

RELEASE_FILES = \
  AUTHORS.md LICENSE.md NEWS.md README.md TODO.md \
  Makefile configure image.i image-start.i \
  c_pseudo_template.h heapsort.h img.h \
  img_copy.c img_cost.c  img_detect.c img_linear.c img_morph.c \
  img_noise.c img_segment.c img_yorick.c watershed.c \
  itempool.c itempool.h \
  itemstack.c itemstack.h \
  memstack.c memstack.h

# -------------------------------- standard targets and rules (in Makepkg)

# set macros Makepkg uses in target and dependency names
# DLL_TARGETS, LIB_TARGETS, EXE_TARGETS
# are any additional targets (defined below) prerequisite to
# the plugin library, archive library, and executable, respectively
PKG_I_DEPS=$(PKG_I)
Y_DISTMAKE=distmake

ifeq (,$(strip $(Y_MAKEDIR)))
$(info *** WARNING: Y_MAKEDIR not defined, you may run 'yorick -batch make.i' first)
else
include $(Y_MAKEDIR)/Make.cfg
include $(Y_MAKEDIR)/Makepkg
include $(Y_MAKEDIR)/Make$(TGT)
endif

# override macros Makepkg sets for rules and other macros
# Y_HOME and Y_SITE in Make.cfg may not be correct (e.g.- relocatable)
Y_HOME=$(Y_EXE_HOME)
Y_SITE=$(Y_EXE_SITE)

# reduce chance of yorick-1.5 corrupting this Makefile
MAKE_TEMPLATE = protect-against-1.5

# ------------------------------------- targets and rules for this package

# Dummy default target in case Y_MAKEDIR was not defined:
dummy-default:
	@echo >&2 "*** ERROR: Y_MAKEDIR not defined, aborting..."; false

%.o: ${srcdir}/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

# simple example:
#myfunc.o: myapi.h
# more complex example (also consider using PKG_CFLAGS above):
#myfunc.o: myapi.h myfunc.c
#	$(CC) $(CPPFLAGS) $(CFLAGS) -DMY_SWITCH -o $@ -c myfunc.c

itempool.o: $(srcdir)/itempool.h
itemstack.o: $(srcdir)/itemstack.h
memstack.o: $(srcdir)/memstack.h
img_linear.o: $(INCS)
img_morph.o: $(INCS)
img_segment.o: $(INCS) $(srcdir)/heapsort.h $(srcdir)/itempool.h $(srcdir)/itemstack.h
img_copy.o: $(INCS)
img_cost.o: $(INCS)
img_utils.o: $(INCS)
img_yorick.o: $(INCS) img_version.h
watershed.o: $(srcdir)/watershed.c
img_version.h: $(srcdir)/VERSION
	sed -re 's/^([0-9]+)\.([0-9]+)\.([0-9]+).*/#define IMG_VERSION_MAJOR \1\n#define IMG_VERSION_MINOR \2\n#define IMG_VERSION_PATCH \3\n/' <"$<" >"$@"

fnlist: image.i Makefile
	grep -E '^[ 	]*(extern|func)[ 	]' "$<" | sed -re \
	's/^[ 	]*(extern|func)[ 	]([a-zA-Z_0-9]+).*/\2/' | sort >"$@"

# ------------------------------------- distribution

release:
	@if test "x$(VERSION)" = "x"; then \
	  if test -f VERSION -a -r VERSION; then \
	    for version in `cat VERSION`; do break; done; \
	  fi; \
	  if test "x$$version" = "x"; then \
	    echo >&2 "error: bad VERSION file; try: \"make $@ VERSION=X.Y.Z\""; \
	    return 1; \
	  fi; \
	else \
	  version=$(VERSION); \
	fi; \
	pkgdir=YImage-$$version; \
	archive=$$pkgdir.tar.bz2; \
	if test -e "$$pkgdir"; then \
	  echo >&2 "error: $$pkgdir already exists"; \
	  return 1; \
	fi; \
	if test -e "$$archive"; then \
	  echo >&2 "error: $$archive already exists"; \
	  return 1; \
	fi; \
	mkdir "$$pkgdir"; \
	for file in $(RELEASE_FILES); do \
	  cp -a "$$file" "$$pkgdir/."; \
	done; \
	rm -f "$$pkgdir/VERSION" "$$pkgdir/img_version.h"; \
	echo "$$version" > "$$pkgdir/VERSION"; \
	(cd "$$pkgdir"; make img_version.h); \
	tar cf - "$$pkgdir" | bzip2 -9 > "$$archive"; \
	rm -rf "$$pkgdir"; \
	echo "archive $$archive created"; \
	return 0

.PHONY: clean default all check distclean release update install

# -------------------------------------------------------- end of Makefile
