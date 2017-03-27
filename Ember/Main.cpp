#include "UCI.h"

#include <conio.h>

int main()
{
	srand(time(0));

	TTinit();
	attacksinit();
	magicinit();
	datainit();

	Engine e;
	printf("Loading weights\n");
	e.mNet->load("Data/variables_v2.bin");
	printf("Starting game\n");

	//e.CurrentPos.loadFromFEN("2k2b1r/pp1r1ppp/2n1pq2/2p5/2PPQB2/2P2N2/P4PPP/1R2K2R b K - 2 0");
	//e.CurrentPos.display(0);
	//PositionNN pos(e.CurrentPos);
	//e.mNet->mBoard->forward(pos.mTensor);
	//printf("%f %d\n", e.mNet->OutputEvalFC->Data(0), e.LeafEval());
	//Move m = e.go();
	//printf("%s\n", m.toString().c_str());
	
	/*e.learn_eval_NN(100);
	
	printf("Saving weights\n");
	e.mNet->save("Data/variables.bin");
	printf("Done\n");*/

	UCI uci;
	uci.run_uci();


	/*Engine e;
	e.CurrentPos.display(0);

	std::vector<Move> vec;
	e.CurrentPos.generateMoves(vec);
	printBitset(e.CurrentPos.Pieces[COLOR_WHITE][PIECE_PAWN]);
	for (int i = 0; i < vec.size(); i++)
	{
		printf("%s\n", vec[i].toString());
	}

	for (int i = 1; i < 7; i++)
	{
		printf("%d\n", e.perft(i));
	}*/
	//_getch();

	return 0;
}