#	concat_dem
#	by Orion Lawlor, ASF, 1998
#	A program to overlay a series of DEMs atop
#       one another.

include ../../make_support/system_rules

LIBS  = \
	$(LIBDIR)/asf_meta.a \
	$(GSL_LIBS) \
	$(LIBDIR)/libasf_proj.a \
	$(LIBDIR)/asf.a \
	$(PROJ_LIBS) \
	$(XML_LIBS) \
	-lm

OBJS  = open_dem.o \
	extents.o \
	projection.o \
	concat_dem.o


all: concat_dem
	rm *.o

concat_dem: $(OBJS)
	$(CC) $(CFLAGS) -o concat_dem $(OBJS) $(LIBS) $(LDFLAGS)
	mv concat_dem$(BIN_POSTFIX) $(BINDIR)
	cp concat_dem.1 ../../man/cat1/concat_dem.1

clean:
	rm -f core $(OBJS) *.o
