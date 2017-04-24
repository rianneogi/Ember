#include "EvalNet.h"

EvalNet::EvalNet() : BatchSize(20)
{
	init_net();
}

EvalNet::EvalNet(uint64_t batch_size) : BatchSize(batch_size)
{
	init_net();
}

EvalNet::~EvalNet()
{
	delete mBoard;
	mBoard = NULL;
}

void EvalNet::init_net()
{
	mBoard = new Board();

	Tensor OOB(make_shape(14));
	OOB.setzero();
	OOB(SQUARE_INVALID) = 1;

	Input_Pos = mBoard->newBlob(make_shape(BatchSize, POSITION_TENSOR_SIZE));

	mBoard->addPlaceholder(&Input_Pos->Data);

	OutputEvalFC = mBoard->newBlob(make_shape(BatchSize, 128));
	Output_Eval = mBoard->newBlob(make_shape(BatchSize, 1));

	mBoard->setOptimizer(new AdamOptimizer(0.001));

	mBoard->addNeuron(new FullyConnectedNeuron(Input_Pos, OutputEvalFC, 1));
	mBoard->addNeuron(new FullyConnectedNeuron(OutputEvalFC, Output_Eval, 1));

	mBoard->addErrorFunction(new L1Error(Output_Eval));
	mBoard->addPlaceholder(&mBoard->mErrorFuncs[0]->mTarget);
}

Float EvalNet::train(Tensor input_pos, Tensor input_move, Tensor output_eval, Tensor output_move)
{
	std::vector<Tensor> v;
	v.push_back(input_pos);
	//v.push_back(input_move);
	v.push_back(output_eval);
	//v.push_back(output_move);
	Float error = mBoard->backprop(v);
	/*for (int i = 0; i < BatchSize; i++)
	{
		printf("s %f %f %f ", Output_Move->Data(i), Output_Move->Delta(i), output_move->operator()(i));
	}
	printf("\n");*/
	mBoard->mOptimizer->optimize();
	return error;
}

Float EvalNet::get_eval(Tensor input)
{
	mBoard->forward(input);
	return Output_Eval->Data(0);
}

void EvalNet::save(std::string filename)
{
	mBoard->save_variables(filename);
}

void EvalNet::load(std::string filename)
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
