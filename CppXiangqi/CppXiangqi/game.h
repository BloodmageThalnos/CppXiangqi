#pragma once
#include "main.h"

struct State;
struct Moves;

void initEverything();
void showPanel();
// move a chess, return success or not
bool moveAChess(Moves);
void redoLastmove();

struct State{
	// chess on each position, mapped in one degree
	// chess kind:
	// 0-2bit: 001-111 Ë§-Ïà 000 Empty
	//  3 bit: 0 °µ  1 Ã÷
	//  4 bit: 0 Red 1 Black
	// -1 bit: 1 out_of_board
	char board[182];
	// number of hidden chess, 0R1B
	char hid[2][8];
	// total hidden, 0R1B
	int tothid[2];
	vector<Moves> allmove;

	// inline function
	char get(int x, int y){ return board[x+13*y]; }
	// print onto the screen
	void show();
	// put the chesses by rule when the game starts
	void init();
	// update movement
	void move(Moves m);
	// get a random hidden char
	int getRandom(int);
};

struct Moves{
	int startx, starty;
	int endx, endy;
	int newchess;
	// convert from input scale to data scale
	int format_and_check();
};