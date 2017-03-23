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

Net::Net()
{
	init_net();
}

Net::~Net()
{

}

void Net::init_net()
{
	mBoard = new Board();
	Input = mBoard->newBlob(make_shape(14, 8, 8));
	ConvKing = mBoard->newBlob(make_shape(100, 8, 8));
	FCKing = mBoard->newBlob(make_shape(100, 8, 8));
	ActKing = mBoard->newBlob(make_shape(100, 8, 8));

	FullFC1 = mBoard->newBlob(make_shape(100));
	FullFCAct1 = mBoard->newBlob(make_shape(100));
	FullFC2 = mBoard->newBlob(make_shape(2, 8, 8));
	Output = mBoard->newBlob(make_shape(2, 8, 8));

	mBoard->setOptimizer(new AdamOptimizer(0.05));

	mBoard->addNeuron(new Im2ColNeuron(Input, ConvKing, 3, 3));
	mBoard->addNeuron(new ConvNeuron(ConvKing, FCKing, 1));
	mBoard->addNeuron(new LeakyReLUNeuron(FCKing, ActKing, 0.05));
	mBoard->addNeuron(new FullyConnectedNeuron(ActKing, FullFC1, 1));
	mBoard->addNeuron(new LeakyReLUNeuron(FullFC1, FullFCAct1, 0.05));
	mBoard->addNeuron(new FullyConnectedNeuron(FullFCAct1, FullFC2, 1));
	mBoard->addNeuron(new TanhNeuron(FullFC2, Output));

	mBoard->setErrorFunction(new MeanSquaredError(Input, Output, nullptr));
}

void train(Tensor inputs, Tensor outputs)
{
}

Tensor moveToTensor(Move m)
{
	return Tensor();
}
