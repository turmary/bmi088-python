# tary, 15:18 2018/12/20
#
SHELL = /bin/sh

.SUFFIXES: .c .o

prefix ?= ./usr/local

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644
RM = rm -f

# Get source files directory
# To separate build & source directory
srcdir := $(dir $(firstword ${MAKEFILE_LIST}))
srcdir := $(shell cd ${srcdir}; pwd)

OBJS_BMI088 = bmi088.o bmi08a.o bmi08g.o rpi_bmi088.o rpi_i2c.o
OBJS_AKICM = rpi_icm20600.o rpi_ak09918.o rpi_i2c.o

TST_BMI088   = test_bmi088
TST_ICM20600 = test_icm20600
TST_AK09918  = test_ak09918

LIB_BMI088   = libbmi088.so
LIB_AKICM    = libakicm.so

TARGETS = $(TST_BMI088) $(TST_ICM20600) $(TST_AK09918)
LIBS    = $(LIB_BMI088) $(LIB_AKICM)


# $(warning srcdir=$(srcdir))
VPATH = $(srcdir)/src:$(srcdir)/bosch-lib

CPPFLAGS   = -I. -I$(srcdir)/src -I$(srcdir)/bosch-lib -DBMI08X_ENABLE_BMI085=0 -DBMI08X_ENABLE_BMI088=1
CFLAGS     = -g
ALL_CFLAGS = $(CPPFLAGS) $(CFLAGS)

all: $(TARGETS) $(LIBS)

# option -Wl,--rpath=./ used by bmi088_test under developing environment
$(TST_BMI088): test_bmi088.o $(LIB_BMI088)
	$(CC)  $(ALL_CFLAGS) -o $@ -L./ -Wl,-\( -lbmi088 -Wl,--rpath=./ $< -Wl,-\)

$(TST_ICM20600): test_icm20600.o $(LIB_AKICM)
	$(CC)  $(ALL_CFLAGS) -o $@ -L./ -Wl,-\( -lakicm -Wl,--rpath=./ $< -Wl,-\)

$(TST_AK09918): test_ak09918.o $(LIB_AKICM)
	$(CC)  $(ALL_CFLAGS) -o $@ -L./ -Wl,-\( -lakicm -Wl,--rpath=./ $< -Wl,-\)

$(LIB_BMI088): $(OBJS_BMI088)
	$(CC)  $(ALL_CFLAGS) --shared -o $@ $^

$(LIB_AKICM): $(OBJS_AKICM)
	$(CC)  $(ALL_CFLAGS) --shared -o $@ $^

install: all
	$(INSTALL) -D $(TST_BMI088) $(DESTDIR)$(prefix)/bin/$(TST_BMI088)
	$(INSTALL) -D $(TST_ICM20600) $(DESTDIR)$(prefix)/bin/$(TST_ICM20600)
	$(INSTALL) -D $(TST_AK09918) $(DESTDIR)$(prefix)/bin/$(TST_AK09918)
	$(INSTALL) -D $(LIB_BMI088) $(DESTDIR)$(prefix)/lib/$(LIB_BMI088)
	$(INSTALL) -D $(LIB_AKICM) $(DESTDIR)$(prefix)/lib/$(LIB_AKICM)

uninstall:
	-$(RM) $(DESTDIR)$(prefix)/bin/$(TST_BMI088)
	-$(RM) $(DESTDIR)$(prefix)/bin/$(TST_AK09918)
	-$(RM) $(DESTDIR)$(prefix)/bin/$(TST_ICM20600)
	-$(RM) $(DESTDIR)$(prefix)/lib/$(LIB_BMI088)
	-$(RM) $(DESTDIR)$(prefix)/lib/$(LIB_AKICM)

clean:
	-$(RM) *.o $(TARGETS) $(LIBS)

.PHONY: all clean install uninstall

