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
	e.mNet->load("Data/variables.bin");
	printf("Starting game\n");

	/*e.CurrentPos.loadFromFEN("8/6k1/p2r1p1p/1p1p2P1/3Rb3/P1P1N1P1/1P2K3/8 w - - 0 0");
	e.CurrentPos.display(0);
	Move m = e.go();
	printf("%s\n", m.toString().c_str());*/
	e.learn_eval(256);
	
	printf("Saving weights\n");
	e.mNet->save("Data/variables.bin");
	printf("Done\n");

	UCI uci;
	uci.run();


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