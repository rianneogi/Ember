#include "UCI.h"

#include <conio.h>

int main()
{
	TTinit();
	datainit();

	Engine e;
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
	}
	_getch();

	return 0;
}