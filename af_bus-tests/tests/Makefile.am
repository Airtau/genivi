TESTS = check_af_bus
check_PROGRAMS = check_af_bus

check_af_bus_SOURCES = check_af_bus.c $(top_builddir)/src/afbus.h
check_af_bus_CFLAGS = @CHECK_CFLAGS@
check_af_bus_LDADD = $(top_builddir)/src/libafbus.la @CHECK_LIBS@
