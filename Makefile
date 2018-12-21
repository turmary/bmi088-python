# tary, 15:18 2018/12/20
#
TARGET     = bmi088_test
BMI088_LIB = libbmi088.so
OBJS       = bmi088.o bmi08a.o bmi08g.o rpi_bmi088.o

VPATH = src:bosch-lib

CPPFLAGS= -Isrc -Ibosch-lib -DBMI08X_ENABLE_BMI085=0 -DBMI08X_ENABLE_BMI088=1

all: $(TARGET) $(BMI088_LIB)

# option -Wl,--rpath=./ used by bmi088_test under developing environment
$(TARGET): main.o $(BMI088_LIB)
	$(CC)  $(CPPFLAGS) -o $@ -L./ -lbmi088 -Wl,--rpath=./ $<

$(BMI088_LIB): $(OBJS)
	$(CC)  $(CPPFLAGS) --shared -o $@ $^

install: all
	install -D $(TARGET) $(DESTDIR)$(prefix)/bin/$(TARGET)
	install -D $(BMI088_LIB) $(DESTDIR)$(prefix)/lib/${BMI088_LIB}

uninstall:
	-rm -f $(DESTDIR)$(prefix)/bin/$(TARGET)
	-rm -f $(DESTDIR)$(prefix)/lib/${BMI088_LIB}

clean:
	rm -f *.o $(TARGET) $(BMI088_LIB)

.PHONY: all clean install uninstall

