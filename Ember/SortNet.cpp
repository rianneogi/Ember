#include "SortNet.h"

SortNet::SortNet() : BatchSize(1)
{
	init_net();
}

SortNet::SortNet(uint64_t batch_size) : BatchSize(batch_size)
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

	Tensor OOB(make_shape(14));
	OOB.setzero();
	OOB(SQUARE_INVALID) = 1;

	Input_Pos = mBoard->newBlob(make_shape(BatchSize, 8, 8, 14));
	Input_Move = mBoard->newBlob(make_shape(BatchSize, MOVE_TENSOR_SIZE));

	mBoard->addPlaceholder(&Input_Pos->Data);
	mBoard->addPlaceholder(&Input_Move->Data);

	int king_output_size = 1;
	int diag_output_size = 1;
	int file_output_size = 1;
	int rank_output_size = 1;
	int move_output_size = 1;
	int fc_size = 32;

	//parameter count: (14*9+1)*king_output_size + (8*14+1) * diag_output_size + (8*14+1) * file_output_size 
	//                 + (8*14+1) * rank_output_size  + (MOVE_TENSOR_SIZE+1)*move_output_size 
	//                 + fc_size+1
	//                 + (8*8*king_output_size + 30*diag_output_size + 8*(file_output_size + rank_output_size) 
	//                           + move_output_size + 1)*fc_size

	ConvAct = mBoard->newBlob(make_shape(BatchSize, king_output_size * 8 * 8
		+ 30 * diag_output_size + 8 * file_output_size + 8 * rank_output_size
		+ move_output_size));

	ConvKing = mBoard->newBlob(make_shape(BatchSize * 8 * 8, 14 * 9));
	FCKing = mBoard->newBlob(make_shape(BatchSize * 8 * 8, king_output_size));
	ActKing = ConvAct->cut2(0, king_output_size * 8 * 8); //resize in activation neuron

	ConvDiag = mBoard->newBlob(make_shape(BatchSize * 30, 8 * 14));
	FCDiag = mBoard->newBlob(make_shape(BatchSize * 30, diag_output_size));
	ActDiag = ConvAct->cut2(king_output_size * 8 * 8, 30 * diag_output_size);

	ConvFile = mBoard->newBlob(make_shape(BatchSize * 8, 8 * 14));
	FCFile = mBoard->newBlob(make_shape(BatchSize * 8, file_output_size));
	ActFile = ConvAct->cut2(king_output_size * 8 * 8 + 30 * diag_output_size, 8 * file_output_size);

	FCRank = mBoard->newBlob(make_shape(BatchSize * 8, rank_output_size));
	ActRank = ConvAct->cut2(king_output_size * 8 * 8 + 30 * diag_output_size +
		8 * file_output_size, 8 * rank_output_size);

	MoveFC = mBoard->newBlob(make_shape(BatchSize, move_output_size));
	MoveFCAct = ConvAct->cut2(king_output_size * 8 * 8 + 30 * diag_output_size +
		8 * file_output_size + 8 * rank_output_size, move_output_size);

	FullFC = mBoard->newBlob(make_shape(BatchSize, fc_size));
	FullFCAct = mBoard->newBlob(make_shape(BatchSize, fc_size));

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
	mBoard->addNeuron(new LeakyReLUNeuron(OutputMoveFC, Output_Move, 1));

	mBoard->addErrorFunction(new L1Error(Output_Move));
	//mBoard->addErrorFunction(new MeanSquaredError(Output_Move));

	mBoard->addPlaceholder(&mBoard->mErrorFuncs[0]->mTarget);
	//mBoard->addPlaceholder(mBoard->mErrorFuncs[1]->mTarget);



	//Input_Pos = mBoard->newBlob(make_shape(BatchSize, 8, 8, 14));
	//Input_Move = mBoard->newBlob(make_shape(BatchSize, MOVE_TENSOR_SIZE));

	////ConvFull = mBoard->newBlob(make_shape(BatchSize, 12 + 8 * 8));
	//ConvAct = mBoard->newBlob(make_shape(BatchSize, 6 * 6*1 + 24));

	//ConvKing = mBoard->newBlob(make_shape(BatchSize * 6 * 6, 14 * 9));
	//FCKing = mBoard->newBlob(make_shape(BatchSize*6*6, 1));
	//ActKing = ConvAct->cut2(0, 6*6*1); //resize in activation neuron

	//MoveFC = mBoard->newBlob(make_shape(BatchSize, 24));
	//MoveFCAct = ConvAct->cut2(6 * 6 * 1, 24);
	//
	//Output_Move = mBoard->newBlob(make_shape(BatchSize, 1));

	//mBoard->setOptimizer(new AdamOptimizer(0.01));

	//mBoard->addPlaceholder(&Input_Pos->Data);
	//mBoard->addPlaceholder(&Input_Move->Data);

	//mBoard->addNeuron(new Im2ColNeuron(Input_Pos, ConvKing, 3, 3));
	//mBoard->addNeuron(new ConvNeuron(ConvKing, FCKing, 1));
	//mBoard->addNeuron(new LeakyReLUNeuron(FCKing, ActKing, 0.05));

	//mBoard->addNeuron(new FullyConnectedNeuron(Input_Move, MoveFC, 1));
	//mBoard->addNeuron(new LeakyReLUNeuron(MoveFC, MoveFCAct, 0.05));

	//mBoard->addNeuron(new FullyConnectedNeuron(ConvAct, Output_Move, 1));

	////mBoard->addErrorFunction(new MeanSquaredError(Output_Move));
	//mBoard->addErrorFunction(new L1Error(Output_Move));

	//mBoard->addPlaceholder(mBoard->mErrorFuncs[0]->mTarget);
}

Float SortNet::train(Tensor input_pos, Tensor input_move, Tensor output_eval, Tensor output_move)
{
	std::vector<Tensor> v;
	v.push_back(input_pos);
	v.push_back(input_move);
	//v.push_back(output_eval);
	v.push_back(output_move);
	Float error = mBoard->backprop(v);
	/*for (int i = 0; i < BatchSize; i++)
	{
		printf("s %f %f %f ", Output_Move->Data(i), Output_Move->Delta(i), output_move(i));
	}
	printf("\n");*/
	mBoard->mOptimizer->optimize();
	return error;
}

Float SortNet::get_sort(Tensor position, Tensor move)
{
	std::vector<Tensor> v;
	v.push_back(position);
	v.push_back(move);
	v.push_back(Tensor());
	mBoard->forward(v);
	//printf("ret %f", Output_Move->Data(0));
	return Output_Move->Data(0);
}

void SortNet::save(std::string filename)
{
	mBoard->save_variables(filename);
}

void SortNet::load(std::string filename)
{
	mBoard->load_variables(filename);
}

void moveToTensorPtr(const Move& m, Float* t)
{
	memset(t, 0, sizeof(Float) * MOVE_TENSOR_SIZE);
	t[m.getFrom()] = 1;
	t[64 + m.getTo()] = 1;
	t[128 + m.getMovingPiece()] = 1;
	t[128 + 6 + getSquare2Piece(m.getCapturedPiece())] = 1;
	t[128 + 13 + m.getSpecial()] = 1;
}