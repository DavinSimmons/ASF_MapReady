#include "asf.h"
#include "asf_meta.h"


meta_state_vectors *meta_state_vectors_init(int vector_count);



meta_parameters *meta_copy(meta_parameters *src)
{
  meta_parameters *ret = raw_init();
  ret->meta_version = src->meta_version;

  // the general block should never be NULL, but we might as well handle
  // all scenarios in here
  if (src->general) {
    if (!ret->general) ret->general = meta_general_init();
    memcpy(ret->general, src->general, sizeof(meta_general));
  } else
    ret->general = NULL;

  if (src->sar) {
    if (!ret->sar) ret->sar = meta_sar_init();
    memcpy(ret->sar, src->sar, sizeof(meta_sar));
  } else
    ret->sar = NULL;

  if (src->projection) {
    if (!ret->projection)
      ret->projection = (meta_projection*)MALLOC(sizeof(meta_projection));
    memcpy(ret->projection, src->projection, sizeof(meta_projection));
  } else
    ret->projection = NULL;

  if (src->state_vectors) {
    int vector_count = src->state_vectors->vector_count;
    ret->state_vectors = meta_state_vectors_init(vector_count);
    memcpy(ret->state_vectors, src->state_vectors, sizeof(meta_state_vectors)+vector_count*sizeof(state_loc));
  } else
    ret->state_vectors = NULL;

  if (src->stats) {
    if (!ret->stats) ret->stats = meta_statistics_init(src->stats->band_count);
    memcpy(ret->stats, src->stats, sizeof(meta_stats));
  } else
    ret->stats = NULL;

  if (src->transform) {
    if (!ret->transform) ret->transform = meta_transform_init();
    memcpy(ret->transform, src->transform, sizeof(meta_transform));
  } else
    ret->transform = NULL;

  if (src->location) {
    if (!ret->location) ret->location = meta_location_init();
    memcpy(ret->location, src->location, sizeof(meta_location));
  } else
    ret->location = NULL;

  if (src->airsar) {
    if (!ret->airsar) ret->airsar = meta_airsar_init();
    memcpy(ret->airsar, src->airsar, sizeof(meta_airsar));
  } else
    ret->airsar = NULL;

  if (src->optical) {
    if (!ret->optical) ret->optical = meta_optical_init();
    memcpy(ret->optical, src->optical, sizeof(meta_optical));
  } else
    ret->optical = NULL;

  if (src->thermal) {
    if (!ret->thermal) ret->thermal = meta_thermal_init();
    memcpy(ret->thermal, src->thermal, sizeof(meta_thermal));
  } else
    ret->thermal = NULL;

  if (src->uavsar) {
    if (!ret->uavsar) ret->uavsar = meta_uavsar_init();
    memcpy(ret->uavsar, src->uavsar, sizeof(meta_uavsar));
  } else
    ret->uavsar = NULL;

  if (src->insar) {
    if (!ret->insar) ret->insar = meta_insar_init();
    memcpy(ret->insar, src->insar, sizeof(meta_insar));
  } else
    ret->insar = NULL;

  if (src->dem) {
    if (!ret->dem) ret->dem = meta_dem_init();
    memcpy(ret->dem, src->dem, sizeof(meta_dem));
  } else
    ret->dem = NULL;

  if (src->calibration) {
    if (!ret->calibration) ret->calibration = meta_calibration_init();
    memcpy(ret->calibration, src->calibration, sizeof(meta_calibration));
    if(src->calibration->asf) {
      ret->calibration->asf = (asf_cal_params *) MALLOC(sizeof(asf_cal_params));
      memcpy(ret->calibration->asf, src->calibration->asf, sizeof(asf_cal_params));
    }
    if(src->calibration->asf_scansar) {
      ret->calibration->asf_scansar = (asf_scansar_cal_params *) MALLOC(sizeof(asf_scansar_cal_params));
      memcpy(ret->calibration->asf_scansar, src->calibration->asf_scansar, sizeof(asf_scansar_cal_params));
    }
    if(src->calibration->esa) {
      ret->calibration->esa = (esa_cal_params *) MALLOC(sizeof(esa_cal_params));
      memcpy(ret->calibration->esa, src->calibration->esa, sizeof(esa_cal_params));
    }
    if(src->calibration->rsat) {
      ret->calibration->rsat = (rsat_cal_params *) MALLOC(sizeof(rsat_cal_params));
      memcpy(ret->calibration->rsat, src->calibration->rsat, sizeof(rsat_cal_params));
    }
    if(src->calibration->alos) {
      ret->calibration->alos = (alos_cal_params *) MALLOC(sizeof(alos_cal_params));
      memcpy(ret->calibration->alos, src->calibration->alos, sizeof(alos_cal_params));
    }
    if(src->calibration->tsx) {
      ret->calibration->tsx = (tsx_cal_params *) MALLOC(sizeof(tsx_cal_params));
      memcpy(ret->calibration->tsx, src->calibration->tsx, sizeof(tsx_cal_params));
    }
    if(src->calibration->uavsar) {
      ret->calibration->uavsar = 
	(uavsar_cal_params *) MALLOC(sizeof(uavsar_cal_params));
      memcpy(ret->calibration->uavsar, src->calibration->uavsar,
	     sizeof(uavsar_cal_params));
    }
  } else
    ret->calibration = NULL;

  if (src->colormap) {
    // free default created one, if there
    if (ret->colormap) {
      FREE(ret->colormap->rgb);
      FREE(ret->colormap);
    }
    // now create the copy
    ret->colormap = meta_colormap_init();
    memcpy(ret->colormap, src->colormap, sizeof(meta_colormap));
    size_t sz = sizeof(meta_rgb)*ret->colormap->num_elements;
    ret->colormap->rgb = MALLOC(sz);
    memcpy(ret->colormap->rgb, src->colormap->rgb, sz);
  }

/* Copy Depricated structures
  memcpy(ret->geo, src->geo, sizeof(geo_parameters));
  memcpy(ret->ifm, src->ifm, sizeof(ifm_parameters));
  if (src->info) {
    ret->info = (extra_info*)MALLOC(sizeof(extra_info));
    memcpy(ret->info, src->info, sizeof(extra_info));
  }

* Set depricated redundant stVec to point at new state vectors structure *
ret->stVec = ret->state_vectors;*/

  return ret;
}
