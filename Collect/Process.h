#pragma once
//struct Data
//{
//	float* data;
//	int* tag;
//	int SIZE;
//	int DIM;
//};

float* Add_Data(float* sample, int Size, float* x, int Dim);
float* Add_Labels(float* Labels, int Size, int label);
float* init_array_random(int len);
float* init_array_zero(int len);
void push_back(float*** momentW, int lIndex, int wIndex, int size, float item);
float* Batch_Norm(float* Samples, int numSample, int inputDim, float mean[], float variance[], bool copy = true);
int YPoint(int x, float w[], float bias, float Carpan = 1.0);