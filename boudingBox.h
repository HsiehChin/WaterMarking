#pragma once
#ifndef boundingBox
#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include <filesystem>
#include "fileReader.h"
#include "histogramTable.h"

typedef struct Coord_t {
	int x, y, z;
	int data;
}Coord_t;

typedef struct BoundingBox_t {
	int *min;
	int *max;
	Coord_t *isoSurface;//record output coordinate
}BoundingBox_t;

float* checkValue(BoundingBox_t *box, int startIsoVal, int endIsoVal, VolumeData_t * volumeData);
void createBoundingBox(VolumeData_t* volumeData, BoundingBox_t *box);
void writeFile(BoundingBox_t box, int index);
void writeFloatRaw(VolumeData_t* volumeData, float* result, char* fileName);
void writeCharRaw(VolumeData_t* volumeData, unsigned char* result, char* fileName);

#endif // !boundingBox