#pragma once
#ifndef distAdjust
#include"fileReader.h"

void adjustDistValue(VolumeData_t* model, VolumeData_t* dist);
int getWaterMarkingSize(VolumeData_t * distData);
Point_t* getWaterMarkingPos(VolumeData_t * distData);
unsigned char* generateModel(VolumeData_t* distData);

#endif // !distAdjust
