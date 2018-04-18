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
// calculate best move for RED/BLACK
void aiDoIt(char side);

struct Moves{
	char startx, starty;
	char endx, endy;
	char newchess;
	// convert from input scale to data scale
	int check();
	~Moves(){
		//if(next)delete next;
	}
};

// a kind of blueviolet hexagen crystal
struct Mana{
	// chess kind:
	// 0-2bit: 001-111 Ë§-Ïà 000 Empty
	//  3 bit: 0 °µ  1 Ã÷
	//  4 bit: 0 Red 1 Black
	//  5 bit: 1 out_of_board
	char kind;
	char posx, posy;
	~Mana(){
		//if(head)delete head;
	}
};

struct State{
	// the board
	Mana* board[14][13];
	// number of hidden chess, 0R1B
	char hid[2][8];
	// total hidden, 0R1B
	int tothid[2];
	// 32 chesses
	Mana na[2][16];
	// captured by one side
	char capture[2][14][13];

	// inline function
	char get(char x, char y){ return board[y][x]->kind; }
	// print onto the screen
	void show();
	// put the chesses by rule when the game starts
	void init();
	// update movement
	void move(Moves m);
	// get a random hidden char
	char getRandom(char);
	// find out possible moves
	void calcMoves(list<Moves>&, char);
	// find out the score of nowstate
	int getScore(char);
	// solve some p
	void deepCopy();
};