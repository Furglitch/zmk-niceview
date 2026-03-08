#ifndef SHAYMIN_WPM_IMAGES_H
#define SHAYMIN_WPM_IMAGES_H
#ifdef __cplusplus
extern "C" {
#endif
#include "lvgl.h"
// -------- Image Descriptors Declarations --------
LV_IMG_DECLARE(shaymin_wpm_00);
LV_IMG_DECLARE(shaymin_wpm_01);
LV_IMG_DECLARE(shaymin_wpm_02);
LV_IMG_DECLARE(shaymin_wpm_03);

// -------- Array of Pointers to Image Descriptors --------
const lv_img_dsc_t *shaymin_wpm_images[4] = {
    &shaymin_wpm_00,
    &shaymin_wpm_01,
    &shaymin_wpm_02,
    &shaymin_wpm_03,
};

#define SHAYMIN_WPM_IMAGES_NUM_IMAGES 4

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* SHAYMIN_WPM_IMAGES_H */
