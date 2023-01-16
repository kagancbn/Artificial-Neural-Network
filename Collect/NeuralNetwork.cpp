#include "pch.h"
#include "NeuralNetwork.h"
#include "Process.h"
#include <math.h>
#include <cfloat>
#include <fstream>
void ANN::Init(const int h_layer_count, int* neuron_count, const int inputDim, const int numClass)
{
	this->layer = new Layer[h_layer_count + 1];// hidden layer count + output layer
	//Hidden layers
	for (int i = 0; i < h_layer_count; i++)
	{
		this->layer[i].neuron = new Neuron[neuron_count[i]];
		this->layer[i].neuron_size = neuron_count[i];
	}
	//Output layer initializin according to Class numbers
	this->layer[h_layer_count].neuron = new Neuron[numClass];
	this->layer[h_layer_count].neuron_size = numClass;

	//---Weights initializing-----input--hidden--output

	//input layer's weights
	this->weights = new float* [h_layer_count + 1];
	this->bias = new float* [h_layer_count + 1];				 // (+1 for input layer bias) 
	this->weights[0] = init_array_random(inputDim * neuron_count[0]);	// 2D vector in 1D array 
	this->bias[0] = init_array_random(neuron_count[0]);					// 1D vector
	//hidden layer's weigths
	for (int i = 0; i < h_layer_count - 1; i++)
	{
		this->weights[i + 1] = init_array_random(neuron_count[i] * neuron_count[i + 1]);
		this->bias[i + 1] = init_array_random(neuron_count[i + 1]);
	}

	//output layer's weights
	this->weights[h_layer_count] = init_array_random(neuron_count[h_layer_count - 1] * numClass);
	this->bias[h_layer_count] = init_array_random(numClass);

	this->LAYERCOUNT = h_layer_count;
	this->INPUTDIM = inputDim;
	this->NUMCLASS = numClass;

}
int ANN::TrainSGD(float* data, float* tag, int numSample)
{
	float desired, total_error = 0, error_rmse = 0 , error_mse;
	this->Error_arr = new double[CYCLE_MAX];

	float** deltaSignal = new float* [this->LAYERCOUNT + 1];

	for (int l = 0; l < this->LAYERCOUNT + 1; l++)
		deltaSignal[l] = new float[layer[l].neuron_size];

	for (int cycle = 0; cycle < CYCLE_MAX; cycle++)
	{
		total_error = 0;

		for (int sample = 0; sample < numSample; sample++)
		{
			//Input Layer Feed Forward 
			for (int j = 0; j < layer[0].neuron_size; j++)
				layer[0].neuron[j].net = 0;

			for (int j = 0; j < layer[0].neuron_size; j++)
			{
				for (int i = 0; i < this->INPUTDIM; i++)
				{
					layer[0].neuron[j].net += data[(sample * this->INPUTDIM) + i] * this->weights[0][(j * this->INPUTDIM) + i];
				}//feedforward matrix-vector multiplication
				layer[0].neuron[j].net += bias[0][j];							// BIAS
				layer[0].neuron[j].output = (float)tanh(layer[0].neuron[j].net);// ACTIVATION
			}//Neuron Loop


			//Hidden and Output Layer Feed Forward
			for (int l = 1; l < this->LAYERCOUNT + 1; l++)
			{
				for (int j = 0; j < layer[l].neuron_size; j++)
					layer[l].neuron[j].net = 0;

				for (int j = 0; j < layer[l].neuron_size; j++)
				{
					for (int i = 0; i < layer[l - 1].neuron_size; i++)
					{
						layer[l].neuron[j].net += layer[l - 1].neuron[i].output * this->weights[l][j * (layer[l - 1].neuron_size) + i];
					}// matrix-vector multiplication

					layer[l].neuron[j].net += bias[l][j];							// BIAS	
					layer[l].neuron[j].output = (float)tanh(layer[l].neuron[j].net); // ACTIVATION

					if (l == LAYERCOUNT)// IF OUTPUT LAYER
					{
						if (j == tag[sample])
							desired = +1;
						else
							desired = -1;
						// --------OUTPUT LAYER FEED BACK-------
						float f_deriv = 1 - pow(layer[l].neuron[j].output, 2);
						deltaSignal[l][j] = (desired - layer[l].neuron[j].output) * f_deriv;
						for (int i = 0; i < layer[l - 1].neuron_size; i++)
						{
							this->weights[l][j * (layer[l - 1].neuron_size) + i] += LEARNING_RATE * deltaSignal[l][j] * layer[l - 1].neuron[i].output;
						}
						this->bias[l][j] += LEARNING_RATE * deltaSignal[l][j];

						//--------TOTAL ERROR-----------
						total_error += pow((desired - layer[l].neuron[j].output), 2);

					}// FEED FORWARD ---- OUTPUT LAYER FEEDBACK -----  TOTAL ERROR---- 
				}//Neuron Loop
			}//Hidden layer Loop

			//------------ HIDDEN LAYER FEED BACK-------
			for (int l = this->LAYERCOUNT - 1; l > 0; l--)
			{
				for (int j = 0; j < layer[l].neuron_size; j++) // HIDDEN LAYER WEIGHT UPDATING 
				{
					float f_deriv = 1 - pow(layer[l].neuron[j].output, 2);
					float sum = 0;
					for (int k = 0; k < layer[l + 1].neuron_size; k++) // ALREADY WEIGHT UPDATED LAYER (SIGNAL)
					{
						sum += deltaSignal[l + 1][k] * this->weights[l + 1][k * layer[l].neuron_size + j];
					}

					deltaSignal[l][j] = f_deriv * sum;
					for (int i = 0; i < layer[l - 1].neuron_size; i++)// WEIGHT UPTADE -- (INPUT SIGNAL)
					{
						this->weights[l][j * layer[l - 1].neuron_size + i] += LEARNING_RATE * deltaSignal[l][j] * layer[l - 1].neuron[i].output;
					}
					this->bias[l][j] += LEARNING_RATE * deltaSignal[l][j];
				}// NEURON LOOP
			}//LAYER LOOP (HIDDEN) ---- FEED BACK

			//INPUT LAYER FEED BACK
			for (int j = 0; j < layer[0].neuron_size; j++) // INPUT LAYER WEIGHT UPDATING
			{
				float f_deriv = 1 - pow(layer[0].neuron[j].output, 2);
				float sum = 0;
				for (int k = 0; k < layer[1].neuron_size; k++) // SIGNAL FROM ALREADY WEIGHT UPDATED LAYERS
				{
					sum += deltaSignal[1][k] * this->weights[1][k * layer[0].neuron_size + j];
				}
				deltaSignal[0][j] = f_deriv * sum;
				for (int i = 0; i < this->INPUTDIM; i++)// WEIGHT UPTADE -- INPUT SIGNAL
				{
					this->weights[0][j * this->INPUTDIM + i] += LEARNING_RATE * deltaSignal[0][j] * data[(sample * this->INPUTDIM) + i];
				}
				bias[0][j] += LEARNING_RATE * deltaSignal[0][j];
			}//NEURON LOOP (INPUT LAYER) ---- FEED BACK
		}//Sample Loop

		//error_mse = (total_error) / (numSample * this->NUMCLASS);
		//Root-Mean-Square Normalized Error(RMSE)	
		error_rmse = sqrt(total_error / (numSample * this->NUMCLASS));
		this->Error_arr[cycle] = (double)error_rmse;
		if (error_rmse < EMAX)
		{
			for (int l = 0; l < this->LAYERCOUNT + 1; l++)
			{
				delete[] deltaSignal[l];
			}
			delete[] deltaSignal;
			return cycle;
		}
	}
	return 0;
}
int ANN::TrainSGDwMoment(float* data, float* tag, int numSample)
{
	float desired, total_error = 0, error_rmse = 0, error_mse = 0;
	this->Error_arr = new double[CYCLE_MAX];

	float** deltaSignal = new float* [this->LAYERCOUNT + 1];
	float*** moment = new float** [this->LAYERCOUNT + 1]; //layer pointer initializing
	float*** moment_bias = new float** [this->LAYERCOUNT + 1];

	for (int l = 0; l < this->LAYERCOUNT + 1; l++)
		deltaSignal[l] = new float[layer[l].neuron_size];


	//input layers moment memory;
	int input_size = this->INPUTDIM * layer[0].neuron_size;
	moment[0] = new float* [input_size];
	for (int i = 0; i < input_size; i++)
		moment[0][i] = init_array_zero(T_SIZE);

	// moment[layer][inputneuron*outputneuron][t,t-1,t-2,t-3];
	//hidden and output layer weight storage for momentum
	for (int l = 1; l < this->LAYERCOUNT + 1; l++)
	{
		int size = layer[l - 1].neuron_size * layer[l].neuron_size; //weightsize
		moment[l] = new float* [size];

		for (int i = 0; i < size; i++)
			moment[l][i] = init_array_zero(T_SIZE);
	}
	for (int l = 0; l < this->LAYERCOUNT + 1; l++)
	{
		int size = layer[l].neuron_size; //
		moment_bias[l] = new float* [size];
		for (int i = 0; i < size; i++)
			moment_bias[l][i] = init_array_zero(T_SIZE);
	}


	for (int cycle = 0; cycle < CYCLE_MAX; cycle++)
	{
		total_error = 0;

		for (int sample = 0; sample < numSample; sample++)
		{
			//Input Layer Feed Forward 
			for (int j = 0; j < layer[0].neuron_size; j++)
				layer[0].neuron[j].net = 0;

			for (int j = 0; j < layer[0].neuron_size; j++)
			{
				for (int i = 0; i < this->INPUTDIM; i++)
				{
					layer[0].neuron[j].net += data[(sample * this->INPUTDIM) + i] * this->weights[0][(j * this->INPUTDIM) + i];
				}//feedforward matrix-vector multiplication
				layer[0].neuron[j].net += bias[0][j];							// BIAS
				layer[0].neuron[j].output = (float)tanh(layer[0].neuron[j].net);// ACTIVATION
			}//Neuron Loop


			//Hidden and Output Layer Feed Forward
			for (int l = 1; l < this->LAYERCOUNT + 1; l++)
			{
				for (int j = 0; j < layer[l].neuron_size; j++)
					layer[l].neuron[j].net = 0;

				for (int j = 0; j < layer[l].neuron_size; j++)
				{
					for (int i = 0; i < layer[l - 1].neuron_size; i++)
					{
						layer[l].neuron[j].net += layer[l - 1].neuron[i].output * this->weights[l][j * (layer[l - 1].neuron_size) + i];
					}// matrix-vector multiplication

					layer[l].neuron[j].net += bias[l][j];							// BIAS	
					layer[l].neuron[j].output = (float)tanh(layer[l].neuron[j].net); // ACTIVATION

					if (l == LAYERCOUNT)// IF OUTPUT LAYER
					{
						if (j == tag[sample])
							desired = +1;
						else
							desired = -1;
						// --------OUTPUT LAYER FEED BACK-------
						float f_deriv = 1 - pow(layer[l].neuron[j].output, 2);
						deltaSignal[l][j] = (desired - layer[l].neuron[j].output) * f_deriv;
						for (int i = 0; i < layer[l - 1].neuron_size; i++)
						{
							int w_index = j * (layer[l - 1].neuron_size) + i;
							float delta_w = LEARNING_RATE * deltaSignal[l][j] * layer[l - 1].neuron[i].output;
							float MOMENT = 0;
							for (int t = 0; t < T_SIZE - 1; t++)
								MOMENT += MOMENT_RATE * moment[l][w_index][t];

							this->weights[l][j * (layer[l - 1].neuron_size) + i] += delta_w + MOMENT;
							push_back(moment, l, w_index, T_SIZE, delta_w);

						}
						float delta_b = LEARNING_RATE * deltaSignal[l][j];
						float MOMENT = 0;
						for (int t = 0; t < T_SIZE - 1; t++)
							MOMENT += MOMENT_RATE * moment_bias[l][j][t];
						this->bias[l][j] += delta_b + MOMENT;
						push_back(moment_bias, l, j, T_SIZE, delta_b);


						//--------TOTAL ERROR-----------
						total_error += pow((desired - layer[l].neuron[j].output), 2);

					}// FEED FORWARD ---- OUTPUT LAYER FEEDBACK -----  TOTAL ERROR---- 
				}//Neuron Loop
			}//Hidden layer Loop

			//------------ HIDDEN LAYER FEED BACK-------
			for (int l = this->LAYERCOUNT - 1; l > 0; l--)
			{
				for (int j = 0; j < layer[l].neuron_size; j++) // CURRENT HIDDEN LAYER WEIGHT UPDATING 
				{
					float f_deriv = 1 - pow(layer[l].neuron[j].output, 2);
					float sum = 0;
					for (int k = 0; k < layer[l + 1].neuron_size; k++) // ALREADY WEIGHT UPDATED LAYER (OUTPUT - SIGNAL FROM BACK)
					{
						sum += deltaSignal[l + 1][k] * this->weights[l + 1][k * layer[l].neuron_size + j];
					}

					deltaSignal[l][j] = f_deriv * sum;

					for (int i = 0; i < layer[l - 1].neuron_size; i++)// WEIGHT UPTADE -- (INPUT - SIGNAL FROM FRONT)
					{
						float delta_w = LEARNING_RATE * deltaSignal[l][j] * layer[l - 1].neuron[i].output;
						int w_index = j * layer[l - 1].neuron_size + i;
						float MOMENT = 0;
						for (int t = 0; t < T_SIZE - 1; t++)
							MOMENT += MOMENT_RATE * moment[l][w_index][t];
						this->weights[l][j * layer[l - 1].neuron_size + i] += delta_w + MOMENT;
						push_back(moment, l, w_index, T_SIZE, delta_w);

					}
					float delta_b = LEARNING_RATE * deltaSignal[l][j];
					float MOMENT = 0;
					for (int t = 0; t < T_SIZE - 1; t++)
						MOMENT += MOMENT_RATE * moment_bias[l][j][t];
					this->bias[l][j] += delta_b + MOMENT;
					push_back(moment_bias, l, j, T_SIZE, delta_b);
				}// NEURON LOOP
			}//LAYER LOOP (HIDDEN) ---- FEED BACK

			//INPUT LAYER FEED BACK
			for (int j = 0; j < layer[0].neuron_size; j++) // INPUT LAYER WEIGHT UPDATING
			{
				float f_deriv = 1 - pow(layer[0].neuron[j].output, 2);
				float sum = 0;
				for (int k = 0; k < layer[1].neuron_size; k++) // SIGNAL FROM ALREADY WEIGHT UPDATED LAYERS
				{
					sum += deltaSignal[1][k] * this->weights[1][k * layer[0].neuron_size + j];
				}
				deltaSignal[0][j] = f_deriv * sum;
				for (int i = 0; i < this->INPUTDIM; i++)// WEIGHT UPTADE -- INPUT SIGNAL
				{
					float delta_w = LEARNING_RATE * deltaSignal[0][j] * data[(sample * this->INPUTDIM) + i];
					int w_index = j * this->INPUTDIM + i;
					float MOMENT = 0;
					for (int t = 0; t < T_SIZE; t++)
						MOMENT += MOMENT_RATE * moment[0][w_index][t];
					this->weights[0][j * this->INPUTDIM + i] += delta_w + MOMENT;
					push_back(moment, 0, w_index, T_SIZE, delta_w);

				}
				float delta_b = LEARNING_RATE * deltaSignal[0][j];
				float MOMENT = 0;
				for (int t = 0; t < T_SIZE - 1; t++)
					MOMENT += MOMENT_RATE * moment_bias[0][j][t];
				this->bias[0][j] += delta_b + MOMENT;
				push_back(moment_bias, 0, j, T_SIZE, delta_b);
			}//NEURON LOOP (INPUT LAYER) ---- FEED BACK
		}//Sample Loop


		//Root-Mean-Square Normalized Error(RMSE)
		//error_mse = (total_error) / (numSample * this->NUMCLASS);
		error_rmse = sqrt(total_error / (numSample * this->NUMCLASS));
		this->Error_arr[cycle] = (double)error_rmse;
		if (error_rmse < EMAX)
		{
			for (int l = 1; l < this->LAYERCOUNT + 1; l++)
			{
				int size = layer[l - 1].neuron_size * layer[l].neuron_size; //weightsize
				for (int i = 0; i < size; i++)
					delete[] moment[l][i];
			}
			for (int i = 0; i < input_size; i++)
				delete[] moment[0][i];

			for (int l = 0; l < this->LAYERCOUNT + 1; l++) {
				int size = layer[l].neuron_size;
				for (int i = 0; i < size; i++)
					delete[] moment_bias[l][i];
				delete[] deltaSignal[l];
				delete[] moment[l];
				delete[] moment_bias[l];

			}
			delete[] moment_bias;
			delete[] moment;
			delete[] deltaSignal;
			return cycle;
		}
	}
	return 0;
}
void ANN::Test(float* testData, int* tag, int DATASIZE)
{
	int index = 0;
	for (int sample = 0; sample < DATASIZE; sample++)
	{
		//Input Layer Feed Forward 
		for (int j = 0; j < layer[0].neuron_size; j++)
			layer[0].neuron[j].net = 0;

		for (int j = 0; j < layer[0].neuron_size; j++)
		{
			for (int i = 0; i < this->INPUTDIM; i++)
			{
				layer[0].neuron[j].net += testData[(sample * this->INPUTDIM) + i] * this->weights[0][(j * this->INPUTDIM) + i];
			}//feedforward matrix-vector multiplication
			layer[0].neuron[j].net += bias[0][j];							// BIAS
			layer[0].neuron[j].output = (float)tanh(layer[0].neuron[j].net);// ACTIVATION
		}//Neuron Loop

		//Hidden and Output Layer Feed Forward
		for (int l = 1; l < this->LAYERCOUNT + 1; l++)
		{
			for (int j = 0; j < layer[l].neuron_size; j++)
				layer[l].neuron[j].net = 0;

			for (int j = 0; j < layer[l].neuron_size; j++)
			{
				for (int i = 0; i < layer[l - 1].neuron_size; i++)
				{
					layer[l].neuron[j].net += layer[l - 1].neuron[i].output * this->weights[l][j * (layer[l - 1].neuron_size) + i];
				}// matrix-vector multiplication

				layer[l].neuron[j].net += bias[l][j];							// BIAS	
				layer[l].neuron[j].output = (float)tanh(layer[l].neuron[j].net); // ACTIVATION
			}//Neuron Loop
		}//Hidden layer Loop

		float temp_max = -FLT_MAX;
		int l = this->LAYERCOUNT; // OUTPUT LAYER LAYER
		for (int j = 0; j < this->NUMCLASS; j++)
		{
			if (layer[l].neuron[j].output > temp_max)
			{
				temp_max = layer[l].neuron[j].output;
				index = j;
			}
		}
		tag[sample] = index;
	}//Sample
}
void ANN::SaveWeights()
{
	char** c = new char* [1];
	// Veri Kümesi yazýlacak
	c[0] = "../Data/weights.txt";
	std::ofstream file(c[0]);
	if (!file.bad()) {
		// #Layer Dimension numClass weights biases
		file << this->LAYERCOUNT << " " << this->INPUTDIM << " " << this->NUMCLASS;
		for (int i = 0; i < this->LAYERCOUNT; i++)
			file << " " << layer[i].neuron_size;
		file << std::endl;
		int size = this->INPUTDIM * this->layer[0].neuron_size;
		for (int k = 0; k < size; k++)
			file << weights[0][k] << " ";
		file << std::endl;
		for (int k = 0; k < this->layer[0].neuron_size; k++)
			file << bias[0][k] << " ";
		file << std::endl;
		for (int l = 0; l < this->LAYERCOUNT; l++) {
			int size = this->layer[l].neuron_size * this->layer[l + 1].neuron_size;
			for (int k = 0; k < size; k++)
				file << weights[l + 1][k] << " ";
			file << std::endl;
			for (int k = 0; k < this->layer[l + 1].neuron_size; k++)
				file << bias[l + 1][k] << " ";
			file << std::endl;
		}
		file.close();
	}
	else System::Windows::Forms::MessageBox::Show("Sample icin dosya acilamadi");
	delete[]c;
}
void ANN::InitFromFile()
{
	char** c = new char* [1];
	//Get weights
	std::ifstream file;
	int LayerNum, Dim, numclass;
	int* neuronCount;
	c[0] = "../Data/weights.txt";
	file.open(c[0]);
	if (file.is_open()) {
		file >> LayerNum >> Dim >> numclass;
		neuronCount = new int[LayerNum];
		for (int i = 0; i < LayerNum; i++)
			file >> neuronCount[i];
		this->Init(LayerNum, neuronCount, Dim, numclass);
		int size = this->INPUTDIM * this->layer[0].neuron_size;
		for (int k = 0; k < size; k++)
			file >> weights[0][k];
		for (int k = 0; k < this->layer[0].neuron_size; k++)
			file >> bias[0][k];
		for (int l = 0; l < this->LAYERCOUNT; l++) {
			int size = this->layer[l].neuron_size * this->layer[l + 1].neuron_size;
			for (int k = 0; k < size; k++)
				file >> weights[l + 1][k];
			for (int k = 0; k < this->layer[l + 1].neuron_size; k++)
				file >> bias[l + 1][k];
		}
		file.close();
		System::String^ StringArray;
		for (int i = 0; i < LayerNum; i++)
			StringArray += System::Convert::ToString(neuronCount[i]) + " ";
		System::Windows::Forms::MessageBox::Show("Dosya basari ile okundu" + "\r\n"
			+ "Dimension:  " + System::Convert::ToString(Dim) + "\r\n"
			+ "Hidden Layer:  " + System::Convert::ToString(LayerNum) + "\r\n"
			+ "Neurons:  " + StringArray + "\r\n"
			+ "numClass:  " + System::Convert::ToString(numclass) + "\r\n"
		);


	}//file.is_open
	else System::Windows::Forms::MessageBox::Show("Weight icin dosya acilamadi");

}
ANN::ANN()
{
}
ANN::~ANN()
{
	for (int i = 0; i < LAYERCOUNT + 1; i++)
	{
		delete[] weights[i];
		delete[] bias[i];
	}
	delete[] weights;
	delete[] bias;
	delete[] layer;
	delete[] Error_arr;
}