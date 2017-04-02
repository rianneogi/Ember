#include "Net.h"
#include <Neurons\FullyConnectedNeuron.h>
#include <Neurons\ConvNeuron.h>
#include <Neurons\KingNeuron.h>
#include <Neurons\FileNeuron.h>
#include <Neurons\Im2ColNeuron.h>
#include <Neurons\TanhNeuron.h>
#include <Neurons\LeakyReLUNeuron.h>
#include <ErrorFunctions\MeanSquaredError.h>
#include <ErrorFunctions\L1Error.h>
#include <ErrorFunctions\UnitError.h>
#include <Optimizers\StandardOptimizer.h>
#include <Optimizers\AdamOptimizer.h>

Net::Net() : BatchSize(20)
{
	init_net();
}

Net::Net(uint64_t batch_size) : BatchSize(batch_size)
{
	init_net();
}

Net::~Net()
{
	delete mBoard;
	mBoard = NULL;
}

void Net::init_net()
{
	mBoard = new Board();
	Input = mBoard->newBlob(make_shape(BatchSize, 8, 8, 14));
	ConvKing = mBoard->newBlob(make_shape(BatchSize*6*6, 14*9));
	FCKing = mBoard->newBlob(make_shape(BatchSize*6*6, 12));
	ActKing = mBoard->newBlob(make_shape(BatchSize, 12*6*6)); //resize in activation neuron

	FullFC1 = mBoard->newBlob(make_shape(BatchSize, 10));
	FullFCAct1 = mBoard->newBlob(make_shape(BatchSize, 10));
	OutputMoveFC = mBoard->newBlob(make_shape(BatchSize, 2,64));
	Output_Move = mBoard->newBlob(make_shape(BatchSize, 2,64));
	OutputEvalFC = mBoard->newBlob(make_shape(BatchSize, 1));
	Output_Eval = mBoard->newBlob(make_shape(BatchSize, 1));

	mBoard->setOptimizer(new AdamOptimizer(0.00001));

	mBoard->addNeuron(new Im2ColNeuron(Input, ConvKing, 3, 3));
	mBoard->addNeuron(new ConvNeuron(ConvKing, FCKing, 1));
	mBoard->addNeuron(new LeakyReLUNeuron(FCKing, ActKing, 0.05));
	//mBoard->addNeuron(new FullyConnectedNeuron(ActKing, FullFC1, 1));
	//mBoard->addNeuron(new LeakyReLUNeuron(FullFC1, FullFCAct1, 0.05));
	//mBoard->addNeuron(new FullyConnectedNeuron(FullFCAct1, OutputMoveFC, 1));
	//mBoard->addNeuron(new TanhNeuron(OutputMoveFC, Output_Move));
	mBoard->addNeuron(new FullyConnectedNeuron(ActKing, OutputEvalFC, 1));
	mBoard->addNeuron(new LeakyReLUNeuron(OutputEvalFC, Output_Eval, 1));

	//mBoard->addErrorFunction(new MeanSquaredError(Input, Output_Move, nullptr));
	mBoard->addErrorFunction(new L1Error(Input, Output_Eval));
}

Float Net::train(Tensor inputs, Tensor* output_move, Tensor* output_eval)
{
	std::vector<Tensor*> v;
	//v.push_back(output_move);
	v.push_back(output_eval);
	Float error = mBoard->backprop(inputs, v);
	mBoard->mOptimizer->optimize();
	return error;
}

Float Net::get_eval(Tensor input)
{
	mBoard->forward(input);
	return Output_Eval->Data(0);
}

void Net::save(std::string filename)
{
	mBoard->save_variables(filename);
}

void Net::load(std::string filename)
{
	mBoard->load_variables(filename);
}

void moveToTensor(Move m, Tensor* t)
{
	//Tensor t(make_shape(2, 64));
	t->setzero();
	t->operator()(0, m.getFrom()) = 1;
	t->operator()(1, m.getTo()) = 1;
}

Move tensorToMove(Tensor* tensor)
{
	int t, f;
	for (int i = 0; i < 64; i++)
	{
		if (tensor->operator()(0, i) == 1)
		{
			f = i;
		}
		if (tensor->operator()(1, i) == 1)
		{
			t = i;
		}
	}
	Move m(f, t, 0, 0, 0, 0, 0, 0, 0, 0);
	return m;
}
