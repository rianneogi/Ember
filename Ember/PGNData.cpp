#include "PGNData.h"

PGNData::PGNData()
{
}

PGNData::PGNData(std::string file)
{
	loadFromFile(file);
}

PGNData::PGNData(std::string file, size_t num_games)
{
	loadFromFile(file, num_games);
}

PGNData::~PGNData()
{
}

void PGNData::loadFromFile(std::string file)
{
	loadFromFile(file, 0);
}

void PGNData::loadFromFile(std::string file, size_t num_games)
{
	using std::cout;
	using std::endl;

	std::fstream f(file, std::ios::in);
	if (!f.is_open())
	{
		cout << "ERROR: cant open file: " << file << endl;
	}
	std::string s;
	enum { SEARCH, PARSE, SQUARE, CURLY };
	int phase = SEARCH;
	std::string movestr = "";
	int gameno = 1;

	Position pos;
	pos.setStartPos();

	PGNGame currentGame;

	while (!f.eof())
	{
		char c = f.get();
		if (phase == SEARCH)
		{
			if (c == '[')
			{
				phase = SQUARE;
				continue;
			}
			if (c == '{')
			{
				phase = CURLY;
				continue;
			}
			if (c != '.' && c != ' ' && c != '\n')
			{
				phase = PARSE;
				movestr += c;
			}
		}
		else if (phase == PARSE)
		{
			if (c != '.' && c != ' ' && c != '\n')
			{
				movestr += c;
			}
			else
			{
				if (c == '[')
					phase = SQUARE;
				else if (c == '{')
					phase = CURLY;
				else
					phase = SEARCH;

				if ((movestr.at(0) >= '0' && movestr.at(0) <= '9') || movestr == "*") //found result
				{
					int change = 0;
					if (movestr == "*")
					{
						currentGame.Result = -1;
						change = 1;
					}
					if (movestr == "1-0")
					{
						currentGame.Result = 1;
						change = 1;
					}
					if (movestr == "0-1")
					{
						currentGame.Result = 0;
						change = 1;
					}
					if (movestr == "1/2-1/2")
					{
						currentGame.Result = 0.5;
						change = 1;
					}

					if (change == 1)
					{
						Games.push_back(currentGame);
						if (Games.size() >= num_games && num_games != 0)
							return;

						currentGame = PGNGame();

						pos.setStartPos();
						gameno++;
					}
					movestr = "";
					continue;
				}

				std::string move = convertMoveNotation(movestr, pos);
				if (move == "error")
				{
					cout << "ERROR " << gameno << " " << movestr << " " << pos.movelist.size() << " " << pos.Turn << endl;
					pos.display(0);
					cout << endl;
				}
				else
				{
					std::vector<Move> moves;
					pos.generateMoves(moves);
					Move theMove = CONST_NULLMOVE;
					for (int i = 0; i < moves.size(); i++)
					{
						if (moves.at(i).toString() == move)
						{
							theMove = moves.at(i);
						}
					}
					if (theMove.isNullMove())
					{
						cout << "the move is null error\n";
					}
					else
					{
						pos.makeMove(theMove);
						currentGame.Moves.push_back(theMove);
					}
				}
				movestr = "";
			}
		}
		else if (phase == CURLY)
		{
			if (c == '}')
			{
				phase = SEARCH;
				continue;
			}
		}
		else if (phase == SQUARE)
		{
			if (c == ']')
			{
				phase = SEARCH;
				continue;
			}
		}
	}
	f.close();
}

void PGNData::printData()
{
	printf("%d\n", Games.size());
	for (int i = 0; i < Games.size(); i++)
	{
		for (int j = 0; j < Games[i].Moves.size(); j++)
		{
			printf("%s ", Games[i].Moves[j].toString().c_str());
		}
		printf("\n");
	}
}

std::string convertMoveNotation(std::string move, Position& pos) //converts move notation (eg. Nf3 into g1f3) 
{
	std::vector<Move> moves;
	pos.generateMoves(moves);

	if (move.at(move.size() - 1) == ';')
	{
		move = move.substr(0, move.size() - 1);
	}

	if (move.at(move.size() - 1) == '+' || move.at(move.size() - 1) == '#')
	{
		move = move.substr(0, move.size() - 1);
	}

	if (move == "O-O")
	{
		if (pos.Turn == COLOR_WHITE)
		{
			return "e1g1";
		}
		else
		{
			return "e8g8";
		}
	}
	else if (move == "O-O-O")
	{
		if (pos.Turn == COLOR_WHITE)
		{
			return "e1c1";
		}
		else
		{
			return "e8c8";
		}
	}
	int prom = 0;
	char prompiece = ' ';
	if (move.at(move.size() - 2) == '=') //promotion
	{
		prom = 1;
		prompiece = move.at(move.size() - 1);
		move = move.substr(0, move.size() - 2);
	}

	for (int i = 0; i < moves.size(); i++)
	{
		Move m = moves.at(i);
		std::string s = m.toString();
		if (Int2Sq(m.getTo()) == move.substr(move.size() - 2))
		{
			if (PieceStrings[m.getMovingPiece() + 1].at(0) == move.at(0) && move.size()>2) //piece
			{
				if (move.size() >= 4 && move.at(1) != 'x')
				{
					if (move.at(1) >= '1' && move.at(1) <= '8')
					{
						int rank = move.at(1) - 49;
						if (m.getFrom() >= 8 * rank && m.getFrom() <= 8 * rank + 7)
						{
							if (pos.isMoveLegal(m))
								return s;
						}
					}
					else
					{
						int file = move.at(1) - 97;
						if (m.getFrom() % 8 == (7 - file))
						{
							if (pos.isMoveLegal(m))
								return s;
						}
					}
				}
				else
				{
					if (pos.isMoveLegal(m))
						return s;
				}
			}
			else if ((move.at(0) != 'Q' && move.at(0) != 'B' && move.at(0) != 'R' && move.at(0) != 'N' && move.at(0) != 'K' &&
				(move.size() > 2 && move.at(0) >= 97 && move.at(0) <= 97 + 8 && move.at(1) == 'x'))) //pawn capture
			{
				int file = move.at(0) - 97;
				if (m.getFrom() % 8 == (7 - file))
				{
					if (prom)
					{
						if (PieceStrings[m.getSpecial() + 1].at(0) == prompiece)
						{
							if (pos.isMoveLegal(m))
								return s;
						}
					}
					else
					{
						if (pos.isMoveLegal(m))
							return s;
					}
				}
			}
			else if (move.at(0) != 'Q' && move.at(0) != 'B' && move.at(0) != 'R' && move.at(0) != 'N' && move.at(0) != 'K' &&
				(move.size() == 2 || (move.size() == 4 && move.at(1) == 'x'))) //pawn move
			{
				if (prom)
				{
					if (PieceStrings[m.getSpecial() + 1].at(0) == prompiece)
					{
						if (pos.isMoveLegal(m))
							return s;
					}
				}
				else
				{
					if (pos.isMoveLegal(m))
						return s;
				}
			}
		}
	}
	return "error";
}
