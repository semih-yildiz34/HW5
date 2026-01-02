/*
 * lib_image.c
 */

#include "lib_image.h"
#include <math.h>

#define __LIB_IMAGE_CHECK_PARAM(param)				{if(param == 0) return IMAGE_ERROR;}

/**
  * @brief Initialize the image structure with required information
  * @param img     Pointer to image structure
  * @param pImg    Pointer to image buffer
  * @param height  height of the image
  * @param width   width of the image
  * @param format  Choose IMAGE_FORMAT_GRAYSCALE, IMAGE_FORMAT_RGB565, or IMAGE_FORMAT_RGB888
  * @retval 0 if successfully initialized
  */
int8_t LIB_IMAGE_InitStruct(IMAGE_HandleTypeDef * img, uint8_t *pImg, uint16_t height, uint16_t width, IMAGE_Format format)
{
	__LIB_IMAGE_CHECK_PARAM(img);
	__LIB_IMAGE_CHECK_PARAM(pImg);
	__LIB_IMAGE_CHECK_PARAM(format);
	__LIB_IMAGE_CHECK_PARAM(width);
	__LIB_IMAGE_CHECK_PARAM(height);
	img->format = format;
	img->height = height;
	img->width 	= width;
	img->pData 	= pImg;
	img->size 	= (uint32_t)img->format * (uint32_t)img->height * (uint32_t)img->width;
	return IMAGE_OK;
}

/**
  * @brief Calculate Hu moments from grayscale image (binaryImage=True like OpenCV)
  * @param img Pointer to grayscale image (must be IMAGE_FORMAT_GRAYSCALE)
  * @param huMoments Output array for 7 Hu moments (must be allocated)
  * @retval IMAGE_OK if successful, IMAGE_ERROR otherwise
  * 
  * Algorithm matches OpenCV's cv2.moments(img, binaryImage=True) and cv2.HuMoments()
  * Non-zero pixels are treated as 1.0, zero pixels as 0.0
  */
int8_t LIB_IMAGE_CalculateHuMoments(IMAGE_HandleTypeDef * img, float *huMoments)
{
	// Validate input: must be grayscale image
	if (img->format != IMAGE_FORMAT_GRAYSCALE || huMoments == NULL)
	{
		return IMAGE_ERROR;
	}
	
	uint8_t *pData = img->pData;
	
	// ========================================================================
	// STEP 1: CALCULATE RAW MOMENTS (binaryImage=True: non-zero = 1.0)
	// ========================================================================
	float m00 = 0.0f, m10 = 0.0f, m01 = 0.0f;
	float m11 = 0.0f, m20 = 0.0f, m02 = 0.0f;
	float m12 = 0.0f, m21 = 0.0f, m30 = 0.0f, m03 = 0.0f;
	
	for (uint16_t y = 0; y < img->height; y++)
	{
		for (uint16_t x = 0; x < img->width; x++)
		{
			uint8_t pixel = pData[y * img->width + x];
			float weight = (pixel > 0) ? 1.0f : 0.0f;  // Binary: non-zero = 1.0
			
			float xf = (float)x;
			float yf = (float)y;
			
			m00 += weight;
			m10 += xf * weight;
			m01 += yf * weight;
			m11 += xf * yf * weight;
			m20 += xf * xf * weight;
			m02 += yf * yf * weight;
			m12 += xf * yf * yf * weight;
			m21 += xf * xf * yf * weight;
			m30 += xf * xf * xf * weight;
			m03 += yf * yf * yf * weight;
		}
	}
	
	// Avoid division by zero
	if (m00 == 0.0f)
	{
		return IMAGE_ERROR;
	}
	
	// ========================================================================
	// STEP 2: CALCULATE CENTROID
	// ========================================================================
	float cx = m10 / m00;
	float cy = m01 / m00;
	
	// ========================================================================
	// STEP 3: CALCULATE CENTRAL MOMENTS
	// ========================================================================
	float mu11 = m11 - cy * m10;
	float mu20 = m20 - cx * m10;
	float mu02 = m02 - cy * m01;
	float mu30 = m30 - 3.0f * cx * m20 + 2.0f * cx * cx * m10;
	float mu03 = m03 - 3.0f * cy * m02 + 2.0f * cy * cy * m01;
	float mu12 = m12 - 2.0f * cy * m11 - cx * m02 + 2.0f * cy * cy * m10;
	float mu21 = m21 - 2.0f * cx * m11 - cy * m20 + 2.0f * cx * cx * m01;
	
	// ========================================================================
	// STEP 4: CALCULATE NORMALIZED CENTRAL MOMENTS
	// ========================================================================
	float m00_2 = m00 * m00;
	float m00_2_5 = m00_2 * m00 * sqrtf(m00);
	
	float eta11 = mu11 / m00_2;
	float eta20 = mu20 / m00_2;
	float eta02 = mu02 / m00_2;
	float eta30 = mu30 / m00_2_5;
	float eta03 = mu03 / m00_2_5;
	float eta12 = mu12 / m00_2_5;
	float eta21 = mu21 / m00_2_5;
	
	// ========================================================================
	// STEP 5: CALCULATE HU MOMENTS
	// ========================================================================
	huMoments[0] = eta20 + eta02;
	huMoments[1] = (eta20 - eta02) * (eta20 - eta02) + 4.0f * eta11 * eta11;
	huMoments[2] = (eta30 - 3.0f * eta12) * (eta30 - 3.0f * eta12) + 
	               (3.0f * eta21 - eta03) * (3.0f * eta21 - eta03);
	huMoments[3] = (eta30 + eta12) * (eta30 + eta12) + 
	               (eta21 + eta03) * (eta21 + eta03);
	huMoments[4] = (eta30 - 3.0f * eta12) * (eta30 + eta12) * 
	               ((eta30 + eta12) * (eta30 + eta12) - 3.0f * (eta21 + eta03) * (eta21 + eta03)) +
	               (3.0f * eta21 - eta03) * (eta21 + eta03) * 
	               (3.0f * (eta30 + eta12) * (eta30 + eta12) - (eta21 + eta03) * (eta21 + eta03));
	huMoments[5] = (eta20 - eta02) * ((eta30 + eta12) * (eta30 + eta12) - (eta21 + eta03) * (eta21 + eta03)) +
	               4.0f * eta11 * (eta30 + eta12) * (eta21 + eta03);
	huMoments[6] = (3.0f * eta21 - eta03) * (eta30 + eta12) * 
	               ((eta30 + eta12) * (eta30 + eta12) - 3.0f * (eta21 + eta03) * (eta21 + eta03)) -
	               (eta30 - 3.0f * eta12) * (eta21 + eta03) * 
	               (3.0f * (eta30 + eta12) * (eta30 + eta12) - (eta21 + eta03) * (eta21 + eta03));
	
	return IMAGE_OK;
}
