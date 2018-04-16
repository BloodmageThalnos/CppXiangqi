#pragma once
#include "main.h"

struct State;
struct Moves;
struct Mana;

void initEverything();
void showPanel();
// move a chess, return success or not
bool moveAChess(Moves);
// return to last move, can be done several times
void redoLastmove();

struct Moves{
	char startx, starty;
	char endx, endy;
	char newchess;
	// convert from input scale to data scale
	int format_and_check();
};

// a kind of blueviolet hexagen crystal
struct Mana{
	// chess kind:
	// 0-2bit: 001-111 ˧-�� 000 Empty
	//  3 bit: 0 ��  1 ��
	//  4 bit: 0 Red 1 Black
	//  5 bit: 1 out_of_board
	char kind;
	char posx, posy;
	vector<Moves>Mov;
	Mana() :Mov(){}
};

struct State{
	// the board
	Mana* board[182];
	// number of hidden chess, 0R1B
	char hid[2][8];
	// total hidden, 0R1B
	int tothid[2];
	// 32 chesses
	Mana na[2][16];

	// inline function
	char get(char x, char y){ return board[x+13*y]->kind; }
	// print onto the screen
	void show();
	// put the chesses by rule when the game starts
	void init();
	// find out possible moves
	void calcMoves();
	// update movement
	void move(Moves m);
	// get a random hidden char
	char getRandom(char);
};