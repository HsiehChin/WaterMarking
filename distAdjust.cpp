#include "fileReader.h";
#include "boudingBox.h";
/********************************************************************************
* Compare the distanced model and original model.
* If the distance outside the original model, set the value of voxel to negative,
* else keep the original value.
*  
* model: original data
* dist : distanced data
*/
void adjustDistValue(VolumeData_t* model, VolumeData_t* dist) {

	/*----Use threeD to keep the model data----*/
	long long int distSize = getVolumeSize(dist), modelSize = getVolumeSize(model);
	long long int index = 0;
	int extendedWidth = (dist->resolution[0] - model->resolution[0]) / 2;
	int*** threeD = (int***)malloc(sizeof(int**) * dist->resolution[0]); 

	for (int i = 0; i < dist->resolution[0]; i++) {
		threeD[i] = (int**)malloc(sizeof(int*) * dist->resolution[1]);
	}
	for (int i = 0; i < dist->resolution[0]; i++) {
		for (int j = 0; j < dist->resolution[1]; j++) {
			threeD[i][j] = (int*)malloc(sizeof(int) * dist->resolution[2]);
		}
	}

	/*----Initialize the extended voxel to zero----*/
	for (int z = 0; z < dist->resolution[2]; z++) {
		for (int y = 0; y < dist->resolution[1]; y++) {
			for (int x = 0; x < dist->resolution[0]; x++) {
				threeD[x][y][z] = 0; //zero means AIR
			}
		}
	}

	/*----Assign the value of model to threeD----*/
	index = 0;
	for (int z = extendedWidth; z < model->resolution[2] + extendedWidth; z++) {
		for (int y = extendedWidth; y < model->resolution[1] + extendedWidth; y++) {
			for (int x = extendedWidth; x < model->resolution[0] + extendedWidth; x++) {
				threeD[x][y][z] = (int)model->distField->oneD[index++];
			}
		}
	}

	/*----Compare and adjust the value of dist----*/
	index = 0;
	int newBBValue = -10000;
	int waterMarkValue = -10, newWaterMarkValue = -20000;
	for (int z = 0; z < dist->resolution[2]; z++) {
		for (int y = 0; y < dist->resolution[1]; y++) {
			for (int x = 0; x < dist->resolution[0]; x++) {
				if ((int)dist->distField->oneD[index] == -1)
					dist->distField->oneD[index] = newBBValue;
				if (threeD[x][y][z] == 0 && (int)dist->distField->oneD[index] != newBBValue)//Not inside the original model
					dist->distField->oneD[index] = -dist->distField->oneD[index];
				if (threeD[x][y][z] != 0 && (int)dist->distField->oneD[index] == waterMarkValue)//Not inside the original model
					dist->distField->oneD[index] = newWaterMarkValue;

				index++;
			}
		}
	}
	/*----Release the memory of threeD----*/
	for (int i = 0; i < dist->resolution[0]; i++) {
		for (int j = 0; j < dist->resolution[1]; j++) {
			free(threeD[i][j]);
		}
	}
	for (int i = 0; i < dist->resolution[0]; i++) {
		free(threeD[i]);
	}
	//free(model->distField->oneD);
}

int getWaterMarkingSize(VolumeData_t * distData) {
	int waterMarkingSize = 0, size = getVolumeSize(distData);
	for (int i = 0; i < size; i++)
		if ((int)distData->distField->oneD[i] == -20000) waterMarkingSize++;
	return waterMarkingSize;
}

Point_t* getWaterMarkingPos(VolumeData_t * distData) {
	Point_t * temp;
	int waterMarkingSize = getWaterMarkingSize(distData);
	temp = (Point_t*)malloc(sizeof(Point_t) * waterMarkingSize);

	if (temp) {
		int index = 0, t_index = 0;
		for (int z = 0; z < distData->resolution[2]; z++) {
			for (int y = 0; y < distData->resolution[1]; y++) {
				for (int x = 0; x < distData->resolution[0]; x++) {
					if ((int)distData->distField->oneD[index] == -20000) {
						temp[t_index].x = x;
						temp[t_index].y = y;
						temp[t_index].z = z;
						t_index++;

					}
					index++;
				}
			}
		}
	}
	return temp;
}


unsigned char* generateModel(VolumeData_t* distData) {
	int distSize = getVolumeSize(distData);
	/*----put result in this array----*/
	unsigned char* resultRaw = (unsigned char*)malloc(sizeof(unsigned char) * distSize);
	for (int i = 0; i < distSize; i++) {
		if (distData->distField->oneD[i] >= 0) resultRaw[i] = 255;
		else resultRaw[i] = 0;
	}
	return resultRaw;
}