#include "Position.h"

std::string pieceStrings[] = { "- ","P ","N ","B ","R ","Q ","K ","p ","n ","b ","r ","q ","k " };

Position::Position()
{
	setStartPos();
}

Position::~Position()
{
}

void Position::setStartPos()
{
	Pieces[COLOR_WHITE][PIECE_PAWN] = 0x000000000000ff00;
	Pieces[COLOR_BLACK][PIECE_PAWN] = 0x00ff000000000000;
	Pieces[COLOR_WHITE][PIECE_ROOK] = 0x0000000000000081;
	Pieces[COLOR_BLACK][PIECE_ROOK] = 0x8100000000000000;
	Pieces[COLOR_WHITE][PIECE_KNIGHT] = 0x0000000000000042;
	Pieces[COLOR_BLACK][PIECE_KNIGHT] = 0x4200000000000000;
	Pieces[COLOR_WHITE][PIECE_BISHOP] = 0x0000000000000024;
	Pieces[COLOR_BLACK][PIECE_BISHOP] = 0x2400000000000000;
	Pieces[COLOR_WHITE][PIECE_QUEEN] = 0x0000000000000010;
	Pieces[COLOR_BLACK][PIECE_QUEEN] = 0x1000000000000000;
	Pieces[COLOR_WHITE][PIECE_KING] = 0x0000000000000008;
	Pieces[COLOR_BLACK][PIECE_KING] = 0x0800000000000000;
	OccupiedSq = 0xffff00000000ffff;
	/*OccupiedSq90 = 0xc3c3c3c3c3c3c3c3;
	OccupiedSq45 = 0x0;
	OccupiedSq135 = 0x0;*/
	for (int i = 0; i<64; i++)
	{
		/*OccupiedSq45 |= getPos2Bit(getturn45(i))*((OccupiedSq >> i) % 2);
		OccupiedSq135 |= getPos2Bit(getturn135(i))*((OccupiedSq >> i) % 2);*/
		Squares[i] = SQUARE_EMPTY;
	}

	for (int i = 0; i<2; i++)
	{
		for (int j = 0; j<6; j++)
		{
			Bitset b = Pieces[i][j];
			while (b != 0)
			{
				unsigned long x;
				BitscanForward(&x, b);
				/*if (x == -1)
				{
					break;
				}*/
				Squares[x] = i * 6 + j + 1;
				b ^= getPos2Bit(x);
			}
		}
	}

	Turn = COLOR_WHITE;

	for (int i = 0; i<2; i++)
	{
		for (int j = 0; j<2; j++)
		{
			Castling[i][j] = 1;
		}
	}
	EPSquare = 0;

	HashKey = 0x0;
	//PawnKey = 0x0;
	for (int i = 0; i<64; i++)
	{
		if (Squares[i] != SQUARE_EMPTY)
		{
			HashKey ^= TT_PieceKey[getSquare2Color(Squares[i])][getSquare2Piece(Squares[i])][i];
			/*if (getSquare2Piece(Squares[i]) == PIECE_PAWN)
			{
				PawnKey ^= TT_PieceKey[getSquare2Color(Squares[i])][PIECE_PAWN][i];
			}*/
		}
	}
	for (int i = 0; i<2; i++)
	{
		for (int j = 0; j<2; j++)
		{
			if (Castling[i][j] == 1)
				HashKey ^= TT_CastlingKey[i][j];
		}
	}
	HashKey ^= TT_EPKey[EPSquare];

	//movelist = vector<Move>(0);
	//hashlist = vector<Bitset>(0);
}

void Position::initializeBitsets()
{
	OccupiedSq = 0;
	/*OccupiedSq90 = 0;
	OccupiedSq45 = 0;
	OccupiedSq135 = 0;*/
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			Pieces[i][j] = 0;
		}
	}
	for (int i = 0; i < 64; i++)
	{
		if (Squares[i] != SQUARE_EMPTY)
		{
			Pieces[getSquare2Color(Squares[i])][getSquare2Piece(Squares[i])] |= getPos2Bit(i);
			OccupiedSq |= getPos2Bit(i);
		}

	}
	/*for (int i = 0;i<64;i++)
	{
	OccupiedSq45 |= getPos2Bit(getturn45(i))*((OccupiedSq >> i) % 2);
	OccupiedSq135 |= getPos2Bit(getturn135(i))*((OccupiedSq >> i) % 2);
	OccupiedSq90 |= getPos2Bit(getturn90(i))*((OccupiedSq >> i) % 2);
	}*/
	HashKey = 0x0;
	//PawnKey = 0x0;
	for (int i = 0; i<64; i++)
	{
		if (Squares[i] != SQUARE_EMPTY)
		{
			HashKey ^= TT_PieceKey[getSquare2Color(Squares[i])][getSquare2Piece(Squares[i])][i];
			if (Squares[i] == SQUARE_WHITEPAWN || Squares[i] == SQUARE_BLACKPAWN)
			{
				HashKey ^= TT_PieceKey[getSquare2Color(Squares[i])][PIECE_PAWN][i];
			}
		}
	}
	for (int i = 0; i<2; i++)
	{
		for (int j = 0; j<2; j++)
		{
			if (Castling[i][j] == 1)
				HashKey ^= TT_CastlingKey[i][j];
		}
	}
	HashKey ^= TT_EPKey[EPSquare];
	if (Turn == COLOR_BLACK)
		HashKey ^= TT_ColorKey;
}

void Position::makeMove(Move const& m)
{
	/*if(DEBUG)
	cout << "made " << m.toString() << endl;*/
	movelist.push_back(m);
	hashlist.push_back(HashKey);
	if (m.isNullMove()) //nullmove
	{
		Turn = getOpponent(Turn);
		if (EPSquare != 0)
		{
			HashKey ^= TT_EPKey[EPSquare];
			HashKey ^= TT_EPKey[0];
			EPSquare = 0;
		}
		HashKey ^= TT_ColorKey;
		return;
	}
	Bitset from = m.getFrom();
	Bitset to = m.getTo();
	Bitset movingpiece = m.getMovingPiece();
	Bitset capturedpiece = m.getCapturedPiece();
	Bitset special = m.getSpecial();
	int cast[2][2];
	cast[0][0] = Castling[0][0];
	cast[0][1] = Castling[0][1];
	cast[1][0] = Castling[1][0];
	cast[1][1] = Castling[1][1];
	if (capturedpiece) //if a piece is captured update captured player's pieces
	{
		if (special == PIECE_QUEEN || special == PIECE_KNIGHT || special == PIECE_ROOK || special == PIECE_BISHOP) //promotion
		{
			OccupiedSq ^= getPos2Bit(from);
			Pieces[Turn][movingpiece] ^= getPos2Bit(from);
			Pieces[getOpponent(Turn)][getSquare2Piece(capturedpiece)] ^= getPos2Bit(to);
			Pieces[Turn][special] |= getPos2Bit(to);
			Squares[to] = getPiece2Square(special, Turn);
			Squares[from] = SQUARE_EMPTY;
			HashKey ^= TT_PieceKey[getOpponent(Turn)][getSquare2Piece(capturedpiece)][to];
			HashKey ^= TT_PieceKey[Turn][movingpiece][from];
			HashKey ^= TT_PieceKey[Turn][special][to];
		}
		else
		{
			OccupiedSq ^= getPos2Bit(from);
			Pieces[Turn][movingpiece] ^= m.getbit();
			Pieces[getOpponent(Turn)][getSquare2Piece(capturedpiece)] ^= getPos2Bit(to);
			Squares[to] = Squares[from];
			Squares[from] = SQUARE_EMPTY;
			HashKey ^= TT_PieceKey[getOpponent(Turn)][getSquare2Piece(capturedpiece)][to];
			HashKey ^= TT_PieceKey[Turn][movingpiece][from];
			HashKey ^= TT_PieceKey[Turn][movingpiece][to];
		}
	}
	else if (special == PIECE_QUEEN || special == PIECE_KNIGHT || special == PIECE_ROOK || special == PIECE_BISHOP) //promotion
	{
		OccupiedSq ^= m.getbit();
		Pieces[Turn][movingpiece] ^= getPos2Bit(from);
		Pieces[Turn][special] |= getPos2Bit(to);
		Squares[to] = getPiece2Square(special, Turn);
		Squares[from] = SQUARE_EMPTY;
		HashKey ^= TT_PieceKey[Turn][movingpiece][from];
		HashKey ^= TT_PieceKey[Turn][special][to];
	}
	else
	{
		Bitset bit = m.getbit();
		OccupiedSq ^= bit;
		Pieces[Turn][movingpiece] ^= bit;
		Squares[to] = Squares[from];
		Squares[from] = SQUARE_EMPTY;
		HashKey ^= TT_PieceKey[Turn][movingpiece][from];
		HashKey ^= TT_PieceKey[Turn][movingpiece][to];
	}
	//cout << m.getFrom() << " " << m.getTo() << " " << m.getMovingPiece() << " " << m.getCapturedPiece() << " " << m.getSpecial() << endl;

	if (special == PIECE_KING) //castling
	{
		int f = -1;
		int t = -1;
		if (Turn == COLOR_WHITE)
		{
			if (to == 1)
			{
				f = 0;
				t = 2;
			}
			if (to == 5)
			{
				f = 7;
				t = 4;
			}
		}
		else
		{
			if (to == 57)
			{
				f = 56;
				t = 58;
			}
			if (to == 61)
			{
				f = 63;
				t = 60;
			}
		}
		Bitset cas = getPos2Bit(f) | getPos2Bit(t);
		/*Bitset cas45 = getPos2Bit(getturn45(f)) | getPos2Bit(getturn45(t));
		Bitset cas135 = getPos2Bit(getturn135(f)) | getPos2Bit(getturn135(t));
		Bitset cas90 = getPos2Bit(getturn90(f)) | getPos2Bit(getturn90(t));*/
		OccupiedSq ^= cas;
		Pieces[Turn][PIECE_ROOK] ^= cas;
		Squares[t] = Squares[f];
		Squares[f] = SQUARE_EMPTY;

		HashKey ^= TT_PieceKey[Turn][PIECE_ROOK][f];
		HashKey ^= TT_PieceKey[Turn][PIECE_ROOK][t];
	}
	else if (special == PIECE_PAWN) //en passant
	{
		int x = getMinus8(Turn, to);
		Pieces[getOpponent(Turn)][PIECE_PAWN] ^= getPos2Bit(x);
		HashKey ^= TT_PieceKey[getOpponent(Turn)][PIECE_PAWN][x];
		OccupiedSq ^= getPos2Bit(x);
		Squares[x] = SQUARE_EMPTY;
	}

	if (movingpiece == PIECE_KING) //removing castling rights
	{
		if (Turn == COLOR_WHITE)
		{
			Castling[0][0] = 0;
			Castling[0][1] = 0;
		}
		else
		{
			Castling[1][0] = 0;
			Castling[1][1] = 0;
		}
	}

	if (movingpiece == PIECE_ROOK) //removing castling rights
	{
		if (from == 0)
		{
			Castling[0][0] = 0;
		}
		if (from == 7)
		{
			Castling[0][1] = 0;
		}
		if (from == 56)
		{
			Castling[1][0] = 0;
		}
		if (from == 63)
		{
			Castling[1][1] = 0;
		}
	}

	if (capturedpiece == SQUARE_WHITEROOK) //removing castling rights if rook gets captured
	{
		if (to == 0)
		{
			Castling[0][0] = 0;
		}
		else if (to == 7)
		{
			Castling[0][1] = 0;
		}
	}
	if (capturedpiece == SQUARE_BLACKROOK) //removing castling rights if rook gets captured
	{
		if (to == 56)
		{
			Castling[1][0] = 0;
		}
		else if (to == 63)
		{
			Castling[1][1] = 0;
		}
	}

	if (movingpiece == PIECE_PAWN) //set ep square
	{
		if (Turn == COLOR_WHITE && getRank(int(from)) == 1 && getRank(int(to)) == 3)
		{
			HashKey ^= TT_EPKey[EPSquare];
			EPSquare = getPlus8(COLOR_WHITE, from);
			HashKey ^= TT_EPKey[EPSquare];
		}
		else if (Turn == COLOR_BLACK && getRank(int(from)) == 6 && getRank(int(to)) == 4)
		{
			HashKey ^= TT_EPKey[EPSquare];
			EPSquare = getPlus8(COLOR_WHITE, to);
			HashKey ^= TT_EPKey[EPSquare];
		}
		else if (EPSquare != 0)
		{
			HashKey ^= TT_EPKey[EPSquare];
			EPSquare = 0;
			HashKey ^= TT_EPKey[0];
		}
	}
	else if (EPSquare != 0)
	{
		HashKey ^= TT_EPKey[EPSquare];
		EPSquare = 0;
		HashKey ^= TT_EPKey[0];
	}

	if (cast[0][0] != Castling[0][0]) //check if castling values have changed
	{
		HashKey ^= TT_CastlingKey[0][0];
	}
	if (cast[0][1] != Castling[0][1])
	{
		HashKey ^= TT_CastlingKey[0][1];
	}
	if (cast[1][0] != Castling[1][0])
	{
		HashKey ^= TT_CastlingKey[1][0];
	}
	if (cast[1][1] != Castling[1][1])
	{
		HashKey ^= TT_CastlingKey[1][1];
	}

	/*if (movingpiece == PIECE_PAWN)
	{
		PawnKey ^= TT_PieceKey[Turn][movingpiece][from];
		if (special == PIECE_NONE || special == PIECE_PAWN)
			PawnKey ^= TT_PieceKey[Turn][movingpiece][to];
	}*/
	/*if (getSquare2Piece(capturedpiece) == PIECE_PAWN)
	{
		if (special == PIECE_PAWN)
		{
			PawnKey ^= TT_PieceKey[getOpponent(Turn)][PIECE_PAWN][getMinus8(Turn, to)];
		}
		else
		{
			PawnKey ^= TT_PieceKey[getOpponent(Turn)][PIECE_PAWN][to];
		}
	}*/

	Turn = getOpponent(Turn);
	HashKey ^= TT_ColorKey;
}

void Position::unmakeMove(Move const& m)
{
	/*if(DEBUG)
	cout << "unmade " << m.toString() << endl;*/
	movelist.pop_back();
	hashlist.pop_back();
	if (m.isNullMove()) //nullmove
	{
		Turn = getOpponent(Turn);
		HashKey ^= TT_ColorKey;
		int ep = m.getEP(); //setting ep square
		if (EPSquare != ep)
		{
			HashKey ^= TT_EPKey[EPSquare];
			HashKey ^= TT_EPKey[ep];
			EPSquare = ep;
		}
		return;
	}
	Bitset from = m.getFrom();
	Bitset to = m.getTo();
	Bitset movingpiece = m.getMovingPiece();
	Bitset capturedpiece = m.getCapturedPiece();
	Bitset special = m.getSpecial();
	Bitset bit = m.getbit();
	if (capturedpiece)
	{
		if (special == PIECE_QUEEN || special == PIECE_KNIGHT || special == PIECE_ROOK || special == PIECE_BISHOP) //promotion
		{
			OccupiedSq |= getPos2Bit(from);
			Pieces[Turn][getSquare2Piece(capturedpiece)] |= getPos2Bit(to);

			HashKey ^= TT_PieceKey[Turn][getSquare2Piece(capturedpiece)][to];

			Turn = getOpponent(Turn);
			HashKey ^= TT_ColorKey;

			Pieces[Turn][movingpiece] |= getPos2Bit(from);
			Pieces[Turn][special] ^= getPos2Bit(to);

			Squares[from] = getPiece2Square(PIECE_PAWN, Turn);
			Squares[to] = capturedpiece;

			HashKey ^= TT_PieceKey[Turn][movingpiece][from];
			HashKey ^= TT_PieceKey[Turn][special][to];
		}
		else
		{
			OccupiedSq |= getPos2Bit(from);
			Pieces[Turn][getSquare2Piece(capturedpiece)] |= getPos2Bit(to);

			HashKey ^= TT_PieceKey[Turn][getSquare2Piece(capturedpiece)][to];

			Turn = getOpponent(Turn);
			HashKey ^= TT_ColorKey;
			Pieces[Turn][movingpiece] ^= bit;

			Squares[from] = getPiece2Square(movingpiece, Turn);
			Squares[to] = capturedpiece;

			HashKey ^= TT_PieceKey[Turn][movingpiece][from];
			HashKey ^= TT_PieceKey[Turn][movingpiece][to];
		}
	}
	else if (special == PIECE_QUEEN || special == PIECE_KNIGHT || special == PIECE_ROOK || special == PIECE_BISHOP) //promotion
	{
		Bitset bit = m.getbit();
		OccupiedSq ^= bit;

		Turn = getOpponent(Turn);
		HashKey ^= TT_ColorKey;

		Pieces[Turn][movingpiece] |= getPos2Bit(from);
		Pieces[Turn][special] ^= getPos2Bit(to);

		Squares[from] = getPiece2Square(PIECE_PAWN, Turn);
		Squares[to] = capturedpiece;

		HashKey ^= TT_PieceKey[Turn][movingpiece][from];
		HashKey ^= TT_PieceKey[Turn][special][to];
	}
	else
	{
		OccupiedSq ^= bit;

		Turn = getOpponent(Turn);
		HashKey ^= TT_ColorKey;
		Pieces[Turn][movingpiece] ^= bit;

		Squares[from] = getPiece2Square(movingpiece, Turn);
		Squares[to] = capturedpiece;

		HashKey ^= TT_PieceKey[Turn][movingpiece][from];
		HashKey ^= TT_PieceKey[Turn][movingpiece][to];
	}

	if (special == PIECE_KING) //castling
	{
		int f = -1;
		int t = -1;
		if (Turn == COLOR_WHITE)
		{
			if (to == 1)
			{
				f = 0;
				t = 2;
			}
			if (to == 5)
			{
				f = 7;
				t = 4;
			}
		}
		else
		{
			if (to == 57)
			{
				f = 56;
				t = 58;
			}
			if (to == 61)
			{
				f = 63;
				t = 60;
			}
		}
		Bitset cas = getPos2Bit(f) | getPos2Bit(t);
		/*Bitset cas45 = getPos2Bit(getturn45(f)) | getPos2Bit(getturn45(t));
		Bitset cas135 = getPos2Bit(getturn135(f)) | getPos2Bit(getturn135(t));
		Bitset cas90 = getPos2Bit(getturn90(f)) | getPos2Bit(getturn90(t));*/
		OccupiedSq ^= cas;
		Pieces[Turn][PIECE_ROOK] ^= cas;
		Squares[f] = Squares[t];
		Squares[t] = SQUARE_EMPTY;

		HashKey ^= TT_PieceKey[Turn][PIECE_ROOK][f];
		HashKey ^= TT_PieceKey[Turn][PIECE_ROOK][t];
	}
	else if (special == PIECE_PAWN) //en passant
	{
		int x = getMinus8(Turn, to);
		Pieces[getOpponent(Turn)][PIECE_PAWN] ^= getPos2Bit(x);
		Squares[x] = getPiece2Square(PIECE_PAWN, getOpponent(Turn));
		HashKey ^= TT_PieceKey[getOpponent(Turn)][PIECE_PAWN][x];
		OccupiedSq ^= getPos2Bit(x);
	}

	int wkc = m.getWKC();
	int wqc = m.getWQC();
	int bkc = m.getBKC();
	int bqc = m.getBQC();
	if (Castling[COLOR_WHITE][CASTLE_KINGSIDE] != wkc) //reset castling
	{
		Castling[COLOR_WHITE][CASTLE_KINGSIDE] = wkc;
		HashKey ^= TT_CastlingKey[COLOR_WHITE][CASTLE_KINGSIDE];
	}
	if (Castling[COLOR_WHITE][CASTLE_QUEENSIDE] != wqc)
	{
		Castling[COLOR_WHITE][CASTLE_QUEENSIDE] = wqc;
		HashKey ^= TT_CastlingKey[COLOR_WHITE][CASTLE_QUEENSIDE];
	}
	if (Castling[COLOR_BLACK][CASTLE_KINGSIDE] != bkc)
	{
		Castling[COLOR_BLACK][CASTLE_KINGSIDE] = bkc;
		HashKey ^= TT_CastlingKey[COLOR_BLACK][CASTLE_KINGSIDE];
	}
	if (Castling[COLOR_BLACK][CASTLE_QUEENSIDE] != bqc)
	{
		Castling[COLOR_BLACK][CASTLE_QUEENSIDE] = bqc;
		HashKey ^= TT_CastlingKey[COLOR_BLACK][CASTLE_QUEENSIDE];
	}
	int ep = m.getEP(); //setting ep square
	if (EPSquare != ep)
	{
		HashKey ^= TT_EPKey[EPSquare];
		HashKey ^= TT_EPKey[ep];
		EPSquare = ep;
	}

	/*if (movingpiece == PIECE_PAWN)
	{
		PawnKey ^= TT_PieceKey[Turn][movingpiece][from];
		if (special == PIECE_NONE || special == PIECE_PAWN)
			PawnKey ^= TT_PieceKey[Turn][movingpiece][to];
	}
	if (getSquare2Piece(capturedpiece) == PIECE_PAWN)
	{
		if (special == PIECE_PAWN)
		{
			PawnKey ^= TT_PieceKey[getOpponent(Turn)][PIECE_PAWN][getMinus8(Turn, to)];
		}
		else
			PawnKey ^= TT_PieceKey[getOpponent(Turn)][PIECE_PAWN][to];
	}*/
}

void Position::addMove(std::vector<Move>& vec, Move const& m)
{
	makeMove(m);
	if(!underCheck(getOpponent(Turn)))
	{
		vec.push_back(m);
	//printf("added move %s\n", m.toString());
	}
	unmakeMove(m);
}

void Position::generateMoves(std::vector<Move>& moves)
{
	//std::vector<Move> moves(0);
	//moves.reserve(128);
	//moves.clear();

	Bitset ColorPieces[2];

	ColorPieces[COLOR_WHITE] = Pieces[COLOR_WHITE][PIECE_PAWN] | Pieces[COLOR_WHITE][PIECE_KNIGHT] |
		Pieces[COLOR_WHITE][PIECE_BISHOP] | Pieces[COLOR_WHITE][PIECE_ROOK] |
		Pieces[COLOR_WHITE][PIECE_QUEEN] | Pieces[COLOR_WHITE][PIECE_KING];
	ColorPieces[COLOR_BLACK] = Pieces[COLOR_BLACK][PIECE_PAWN] | Pieces[COLOR_BLACK][PIECE_KNIGHT] |
		Pieces[COLOR_BLACK][PIECE_BISHOP] | Pieces[COLOR_BLACK][PIECE_ROOK] |
		Pieces[COLOR_BLACK][PIECE_QUEEN] | Pieces[COLOR_BLACK][PIECE_KING];

	Bitset b; //current piece bitboard
	unsigned long n = 0; //from square index
	unsigned long k = 0; //to square index
	
	//Pawn Moves
	b = Pieces[Turn][PIECE_PAWN];
	while (b)
	{
		//unsigned long n = 0;
		BitscanForward(&n, b);
		b ^= getPos2Bit(n);
		Bitset m = PawnMoves[Turn][n] & (PawnMoves[Turn][n] ^ OccupiedSq);
		while (m)
		{
			//unsigned long k = 0;
			BitscanForward(&k, m);
			m ^= getPos2Bit(k);

			if (getRank(k) == 7 || getRank(k) == 0) //promotion
			{
				Move movq(n, k, PIECE_PAWN, Squares[k], PIECE_QUEEN, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, movq);
				Move movn(n, k, PIECE_PAWN, Squares[k], PIECE_KNIGHT, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, movn);
				Move movr(n, k, PIECE_PAWN, Squares[k], PIECE_ROOK, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, movr);
				Move movb(n, k, PIECE_PAWN, Squares[k], PIECE_BISHOP, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, movb);
			}
			else
			{
				Move mov(n, k, PIECE_PAWN, Squares[k], PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, mov);
			}
		}
		//double moves
		if (Turn == COLOR_WHITE)
		{
			if (n == 8 && Squares[16] == SQUARE_EMPTY && Squares[24] == SQUARE_EMPTY)
			{
				Move mov(n, 24, PIECE_PAWN, SQUARE_EMPTY, PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, mov);
			}
			else if (n == 9 && Squares[17] == SQUARE_EMPTY && Squares[25] == SQUARE_EMPTY)
			{
				Move mov(n, 25, PIECE_PAWN, SQUARE_EMPTY, PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, mov);
			}
			else if (n == 10 && Squares[18] == SQUARE_EMPTY && Squares[26] == SQUARE_EMPTY)
			{
				Move mov(n, 26, PIECE_PAWN, SQUARE_EMPTY, PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, mov);
			}
			else if (n == 11 && Squares[19] == SQUARE_EMPTY && Squares[27] == SQUARE_EMPTY)
			{
				Move mov(n, 27, PIECE_PAWN, SQUARE_EMPTY, PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, mov);
			}
			else if (n == 12 && Squares[20] == SQUARE_EMPTY && Squares[28] == SQUARE_EMPTY)
			{
				Move mov(n, 28, PIECE_PAWN, SQUARE_EMPTY, PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, mov);
			}
			else if (n == 13 && Squares[21] == SQUARE_EMPTY && Squares[29] == SQUARE_EMPTY)
			{
				Move mov(n, 29, PIECE_PAWN, SQUARE_EMPTY, PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, mov);
			}
			else if (n == 14 && Squares[22] == SQUARE_EMPTY && Squares[30] == SQUARE_EMPTY)
			{
				Move mov(n, 30, PIECE_PAWN, SQUARE_EMPTY, PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, mov);
			}
			else if (n == 15 && Squares[23] == SQUARE_EMPTY && Squares[31] == SQUARE_EMPTY)
			{
				Move mov(n, 31, PIECE_PAWN, SQUARE_EMPTY, PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, mov);
			}
		}
		else
		{
			if (n == 48 && Squares[40] == SQUARE_EMPTY && Squares[32] == SQUARE_EMPTY)
			{
				Move mov(n, 32, PIECE_PAWN, SQUARE_EMPTY, PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, mov);
			}
			else if (n == 49 && Squares[41] == SQUARE_EMPTY && Squares[33] == SQUARE_EMPTY)
			{
				Move mov(n, 33, PIECE_PAWN, SQUARE_EMPTY, PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, mov);
			}
			else if (n == 50 && Squares[42] == SQUARE_EMPTY && Squares[34] == SQUARE_EMPTY)
			{
				Move mov(n, 34, PIECE_PAWN, SQUARE_EMPTY, PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, mov);
			}
			else if (n == 51 && Squares[43] == SQUARE_EMPTY && Squares[35] == SQUARE_EMPTY)
			{
				Move mov(n, 35, PIECE_PAWN, SQUARE_EMPTY, PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, mov);
			}
			else if (n == 52 && Squares[44] == SQUARE_EMPTY && Squares[36] == SQUARE_EMPTY)
			{
				Move mov(n, 36, PIECE_PAWN, SQUARE_EMPTY, PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, mov);
			}
			else if (n == 53 && Squares[45] == SQUARE_EMPTY && Squares[37] == SQUARE_EMPTY)
			{
				Move mov(n, 37, PIECE_PAWN, SQUARE_EMPTY, PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, mov);
			}
			else if (n == 54 && Squares[46] == SQUARE_EMPTY && Squares[38] == SQUARE_EMPTY)
			{
				Move mov(n, 38, PIECE_PAWN, SQUARE_EMPTY, PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, mov);
			}
			else if (n == 55 && Squares[47] == SQUARE_EMPTY && Squares[39] == SQUARE_EMPTY)
			{
				Move mov(n, 39, PIECE_PAWN, SQUARE_EMPTY, PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, mov);
			}
		}
		
		//Pawn Attacks
		Bitset x = 0x0;
		if (EPSquare == 0)
			x = PawnAttacks[Turn][n] & (ColorPieces[getOpponent(Turn)]);
		else
			x = PawnAttacks[Turn][n] & (ColorPieces[getOpponent(Turn)] | getPos2Bit(EPSquare));
		while (x)
		{
			//unsigned long k = 0;
			BitscanForward(&k, x);
			x ^= getPos2Bit(k);
			if (getRank(int(k)) == 7 || getRank(int(k)) == 0) //promotion
			{
				Move movq(n, k, PIECE_PAWN, Squares[k], PIECE_QUEEN, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, movq);
				Move movn(n, k, PIECE_PAWN, Squares[k], PIECE_KNIGHT, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, movn);
				Move movr(n, k, PIECE_PAWN, Squares[k], PIECE_ROOK, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, movr);
				Move movb(n, k, PIECE_PAWN, Squares[k], PIECE_BISHOP, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, movb);
			}
			else
			{
				Move mov(n, k, PIECE_PAWN, Squares[k], PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				if (k == EPSquare && EPSquare != 0)
					mov = Move(n, k, PIECE_PAWN, Squares[k], PIECE_PAWN, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
				addMove(moves, mov);
			}
		}
	}
	
	//Knight Moves
	b = Pieces[Turn][PIECE_KNIGHT];
	while (b)
	{
		//unsigned long n = 0;
		BitscanForward(&n, b);
		b ^= getPos2Bit(n);
		Bitset m = KnightMoves[n] & (KnightMoves[n] ^ ColorPieces[Turn]);
		while (m)
		{
			//unsigned long k = 0;
			BitscanForward(&k, m);
			m ^= getPos2Bit(k);
			Move mov(n, k, PIECE_KNIGHT, Squares[k], PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
			addMove(moves, mov);
		}
	}

	//King Moves
	b = Pieces[Turn][PIECE_KING];
	while (b)
	{
		//unsigned long n = 0;
		BitscanForward(&n, b);
		b ^= getPos2Bit(n);
		Bitset m = getKingMoves(n)&(getKingMoves(n) ^ ColorPieces[Turn]);
		while (m)
		{
			//unsigned long k = 0;
			BitscanForward(&k, m);
			m ^= getPos2Bit(k);
			Move mov(n, k, PIECE_KING, Squares[k], PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
			addMove(moves, mov);
		}
	}

	//Rook Moves
	b = Pieces[Turn][PIECE_ROOK];
	while (b)
	{
		//unsigned long n = 0;
		BitscanForward(&n, b);
		b ^= getPos2Bit(n);
		Bitset m = getRookAttacks(n, OccupiedSq);
		m &= m^ColorPieces[Turn];
		while (m)
		{
			//unsigned long k = 0;
			BitscanForward(&k, m);
			m ^= getPos2Bit(k);
			Move mov(n, k, PIECE_ROOK, Squares[k], PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
			addMove(moves, mov);
		}
	}

	//Bishop Moves
	b = Pieces[Turn][PIECE_BISHOP];
	while (b)
	{
		//unsigned long n = 0;
		BitscanForward(&n, b);
		b ^= getPos2Bit(n);
		Bitset m = getBishopAttacks(n, OccupiedSq);
		m &= m^ColorPieces[Turn];
		while (m)
		{
			//unsigned long k = 0;
			BitscanForward(&k, m);
			m ^= getPos2Bit(k);
			Move mov(n, k, PIECE_BISHOP, Squares[k], PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
			addMove(moves, mov);
		}
	}

	//Queen Moves
	b = Pieces[Turn][PIECE_QUEEN];
	while (b)
	{
		//unsigned long n = 0;
		BitscanForward(&n, b);
		b ^= getPos2Bit(n);
		Bitset m = getQueenAttacks(n, OccupiedSq);
		m &= m^ColorPieces[Turn];
		while (m)
		{
			//unsigned long k = 0;
			BitscanForward(&k, m);
			m ^= getPos2Bit(k);
			Move mov(n, k, PIECE_QUEEN, Squares[k], PIECE_NONE, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
			addMove(moves, mov);
		}
	}

	//Castling
	if (Turn == COLOR_WHITE)
	{
		if (Castling[Turn][CASTLE_KINGSIDE] && !isAttacked(Turn, 3) && !isAttacked(Turn, 2) && !isAttacked(Turn, 1) && Squares[2] == SQUARE_EMPTY && Squares[1] == SQUARE_EMPTY)
		{
			Move mov(3, 1, PIECE_KING, SQUARE_EMPTY, PIECE_KING, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);

			addMove(moves, mov);
		}
		if (Castling[Turn][CASTLE_QUEENSIDE] && !isAttacked(Turn, 3) && !isAttacked(Turn, 4) && !isAttacked(Turn, 5) && Squares[4] == SQUARE_EMPTY && Squares[5] == SQUARE_EMPTY && Squares[6] == SQUARE_EMPTY)
		{
			Move mov(3, 5, PIECE_KING, SQUARE_EMPTY, PIECE_KING, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
			addMove(moves, mov);
		}
	}
	else
	{
		if (Castling[Turn][CASTLE_KINGSIDE] && !isAttacked(Turn, 59) && !isAttacked(Turn, 58) && !isAttacked(Turn, 57) && Squares[58] == SQUARE_EMPTY && Squares[57] == SQUARE_EMPTY)
		{
			Move mov(59, 57, PIECE_KING, SQUARE_EMPTY, PIECE_KING, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
			addMove(moves, mov);
		}
		if (Castling[Turn][CASTLE_QUEENSIDE] && !isAttacked(Turn, 59) && !isAttacked(Turn, 60) && !isAttacked(Turn, 61) && Squares[60] == SQUARE_EMPTY && Squares[61] == SQUARE_EMPTY && Squares[62] == SQUARE_EMPTY)
		{
			Move mov(59, 61, PIECE_KING, SQUARE_EMPTY, PIECE_KING, Castling[0][0], Castling[0][1], Castling[1][0], Castling[1][1], EPSquare);
			addMove(moves, mov);
		}
	}
}

bool Position::isAttacked(int turn, int n) const
{
	int opp = getOpponent(turn);
	Bitset b = getBishopAttacks(n, OccupiedSq)&(Pieces[opp][PIECE_BISHOP] | Pieces[opp][PIECE_QUEEN]);
	if (b != 0)
		return true;
	b = getRookAttacks(n, OccupiedSq)&(Pieces[opp][PIECE_ROOK] | Pieces[opp][PIECE_QUEEN]);
	if (b != 0)
		return true;
	b = PawnAttacks[turn][n] & Pieces[opp][PIECE_PAWN];
	if (b != 0)
		return true;
	b = KnightMoves[n] & Pieces[opp][PIECE_KNIGHT];
	if (b != 0)
		return true;
	b = KingMoves[n] & Pieces[opp][PIECE_KING];
	if (b != 0)
		return true;
	return false;
}

bool Position::underCheck(int turn) const
{
	unsigned long k = 0;
	BitscanForward(&k, Pieces[turn][PIECE_KING]);
	return isAttacked(turn, k);
}

int Position::getGameStatus()
{
	/*if (isRepetition())
	{
		return STATUS_3FOLDREP;
	}*/
	std::vector<Move> vec;
	vec.reserve(128);
	generateMoves(vec);
	if (vec.size() == 0)
	{
		if (underCheck(Turn) && Turn == COLOR_WHITE)
		{
			return STATUS_WHITEMATED;
		}
		else if (underCheck(Turn) && Turn == COLOR_BLACK)
		{
			return STATUS_BLACKMATED;
		}
		else
		{
			return STATUS_STALEMATE;
		}
	}
	return STATUS_NOTOVER;
}

bool Position::isRepetition()
{
	int rep = 0;
	int i;
	int size = hashlist.size();
	for (i = 1; i <= size; i++)
	{
		Move m = movelist.at(size - i);
		if (!m.isNullMove() && m.getMovingPiece() != PIECE_PAWN && m.getCapturedPiece() == SQUARE_EMPTY && m.getSpecial() != PIECE_PAWN)
		{
			if (hashlist.at(size - i) == HashKey)
			{
				rep++;
				if (rep >= 1)
				{
					return true;
				}
			}
		}
		else
		{
			return false;
		}
	}
	return false;
}

void Position::display(int flip)
{
	for (int i = 0; i<64; i++)
	{
		if (i % 8 == 0)
			printf("\n");
		if (flip == 1)
			printf("%s", pieceStrings[Squares[i]]);
		else
			printf("%s", pieceStrings[Squares[63 - i]]);
	}
	printf("\n");
}

void Position::clearBoard()
{
	Pieces[COLOR_WHITE][PIECE_PAWN] = 0;
	Pieces[COLOR_BLACK][PIECE_PAWN] = 0;
	Pieces[COLOR_WHITE][PIECE_ROOK] = 0;
	Pieces[COLOR_BLACK][PIECE_ROOK] = 0;
	Pieces[COLOR_WHITE][PIECE_KNIGHT] = 0;
	Pieces[COLOR_BLACK][PIECE_KNIGHT] = 0;
	Pieces[COLOR_WHITE][PIECE_BISHOP] = 0;
	Pieces[COLOR_BLACK][PIECE_BISHOP] = 0;
	Pieces[COLOR_WHITE][PIECE_QUEEN] = 0;
	Pieces[COLOR_BLACK][PIECE_QUEEN] = 0;
	Pieces[COLOR_WHITE][PIECE_KING] = 0;
	Pieces[COLOR_BLACK][PIECE_KING] = 0;
	OccupiedSq = 0;
	/*OccupiedSq90 = 0;
	OccupiedSq45 = 0;
	OccupiedSq135 = 0;*/
	for (int i = 0; i<64; i++)
	{
		Squares[i] = SQUARE_EMPTY;
	}

	Turn = COLOR_WHITE;

	for (int i = 0; i<2; i++)
	{
		for (int j = 0; j<2; j++)
		{
			Castling[i][j] = 0;
		}
	}
	EPSquare = 0;

	HashKey = 0x0;
	//PawnKey = 0x0;
	for (int i = 0; i<64; i++)
	{
		if (Squares[i] != SQUARE_EMPTY)
		{
			HashKey ^= TT_PieceKey[getSquare2Color(Squares[i])][getSquare2Piece(Squares[i])][i];
			/*if (Squares[i] == SQUARE_WHITEPAWN || Squares[i] == SQUARE_BLACKPAWN)
			{
				PawnKey ^= TT_PieceKey[getSquare2Color(Squares[i])][PIECE_PAWN][i];
			}*/
		}
	}
	for (int i = 0; i<2; i++)
	{
		for (int j = 0; j<2; j++)
		{
			if (Castling[i][j] == 1)
				HashKey ^= TT_CastlingKey[i][j];
		}
	}
	HashKey ^= TT_EPKey[EPSquare];

	movelist = std::vector<Move>(0);
	hashlist = std::vector<Bitset>(0);
}

void Position::placePiece(int square, int location)
{
	Squares[location] = square;
}

void Position::loadFromFEN(std::string fen)
{
	clearBoard();
	std::string s = getStringToken(fen, ' ', 1);
	int currsquare = 63;
	for (int i = 0; i < s.size(); i++)
	{
		//cout << "info string fen " << s.at(i) << endl;
		if (s.at(i) >= '0' && s.at(i) <= '9')
		{
			currsquare -= s.at(i) - 48;
		}
		else if (s.at(i) == 'K')
		{
			placePiece(SQUARE_WHITEKING, currsquare);
			currsquare--;
		}
		else if (s.at(i) == 'k')
		{
			placePiece(SQUARE_BLACKKING, currsquare);
			currsquare--;
		}
		else if (s.at(i) == 'Q')
		{
			placePiece(SQUARE_WHITEQUEEN, currsquare);
			currsquare--;
		}
		else if (s.at(i) == 'q')
		{
			placePiece(SQUARE_BLACKQUEEN, currsquare);
			currsquare--;
		}
		else if (s.at(i) == 'R')
		{
			placePiece(SQUARE_WHITEROOK, currsquare);
			currsquare--;
		}
		else if (s.at(i) == 'r')
		{
			placePiece(SQUARE_BLACKROOK, currsquare);
			currsquare--;
		}
		else if (s.at(i) == 'N')
		{
			placePiece(SQUARE_WHITEKNIGHT, currsquare);
			currsquare--;
		}
		else if (s.at(i) == 'n')
		{
			placePiece(SQUARE_BLACKKNIGHT, currsquare);
			currsquare--;
		}
		else if (s.at(i) == 'B')
		{
			placePiece(SQUARE_WHITEBISHOP, currsquare);
			currsquare--;
		}
		else if (s.at(i) == 'b')
		{
			placePiece(SQUARE_BLACKBISHOP, currsquare);
			currsquare--;
		}
		else if (s.at(i) == 'P')
		{
			placePiece(SQUARE_WHITEPAWN, currsquare);
			currsquare--;
		}
		else if (s.at(i) == 'p')
		{
			placePiece(SQUARE_BLACKPAWN, currsquare);
			currsquare--;
		}
	}

	s = getStringToken(fen, ' ', 2);
	if (s == "b")
	{
		Turn = COLOR_BLACK;
	}
	else
	{
		Turn = COLOR_WHITE;
	}

	s = getStringToken(fen, ' ', 3);
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			Castling[i][j] = 0;
		}
	}
	if (s != "-")
	{
		for (int i = 0; i < s.size(); i++)
		{
			if (s.at(i) == 'K')
			{
				Castling[0][0] = 1;
			}
			if (s.at(i) == 'Q')
			{
				Castling[0][1] = 1;
			}
			if (s.at(i) == 'k')
			{
				Castling[1][0] = 1;
			}
			if (s.at(i) == 'q')
			{
				Castling[1][1] = 1;
			}
		}
	}

	s = getStringToken(fen, ' ', 4);
	if (s != "-")
	{
		EPSquare = Sq2Int(s);
	}
	else
	{
		EPSquare = 0;
	}

	initializeBitsets();
}

std::string getStringToken(std::string str, char delimiter, int token)
{
	int x = 1;
	std::string s = "";
	for (int i = 0; i<str.size(); i++)
	{
		if (str.at(i) == delimiter && i != 0 && str.at(i - 1) != delimiter)
		{
			x++;
		}
		else if (x == token && i == str.size() - 1)
		{
			s += str.at(i);
			return s;
		}
		else if (x>token)
		{
			return s;
		}
		else if (x == token)
		{
			s += str.at(i);
		}
		else if (i == str.size() - 1 && x<token)
		{
			return "";
		}
	}
	return "";
}

int getStringTokenPosition(std::string str, char delimiter, int token)
{
	int x = 1;
	for (int i = 0; i<str.size(); i++)
	{
		if (str.at(i) == delimiter)
		{
			x++;
		}
		if (x == token)
		{
			return i + 1;
		}
		else if (i == str.size() - 1 && x<token)
		{
			return -1;
		}
	}
	return -1;
}


