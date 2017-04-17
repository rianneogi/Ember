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

	Input_Pos = mBoard->newBlob(make_shape(BatchSize, 8, 8, 14));
	Input_Move = mBoard->newBlob(make_shape(BatchSize, MOVE_TENSOR_SIZE));

	mBoard->addPlaceholder(&Input_Pos->Data);
	mBoard->addPlaceholder(&Input_Move->Data);

	ConvAct = mBoard->newBlob(make_shape(BatchSize, 4*8*8 + 30*8*4 + 8*8*4 + 8*8*4 + 16));

	ConvKing = mBoard->newBlob(make_shape(BatchSize*8*8, 14*9));
	FCKing = mBoard->newBlob(make_shape(BatchSize*8*8, 4));
	ActKing = ConvAct->cut2(0, 4 * 8 * 8); //resize in activation neuron

	ConvDiag = mBoard->newBlob(make_shape(BatchSize * 30, 8 * 14));
	FCDiag = mBoard->newBlob(make_shape(BatchSize * 30, 8 * 4));
	ActDiag = ConvAct->cut2(4*8*8, 30*8*4);

	ConvFile = mBoard->newBlob(make_shape(BatchSize * 8, 8 * 14));
	FCFile = mBoard->newBlob(make_shape(BatchSize * 8, 8 * 4));
	ActFile = ConvAct->cut2(4 * 8 * 8 + 30*8*4, 8 * 8 * 4);

	FCRank = mBoard->newBlob(make_shape(BatchSize * 8, 8 * 4));
	ActRank = ConvAct->cut2(4 * 8 * 8 + 30 * 8 * 4 + 8 * 8 * 4, 8 * 8 * 4);

	MoveFC = mBoard->newBlob(make_shape(BatchSize, 16));
	MoveFCAct = ConvAct->cut2(4 * 8 * 8 + 30 * 8 * 4 + 8 * 8 * 4 + 8 * 8 * 4, 16);

	FullFC = mBoard->newBlob(make_shape(BatchSize, 32));
	FullFCAct = mBoard->newBlob(make_shape(BatchSize, 32));

	OutputMoveFC = mBoard->newBlob(make_shape(BatchSize, 1));
	Output_Move = mBoard->newBlob(make_shape(BatchSize, 1));

	OutputEvalFC = mBoard->newBlob(make_shape(BatchSize, 1));
	Output_Eval = mBoard->newBlob(make_shape(BatchSize, 1));

	mBoard->setOptimizer(new AdamOptimizer(0.001));

	mBoard->addNeuron(new KingNeuron(Input_Pos, ConvKing, 3, 3, OOB));
	mBoard->addNeuron(new ConvNeuron(ConvKing, FCKing, 1));
	mBoard->addNeuron(new LeakyReLUNeuron(FCKing, ActKing, 0.05));

	mBoard->addNeuron(new DiagNeuron(Input_Pos, ConvDiag, OOB));
	mBoard->addNeuron(new ConvNeuron(ConvDiag, FCDiag, 1));
	mBoard->addNeuron(new LeakyReLUNeuron(FCDiag, ActDiag, 0.05));

	mBoard->addNeuron(new FileNeuron(Input_Pos, ConvFile));
	mBoard->addNeuron(new ConvNeuron(ConvFile, FCFile, 1));
	mBoard->addNeuron(new LeakyReLUNeuron(FCFile, ActFile, 0.05));

	mBoard->addNeuron(new ReshapeNeuron(Input_Pos, Input_Pos, make_shape(BatchSize * 8, 8 * 14)));
	Input_Pos->reshape(make_shape(BatchSize * 8, 8 * 14));   //reshape before initializing Rank FC
	mBoard->addNeuron(new ConvNeuron(Input_Pos, FCRank, 1));
	mBoard->addNeuron(new LeakyReLUNeuron(FCRank, ActRank, 0.05));
	mBoard->addNeuron(new ReshapeNeuron(Input_Pos, Input_Pos, make_shape(BatchSize, 8, 8, 14)));
	Input_Pos->reshape(make_shape(BatchSize, 8, 8, 14));

	mBoard->addNeuron(new FullyConnectedNeuron(Input_Move, MoveFC, 1));
	mBoard->addNeuron(new LeakyReLUNeuron(MoveFC, MoveFCAct, 0.05));

	mBoard->addNeuron(new FullyConnectedNeuron(ConvAct, FullFC, 1));
	mBoard->addNeuron(new LeakyReLUNeuron(FullFC, FullFCAct, 0.05));

	//mBoard->addNeuron(new FullyConnectedNeuron(FullFCAct, Output_Eval, 1));
	//mBoard->addNeuron(new LeakyReLUNeuron(OutputEvalFC, Output_Eval, 1));

	mBoard->addNeuron(new FullyConnectedNeuron(FullFCAct, OutputMoveFC, 1));
	mBoard->addNeuron(new SigmoidNeuron(OutputMoveFC, Output_Move));

	//mBoard->addErrorFunction(new L1Error(Input_Pos, Output_Eval));
	mBoard->addErrorFunction(new L1Error(Input_Move, Output_Move));

	mBoard->addPlaceholder(mBoard->mErrorFuncs[0]->mTarget);
	//mBoard->addPlaceholder(mBoard->mErrorFuncs[1]->mTarget);



	//mBoard = new Board();
	//Input_Pos = mBoard->newBlob(make_shape(BatchSize* 8*8,  14));
	//ConvKing = mBoard->newBlob(make_shape(BatchSize * 6 * 6, 14 * 9));
	//FCKing = mBoard->newBlob(make_shape(BatchSize * 8*8, 1));
	//ActKing = mBoard->newBlob(make_shape(BatchSize, 8*8)); //resize in activation neuron

	//FullFC = mBoard->newBlob(make_shape(BatchSize, 10));
	//FullFCAct = mBoard->newBlob(make_shape(BatchSize, 10));
	//OutputMoveFC = mBoard->newBlob(make_shape(BatchSize, 2, 64));
	//Output_Move = mBoard->newBlob(make_shape(BatchSize, 2, 64));
	//OutputEvalFC = mBoard->newBlob(make_shape(BatchSize, 1));
	//Output_Eval = mBoard->newBlob(make_shape(BatchSize, 1));

	//mBoard->setOptimizer(new AdamOptimizer(0.001));

	////mBoard->addNeuron(new Im2ColNeuron(Input_Pos, ConvKing, 3, 3));
	//Conv = new ConvNeuron(Input_Pos, FCKing, 1);
	////assert(n1->Weights->Data.mSize == 14);
	///*Conv->Weights->Data.setzero();
	//Conv->Weights->Data(SQUARE_BLACKBISHOP) = -3;
	//Conv->Weights->Data(SQUARE_BLACKKNIGHT) = -3;
	//Conv->Weights->Data(SQUARE_BLACKPAWN) = -1;
	//Conv->Weights->Data(SQUARE_BLACKQUEEN) = -9;
	//Conv->Weights->Data(SQUARE_BLACKROOK) = -5;
	//Conv->Weights->Data(SQUARE_WHITEBISHOP) = 3;
	//Conv->Weights->Data(SQUARE_WHITEKNIGHT) = 3;
	//Conv->Weights->Data(SQUARE_WHITEROOK) = 5;
	//Conv->Weights->Data(SQUARE_WHITEQUEEN) = 9;
	//Conv->Weights->Data(SQUARE_WHITEPAWN) = 1;
	//Conv->Biases->Data(0) = 0;*/
	//mBoard->addNeuron(Conv);
	//mBoard->addNeuron(new LeakyReLUNeuron(FCKing, ActKing, 1));
	////mBoard->addNeuron(new FullyConnectedNeuron(ActKing, FullFC1, 1));
	////mBoard->addNeuron(new LeakyReLUNeuron(FullFC1, FullFCAct1, 0.05));
	////mBoard->addNeuron(new FullyConnectedNeuron(FullFCAct1, OutputMoveFC, 1));
	////mBoard->addNeuron(new TanhNeuron(OutputMoveFC, Output_Move));
	//FC1 = new FullyConnectedNeuron(ActKing, Output_Eval, 1);
	//FC1->Weights->Data.setconstant(1);
	//FC1->Biases->Data(0) = 0;
	//mBoard->addNeuron(FC1);
	////mBoard->addNeuron(new LeakyReLUNeuron(OutputEvalFC, Output_Eval, 1));

	////mBoard->addErrorFunction(new MeanSquaredError(Input_Pos, Output_Eval));
	//mBoard->addErrorFunction(new L1Error(Input_Pos, Output_Eval));
}

Float EvalNet::train(Tensor* input_pos, Tensor* input_move, Tensor* output_eval, Tensor* output_move)
{
	std::vector<Tensor*> v;
	v.push_back(input_pos);
	v.push_back(input_move);
	//v.push_back(output_eval);
	v.push_back(output_move);
	Float error = mBoard->backprop(v);
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
