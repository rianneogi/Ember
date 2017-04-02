#include "SortNet.h"

SortNet::SortNet()
{
	init_net();
}

SortNet::~SortNet()
{
	delete mBoard;
	mBoard = NULL;
}

void SortNet::init_net()
{
	mBoard = new Board();
	Input = mBoard->newBlob(make_shape(1, 64*64));
	Output = mBoard->newBlob(make_shape(1, 1));

	mBoard->setOptimizer(new AdamOptimizer(0.001));

	mBoard->addNeuron(new FullyConnectedNeuron(Input, Output, 1));

	mBoard->addErrorFunction(new MeanSquaredError(Input, Output));
}

Float SortNet::train(Tensor input, Tensor output)
{
	Float error = mBoard->backprop(input, output);
	mBoard->mOptimizer->optimize();
	return error;
}
