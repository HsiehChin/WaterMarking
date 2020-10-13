#pragma once
#ifndef histogramTable
#include "fileReader.h"
#include <math.h>

float* computeHistogram(VolumeData_t* );
float* percentage(VolumeData_t* volumeData);

#endif // !histogramTable