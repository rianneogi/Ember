#include "UCI.h"

std::string ENGINENAME = "Ember";
int ENGINEVERSION = 2;
std::string ENGINEAUTHOR = "Rian Neogi";

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

void UCI::run_uci()
{
	using std::cout;
	using std::cin;
	using std::endl;

	cout << "id name " << ENGINENAME << " " << ENGINEVERSION << endl;
	cout << "id author " << ENGINEAUTHOR << endl;
	cout << "uciok" << endl;
	std::string str = "";
	//char cp[5000];
	cout.setf(std::ios::unitbuf);// Make sure that the outputs are sent straight away to the GUI
	while (true)
	{
		str = "";
		/*fgets(cp,sizeof(cp),stdin);
		for(int i = 0;cp[i]!='\0' && cp[i]!='\n';i++)
		{
		str += cp[i];
		}*/
		getline(cin, str);
		std::string s = getStringToken(str, ' ', 1);
		if (s == "isready" || str == "isready")
		{
			cout << "readyok" << endl;
		}
		else if (s == "quit")
		{
			break;
		}
		else if (s == "uci")
		{
			cout << "id name " << ENGINENAME << " " << ENGINEVERSION << endl;
			cout << "id author " << ENGINEAUTHOR << endl;
			cout << "uciok" << endl;
		}
		else if (s == "go")
		{
			int wtime = 0;
			int btime = 0;
			int winc = 0;
			int binc = 0;
			int mode = MODE_DEFAULT;
			for (int i = 2;; i++)
			{
				s = getStringToken(str, ' ', i);
				if (s == "infinite")
				{
					wtime = -1;
					mode = MODE_MOVETIME;
				}
				if (s == "movetime")
				{
					wtime = atoi(getStringToken(str, ' ', i + 1).c_str());
					mode = MODE_MOVETIME;
					i++;
				}
				if (s == "wtime")
				{
					wtime = atoi(getStringToken(str, ' ', i + 1).c_str());
					i++;
				}
				if (s == "btime")
				{
					btime = atoi(getStringToken(str, ' ', i + 1).c_str());
					i++;
				}
				if (s == "winc")
				{
					winc = atoi(getStringToken(str, ' ', i + 1).c_str());
					i++;
				}
				if (s == "binc")
				{
					binc = atoi(getStringToken(str, ' ', i + 1).c_str());
					i++;
				}
				if (s == "")
					break;
			}

			if (wtime == 0)
				wtime = 1000;
			if (btime == 0)
				btime = 1000;

			Move m = Ember.go(mode, wtime, btime, winc, binc, true);
			cout << "bestmove " << m.toString() << endl;
			//e1.pos.forceMove(m);
		}
		else if (s == "setboard")
		{
			Ember.CurrentPos.loadFromFEN(str.substr(getStringTokenPosition(str, ' ', 2)));
		}
		else if (s == "position")
		{
			s = getStringToken(str, ' ', 2);
			int tokennumber = 3;
			if (s == "startpos")
			{
				//e1.pos = Position();
				Ember.CurrentPos.setStartPos();
			}
			else if (s == "fen")
			{
				//cout << "info string fen enter" << endl;
				Ember.CurrentPos.loadFromFEN(str.substr(getStringTokenPosition(str, ' ', 3)));
				tokennumber = 9;
			}

			s = getStringToken(str, ' ', tokennumber);
			if (s == "moves")
			{
				int i = tokennumber + 1;
				s = getStringToken(str, ' ', i);

				while (s != "")
				{
					std::vector<Move> v;
					v.reserve(128);
					Ember.CurrentPos.generateMoves(v);
					Move m = String2Move(s);
					int flag = 0;
					for (int j = 0; j<v.size(); j++)
					{
						if (v.at(j).getFrom() == m.getFrom() && v.at(j).getTo() == m.getTo() &&
							(
							(m.getSpecial() == v.at(j).getSpecial() &&
								(
									m.getSpecial() == PIECE_BISHOP || m.getSpecial() == PIECE_KNIGHT ||
									m.getSpecial() == PIECE_QUEEN || m.getSpecial() == PIECE_ROOK
									)
								)
								|| m.getSpecial() == PIECE_PAWN || m.getSpecial() == PIECE_KING || m.getSpecial() == PIECE_NONE
								)
							)
						{
							Ember.CurrentPos.makeMove(v.at(j));
							flag = 1;
							//cout << v.at(j).toString() << endl;
						}
					}
					//cout << s << endl;
					//cout << m.getFrom() << " " << m.getTo() << endl;
					if (flag == 0)
					{
						cout << "info string ERROR: Legal move not found: " 
							<< m.toString() << ", move count: " << Ember.CurrentPos.movelist.size() << endl;
					}
					i++;
					s = getStringToken(str, ' ', i);
				}
			}
		}
		else if (s == "move" || s == "makemove" || s == "playmove")
		{
			makeMove(getStringToken(str, ' ', 2));
		}
		else if (s == "display")
		{
			Ember.CurrentPos.display(0);
		}
		/*else if (s == "trace")
		{
			cout << e1.LeafEval<true>() << endl;
		}*/
		else if (s == "eval")
		{
			cout << Ember.LeafEval() << endl;
		}
		else if (s == "evalnn")
		{
			cout << Ember.LeafEval_NN() << endl;
		}
		/*else if (s == "qsearch")
		{
			cout << e1.QuiescenceSearch(CONS_NEGINF, CONS_INF) << endl;
		}*/
		else if (s == "info" || s == "information")
		{
			info();
		}
		else if (s == "ping")
		{
			s = getStringToken(str, ' ', 2);
			cout << "pong " << atoi(s.c_str()) << endl;
		}
		else if (s == "disp" || s == "display")
		{
			display(0);
		}
		else if (s == "dispflip" || s == "displayflip" || s == "displayflipped")
		{
			display(1);
		}
		else if (s == "unmakemove" || s == "unmake" || s == "unmove" || s == "undo")
		{
			unmakeMove();
		}
		else if (s == "perft")
		{
			Clock t;
			t.Start();
			s = getStringToken(str, ' ', 2);
			int pdepth = atoi(s.c_str());
			uint64_t perft = Ember.perft(pdepth);
			t.Stop();
			cout << "Count: " << perft << ", Time: " << t.ElapsedMilliseconds() << ", NPS: " << ((perft * 1000) / t.ElapsedMilliseconds()) << endl;
		}
		/*else if (s == "movesort")
		{
			std::vector<Move> vec;
			Ember.CurrentPos.generateMoves(vec);
			for (int i = 0; i < vec.size(); i++)
			{
				Move bm = e1.getHighestScoringMove(vec, i);
				cout << bm.toString() << " " << e1.getMoveScore(bm) << endl;
			}
		}
		else if (s == "pv")
		{
			cout << "ply = " << e1.ply << endl;
			cout << "pv size: " << e1.PrincipalVariation.size() << endl;
			for (int i = e1.PrincipalVariation.size() - 1; i >= 0; i--)
			{
				cout << e1.PrincipalVariation.at(i).toString() << endl;
			}
		}
		else if (s == "probe")
		{
			cout << Table.Probe(e1.pos.TTKey, 0, CONS_NEGINF, CONS_INF) << endl;
			cout << Table.getBestMove(e1.pos.TTKey).toString() << endl;
		}*/
		else if (s == "exit" || s == "quit")
		{
			//exit();
			break;
		}
		//fflush(stdin);
	}
}
