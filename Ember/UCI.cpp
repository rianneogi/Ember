#include "UCI.h"

UCI::UCI()
{
}

UCI::~UCI()
{
}

void UCI::run()
{
	using namespace std;
	string s;
	while (true)
	{
		if (Ember.CurrentPos.Turn == COLOR_WHITE)
		{
			cout << "White to move" << endl;
		}
		else
		{
			cout << "Black to move" << endl;
		}
		cin >> s;
		if (s == "disp" || s == "display")
		{
			display(0);
		}
		else if (s == "dispflip" || s == "displayflip" || s == "displayflipped")
		{
			display(1);
		}
		else if (s == "move" || s == "makemove" || s == "playmove")
		{
			string m;
			cin >> m;
			makeMove(m);
		}
		else if (s == "unmakemove" || s == "unmake" || s == "unmove" || s == "undo")
		{
			unmakeMove();
		}
		else if (s == "moves" || s == "getmoves")
		{
			getMoves();
		}
		else if (s == "perft")
		{
			string s2;
			cin >> s2;
			int pdepth = atoi(s2.c_str());
			cout << Ember.perft(pdepth) << endl;
		}
		else if (s == "exit" || s == "quit")
		{
			//exit();
			break;
		}
	}
}

void UCI::display(int flip)
{
	Ember.CurrentPos.display(flip);
}

void UCI::makeMove(std::string s)
{
	using namespace std;
	if (s == "null")
	{
		Ember.CurrentPos.makeMove(createNullMove(Ember.CurrentPos.EPSquare));
		//e1.makeMove(createNullMove(e1.pos.epsquare));
		return;
	}
	//cout << endl;
	int from = 0;
	int to = 0;
	int sp = PIECE_NONE;

	if (s.size()<4)
	{
		cout << "Invalid format" << endl;
		return;
	}
	from += (s.at(1) - 49) * 8;
	to += (s.at(3) - 49) * 8;
	from += 7 - (s.at(0) - 97);
	to += 7 - (s.at(2) - 97);
	if (s.size()>4)
	{
		char spc = s.at(4);
		if (spc == 'Q' || spc == 'q')
		{
			sp = PIECE_QUEEN;
		}
		else if (spc == 'N' || spc == 'n')
		{
			sp = PIECE_KNIGHT;
		}
		else if (spc == 'R' || spc == 'r')
		{
			sp = PIECE_ROOK;
		}
		else if (spc == 'B' || spc == 'b')
		{
			sp = PIECE_BISHOP;
		}
	}

	if (from < 0 || from > 63 || to < 0 || to > 63)
	{
		cout << "Invalid format" << endl;
		return;
	}

	vector<Move> moves;
	Ember.CurrentPos.generateMoves(moves);

	int flag = 0;
	for (unsigned int i = 0; i<moves.size(); i++)
	{
		Move m = moves.at(i);
		if (m.getTo() == to && m.getFrom() == from && (m.getSpecial() == sp
			|| m.getSpecial() == PIECE_KING || m.getSpecial() == PIECE_PAWN))
		{
			flag = 1;
			//if (!board.makeMove(m))
			//	continue;
			Ember.CurrentPos.makeMove(m);
			int status = Ember.CurrentPos.getGameStatus();
			if (status == STATUS_WHITEMATED) //post the game end status
			{
				cout << "White is checkmated!" << endl;
			}
			else if (status == STATUS_BLACKMATED)
			{
				cout << "Black is checkmated!" << endl;
			}
			else if (status == STATUS_STALEMATE)
			{
				cout << "Its a stalemate!" << endl;
			}
			else if (status == STATUS_INSUFFICIENTMAT)
			{
				cout << "Its a draw due to insufficient material!" << endl;
			}
			else if (status == STATUS_3FOLDREP)
			{
				cout << "Its a draw due to 3-fold repetition!" << endl;
			}
		}
	}
	if (flag == 0)
	{
		cout << "Illegal Move!" << endl;
	}
}

void UCI::unmakeMove()
{
}

void UCI::getMoves()
{
	using namespace std;
	vector<Move> moves;
	Ember.CurrentPos.generateMoves(moves);
	for (int i = 0; i<moves.size(); i++)
	{
		Move m = moves.at(i);
		string s = Int2Sq(m.getFrom());
		s += Int2Sq(m.getTo());
		cout << s << endl;
	}
	cout << "Total moves: " << moves.size() << endl;
}

void UCI::info()
{
	using namespace std;
	string s;
	cin >> s;
	if (s == "os")
	{
		printBitset(Ember.CurrentPos.OccupiedSq);
	}
	if (s == "eos")
	{
		printBitset(Ember.CurrentPos.OccupiedSq);
	}
	if (s == "wp")
	{
		printBitset(Ember.CurrentPos.Pieces[COLOR_WHITE][PIECE_PAWN]);
	}
	if (s == "ewp")
	{
		printBitset(Ember.CurrentPos.Pieces[COLOR_WHITE][PIECE_PAWN]);
	}
	if (s == "bp")
	{
		printBitset(Ember.CurrentPos.Pieces[COLOR_BLACK][PIECE_PAWN]);
	}
	if (s == "wn")
	{
		printBitset(Ember.CurrentPos.Pieces[COLOR_WHITE][PIECE_KNIGHT]);
	}
	if (s == "bn")
	{
		printBitset(Ember.CurrentPos.Pieces[COLOR_BLACK][PIECE_KNIGHT]);
	}
	if (s == "wb")
	{
		printBitset(Ember.CurrentPos.Pieces[COLOR_WHITE][PIECE_BISHOP]);
	}
	if (s == "bb")
	{
		printBitset(Ember.CurrentPos.Pieces[COLOR_BLACK][PIECE_BISHOP]);
	}
	if (s == "wr")
	{
		printBitset(Ember.CurrentPos.Pieces[COLOR_WHITE][PIECE_ROOK]);
	}
	if (s == "br")
	{
		printBitset(Ember.CurrentPos.Pieces[COLOR_BLACK][PIECE_ROOK]);
	}
	if (s == "wq")
	{
		printBitset(Ember.CurrentPos.Pieces[COLOR_WHITE][PIECE_QUEEN]);
	}
	if (s == "bq")
	{
		printBitset(Ember.CurrentPos.Pieces[COLOR_BLACK][PIECE_QUEEN]);
	}
	if (s == "wk")
	{
		printBitset(Ember.CurrentPos.Pieces[COLOR_WHITE][PIECE_KING]);
	}
	if (s == "bk")
	{
		printBitset(Ember.CurrentPos.Pieces[COLOR_BLACK][PIECE_KING]);
	}
	if (s == "epsq")
	{
		cout << Int2Sq(Ember.CurrentPos.EPSquare) << "(" << Ember.CurrentPos.EPSquare << ")" << endl;
	}
	if (s == "castling")
	{
		cout << Ember.CurrentPos.Castling[0][0] << " " << Ember.CurrentPos.Castling[0][1] << " " 
			<< Ember.CurrentPos.Castling[1][0] << " " << Ember.CurrentPos.Castling[1][1] << endl;
	}
	/*if (s == "epos")
	{
		for (int i = 0; i<64; i++)
		{
			if (i % 8 == 0)
				cout << endl;
			cout << pieceStrings[e1.pos.Squares[63 - i]];
		}
		cout << endl;
	}*/
	if (s == "key")
	{
		cout << Ember.CurrentPos.HashKey << endl;
	}
	if (s == "rep")
	{
		cout << Ember.CurrentPos.isRepetition() << endl;
	}
}
