#ifndef __Utils__
#define __Utils__

#include <stdint.h>


using uint32 = uint32_t; //!< Беззнаковое целое число длиной 4 байта
const int NUMBER_SIZE = 4; //!< Размер чисел, которые мы будем читать, анализировать (4 байта)
const size_t ARRAY_SIZE = 26214400 / 100; //!< Размер массива в 100 Мб 4 байтных чисел - 26214400/ 100 - значение для input

#endif
