#include "game.h"

State _;
deque<State> deque_;

void State::show(){
	const static char chessName[2][8][3]={"暗","帅","马","炮","兵","车","士","相","暗","将","馬","砲","卒","車","仕","象"};
	puts("1  2  3  4  5  6  7  8  9");
	for(int y=2; y<12; y++){
		setConsoleColor(0, 15);
		for(int x=2; x<11; x++){
			char ch=get(x, y);
			if(!ch)printf("　");
			else{
				setConsoleColor(ch&16?0:0xC, 15);
				if(ch&8)printf(chessName[!!(ch&16)][ch&7]);
				else printf(chessName[0][0]);
			}
			if(x<10)putchar(' ');
		}
		setConsoleColor(7, 0);
		printf("%2d\n", y-1);
	}
	puts("1  2  3  4  5  6  7  8  9");
}

void State::init(){
	memset(board, 0, sizeof board);
	board[55]=board[61]=19;
	board[28]=board[36]=21;
	board[29]=board[35]=18;
	board[30]=board[34]=23;
	board[31]=board[33]=22;
	board[32]=25;
	for(int i=1; i<=9; i+=2)board[i+66]=20, board[105+i]=4;
	board[120]=board[126]=3;
	board[145]=board[153]=5;
	board[146]=board[152]=2;
	board[147]=board[151]=7;
	board[148]=board[150]=6;
	board[149]=9;
	for(int i=0; i<2; i++)
		hid[i][2]=hid[i][3]=hid[i][5]=hid[i][6]=2, hid[i][4]=5;
	tothid[0]=tothid[1]=13;
}

int State::getRandom(int side){
	assert(tothid[side]);
	int r = rand()%tothid[side]+1, choosed;
	for(choosed = 2; r>0; choosed++){
		r-=hid[side][choosed];
	}
	return 8|(side<<4)|choosed;
}

void State::move(Moves m){
	int startp = m.startx + 13*m.starty, endp = m.endx + 13*m.endy, bd = board[startp];
	if(bd&8)board[endp]=bd;
	else {
		board[endp]=m.newchess;
	}
	board[startp]=0;
}

int Moves::format_and_check(){
	startx++, starty++, endx++, endy++;
	return _.get(startx, starty);
}

void initEverything(){
	srand(time(0));

	_.init();
	deque_.push_back(_);
}

void showPanel(){
	system("cls");
	_.show();
	puts("\n\nChoose what to do:");
	puts("\t1.Move a chess");
	puts("\t2.Undo last move");
	puts("\t3.Compute Red");
	puts("\t4.Compute Black.");
}

// move a chess, return success or not
bool moveAChess(Moves m){
	int moved=m.format_and_check();
	if(!moved)return false;
	if((~moved&8)	// is hidden
		&& !m.newchess){
		m.newchess=_.getRandom(!!(moved&16));
	}
	_.move(m);
	deque_.push_back(_);
	return true;
}

void redoLastmove(){
	deque_.pop_back();
	_=deque_.back();
}