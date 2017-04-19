#include "UCI.h"

#include <conio.h>

int main()
{
	srand(time(0));

	TTinit();
	attacksinit();
	magicinit();
	datainit();

	std::string path_weights = "Data/variables_superconv.bin";
	std::string path_pgn = "Data/KingBase/KingBase2016-03-A80-A99.pgn";

#ifdef TRAINING_BUILD
	Engine e;
	printf("Loading weights\n");
	//e.load_nets(path_weights);
	//_set_error_mode(_OUT_TO_MSGBOX);
	printf("Loading pgn\n");
	PGNData pgn(path_pgn, 10000);
	
	printf("Starting training\n");

	e.learn_eval_pgn(pgn, 60);
	
	printf("Saving weights\n");
	e.NetTrain->save(path_weights);
	printf("Done\n");

	e.NetPlay->mBoard->copy_variables(e.NetTrain->mBoard);

	e.CurrentPos.loadFromFEN("2k2b1r/pp1r1ppp/2n1pq2/2p5/4QB2/2P2N2/P4PPP/1R2K2R b K - 2 0");
	e.CurrentPos.display(0);
	printf("%f %d\n", e.LeafEval_NN(), e.LeafEval_MatOnly());

	e.CurrentPos.loadFromFEN("rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 1 1");
	e.CurrentPos.display(0);
	printf("%f %d\n", e.LeafEval_NN(), e.LeafEval_MatOnly());
	/*PositionNN pnn(e.CurrentPos);
	Position p;
	pnn.copyToPosition(p);
	printf("%s %s\n", p.toFEN().c_str(), pnn.toFEN().c_str());*/

	e.CurrentPos.loadFromFEN("rnb1kbr1/p2ppppp/1qp5/1p6/2PPNP2/8/PP3KPP/R1BQ1BNR w q - 1 8");
	e.CurrentPos.display(0);
	printf("%f %d\n", e.LeafEval_NN(), e.LeafEval_MatOnly());

	e.CurrentPos.loadFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0");
	e.CurrentPos.display(0);
	printf("%f %d\n", e.LeafEval_NN(), e.LeafEval_MatOnly());
	_getch();

#else
	UCI uci;

	//std::vector<Move> moves;
	//uci.Ember.CurrentPos.generateMoves(moves);
	//for (int i = 0; i < moves.size(); i++)
	//{
	//	pos2posNN(&uci.Ember.PositionTensor(i*POSITION_TENSOR_SIZE), uci.Ember.CurrentPos);
	//	moveToTensorPtr(moves[i], &uci.Ember.MoveTensor(i, 0));
	//}
	//std::vector<Tensor*> v;
	////uci.Ember.PositionTensor.setzero();
	////uci.Ember.MoveTensor.setzero();
	//v.push_back(&uci.Ember.PositionTensor);
	//v.push_back(&uci.Ember.MoveTensor);
	//v.push_back(nullptr);
	//uci.Ember.NetTrain->mBoard->forward(v);
	///*for (int i = 0; i < MOVE_TENSOR_SIZE; i++)
	//{
	//	printf("%f ", uci.Ember.NetTrain->Input_Move->Data(i));
	//}
	//printf("\n");
	//for (int i = 0; i < POSITION_TENSOR_SIZE; i++)
	//{
	//	printf("%f ", uci.Ember.NetTrain->Input_Pos->Data(i));
	//}
	//printf("\n");*/
	//for (int i = 0; i < moves.size(); i++)
	//{
	//	printf("%s %f \n", moves[i].toString().c_str(), uci.Ember.NetTrain->Output_Move->Data(i));
	//}
	//printf("\n");

	//Move tmp = moves[0];
	//moves[0] = moves[1];
	//moves[1] = tmp;

	//for (int i = 0; i < moves.size(); i++)
	//{
	//	pos2posNN(&uci.Ember.PositionTensor(i*POSITION_TENSOR_SIZE), uci.Ember.CurrentPos);
	//	moveToTensorPtr(moves[i], &uci.Ember.MoveTensor(i*MOVE_TENSOR_SIZE));
	//}
	//std::vector<Tensor*> v2;
	////uci.Ember.PositionTensor.setzero();
	////uci.Ember.MoveTensor.setzero();
	//v2.push_back(&uci.Ember.PositionTensor);
	//v2.push_back(&uci.Ember.MoveTensor);
	//v2.push_back(nullptr);
	//uci.Ember.NetTrain->mBoard->forward(v2);
	////for (int i = 0; i < MOVE_TENSOR_SIZE; i++)
	////{
	////	assert(uci.Ember.MoveTensor(i) == uci.Ember.MoveTensor(MOVE_TENSOR_SIZE + i));
	////}
	////for (int i = 0; i < POSITION_TENSOR_SIZE; i++)
	////{
	////	assert(uci.Ember.PositionTensor(i) == uci.Ember.PositionTensor(POSITION_TENSOR_SIZE + i));
	////}
	////for (int i = 0; i < MOVE_TENSOR_SIZE; i++)
	////{
	////	assert(uci.Ember.NetTrain->Input_Move->Data(i) == uci.Ember.NetTrain->Input_Move->Data(MOVE_TENSOR_SIZE + i));
	////}
	////for (int i = 0; i < POSITION_TENSOR_SIZE; i++)
	////{
	////	assert(uci.Ember.NetTrain->Input_Pos->Data(i) == uci.Ember.NetTrain->Input_Pos->Data(POSITION_TENSOR_SIZE + i));
	////}
	////for (int i = 0; i < uci.Ember.NetTrain->MoveFCAct->Data.mSize / 144; i++)
	////{
	////	assert(uci.Ember.NetTrain->MoveFCAct->Data(i) == uci.Ember.NetTrain->MoveFCAct->Data(uci.Ember.NetTrain->MoveFCAct->Data.mSize / 144 + i));
	////	assert(uci.Ember.NetTrain->MoveFCAct->Data(i) == uci.Ember.NetTrain->ConvAct->Data(uci.Ember.NetTrain->ActKing->Data.mSize / 144 + i));
	////}
	////for (int i = 0; i < uci.Ember.NetTrain->FCKing->Data.mSize / 144; i++)
	////{
	////	assert(uci.Ember.NetTrain->FCKing->Data(i) == uci.Ember.NetTrain->FCKing->Data(uci.Ember.NetTrain->FCKing->Data.mSize / 144 + i));
	////}
	////printf("ptr %d %d\n", uci.Ember.NetTrain->ActKing->Data.mData, uci.Ember.NetTrain->ConvAct->Data.mData);
	////printf("ptr %d %d\n", &uci.Ember.NetTrain->ActKing->Data(0), &uci.Ember.NetTrain->ConvAct->Data(0));
	////printf("ld %d %d\n", uci.Ember.NetTrain->ActKing->Data.mLD, uci.Ember.NetTrain->ConvAct->Data.mLD);
	////printf("ptr %d %d\n", &uci.Ember.NetTrain->ActKing->Data(1,0), &uci.Ember.NetTrain->ConvAct->Data(1,0));
	////for (int i = 0; i < uci.Ember.NetTrain->ActKing->Data.mSize/144; i++)
	////{
	////	assert(uci.Ember.NetTrain->ActKing->Data(i) == uci.Ember.NetTrain->ActKing->Data(uci.Ember.NetTrain->ActKing->Data.mSize / 144 + i));
	////	assert(uci.Ember.NetTrain->ActKing->Data(i) == uci.Ember.NetTrain->ConvAct->Data(i));
	////	printf("actk %f %f\n", uci.Ember.NetTrain->ActKing->Data(uci.Ember.NetTrain->ActKing->Data.mSize / 144 + i), uci.Ember.NetTrain->ConvAct->Data(uci.Ember.NetTrain->ConvAct->Data.mSize / 144 + i));
	////	assert(uci.Ember.NetTrain->ActKing->Data(uci.Ember.NetTrain->ActKing->Data.mSize / 144 + i) == uci.Ember.NetTrain->ConvAct->Data(uci.Ember.NetTrain->ConvAct->Data.mSize / 144 + i));
	////}
	////for (int i = 0; i < uci.Ember.NetTrain->ConvAct->Data.mSize / 144; i++)
	////{
	////	//assert(uci.Ember.NetTrain->ConvAct->Data(i) == uci.Ember.NetTrain->ConvAct->Data(uci.Ember.NetTrain->ConvAct->Data.mSize / 144 + i));
	////}
	////for (int i = 0; i < uci.Ember.NetTrain->ConvAct->Data.mSize / 144; i++)
	////{
	////	printf("%f ", uci.Ember.NetTrain->ConvAct->Data(i));
	////}
	////printf("\n");
	////for (int i = 0; i < uci.Ember.NetTrain->ConvAct->Data.mSize / 144; i++)
	////{
	////	printf("%f ", uci.Ember.NetTrain->ConvAct->Data(uci.Ember.NetTrain->ConvAct->Data.mSize / 144 + i));
	////}
	////printf("\n");
	//for (int i = 0; i < moves.size(); i++)
	//{
	//	printf("%s %f \n", moves[i].toString().c_str(), uci.Ember.NetTrain->Output_Move->Data(i));
	//}
	//printf("\n");


	uci.Ember.load_nets(path_weights);
	uci.run_uci();
#endif

	return 0;
}