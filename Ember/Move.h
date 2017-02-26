#pragma once

#include "MagicBitboards.h"

class Move
{
public:
	Bitset data;

	Move();
	Move(Bitset n);
	Move(Bitset f, Bitset t, Bitset m, Bitset c, Bitset s, Bitset wk, Bitset wq, Bitset bk, Bitset bq, Bitset ep);

	Move operator=(Move const& m);
	bool operator==(Move const& m) const;
	bool operator!=(Move const& m) const;
	bool isEquivalent(Move const& m) const; //checks whether to, from and special are equal
	bool operator<(Move const& m) const;
	bool operator>(Move const& m) const;
	Bitset getbit() const;
	Bitset getbit90() const;
	Bitset getbit45() const;
	Bitset getbit135() const;
	Bitset getFrom() const; //int type
	Bitset getTo() const; //int type
	Bitset getMovingPiece() const; //Piece type
	Bitset getSpecial() const; //Piece type
	Bitset getCapturedPiece() const; //Square type
	Bitset getWKC() const;
	Bitset getWQC() const;
	Bitset getBKC() const;
	Bitset getBQC() const;
	Bitset getEP() const;
	Bitset getRelevant() const; //gets all bits except casting and ep bits
	std::string toString() const;
	bool isNullMove() const;
};
