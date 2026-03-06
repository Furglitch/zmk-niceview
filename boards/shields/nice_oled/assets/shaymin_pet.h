#ifndef SHAYMIN_PET_IMAGES_H
#define SHAYMIN_PET_IMAGES_H
#ifdef __cplusplus
extern "C" {
#endif
#include "lvgl.h"
// -------- Image Descriptors Declarations --------
LV_IMG_DECLARE(shaymin_pet_00);
LV_IMG_DECLARE(shaymin_pet_01);
LV_IMG_DECLARE(shaymin_pet_02);
LV_IMG_DECLARE(shaymin_pet_03);

// -------- Array of Pointers to Image Descriptors --------
const lv_img_dsc_t *shaymin_pet_images[4] = {
    &shaymin_pet_00,
    &shaymin_pet_01,
    &shaymin_pet_02,
    &shaymin_pet_03,
};

#define SHAYMIN_PET_IMAGES_NUM_IMAGES 4

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* SHAYMIN_PET_IMAGES_H */
