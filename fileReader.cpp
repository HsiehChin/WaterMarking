#include "fileReader.h"
#define MAX_THREAD_LIMIT 20
int  cmpAttribute(const char* str);
void changeSampleType(int data_type, VolumeData_t* volumeData);
int  getDataType(const char* type);
void readDistRawFile(const char* rawFileName, VolumeData_t* volumeData);
void readModelRawFile(const char* rawFileName, VolumeData_t* volumeData);

/********************************************************************************
* Read raw file.
* rawFileName: the path of the raw file.
* volumeData : store the info about voxelized and distanced model.
* Based on sampleType to distinguish voxelized model or distanced model.
*/
void readRawFile(const char* rawFileName, VolumeData_t* volumeData) {
	int dataType = getDataType(volumeData->sampleType);
	if (dataType == 0) readModelRawFile(rawFileName, volumeData);
	else if (dataType == 1) readDistRawFile(rawFileName, volumeData);
}

void readDistRawFile(const char* rawFileName, VolumeData_t* volumeData) {
	FILE* fp;
	fprintf(stderr, "%s\n", rawFileName);

	long long int volumeSize = getVolumeSize(volumeData);
	volumeData->distField = (DistanceField_t*)malloc(sizeof(DistanceField_t));
	volumeData->distField->oneD = (float*)malloc(sizeof(float) * volumeSize);

	fp = fopen(rawFileName, "rb");
	/*----Read file into oneD array----*/
	fread(volumeData->distField->oneD, sizeof(float), volumeSize, fp);
	fclose(fp);

	volumeData->distField->max = FLT_MIN;
	volumeData->distField->min = FLT_MAX;

	/*----Find the minimum and maximum of model----*/
	for (long long int index = 0; index < volumeSize; index++) {
		float temp = volumeData->distField->oneD[index];
		//if (temp > 1) printf("%lld	%f\n", index, temp);
		if (temp > (volumeData->distField->max))
			volumeData->distField->max = temp;
		if (temp < (volumeData->distField->min))
			volumeData->distField->min = temp;
		index++;
	}

	printf("max: %f, min: %f\n", volumeData->distField->max, volumeData->distField->min);

}

void readModelRawFile(const char* rawFileName, VolumeData_t* volumeData) {
	FILE* fp;
	fprintf(stderr, "%s\n", rawFileName);

	long long int volumeSize = 1;
	for (int i = 0; i < 3; i++)
		volumeSize *= volumeData->resolution[i];

	unsigned char* temp = (unsigned char*)malloc(sizeof(unsigned char) * volumeSize);
	volumeData->distField = (DistanceField_t*)malloc(sizeof(DistanceField_t));
	volumeData->distField->oneD = (float*)malloc(sizeof(float) * volumeSize);

	fp = fopen(rawFileName, "rb");
	/*----read file into oneD array----*/
	fread(temp, sizeof(unsigned char), volumeSize, fp);
	fclose(fp);

	for (long long int i = 0; i < volumeSize; i++) {
		volumeData->distField->oneD[i] = (int)temp[i];
	}
	free(temp);
}

/********************************************************************************
* Read inf file.
* infFileName: the path of the inf file.
* volumeData : store the info about voxelized and distanced model.
* modelValue : store the info about voxelized model.
*/
void readInfFile(const char* infFileName, VolumeData_t* volumeData) {

	FILE* fp;
	fprintf(stderr, "%s\n", infFileName);
	fp = fopen(infFileName, "r");

	if (fp == NULL) fprintf(stderr, "Open File Error!");//failed to open file
	else {//open file successful
		while (!feof(fp)) {
			char str[300];
			while (fgets(str, sizeof(str), fp) != NULL) {
				fprintf(stderr, "%s", str);
				for (int i = 0; i < 300; i++) {
					if (!((str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= '0' && str[i] <= '9') ||
						str[i] == '=' || str[i] == ':' || str[i] == '.' || str[i] == '-' || str[i] == ' '))
						str[i] = '\0';
					break;
				}
				switch (cmpAttribute(str)) {
				case 0://resolution
					sscanf(str, "%*[^=]=%dx%dx%d", &volumeData->resolution[0], &volumeData->resolution[1], &volumeData->resolution[2]);
					break;
				case 1://sample-type
					char tempStr[10];
					sscanf(str, "%*[^=]=%s", tempStr);
					if (!strncmp(tempStr, "unsigned", 8)) changeSampleType(0, volumeData);
					else if (!strncmp(tempStr, "float", 8)) changeSampleType(1, volumeData);
					else if (!strncmp(tempStr, "double", 8)) changeSampleType(2, volumeData);
					break;
				case 2://voxel-size
					sscanf(str, "%*[^=]=%f:%f:%f", &volumeData->voxelSize[0], &volumeData->voxelSize[1], &volumeData->voxelSize[2]);
					break;
				case 3://endian
					sscanf(str, "%*[^=]=%s", volumeData->endian);
					break;
				case 4://min
					sscanf(str, "%*[^=]=%f:%f:%f", &volumeData->min[0], &volumeData->min[1], &volumeData->min[2]);
					break;
				case 5://max
					sscanf(str, "%*[^=]=%f:%f:%f", &volumeData->max[0], &volumeData->max[1], &volumeData->max[2]);
					break;
				default:
					fprintf(stderr, "Unknow Attribute, ignore!\n");
					break;
				}
			}


		}
	}
}


/********************************************************************************
* Compare attributes of infFile.
* str: one line of infFile
*/
int cmpAttribute(const char* str) {
	if (!strncmp(str, "resol", 5) || !strncmp(str, "Resol", 5)) return 0;
	else if (!strncmp(str, "sample", 5) || !strncmp(str, "Sample", 5)) return 1;
	else if (!strncmp(str, "voxel", 5) || !strncmp(str, "ratio", 5)) return 2;
	else if (!strncmp(str, "endian", 5) || !strncmp(str, "Endian", 5)) return 3;
	else if (!strncmp(str, "min", 3) || !strncmp(str, "Min", 3)) return 4;
	else if (!strncmp(str, "max", 3) || !strncmp(str, "Max", 3)) return 5;
	else return -1;
}


/********************************************************************************
* Change sample type of volumeData.
* datatype: 0 = unsigned char; 1 = float; double = 2
*/
void changeSampleType(int data_type, VolumeData_t* volumeData) {
	if (data_type == 0) {//unsigned char
		memset(volumeData->sampleType, '\0', sizeof(volumeData->sampleType));
		strcat(volumeData->sampleType, "unsigned char");
	}
	else if (data_type == 1) {//float
		memset(volumeData->sampleType, '\0', sizeof(volumeData->sampleType));
		strcat(volumeData->sampleType, "float");
	}
	else if (data_type == 2) {//double
		memset(volumeData->sampleType, '\0', sizeof(volumeData->sampleType));
		strcat(volumeData->sampleType, "double");
	}
}


/********************************************************************************
* Get the data type of the input model
* type: the type in string
*/
int getDataType(const char* type) {
	if (strcmp(type, "unsigned char") == 0) return 0;
	else if (strcmp(type, "float") == 0) return 1;
}


/********************************************************************************
* Return the size of volumeData
*/
long long int getVolumeSize(VolumeData_t* data) {
	return (double)data->resolution[0] * data->resolution[1] * data->resolution[2];
}