#pragma once

#include "Position.h"

struct PGNGame
{
	std::vector<Move> Moves;
	float Result;

	PGNGame() : Result(-1) {}
	PGNGame(float res) : Result(res) {}
};

class PGNData
{
public:
	std::vector<PGNGame> Games;

	PGNData();
	PGNData(std::string file);
	PGNData(std::string file, size_t num_games);
	~PGNData();

	void loadFromFile(std::string file);
	void loadFromFile(std::string file, size_t num_games); //loads 'num_games' games from the file
	void printData();
};

std::string convertMoveNotation(std::string move, Position& pos);

