# Makefile for convert2vector

include ../../make_support/system_rules

LIBS  = \
	$(LIBDIR)/libasf_vector.a \
	$(LIBDIR)/libshp.a \
	$(LIBDIR)/libasf_import.a \
	$(LIBDIR)/libasf_raster.a \
	$(LIBDIR)/asf_meta.a \
	$(LIBDIR)/asf.a \
	$(GSL_LIBS) \
	$(LIBDIR)/libasf_proj.a \
	$(LIBDIR)/asf.a \
	$(TIFF_LIBS) \
	$(GEOTIFF_LIBS) \
	$(HDF5_LIBS) \
	$(PROJ_LIBS) \
	$(GLIB_LIBS) \
	$(JPEG_LIBS) \
	$(XML_LIBS) \
	$(ZLIB_LIBS) \
	-lm

CFLAGS += \
	$(GEOTIFF_CFLAGS) \
	$(TIFF_CFLAGS)

OBJS  = convert2vector.o help.o

all: convert2vector clean

convert2vector: $(OBJS)
	$(CC) $(CFLAGS) -o convert2vector $(OBJS) $(LIBS) $(LDFLAGS)
	mv convert2vector$(BIN_POSTFIX) $(BINDIR)

clean:
	rm -f *.o
