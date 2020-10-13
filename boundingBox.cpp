#include "boudingBox.h"

/********************************************************************************
* Check the input value middle whether is between min and max.
*
*/
bool checkRange(int min, int middle, int max) {
	if (min <= middle && max >= middle) return true;
	else return false;
}

/********************************************************************************
* Set range of bounding box
*
* isoVal:iso-surface to add watermarking
* volumeData:distanced data
*/
void createBoundingBox(VolumeData_t* volumeData, BoundingBox_t * boundingBox) {
	char coord[3] = { 'x', 'y', 'z' };
	boundingBox->min = (int*)malloc(sizeof(int) * 3); 
	boundingBox->max = (int*)malloc(sizeof(int) * 3);

	/*----minimum must >= 0 and < resolution----*/
	printf("Please input bounding box minimum value((0, 0, 0) ~ (%d, %d, %d)):\n"
		, volumeData->resolution[0] - 2, volumeData->resolution[1] - 2, volumeData->resolution[2] - 2);
	for (int i = 0; i < 3; i++) {
		printf("min %c : ", coord[i]);
		scanf("%d", &boundingBox->min[i]);
		while (!checkRange(0, boundingBox->min[i], volumeData->resolution[i] - 2)) {
			printf("Input error,min %c must >= 0 and <= %d,please input again\n", coord[i], volumeData->resolution[i] - 2);
			scanf("%d", &boundingBox->min[i]);
		}
	}

	printf("Please input bounding box maximum value((%d, %d, %d) ~ (%d, %d, %d)):\n"
		, boundingBox->min[0] + 1, boundingBox->min[1] + 1, boundingBox->min[2] + 1
		, volumeData->resolution[0] - 1, volumeData->resolution[1] - 1, volumeData->resolution[2] - 1);
	for (int i = 0; i < 3; i++) {
		printf("max %c : ", coord[i]);
		scanf("%d", &boundingBox->max[i]);
		while (!checkRange(boundingBox->min[i] + 1, boundingBox->max[i], volumeData->resolution[i] - 1)) {
			printf("Input error,max %c must > %d and <= %d,please input again\n", coord[i], boundingBox->min[i], volumeData->resolution[i] - 1);
			scanf("%d", &boundingBox->max[i]);
		}
	}
}

/********************************************************************************
* Search voxels in BB and distance layer = isoVal
*
* box:bounding box
* isoVal:layer of iso-suface
* volumeData:distanced data
*/
float* checkValue(BoundingBox_t *box, int startIsoVal, int endIsoVal, VolumeData_t * volumeData) {
	DistanceField_t *volumeDist = volumeData->distField;
	long long int volSize = getVolumeSize(volumeData);
	float* resultRaw = (float*)malloc(sizeof(float) * volSize);
	for (long long int i = 0; i < volSize; i++) {
		resultRaw[i] = -1.0;
	}

	for (int isoVal = startIsoVal; isoVal <= endIsoVal; isoVal++) {
		int index, end = volumeData->distField->histTable[isoVal].frequency;
		for (index = 0; index < end; index++) {
			int x = volumeDist->layerCoord[isoVal][index].x;
			int y = volumeDist->layerCoord[isoVal][index].y;
			int z = volumeDist->layerCoord[isoVal][index].z;
			volumeData->distField->layerCoord[isoVal][index].selected = 0;
			/*----find voxels which 'min >= coordinate <= max' ----*/
			if (checkRange(box->min[0], x, box->max[0])
				&& checkRange(box->min[1], y, box->max[1])
				&& checkRange(box->min[2], z, box->max[2])) {
				volumeData->distField->layerCoord[isoVal][index].selected = 1; // 1 means inside selection box
			}
		}

		for (int i = 0; i < end; i++) {
			int x = volumeDist->layerCoord[isoVal][i].x;
			int y = volumeDist->layerCoord[isoVal][i].y;
			int z = volumeDist->layerCoord[isoVal][i].z;
			if (volumeData->distField->layerCoord[isoVal][i].selected == 1) {
				int temp = x + volumeData->resolution[0] * y
					+ volumeData->resolution[0] * volumeData->resolution[1] * z;
				resultRaw[temp] = volumeData->distField->oneD[temp];
			}
		}
	}

	return resultRaw;
}

/********************************************************************************
* Write raw file
*
* volumeData:distanced model
* result:chossed voxels' value
*/
void writeFloatRaw(VolumeData_t* volumeData, float* result, char * fileName) {
	FILE* file;
	char  path[500] = "./Output/";
	strcat(path, fileName);
	
	file = fopen(path, "wb");
	int sizeXYZ = getVolumeSize(volumeData);
	if (file) {
			fwrite(result, sizeof(float), sizeXYZ, file);
			fclose(file);
			printf("\nSave raw file finish\n");
	}
	else {
		printf("\nOpen failure\n");
	}	
}

void writeCharRaw(VolumeData_t* volumeData, unsigned char* result, char* fileName) {
	FILE* file;
	char  path[500] = "./Output/";
	strcat(path, fileName);

	file = fopen(path, "wb");
	int sizeXYZ = getVolumeSize(volumeData);
	if (file) {
		fwrite(result, sizeof(unsigned char), sizeXYZ, file);
		fclose(file);
		printf("\nSave raw file finish\n");
	}
	else {
		printf("\nOpen failure\n");
	}

}