#include "asf.h"
#include "asf_nan.h"
#include "asf_import.h"
#include <tiff.h>
#include <tiffio.h>
#include <xtiffio.h>
#include "geotiff_support.h"

int PCS_2_UTM(short pcs, char *hem, datum_type_t *datum, unsigned long *zone)
{
  // The GeoTIFF standard defines the UTM zones numerically in a way that
  // let's us pick off the data mathematically (NNNzz where zz is the zone
  // number):
  //
  // For NAD83 datums, Zones 3N through 23N, NNN == 269
  // For NAD27 datums, Zones 3N through 22N, NNN == 367
  // For WGS72 datums, Zones 1N through 60N, NNN == 322
  // For WGS72 datums, Zones 1S through 60S, NNN == 323
  // For WGS84 datums, Zones 1N through 60N, NNN == 326
  // For WGS84 datums, Zones 1S through 60S, NNN == 327
  // For user-defined UTMs, Northern Hemisphere, NNN == 160
  // For user-defined UTMs, Southern Hemisphere, NNN == 161
  // For other user-defined and/or unsupported UTM projections,
  //   then NNN can be a variety of other numbers (see the
  //   GeoTIFF Standard)
  //
  // NOTE: For NAD27 and NAD83, only the restricted range of zones
  // above is supported by the GeoTIFF standard.
  //
  const short NNN_NAD27  = 367;
  const short NNN_NAD83  = 269;
  const short NNN_WGS84N = 326;
  const short NNN_WGS84S = 327;
  const short NNN_USER_DEFINED_NORTH = 160;
  const short NNN_USER_DEFINED_SOUTH = 161;
  int isUTM = 0;
  short datumClassifierNNN;

  datumClassifierNNN = pcs / 100;
  if (datumClassifierNNN == NNN_NAD27) {
      *hem = 'N';
      *datum = NAD27_DATUM;
      *zone = pcs - (pcs / 100)*100;
      isUTM = 1;
      if (*zone < 3 || *zone > 22) {
        *datum = UNKNOWN_DATUM;
        *zone = 0;
        isUTM = 0;
      }
  }
  else if (datumClassifierNNN == NNN_NAD83) {
      *hem = 'N';
      *datum = NAD83_DATUM;
      *zone = pcs - (pcs / 100)*100;
      isUTM = 1;
      if (*zone < 3 || *zone > 23) {
        *datum = UNKNOWN_DATUM;
        *zone = 0;
        isUTM = 0;
      }
  }
  else if (datumClassifierNNN == NNN_WGS84N ||
           datumClassifierNNN == NNN_WGS84S) {
      *hem = datumClassifierNNN == NNN_WGS84N ? 'N' : 'S';
      *datum = WGS84_DATUM;
      *zone = pcs - (pcs / 100)*100;
      isUTM = 1;
      if (*zone < 1 || *zone > 60) {
        *datum = UNKNOWN_DATUM;
        *zone = 0;
        isUTM = 0;
      }
  }
  else if (datumClassifierNNN == NNN_USER_DEFINED_NORTH ||
           datumClassifierNNN == NNN_USER_DEFINED_SOUTH) {
    /*
        NOTE: For the user-defined cases, the datum should be set
        from the geokey GeographicTypeGeoKey (or possibly
        GeogGeodeticDatumGeoKey) before or after the calling of
        PCS_2_UTM(), therefore *datum is not assigned anything
        here.
    */
    *hem = datumClassifierNNN == NNN_USER_DEFINED_NORTH ? 'N' : 'S';
    *zone = pcs - (pcs / 100)*100;
    isUTM = 1;
    if (*zone < 1 || *zone > 60) {
      *datum = UNKNOWN_DATUM;
      *zone = 0;
      isUTM = 0;
    }
  }
  else {
      *hem = '\0';
      *datum = UNKNOWN_DATUM;
      *zone = 0;
      isUTM = 0;
  }

  return isUTM;
}

// NOTE: Because of the degrees-to-radians conversions, this function is ONLY for
// meta_getlatlon()...
void copy_proj_parms(meta_projection *dest, meta_projection *src)
{
  dest->type = src->type;
  dest->startX = src->startX;
  dest->startY = src->startY;
  dest->perX = src->perX;
  dest->perY = src->perY;
  strcpy (dest->units, src->units);
  dest->hem = src->hem;
  dest->spheroid = src->spheroid;
  dest->re_major = src->re_major;
  dest->re_minor = src->re_minor;
  dest->datum = src->datum;
  dest->height = src->height;

  switch (src->type) {
    case UNIVERSAL_TRANSVERSE_MERCATOR:
      dest->param.utm.zone = src->param.utm.zone;
      dest->param.utm.lat0 = D2R*src->param.utm.lat0;
      dest->param.utm.lon0 = D2R*src->param.utm.lon0;
      dest->param.utm.false_easting = src->param.utm.false_easting;
      dest->param.utm.false_northing = src->param.utm.false_northing;
      dest->param.utm.scale_factor = src->param.utm.scale_factor;
      break;
    case POLAR_STEREOGRAPHIC:
      dest->param.ps.slat = D2R*src->param.ps.slat;
      dest->param.ps.slon = D2R*src->param.ps.slon;
      dest->param.ps.is_north_pole = src->param.ps.is_north_pole;
      dest->param.ps.false_easting = src->param.ps.false_easting;
      dest->param.ps.false_northing = src->param.ps.false_northing;
      break;
    case ALBERS_EQUAL_AREA:
      dest->param.albers.std_parallel1 = D2R*src->param.albers.std_parallel1;
      dest->param.albers.std_parallel2 = D2R*src->param.albers.std_parallel2;
      dest->param.albers.center_meridian = D2R*src->param.albers.center_meridian;
      dest->param.albers.orig_latitude = D2R*src->param.albers.orig_latitude;
      dest->param.albers.false_easting = src->param.albers.false_easting;
      dest->param.albers.false_northing = src->param.albers.false_northing;
      break;
    case LAMBERT_CONFORMAL_CONIC:
      dest->param.lamcc.plat1 = D2R*src->param.lamcc.plat1;
      dest->param.lamcc.plat2 = D2R*src->param.lamcc.plat2;
      dest->param.lamcc.lat0 = D2R*src->param.lamcc.lat0;
      dest->param.lamcc.lon0 = D2R*src->param.lamcc.lon0;
      dest->param.lamcc.false_easting = src->param.lamcc.false_easting;
      dest->param.lamcc.false_northing = src->param.lamcc.false_northing;
      dest->param.lamcc.scale_factor = src->param.lamcc.scale_factor;
      break;
    case LAMBERT_AZIMUTHAL_EQUAL_AREA:
      dest->param.lamaz.center_lon = D2R*src->param.lamaz.center_lon;
      dest->param.lamaz.center_lat = D2R*src->param.lamaz.center_lat;
      dest->param.lamaz.false_easting = src->param.lamaz.false_easting;
      dest->param.lamaz.false_northing = src->param.lamaz.false_northing;
      break;
    default:
      asfPrintError("Unsupported projection type found.\n");
      break;
  }
}

int get_tiff_data_config(TIFF *tif,
                         short *sample_format, short *bits_per_sample, short *planar_config,
                         data_type_t *data_type, int *num_bands,
                         int *is_scanline_format)
{
  int     ret = 0;
  uint16  planarConfiguration = 0;
  uint16  bitsPerSample = 0;
  uint16  sampleFormat = 0;
  uint16  samplesPerPixel = 0;

  *data_type = -1;
  *num_bands = 0;

  // Required tag for all images
  ret = 0;
  TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
  if (bitsPerSample != 8  &&
      bitsPerSample != 16 &&
      bitsPerSample != 32)
  {
    // Only support byte, integer16, integer32, and 32-bit floats
    ret = -1;
  }
  else {
    *bits_per_sample = (short)bitsPerSample;
  }

  // Required tag for all images
  if (ret == 0) {
    TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
    if (samplesPerPixel < 1 || samplesPerPixel > MAX_BANDS) {
      // Only support 1 through MAX_BANDS bands in the image
      ret = -1;
    }
    else {
      *num_bands = samplesPerPixel;
    }
  }

  // Required tag only for color (multi-band) images
  if (ret == 0 && samplesPerPixel > 1) {
    TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &planarConfiguration);
    if (planarConfiguration != PLANARCONFIG_CONTIG &&
        planarConfiguration != PLANARCONFIG_SEPARATE)
    {
      // Only support [rgbrgbrgb...] format, not separately-stored color bands
      ret = -1;
    }
    else {
      *planar_config = planarConfiguration;
    }
  }

  // Required tag for all images
  //
  // Only support BYTE, INTEGER16, INTEGER32, and REAL32 at this time...
  if (ret == 0) {
    TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sampleFormat);
    switch (sampleFormat) {
      case SAMPLEFORMAT_UINT:
        {
          *sample_format = sampleFormat;
          switch (bitsPerSample) {
            case 8:
              *data_type = BYTE;
              break;
            case 16:
              *data_type = INTEGER32;
              break;
            case 32:
              *data_type = REAL32;
              break;
            default:
              ret = -1;
              break;
          }
        }
        break;
      case SAMPLEFORMAT_INT:
        {
          *sample_format = sampleFormat;
          switch (bitsPerSample) {
            case 8:
            case 16:
              *data_type = INTEGER16;
              break;
            case 32:
              *data_type = INTEGER32;
              break;
            default:
              ret = -1;
              break;
          }
        }
        break;
      case SAMPLEFORMAT_IEEEFP:
        {
          *sample_format = sampleFormat;
          *data_type = REAL32;
        }
        break;
      default:
        {
          // Either the sample format is unknown or it's one of the complex types
          // Since a complex type would be rare in a TIFF, we assume it isn't and
          // then guess what the data type is based on how many bits are in the
          // samples ...only the results of the ingest will indicate whether this is
          // a good idea or not.
          *sample_format = UNKNOWN_SAMPLE_FORMAT;
          switch (bitsPerSample) {
            case 8:
              // Most likely unsigned 8-bit byte
              *sample_format = SAMPLEFORMAT_UINT;
              *data_type = BYTE;
              break;
            case 16:
              // Most likely 16-bit integer
              *sample_format = SAMPLEFORMAT_INT;
              *data_type = INTEGER16;
              break;
            case 32:
              // Most likely 32-bot float
              *sample_format = SAMPLEFORMAT_IEEEFP;
              *data_type = REAL32;
              break;
            default:
              ret = -1;
              break;
          }
        }
        break;
    }
  }

  // Required to have scanline format
  if (ret == 0) {
    short tile_width;
    TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tile_width);
    if (tile_width > 0)
    {
      // Only support true scanline format TIFFs
      ret = -1;
    }
    else {
      *is_scanline_format = 1;
    }
  }

  return ret;
}
