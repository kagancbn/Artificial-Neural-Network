#pragma once
#define BIAS 1.0
#define LEARNING_RATE 0.1
#define EMAX 0.01
#define CYCLE_MAX 30000
#define MOMENT_RATE 0.99
#define T_SIZE 2

struct Neuron
{
	float net;
	float output;
	Neuron() { net = 0, output = 0; };
};
struct Layer
{
	Neuron* neuron;
	int neuron_size;
	~Layer() {
		delete[] neuron;
	}
};
class ANN
{
public:

	ANN();
	~ANN();
	void Init(const int h_layer_count, int* neuron_count, const int inputDim, const int numClass);
	int TrainSGD(float* data, float* tag, int numSample);
	int TrainSGDwMoment(float* data, float* tag, int numSample);
	void Test(float* testData, int* tag, int DATASIZE);										// It takes data array and predicts data tags-labels
	void SaveWeights();
	void InitFromFile();
	double* Error_arr;
private:


	Layer* layer;
	float** weights;
	float** bias;
	int LAYERCOUNT;// HIDDEN LAYER COUNT
	int INPUTDIM; // INPUT DIMENSION
	int NUMCLASS; // CLASS COUNT
};

