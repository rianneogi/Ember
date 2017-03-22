#include "Net.h"
#include <Neurons\FullyConnectedNeuron.h>
#include <Neurons\ConvNeuron.h>
#include <Neurons\KingNeuron.h>
#include <Neurons\FileNeuron.h>
#include <Neurons\Im2ColNeuron.h>
#include <Neurons\TanhNeuron.h>
#include <Neurons\LeakyReLUNeuron.h>
#include <ErrorFunctions\MeanSquaredError.h>
#include <Optimizers\AdamOptimizer.h>

void net()
{
	Board b;
	Blob* input = b.newBlob(make_shape(14,8,8));

	Blob* conv_king = b.newBlob(make_shape(100,8,8));
	Blob* conv_knight = b.newBlob(make_shape(100, 8, 8));
	Blob* conv_rank = b.newBlob(make_shape(100, 8));
	Blob* conv_file = b.newBlob(make_shape(100, 8));
	Blob* conv_diag = b.newBlob(make_shape(100, 30));

	Blob* fc_king = b.newBlob(make_shape(100, 8, 8));
	Blob* fc_knight = b.newBlob(make_shape(100, 8, 8));
	Blob* fc_rank = b.newBlob(make_shape(100, 8));
	Blob* fc_file = b.newBlob(make_shape(100, 8));
	Blob* fc_diag = b.newBlob(make_shape(100, 30));

	Blob* act_king = b.newBlob(make_shape(100, 8, 8));
	Blob* act_knight = b.newBlob(make_shape(100, 8, 8));
	Blob* act_rank = b.newBlob(make_shape(100, 8));
	Blob* act_file = b.newBlob(make_shape(100, 8));
	Blob* act_diag = b.newBlob(make_shape(100, 30));

	Blob* FC1 = b.newBlob(make_shape(100));
	Blob* act_FC1 = b.newBlob(make_shape(100));
	Blob* FC2 = b.newBlob(make_shape(2,8,8));
	Blob* output = b.newBlob(make_shape(2, 8, 8));

	b.setOptimizer(new AdamOptimizer(0.05));

	b.addNeuron(new Im2ColNeuron(input, conv_king, 3, 3));
	b.addNeuron(new ConvNeuron(conv_king, fc_king, 1));
	b.addNeuron(new LeakyReLUNeuron(fc_king, act_king, 0.05));
	b.addNeuron(new FullyConnectedNeuron(act_king, FC1, 1));
	b.addNeuron(new LeakyReLUNeuron(FC1, act_FC1, 0.05));
	b.addNeuron(new FullyConnectedNeuron(act_FC1, FC2, 1));
	b.addNeuron(new TanhNeuron(FC2, output));

	b.setErrorFunction(new MeanSquaredError(input, output, nullptr));
}

void train(Tensor inputs, Tensor outputs)
{
}

Tensor moveToTensor(Move m)
{
	return Tensor();
}
