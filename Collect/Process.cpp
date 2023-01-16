#include "pch.h"
#include <cmath>

float* Add_Data(float* sample, int Size, float* x, int Dim) {
	float* temp;
	temp = new float[Size * Dim];
	for (int i = 0; i < (Size - 1) * Dim; i++)
		temp[i] = sample[i];
	for (int i = 0; i < Dim; i++)
		temp[(Size - 1) * Dim + i] = x[i];
	//Deallocate memory
	delete[] sample;
	return temp;
}//Add_data
float* Add_Labels(float* Labels, int Size, int label) {
	float* temp;
	temp = new float[Size];
	for (int i = 0; i < Size - 1; i++)
		temp[i] = Labels[i];
	temp[Size - 1] = float(label);
	//Deallocate memory
	delete[] Labels;
	return temp;
}
float* init_array_random(int len) {
	float* arr = new float[len];
	for (int i = 0; i < len; i++)
		arr[i] = ((float)rand() / RAND_MAX) - 0.5f;
	return arr;
}
float* init_array_zero(int len) {
	float* arr = new float[len];
	for (int i = 0; i < len; i++)
		arr[i] = 0;
	return arr;
}
void push_back(float*** arr, int lIndex, int wIndex, int size, float item) {
	for (int t = 0; t < size - 1; t++) {
		arr[lIndex][wIndex][t] = arr[lIndex][wIndex][t + 1];
	}
	arr[lIndex][wIndex][size - 1] = item; //w_delta (t-1)
}
float* Batch_Norm(float* Samples, int numSample, int inputDim, float mean[], float variance[], bool copy)
{
	float* normalizedSamples = new float[numSample * inputDim];
	if (copy == true) {


		for (int i = 0; i < inputDim; i++)
			mean[i] = variance[i] = 0.0f;

		for (int i = 0; i < numSample; i++)
			for (int j = 0; j < inputDim; j++)
				mean[j] += Samples[i * inputDim + j];

		for (int i = 0; i < inputDim; i++)
			mean[i] /= numSample;

		for (int i = 0; i < numSample; i++)
			for (int j = 0; j < inputDim; j++)
				variance[j] += pow((Samples[i * inputDim + j] - mean[j]), 2);

		for (int i = 0; i < inputDim; i++)
			variance[i] /= numSample;

		for (int i = 0; i < numSample; i++)
			for (int j = 0; j < inputDim; j++)
				normalizedSamples[i * inputDim + j] = (Samples[i * inputDim + j] - mean[j]) / sqrt(variance[j]);
	}
	else
	{
		for (int i = 0; i < numSample; i++)
			for (int j = 0; j < inputDim; j++)
				normalizedSamples[i * inputDim + j] = (Samples[i * inputDim + j] - mean[j]) / sqrt(variance[j]);
	}

	return normalizedSamples;
}
int YPoint(int x, float w[], float bias, float Carpan = 1.0) {
	return (int)((Carpan * bias + w[0] * x) / (-w[1]));
}//YPoint