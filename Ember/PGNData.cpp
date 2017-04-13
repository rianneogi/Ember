#include "PGNData.h"

std::string PieceStrings[] = { "- ","P ","N ","B ","R ","Q ","K ","p ","n ","b ","r ","q ","k " };

PGNData::PGNData()
{
}

PGNData::PGNData(std::string file)
{
	loadFromFile(file);
}

PGNData::~PGNData()
{
}

void PGNData::loadFromFile(std::string file)
{
	using std::cout;
	using std::endl;

	//int fenformat = 0;
	std::fstream f("pgn.pgn", std::ios::in);
	if (!f.is_open())
	{
		cout << "ERROR: cant open file: " << file << endl;
	}
	std::string s;
	//double error = 0;
	enum { SEARCH, PARSE, SQUARE, CURLY };
	int phase = SEARCH;
	std::string movestr = "";
	//double Res = 0;
	int gameno = 1;
	//int poscount = 0;

	//std::vector<Move> movelist;
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

				if (movestr.at(0) >= '0' && movestr.at(0) <= '9') //found result
				{
					int change = 0;
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
						//pos.setStartPos();
						//for (int i = 0; i < movelist.size(); i++)
						//{
						//	if (!pos.tryMove(movelist.at(i)))
						//	{
						//		cout << "ILLEGAL MOVE " << movelist.at(i).toString() << " " << i << endl;
						//	}
						//	currentGame.Moves.push_back(movelist[i]);
						//	//cout << "parsing move " << movelist.at(i).toString() << endl;
						//	//int score = e.QuiescenceSearch(CONST_NEGINF, CONST_INF);
						//	//if (score > 20000)
						//	//{
						//	//	e.pos.display(0);
						//	//	cout << endl;
						//	//}
						//	//if (e.pos.turn == COLOR_BLACK)
						//	//	score = -score;
						//	//double err = Res - (1 / (1 + pow(10, (-K*score) / 400)));
						//	//err *= err;
						//	////cout << score << " " << err << endl;
						//	//assert(err >= 0 && err <= 1);
						//	//error += err;
						//	//poscount++;
						//}
						Games.push_back(currentGame);
						currentGame = PGNGame();

						pos.setStartPos();
						//movelist.clear();
						gameno++;
						//cout << "gameno: " << gameno << endl;
					}
					//cout << movestr << endl;
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
						//movelist.push_back(theMove);
						//cout << "added move " << movestr << endl;
					}
				}
				//cout << movestr << endl;
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
	for (int i = 0; i < Games.size(); i++)
	{
		for (int j = 0; j > Games[i].Moves.size(); j++)
		{
			std::cout << Games[i].Moves[j].toString() << " ";
		}
		std::cout << std::endl;
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
