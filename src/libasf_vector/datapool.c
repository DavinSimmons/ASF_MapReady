#include "asf_vector.h"
#include "shapefil.h"
#include "asf_nan.h"
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include "datapool.h"
#include "dateUtil.h"

void datapool_init(datapool_type_t *datapool)
{
  strcpy(datapool->granule_name, MAGIC_UNSET_STRING);
  strcpy(datapool->platform, MAGIC_UNSET_STRING);
  strcpy(datapool->sensor, MAGIC_UNSET_STRING);
  strcpy(datapool->beam_mode, MAGIC_UNSET_STRING);
  strcpy(datapool->beam_mode_description, MAGIC_UNSET_STRING);
  datapool->orbit = MAGIC_UNSET_INT;
  datapool->path_number = MAGIC_UNSET_INT;
  datapool->frame_number = MAGIC_UNSET_INT;
  strcpy(datapool->acquisition_date, MAGIC_UNSET_STRING);
  strcpy(datapool->processing_date, MAGIC_UNSET_STRING);
  strcpy(datapool->start_time, MAGIC_UNSET_STRING);
  strcpy(datapool->end_time, MAGIC_UNSET_STRING);
  datapool->center_lat = MAGIC_UNSET_DOUBLE;
  datapool->center_lon = MAGIC_UNSET_DOUBLE;
  datapool->near_start_lat = MAGIC_UNSET_DOUBLE;
  datapool->near_start_lon = MAGIC_UNSET_DOUBLE;
  datapool->far_start_lat = MAGIC_UNSET_DOUBLE;
  datapool->far_start_lon = MAGIC_UNSET_DOUBLE;
  datapool->near_end_lat = MAGIC_UNSET_DOUBLE;
  datapool->near_end_lon = MAGIC_UNSET_DOUBLE;
  datapool->far_end_lat = MAGIC_UNSET_DOUBLE;
  datapool->far_end_lon = MAGIC_UNSET_DOUBLE;
  datapool->faraday_rotation = MAGIC_UNSET_DOUBLE;
  strcpy(datapool->orbit_direction, MAGIC_UNSET_STRING);
  strcpy(datapool->url, MAGIC_UNSET_STRING);
  datapool->size = MAGIC_UNSET_DOUBLE;
  datapool->off_nadir_angle = MAGIC_UNSET_DOUBLE;
}

static void strip_end_whitesp(char *s)
{
    char *p = s + strlen(s) - 1;
    while (isspace(*p) && p>s)
        *p-- = '\0';
}

static void add_to_kml(FILE *fp, datapool_type_t *datapool, dbf_header_t *dbf,
		       int nCols)
{
  int ii;
  char begin[10], end[10];

  // Print out according to configuration
  fprintf(fp, "<Placemark>\n");
  fprintf(fp, "  <description><![CDATA[\n");
  fprintf(fp, "<table width=\"350\"><tr><td>\n");
  fprintf(fp, "<!-- Format: DATAPOOL (generated by convert2vector "
          "(version %s)) -->\n", SVN_REV);
  for (ii=0; ii<nCols; ii++) {
    if (dbf[ii].visible == 0) {
      strcpy(begin, "<!--");
      strcpy(end, "-->\n");
    }
    else {
      strcpy(begin, "");
      strcpy(end, "\n");
    }
    if (strcmp(dbf[ii].header, "Granule_Name") == 0)
      fprintf(fp, "%s<strong>Granule Name</strong>: %s <br>%s",
	      begin, datapool->granule_name, end);
    else if (strcmp(dbf[ii].header, "Platform") == 0)
      fprintf(fp, "%s<strong>Platform</strong>: %s <br>%s",
	      begin, datapool->platform, end);
    else if (strcmp(dbf[ii].header, "Sensor") == 0)
      fprintf(fp, "%s<strong>Sensor</strong>: %s <br>%s",
	      begin, datapool->sensor, end);
    else if (strcmp(dbf[ii].header, "Beam_Mode") == 0)
      fprintf(fp, "%s<strong>Beam mode</strong>: %s <br>%s",
	      begin, datapool->beam_mode, end);
    else if (strcmp(dbf[ii].header, "Beam_Mode_Description") == 0)
      fprintf(fp, "%s<strong>Beam mode description</strong>: %s <br>%s",
	      begin, datapool->beam_mode_description, end);
    else if (strcmp(dbf[ii].header, "Orbit") == 0)
      fprintf(fp, "%s<strong>Orbit</strong>: %d <br>%s",
	      begin, datapool->orbit, end);
    else if (strcmp(dbf[ii].header, "Path_Number") == 0)
      fprintf(fp, "%s<strong>Path Number</strong>: %d <br>%s",
	      begin, datapool->path_number, end);
    else if (strcmp(dbf[ii].header, "Frame_Number") == 0)
      fprintf(fp, "%s<strong>Frame Number</strong>: %d <br>%s",
	      begin, datapool->frame_number, end);
    else if (strcmp(dbf[ii].header, "Acquisition_Date") == 0)
      fprintf(fp, "%s<strong>Acquisition Date</strong>: %s <br>%s",
	      begin, datapool->acquisition_date, end);
    else if (strcmp(dbf[ii].header, "Processing_Date") == 0)
      fprintf(fp, "%s<strong>Processing Date</strong>: %s <br>%s",
	      begin, datapool->processing_date, end);
    else if (strcmp(dbf[ii].header, "Processing_Level") == 0)
      fprintf(fp, "%s<strong>Processing Level</strong>: %s <br>%s",
	      begin, datapool->processing_level, end);
    else if (strcmp(dbf[ii].header, "Start_Time") == 0)
      fprintf(fp, "%s<strong>Start Time</strong>: %s <br>%s",
	      begin, datapool->start_time, end);
    else if (strcmp(dbf[ii].header, "End_Time") == 0)
      fprintf(fp, "%s<strong>End Time</strong>: %s <br>%s",
	      begin, datapool->end_time, end);
    else if (strcmp(dbf[ii].header, "Center_Lat") == 0)
      fprintf(fp, "%s<strong>Center Lat</strong>: %.4f <br>%s",
	      begin, datapool->center_lat, end);
    else if (strcmp(dbf[ii].header, "Center_Lon") == 0)
      fprintf(fp, "%s<strong>Center Lon</strong>: %.4f <br>%s",
	      begin, datapool->center_lon, end);
    else if (strcmp(dbf[ii].header, "Near_Start_Lat") == 0)
      fprintf(fp, "%s<strong>Near Start Lat</strong>: %.4f <br>%s",
	      begin, datapool->near_start_lat, end);
    else if (strcmp(dbf[ii].header, "Near_Start_Lon") == 0)
      fprintf(fp, "%s<strong>Near Start Lon</strong>: %.4f <br>%s",
	      begin, datapool->near_start_lon, end);
    else if (strcmp(dbf[ii].header, "Far_Start_Lat") == 0)
      fprintf(fp, "%s<strong>Far Start Lat</strong>: %.4f <br>%s",
	      begin, datapool->far_start_lat, end);
    else if (strcmp(dbf[ii].header, "Far_Start_Lon") == 0)
      fprintf(fp, "%s<strong>Far Start Lon</strong>: %.4f <br>%s",
	      begin, datapool->far_start_lon, end);
    else if (strcmp(dbf[ii].header, "Near_End_Lat") == 0)
      fprintf(fp, "%s<strong>Near End Lat</strong>: %.4f <br>%s",
	      begin, datapool->near_end_lat, end);
    else if (strcmp(dbf[ii].header, "Near_End_Lon") == 0)
      fprintf(fp, "%s<strong>Near End Lon</strong>: %.4f <br>%s",
	      begin, datapool->near_end_lon, end);
    else if (strcmp(dbf[ii].header, "Far_End_Lat") == 0)
      fprintf(fp, "%s<strong>Far End Lat</strong>: %.4f <br>%s",
	      begin, datapool->far_end_lat, end);
    else if (strcmp(dbf[ii].header, "Far_End_Lon") == 0)
      fprintf(fp, "%s<strong>Far End Lon</strong>: %.4f <br>%s",
	      begin, datapool->far_end_lon, end);
    else if (strcmp(dbf[ii].header, "Faraday_Rotation") == 0)
      fprintf(fp, "%s<strong>Faraday Rotation</strong>: %.1f <br>%s",
	      begin, datapool->faraday_rotation, end);
    else if (strcmp(dbf[ii].header, "Orbit_Direction") == 0)
      fprintf(fp, "%s<strong>Orbit Direction</strong>: %s <br>%s",
	      begin, datapool->orbit_direction, end);
    else if (strcmp(dbf[ii].header, "Url") == 0)
      fprintf(fp, "%s<strong>URL</strong>: %s <br>%s",
	      begin, datapool->url, end);
    else if (strcmp(dbf[ii].header, "Size") == 0)
      fprintf(fp, "%s<strong>Size (MB)</strong>: %.2f <br>%s",
	      begin, datapool->size, end);
    else if (strcmp(dbf[ii].header, "Off_Nadir_Angle") == 0)
      fprintf(fp, "%s<strong>Off Nadir Angle</strong>: %.1f <br>%s",
	      begin, datapool->off_nadir_angle, end);
  }
  fprintf(fp, "</td></tr></table>\n");
  fprintf(fp, "  ]]></description>\n");
  fprintf(fp, "  <name>%s</name>\n", datapool->granule_name);
  fprintf(fp, "  <LookAt>\n");
  fprintf(fp, "    <longitude>%.4f</longitude>\n", datapool->center_lon);
  fprintf(fp, "    <latitude>%.4f</latitude>\n", datapool->center_lat);
  fprintf(fp, "    <range>400000</range>\n");
  fprintf(fp, "  </LookAt>\n");
  fprintf(fp, "  <visibility>1</visibility>\n");
  fprintf(fp, "  <open>1</open>\n");

  write_kml_style_keys(fp);

  fprintf(fp, "  <Polygon>\n");
  fprintf(fp, "    <extrude>1</extrude>\n");
  fprintf(fp, "    <altitudeMode>%s</altitudeMode>\n", altitude_mode());
  fprintf(fp, "    <outerBoundaryIs>\n");
  fprintf(fp, "     <LinearRing>\n");
  fprintf(fp, "      <coordinates>\n");
  fprintf(fp, "       %.4f,%.4f,7000\n",
      datapool->near_start_lon, datapool->near_start_lat);
  fprintf(fp, "       %.4f,%.4f,7000\n",
      datapool->far_start_lon, datapool->far_start_lat);
  fprintf(fp, "       %.4f,%.4f,7000\n",
      datapool->far_end_lon, datapool->far_end_lat);
  fprintf(fp, "       %.4f,%.4f,7000\n",
      datapool->near_end_lon, datapool->near_end_lat);
  fprintf(fp, "       %.4f,%.4f,7000\n",
      datapool->near_start_lon, datapool->near_start_lat);
  fprintf(fp, "      </coordinates>\n");
  fprintf(fp, "     </LinearRing>\n");
  fprintf(fp, "    </outerBoundaryIs>\n");
  fprintf(fp, "  </Polygon>\n");
  fprintf(fp, "</Placemark>\n");
}

static void open_stack(FILE *fp, datapool_type_t *datapool)
{
  fprintf(fp, "<Placemark>\n");
  fprintf(fp, "  <description><![CDATA[\n");
  fprintf(fp, "<table width=\"350\"><tr><td>\n");
  fprintf(fp, "<!-- Format: DATAPOOL (generated by convert2vector "
          "(version %s)) -->\n", SVN_REV);
  fprintf(fp, "<strong>Platform</strong>: %s <br>\n", datapool->platform);
  fprintf(fp, "<strong>Sensor</strong>: %s <br>\n", datapool->sensor);
  fprintf(fp, "<strong>Beam mode</strong>: %s <br>\n", datapool->beam_mode);
  fprintf(fp, "<strong>Frame Number</strong>: %d <br>\n", datapool->frame_number);
  fprintf(fp, "<strong>Off Nadir Angle</strong>: %.1f <br><br>\n", 
	  datapool->off_nadir_angle);
  fprintf(fp, "<table border=\"1\" cellpadding=\"5\"><tr>\n");
  fprintf(fp, "<td><strong>Granule Name</strong></td>");
  fprintf(fp, "<td><strong>Center Time</strong></td></tr>\n");
}

static void close_stack(FILE *fp, datapool_type_t *datapool)
{
  fprintf(fp, "</table>\n");
  fprintf(fp, "</td></tr></table>\n");
  fprintf(fp, "  ]]></description>\n");
  fprintf(fp, "  <name>%s stack</name>\n", datapool->granule_name);
  fprintf(fp, "  <LookAt>\n");
  fprintf(fp, "    <longitude>%.4f</longitude>\n", datapool->center_lon);
  fprintf(fp, "    <latitude>%.4f</latitude>\n", datapool->center_lat);
  fprintf(fp, "    <range>400000</range>\n");
  fprintf(fp, "  </LookAt>\n");
  fprintf(fp, "  <visibility>1</visibility>\n");
  fprintf(fp, "  <open>1</open>\n");

  write_kml_style_keys(fp);

  fprintf(fp, "  <Polygon>\n");
  fprintf(fp, "    <extrude>1</extrude>\n");
  fprintf(fp, "    <altitudeMode>%s</altitudeMode>\n", altitude_mode());
  fprintf(fp, "    <outerBoundaryIs>\n");
  fprintf(fp, "     <LinearRing>\n");
  fprintf(fp, "      <coordinates>\n");
  fprintf(fp, "       %.4f,%.4f,7000\n",
      datapool->near_start_lon, datapool->near_start_lat);
  fprintf(fp, "       %.4f,%.4f,7000\n",
      datapool->far_start_lon, datapool->far_start_lat);
  fprintf(fp, "       %.4f,%.4f,7000\n",
      datapool->far_end_lon, datapool->far_end_lat);
  fprintf(fp, "       %.4f,%.4f,7000\n",
      datapool->near_end_lon, datapool->near_end_lat);
  fprintf(fp, "       %.4f,%.4f,7000\n",
      datapool->near_start_lon, datapool->near_start_lat);
  fprintf(fp, "      </coordinates>\n");
  fprintf(fp, "     </LinearRing>\n");
  fprintf(fp, "    </outerBoundaryIs>\n");
  fprintf(fp, "  </Polygon>\n");
  fprintf(fp, "</Placemark>\n");
}

static void add_to_stack_kml(FILE *fp, datapool_type_t *datapool, int nCols)
{
  int ii;
  ymd_date startDate, endDate, centerDate;
  hms_time startTime, endTime, centerTime;
  char mon[][5]= 
    {"","JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};
  fprintf(fp, "<tr><td>%s</td>", datapool->granule_name);
  sscanf(datapool->start_time, "%d-%d-%d %d:%d:%lf",
	 &startDate.year, &startDate.month, &startDate.day,
	 &startTime.hour, &startTime.min, &startTime.sec);
  sscanf(datapool->end_time, "%d-%d-%d %d:%d:%lf",
	 &endDate.year, &endDate.month, &endDate.day,
	 &endTime.hour, &endTime.min, &endTime.sec);
  average_ymdTimes(&startDate, &endDate, &startTime, &endTime, 
		   &centerDate, &centerTime);
  fprintf(fp, "<td>%02d-%s-%d %02d:%02d:%02.0f</td></tr>\n",
	  centerDate.day, mon[centerDate.month], centerDate.year,
	  centerTime.hour, centerTime.min, centerTime.sec);
}

static int read_datapool_line(char *header, int n,char *line, datapool_type_t *datapool)
{
  int ii, ok;
  char *test = (char *) MALLOC(sizeof(char)*255);
  for (ii=0; ii<n; ii++) {
    test = get_column(header, ii);
    test[strlen(test)-1] = '\0';
    test++;
    if (strcmp(test, "Granule Name") == 0)
      strcpy(datapool->granule_name, get_str(line, ii));
    else if (strcmp(test, "Platform") == 0)
      strcpy(datapool->platform, get_str(line, ii));
    else if (strcmp(test, "Sensor") == 0)
      strcpy(datapool->sensor, get_str(line, ii));
    else if (strcmp(test, "Beam Mode") == 0)
      strcpy(datapool->beam_mode, get_str(line, ii));
    else if (strcmp(test, "Beam Mode Description") == 0)
      strcpy(datapool->beam_mode_description, get_str(line, ii));
    else if (strcmp(test, "Orbit") == 0)
      datapool->orbit = get_int(line, ii);
    else if (strcmp(test, "Path Number") == 0)
      datapool->path_number = get_int(line, ii);
    else if (strcmp(test, "Frame Number") == 0)
      datapool->frame_number = get_int(line, ii);
    else if (strcmp(test, "Acquisition Date") == 0)
      strcpy(datapool->acquisition_date, get_str(line, ii));
    else if (strcmp(test, "Processing Date") == 0)
      strcpy(datapool->processing_date, get_str(line, ii));
    else if (strcmp(test, "Processing Level") == 0)
      strcpy(datapool->processing_level, get_str(line, ii));
    else if (strcmp(test, "Start Time") == 0)
      strcpy(datapool->start_time, get_str(line, ii));
    else if (strcmp(test, "End Time") == 0)
      strcpy(datapool->end_time, get_str(line, ii));
    else if (strcmp(test, "Center Lat") == 0)
      datapool->center_lat = get_double(line, ii);
    else if (strcmp(test, "Center Lon") == 0)
      datapool->center_lon = get_double(line, ii);
    else if (strcmp(test, "Near Start Lat") == 0)
      datapool->near_start_lat = get_req_double(line, ii, &ok);
    else if (strcmp(test, "Near Start Lon") == 0)
      datapool->near_start_lon = get_req_double(line, ii, &ok);
    else if (strcmp(test, "Far Start Lat") == 0)
      datapool->far_start_lat = get_req_double(line, ii, &ok);
    else if (strcmp(test, "Far Start Lon") == 0)
      datapool->far_start_lon = get_req_double(line, ii, &ok);
    else if (strcmp(test, "Near End Lat") == 0)
      datapool->near_end_lat = get_req_double(line, ii, &ok);
    else if (strcmp(test, "Near End Lon") == 0)
      datapool->near_end_lon = get_req_double(line, ii, &ok);
    else if (strcmp(test, "Far End Lat") == 0)
      datapool->far_end_lat = get_req_double(line, ii, &ok);
    else if (strcmp(test, "Far End Lon") == 0)
      datapool->far_end_lon = get_req_double(line, ii, &ok);
    else if (strcmp(test, "Faraday Rotation") == 0)
      datapool->faraday_rotation = get_double(line, ii);
    else if (strcmp(test, "Ascending or Descending?") == 0)
      strcpy(datapool->orbit_direction, get_str(line, ii));
    else if (strcmp(test, "URL") == 0)
      strcpy(datapool->url, get_str(line, ii));
    else if (strcmp(test, "Size (MB)") == 0)
      datapool->size = get_double(line, ii);
    else if (strcmp(test, "Off Nadir Angle") == 0)
      datapool->off_nadir_angle = get_double(line, ii);
  }

  return ok;
}

// Check location information
static int check_datapool_location(FILE *ifp, char **header_line, int *n)
{
  dbf_header_t *dbf;
  int ii, nCols;
  char *header = (char *) MALLOC(sizeof(char)*1024);
  fgets(header, 1024, ifp);
  strip_end_whitesp(header);
  int nColumns = get_number_columns(header);

  // Read configuration file
  read_header_config("DATAPOOL", &dbf, &nCols);

  // ensure we have the columns we need
  int granule_col = find_str(header, "Granule Name");
  int near_start_lat_col = find_str(header, "Near Start Lat");
  int near_start_lon_col = find_str(header, "Near Start Lon");
  int far_start_lat_col = find_str(header, "Far Start Lat");
  int far_start_lon_col = find_str(header, "Far Start Lon");
  int near_end_lat_col = find_str(header, "Near End Lat");
  int near_end_lon_col = find_str(header, "Near End Lon");
  int far_end_lat_col = find_str(header, "Far End Lat");
  int far_end_lon_col = find_str(header, "Far End Lon");

  // Check whether all visible columns are actually available in the file
  for (ii=0; ii<nCols; ii++) {
    if (find_str(header, dbf[ii].header) < 0)
      dbf[ii].visible = FALSE;
  }

  int all_ok=TRUE;
  if (granule_col < 0) {
    printf("Missing: Granule Name\n");
    all_ok=FALSE;
  }
  if (near_start_lat_col < 0) {
    printf("Missing: Near Start Lat\n");
    all_ok=FALSE;
  }
  if (near_start_lon_col < 0) {
    printf("Missing: Near Start Lon\n");
    all_ok=FALSE;
  }
  if (far_start_lat_col < 0) {
    printf("Missing: Far Start Lat\n");
    all_ok=FALSE;
  }
  if (far_start_lon_col < 0) {
    printf("Missing: Far Start Lon\n");
    all_ok=FALSE;
  }
  if (near_end_lat_col < 0) {
    printf("Missing: Near End Lat\n");
    all_ok=FALSE;
  }
  if (near_end_lon_col < 0) {
    printf("Missing: Near End Lon\n");
    all_ok=FALSE;
  }
  if (far_end_lat_col < 0) {
    printf("Missing: Far End Lat\n");
    all_ok=FALSE;
  }
  if (far_end_lon_col < 0) {
    printf("Missing: Far End Lon\n");
    all_ok=FALSE;
  }
  if (!all_ok) {
    printf("Required data columns missing, cannot process this file.\n");
    return 0;
  }
  *header_line = header;
  *n = nColumns;

  return 1;
}

// Convert datapool to kml file
int datapool2kml(char *in_file, char *out_file, int listFlag, int stack)
{
  datapool_type_t datapool;
  dbf_header_t *dbf;
  char *header;
  int nCols, nColumns;
  char line[1024];

  // Read configuration file
  read_header_config("DATAPOOL", &dbf, &nCols);

  FILE *ifp = FOPEN(in_file, "r");
  assert(ifp);
  check_datapool_location(ifp, &header, &nColumns);

  FILE *ofp = FOPEN(out_file, "w");
  if (!ofp) {
    printf("Failed to open output file %s: %s\n", out_file, strerror(errno));
    return 0;
  }

  kml_header(ofp);

  int ii = 0;
  while (fgets(line, 1022, ifp) != NULL) {
    strip_end_whitesp(line);

    // now get the individual column values
    datapool_init(&datapool);
    if (read_datapool_line(header, nColumns, line, &datapool)) {
      if (stack) {
	if (ii == 0)
	  open_stack(ofp, &datapool);
	add_to_stack_kml(ofp, &datapool, nCols);
      }
      else
	add_to_kml(ofp, &datapool, dbf, nCols);
      ii++;
    }
  }

  if (stack)
    close_stack(ofp, &datapool);
  kml_footer(ofp);

  fclose(ifp);
  fclose(ofp);

  return 1;
}

void shape_datapool_init(char *inFile, char *header)
{
  char *dbaseFile;
  DBFHandle dbase;
  SHPHandle shape;
  dbf_header_t *dbf;
  int ii, nCols, length=32, length2=255;

  // Read configuration file
  read_header_config("DATAPOOL", &dbf, &nCols);

  // Open database for initialization
  dbaseFile = appendExt(inFile, ".dbf");
  dbase = DBFCreate(dbaseFile);
  if (!dbase)
    asfPrintError("Could not create database file '%s'\n", dbaseFile);

  // Add fields to database
  for (ii=0; ii<nCols; ii++) {
    if (strcmp(dbf[ii].header, "Granule_Name") == 0 && dbf[ii].visible) {
      if (DBFAddField(dbase, "GRAN_NAME", FTString, length, 0) == -1)
        asfPrintError("Could not add GRAN_NAME field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Platform") == 0 && dbf[ii].visible) {
      if (DBFAddField(dbase, "PLATFORM", FTString, length, 0) == -1)
        asfPrintError("Could not add PLATFORM field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Sensor") == 0 && dbf[ii].visible) {
      if (DBFAddField(dbase, "SENSOR", FTString, length, 0) == -1)
        asfPrintError("Could not add SENSOR field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Beam_Mode") == 0 && dbf[ii].visible) {
      if (DBFAddField(dbase, "BEAM_MODE", FTString, length2, 0) == -1)
        asfPrintError("Could not add BEAM_MODE field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Beam_Mode_Description") == 0 && 
	     dbf[ii].visible) {
      if (DBFAddField(dbase, "BEAM_MODE_D", FTString, length, 0) == -1)
        asfPrintError("Could not add BEAM_MODE_D field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Orbit") == 0 && dbf[ii].visible) {
      if (DBFAddField(dbase, "ORBIT", FTInteger, 7, 0) == -1)
        asfPrintError("Could not add ORBIT field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Path_Number") == 0 && dbf[ii].visible) {
      if (DBFAddField(dbase, "PATH_NUM", FTInteger, 10, 0) == -1)
        asfPrintError("Could not add PATH_NUM field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Frame_Number") == 0 && dbf[ii].visible) {
      if (DBFAddField(dbase, "FRAME_NUM", FTInteger, 10, 0) == -1)
        asfPrintError("Could not add FRAME_NUM field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Acquisition_Date") == 0 && 
	     dbf[ii].visible) {
      if (DBFAddField(dbase, "ACQ_DATE", FTString, length, 0) == -1)
        asfPrintError("Could not add ACQ_TIME field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Processing_Date") == 0 && 
	     dbf[ii].visible) {
      if (DBFAddField(dbase, "PROC_DATE", FTString, length, 0) == -1)
        asfPrintError("Could not add PROC_TIME field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Processing_Level") == 0 && 
	     dbf[ii].visible) {
      if (DBFAddField(dbase, "PROC_LEVEL", FTString, 10, 0) == -1)
        asfPrintError("Could not add PROC_LEVEL field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Start_Time") == 0 && dbf[ii].visible) {
      if (DBFAddField(dbase, "START_TIME", FTString, length, 0) == -1)
        asfPrintError("Could not add START_TIME field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "End_Time") == 0 && dbf[ii].visible) {
      if (DBFAddField(dbase, "END_TIME", FTString, length, 0) == -1)
        asfPrintError("Could not add END_TIME field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Center_Lat") == 0 && dbf[ii].visible) {
      if (DBFAddField(dbase, "CENTER_LAT", FTDouble, 16, 4) == -1)
        asfPrintError("Could not add CENTER_LAT field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Center_Lon") == 0 && dbf[ii].visible) {
      if (DBFAddField(dbase, "CENTER_LON", FTDouble, 16, 4) == -1)
        asfPrintError("Could not add CENTER_LON field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Near_Start_Lat") == 0 &&
         dbf[ii].visible) {
      if (DBFAddField(dbase, "NSTART_LAT", FTDouble, 16, 4) == -1)
        asfPrintError("Could not add NSTART_LAT field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Near_Start_Lon") == 0 &&
         dbf[ii].visible) {
      if (DBFAddField(dbase, "NSTART_LON", FTDouble, 16, 4) == -1)
        asfPrintError("Could not add NSTART_LON field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Far_Start_Lat") == 0 && dbf[ii].visible) {
      if (DBFAddField(dbase, "FSTART_LAT", FTDouble, 16, 4) == -1)
        asfPrintError("Could not add FSTART_LAT field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Far_Start_Lon") == 0 && dbf[ii].visible) {
      if (DBFAddField(dbase, "FSTART_LON", FTDouble, 16, 4) == -1)
        asfPrintError("Could not add FSTART_LON field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Near_End_Lat") == 0 && dbf[ii].visible) {
      if (DBFAddField(dbase, "N_END_LAT", FTDouble, 16, 4) == -1)
        asfPrintError("Could not add N_END_LAT field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Near_End_Lon") == 0 && dbf[ii].visible) {
      if (DBFAddField(dbase, "N_END_LON", FTDouble, 16, 4) == -1)
        asfPrintError("Could not add N_END_LON field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Far_End_Lat") == 0 && dbf[ii].visible) {
      if (DBFAddField(dbase, "F_END_LAT", FTDouble, 16, 4) == -1)
        asfPrintError("Could not add F_END_LAT field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Far_End_Lon") == 0 && dbf[ii].visible) {
      if (DBFAddField(dbase, "F_END_LON", FTDouble, 16, 4) == -1)
        asfPrintError("Could not add F_END_LON field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Faraday_Rotation") == 0 &&
         dbf[ii].visible) {
      if (DBFAddField(dbase, "FARADAYROT", FTDouble, 16, 1) == -1)
        asfPrintError("Could not add FARADAYROT field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Orbit_Direction") == 0 && 
	     dbf[ii].visible) {
      if (DBFAddField(dbase, "ORBIT_DIR", FTString, length, 0) == -1)
        asfPrintError("Could not add ORBIT_DIR field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Url") == 0 && dbf[ii].visible) {
      if (DBFAddField(dbase, "URL", FTString, length2, 0) == -1)
        asfPrintError("Could not add URL field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Size") == 0 &&
         dbf[ii].visible) {
      if (DBFAddField(dbase, "SIZE_MB", FTDouble, 16, 2) == -1)
        asfPrintError("Could not add SIZE_MB field to database file\n");
    }
    else if (strcmp(dbf[ii].header, "Off_Nadir_Angle") == 0 &&
         dbf[ii].visible) {
      if (DBFAddField(dbase, "OFF_NADIR", FTDouble, 16, 2) == -1)
        asfPrintError("Could not add OFF_NADIR field to database file\n");
    }
  }

  // Close the database for initialization
  DBFClose(dbase);

  // Open shapefile for initialization
  shape = SHPCreate(inFile, SHPT_POLYGON);
  if (!shape)
    asfPrintError("Could not create shapefile '%s'\n", inFile);

  // Close shapefile for initialization
  SHPClose(shape);

  FREE(dbaseFile);

  return;
}

static void get_polygon_location(datapool_type_t *datapool, 
				 double **plat, double **plon)
{
  double *lat = (double *) MALLOC(sizeof(double)*5);
  double *lon = (double *) MALLOC(sizeof(double)*5);
  lat[0] = lat[4] = datapool->near_start_lat;
  lon[0] = lon[4] = datapool->near_start_lon;
  lat[1] = datapool->far_start_lat;
  lon[1] = datapool->far_start_lon;
  lat[2] = datapool->far_end_lat;
  lon[2] = datapool->far_end_lon;
  lat[3] = datapool->near_end_lat;
  lon[3] = datapool->near_end_lon;
  *plat = lat;
  *plon = lon;
}

static void add_to_shape(DBFHandle dbase, SHPHandle shape, datapool_type_t *datapool,
			 dbf_header_t *dbf, int nCols, int n, 
			 double *lat, double *lon)
{
  int ii, field = 0;

  // Write fields into the database
  for (ii=0; ii<nCols; ii++) {
    if (strcmp(dbf[ii].header, "Granule_Name") == 0 && dbf[ii].visible) {
      DBFWriteStringAttribute(dbase, n, field, datapool->granule_name);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Platform") == 0 && dbf[ii].visible) {
      DBFWriteStringAttribute(dbase, n, field, datapool->platform);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Sensor") == 0 && dbf[ii].visible) {
      DBFWriteStringAttribute(dbase, n, field, datapool->sensor);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Beam_Mode") == 0 && dbf[ii].visible) {
      DBFWriteStringAttribute(dbase, n, field, datapool->beam_mode);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Beam_Mode_Description") == 0 && 
	     dbf[ii].visible) {
      DBFWriteStringAttribute(dbase, n, field, datapool->beam_mode_description);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Orbit") == 0 && dbf[ii].visible) {
      DBFWriteIntegerAttribute(dbase, n, field, datapool->orbit);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Path_Number") == 0 && dbf[ii].visible) {
      DBFWriteIntegerAttribute(dbase, n, field, datapool->path_number);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Frame_Number") == 0 && dbf[ii].visible) {
      DBFWriteIntegerAttribute(dbase, n, field, datapool->frame_number);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Acquisition_Date") == 0 &&
	     dbf[ii].visible) {
      DBFWriteStringAttribute(dbase, n, field, datapool->acquisition_date);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Processing_Date") == 0 && 
	     dbf[ii].visible) {
      DBFWriteStringAttribute(dbase, n, field, datapool->processing_date);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Processing_Level") == 0 &&
	     dbf[ii].visible) {
      DBFWriteStringAttribute(dbase, n, field, datapool->processing_level);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Start_Time") == 0 && dbf[ii].visible) {
      DBFWriteStringAttribute(dbase, n, field, datapool->start_time);
      field++;
    }
    else if (strcmp(dbf[ii].header, "End_Time") == 0 && dbf[ii].visible) {
      DBFWriteStringAttribute(dbase, n, field, datapool->end_time);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Center_Lat") == 0 && dbf[ii].visible) {
      DBFWriteDoubleAttribute(dbase, n, field, datapool->center_lat);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Center_Lon") == 0 && dbf[ii].visible) {
      DBFWriteDoubleAttribute(dbase, n, field, datapool->center_lon);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Near_Start_Lat") == 0 &&
	     dbf[ii].visible) {
      DBFWriteDoubleAttribute(dbase, n, field, datapool->near_start_lat);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Near_Start_Lon") == 0 &&
	     dbf[ii].visible) {
      DBFWriteDoubleAttribute(dbase, n, field, datapool->near_start_lon);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Far_Start_Lat") == 0 && dbf[ii].visible) {
      DBFWriteDoubleAttribute(dbase, n, field, datapool->far_start_lat);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Far_Start_Lon") == 0 && dbf[ii].visible) {
      DBFWriteDoubleAttribute(dbase, n, field, datapool->far_start_lon);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Near_End_Lat") == 0 && dbf[ii].visible) {
      DBFWriteDoubleAttribute(dbase, n, field, datapool->near_end_lat);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Near_End_Lon") == 0 && dbf[ii].visible) {
      DBFWriteDoubleAttribute(dbase, n, field, datapool->near_end_lon);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Far_End_Lat") == 0 && dbf[ii].visible) {
      DBFWriteDoubleAttribute(dbase, n, field, datapool->far_end_lat);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Far_End_Lon") == 0 && dbf[ii].visible) {
      DBFWriteDoubleAttribute(dbase, n, field, datapool->far_end_lon);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Faraday_Rotation") == 0 &&
	     dbf[ii].visible) {
      DBFWriteDoubleAttribute(dbase, n, field, datapool->faraday_rotation);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Orbit_Direction") == 0 && 
	     dbf[ii].visible) {
      DBFWriteStringAttribute(dbase, n, field, datapool->orbit_direction);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Url") == 0 && dbf[ii].visible) {
      DBFWriteStringAttribute(dbase, n, field, datapool->url);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Size") == 0 && dbf[ii].visible) {
      DBFWriteDoubleAttribute(dbase, n, field, datapool->size);
      field++;
    }
    else if (strcmp(dbf[ii].header, "Off_Nadir_Angle") == 0 &&
	     dbf[ii].visible) {
      DBFWriteDoubleAttribute(dbase, n, field, datapool->off_nadir_angle);
      field++;
    }
  }

  SHPObject *shapeObject=NULL;
  shapeObject = SHPCreateSimpleObject(SHPT_POLYGON, 5, lon, lat, NULL);
  SHPWriteObject(shape, -1, shapeObject);
  SHPDestroyObject(shapeObject);
}

int datapool2shape(char *inFile, char *outFile, int listFlag, int stack)
{
  DBFHandle dbase;
  SHPHandle shape;
  datapool_type_t datapool;
  dbf_header_t *dbf;
  char *header, line[1024];
  int nCols, nColumns, ii=0;
  double *lat, *lon;

  // Read configuration file
  read_header_config("DATAPOOL", &dbf, &nCols);

  // Read datapool file
  FILE *ifp = FOPEN(inFile, "r");
  assert(ifp);
  check_datapool_location(ifp, &header, &nColumns);

  // Initalize the database file
  shape_datapool_init(outFile, header);
  open_shape(outFile, &dbase, &shape);

  while (fgets(line, 1022, ifp) != NULL) {
    strip_end_whitesp(line);

    // now get the individual column values
    datapool_init(&datapool);
    if (read_datapool_line(header, nColumns, line, &datapool)) {
      if (ii == 0)
	get_polygon_location(&datapool, &lat, &lon);
      else if (!stack)
	get_polygon_location(&datapool, &lat, &lon);
      add_to_shape(dbase, shape, &datapool, dbf, nCols, ii, lat, lon);
      ii++;
    }
  }

  // Clean up
  close_shape(dbase, shape);
  write_esri_proj_file(outFile);

  FCLOSE(ifp);

  return 1;
}
