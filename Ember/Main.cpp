#include "UCI.h"

#include <conio.h>

int main()
{
	TTinit();
	attacksinit();
	magicinit();
	datainit();

	Engine e;
	e.CurrentPos.loadFromFEN("r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1");
	for (int i = 1; i <= 6; i++)
	{
		printf("%d\n",e.perft(i));
	}

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