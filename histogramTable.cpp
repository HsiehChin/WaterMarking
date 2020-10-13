#include "histogramTable.h"
#define MAX_LAYER_LIMIT 12

/********************************************************************************
* Create histogram of distanced model.
* Floor the value of model to classify layer 0 to 11.
*
* volumeData: distanced model
*/
float* computeHistogram(VolumeData_t* volumeData) {

	/*----Initialize the histogram table----*/
	if (volumeData->distField != NULL) {
		volumeData->distField->histSize = MAX_LAYER_LIMIT;
		volumeData->distField->histTable = (HistInfo_t*)malloc(sizeof(HistInfo_t) * MAX_LAYER_LIMIT);
		volumeData->distField->layerCoord = (Point_t**)malloc(sizeof(Point_t*) * MAX_LAYER_LIMIT);
		for (int i = 0; i < MAX_LAYER_LIMIT; i++) {
			volumeData->distField->histTable[i].layer = i;// 0 ~ 11
			volumeData->distField->histTable[i].frequency = 0;
		}

		/*----Evaluate the frequency of layer 0 to 11----*/
		long long int volumeSize = getVolumeSize(volumeData), index = 0;
		for (long long int i = 0; i < volumeSize; i++) {
			float temp = volumeData->distField->oneD[i];
			if (temp >= 0 && (int)(temp + 0.5) < MAX_LAYER_LIMIT) {
				int layerValue = (int)(temp + 0.5);
				volumeData->distField->histTable[layerValue].frequency++;
			}
		}
		/*----Allocate memory----*/
		for (int i = 0; i < MAX_LAYER_LIMIT; i++) {
			int frequency = volumeData->distField->histTable[i].frequency;
			volumeData->distField->layerCoord[i] = (Point_t*)malloc(sizeof(Point_t) * frequency);
		}

		/*----Assign the coordinate of voxel----*/
		int layerIndex[MAX_LAYER_LIMIT] = { 0 };
		for (int z = 0; z < volumeData->resolution[2]; z++) {
			for (int y = 0; y < volumeData->resolution[1]; y++) {
				for (int x = 0; x < volumeData->resolution[0]; x++) {
					float temp = volumeData->distField->oneD[index++];
					if (temp >= 0 && (int)(temp + 0.5) < MAX_LAYER_LIMIT) {
						int layerValue = (int)(temp + 0.5);
						int lc = layerIndex[layerValue];
						volumeData->distField->layerCoord[layerValue][lc].selected = 0;
						volumeData->distField->layerCoord[layerValue][lc].x = x;
						volumeData->distField->layerCoord[layerValue][lc].y = y;
						volumeData->distField->layerCoord[layerValue][lc].z = z;
						layerIndex[layerValue]++;
					}
				}
			}
		}
	}
	float* data = percentage(volumeData);
	return data;
}

/********************************************************************************
* Compute percentage of all layer frequrncy
* Make all frequency number between 0.0~1.0*5
*
* volumeData : distanced model
*/
float* percentage(VolumeData_t* volumeData) {
	float data_f[MAX_LAYER_LIMIT] = { 0.0 };
	int sum = 0;
	for (int i = 0; i < MAX_LAYER_LIMIT; i++) {
		printf("layer:%d, freq: %d\n", i, volumeData->distField->histTable[i].frequency);
		sum += volumeData->distField->histTable[i].frequency;
	}
	printf("sum: %d\n", sum);
	for (int i = 0; i < MAX_LAYER_LIMIT; i++) {
		/*----histogram too short so make it 5 times longer----*/
		data_f[i] = 5*(float)(volumeData->distField->histTable[i].frequency)/sum;
		//printf("layer:%d, freq: %f\n", i, data_f[i]);
	}
	return data_f;
}