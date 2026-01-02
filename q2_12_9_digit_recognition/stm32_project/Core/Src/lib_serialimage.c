/*
 * lib_serialimage.c
 */

#include "lib_serialimage.h"

/**
  * @brief Receives an image with its required information
  * @param img Pointer to image structure
  * @retval 0 if successfully transmitted
  */
int8_t LIB_SERIAL_IMG_Receive(IMAGE_HandleTypeDef * img)
{
	uint8_t __header[3] = "STR", __count = 0;
	uint16_t __quotient = 0, __remainder = 0;
	uint16_t divisor = UINT16_MAX;
	uint8_t * __pData = img->pData;

	__quotient 	= img->size / divisor;
	__remainder = img->size % divisor;

	HAL_UART_Transmit(&__huart, __header, 3, 10);
	HAL_UART_Transmit(&__huart, (uint8_t*)&img->height, 2, 10);
	HAL_UART_Transmit(&__huart, (uint8_t*)&img->width,  2, 10);
	HAL_UART_Transmit(&__huart, (uint8_t*)&img->format, 1, 10);
	while(__count < __quotient)
	{
		if(HAL_UART_Receive(&__huart, __pData, UINT16_MAX, 10000) != HAL_OK)
		{
			return SERIAL_ERROR;
		}
		__count++;
		__pData += UINT16_MAX;
	}
	if (__remainder)
	{
		if(HAL_UART_Receive(&__huart, __pData, __remainder, 10000) != HAL_OK)
		{
			return SERIAL_ERROR;
		}
	}
	return SERIAL_OK;
}

/**
  * @brief Send a single digit (0-9) over UART
  * @param digit Digit value (0-9)
  * @retval SERIAL_OK if successful, SERIAL_ERROR otherwise
  */
int8_t LIB_SERIAL_SendDigit(uint8_t digit)
{
	if (digit > 9)
	{
		return SERIAL_ERROR;  // Invalid digit
	}
	
	// Send digit as single byte
	if (HAL_UART_Transmit(&__huart, &digit, 1, 100) == HAL_OK)
	{
		return SERIAL_OK;
	}
	return SERIAL_ERROR;
}


//
//void LIB_SERIAL_ImageCapture(IMAGE_HandleTypeDef * img)
//{
//    uint8_t request_start_sequence[3] = "STR";
//
//    uint16_t _blocksize = 65535, _lastblocksize = 0;
//    uint32_t i = 0, _blockCount = 0;
//
//    if (img->size.size < 65536)
//        _blocksize = img->size.size;
//
//    _blockCount = img->size.size / _blocksize;
//    _lastblocksize = (uint16_t)(img->size.size % _blocksize);
//
//    HAL_UART_Transmit(&huart1, request_start_sequence, 3, 100);
//    HAL_Delay(1);
//
//    HAL_UART_Transmit(&huart1, (uint8_t *)&img->size.width, sizeof(img->size.width), 100);
//    HAL_UART_Transmit(&huart1, (uint8_t *)&img->size.height, sizeof(img->size.height), 100);
//    HAL_UART_Transmit(&huart1, (uint8_t *)&img->format, sizeof(img->format), 100);
//
//    for (i = 0; i < _blockCount; i++)
//        HAL_UART_Receive(&huart1, img->pData + (i * _blocksize), _blocksize, 10000);
//
//    if (_lastblocksize)
//        HAL_UART_Receive(&huart1, img->pData + (i * _blocksize), _lastblocksize, 10000);
//}
//
//void LIB_SERIAL_ImageSend(IMAGE_HandleTypeDef * img)
//{
//    uint8_t request_start_sequence[3] = "STW";
//
//    uint16_t _blocksize = 65535, _lastblocksize = 0;
//    uint32_t i = 0, _blockCount = 0;
//
//    if (img->size.size < 65536)
//        _blocksize = img->size.size;
//
//    _blockCount = img->size.size / _blocksize;
//    _lastblocksize = (uint16_t)(img->size.size % _blocksize);
//
//    HAL_UART_Transmit(&huart1, request_start_sequence, 3, 100);
//    HAL_Delay(1);
//
//    HAL_UART_Transmit(&huart1, (uint8_t *)&img->size.width, sizeof(img->size.width), 100);
//    HAL_UART_Transmit(&huart1, (uint8_t *)&img->size.height, sizeof(img->size.height), 100);
//    HAL_UART_Transmit(&huart1, (uint8_t *)&img->format, sizeof(img->format), 100);
//    HAL_Delay(200);
//
//    for (i = 0; i < _blockCount; i++)
//        HAL_UART_Transmit(&huart1, img->pData + (i * _blocksize), _blocksize, 100000);
//
//    if (_lastblocksize)
//        HAL_UART_Transmit(&huart1, img->pData + (i * _blocksize), _lastblocksize, 100000);
//
//    HAL_Delay(200);
//}
