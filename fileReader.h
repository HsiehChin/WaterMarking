#pragma once
#ifndef fileReader
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>
#include <string>

typedef struct Point_t {
	int selected;
	int x, y, z;
}Point_t;

typedef struct HistInfo_t {
	int layer;
	int frequency;
	struct HistInfo_t* nextPtr;
}HistInfo_t;

typedef struct DistanceField_t {
	float max, min;
	float* oneD; // oneD array, only stores value of voxel.
	Point_t** layerCoord; // record the coord of layer 0 to 11
	int histSize;
	HistInfo_t* histTable;
}DistanceField_t;


typedef struct VolumeData_t {
	int resolution[3];
	float voxelSize[3];
	char sampleType[15];
	char endian[10];
	float min[3];
	float max[3];
	DistanceField_t *distField;
}VolumeData_t;


void readInfFile(const char* , VolumeData_t *);
void readRawFile(const char* , VolumeData_t *);
long long int getVolumeSize(VolumeData_t*);

#endif // !fileReader
