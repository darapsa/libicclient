lib_LTLIBRARIES = libicclient.la
libicclient_la_SOURCES = \
			 client.c \
			 request.c \
			 handler.c \
			 ord.c \
			 login.c \
			 member.c \
			 admin.c
libicclient_la_CPPFLAGS = -I${prefix}/include $(TIDY_CFLAGS)
libicclient_la_LDFLAGS =  $(TIDY_LIBS)
if IOS
libicclient_la_LDFLAGS += -static
endif
if WASM
libicclient_la_LDFLAGS += -static
else
libicclient_la_LDFLAGS += -lcurl
endif
include_HEADERS = icclient.h
pkginclude_HEADERS = \
		  icclient/typedefs.h \
		  icclient/ord.h \
		  icclient/member.h \
		  icclient/admin.h
#if !WASM
#bin_PROGRAMS = icclient
#icclient_SOURCES = main.c client.c request.c handler.c login.c member.c admin.c
#icclient_CPPFLAGS = $(TIDY_CFLAGS)
#icclient_LDFLAGS = -lcurl $(TIDY_LIBS)
#endif
