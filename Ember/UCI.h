#pragma once

#include "Engine.h"

class UCI
{
public:
	Engine Ember;

	UCI();
	~UCI();

	void run();
	void run_uci();
	void display(int flip);
	void makeMove(std::string s);
	void unmakeMove();
	void getMoves();
	void info();
};

