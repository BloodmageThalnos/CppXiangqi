#include "game.h"

State _;
deque<State> deque_;
Mana noMana, outMana;

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
	outMana.kind=32;
	int t[188]={0};
	t[55]=t[61]=19;
	t[28]=t[36]=21;
	t[29]=t[35]=18;
	t[30]=t[34]=23;
	t[31]=t[33]=22;
	t[32]=25;
	for(int i=1; i<=9; i+=2)t[i+66]=20, t[105+i]=4;
	t[120]=t[126]=3;
	t[145]=t[153]=5;
	t[146]=t[152]=2;
	t[147]=t[151]=7;
	t[148]=t[150]=6;
	t[149]=9;
	for(int i=0, cnt[2]={0}; i<182; i++)if(t[i]){
		Mana& m = na[!!(t[i]&16)][cnt[!!(t[i]&16)]++];
		m.posx=i%13;
		m.posy=i/13;
		m.kind=t[i];
		board[i]=&m;
	}
	else{
		if(i%13<2 || i%13>10 || i/13<2 || i/13>10)board[i]=&outMana;
		else board[i]=&noMana;
	}
	for(int i=0; i<2; i++)
		hid[i][2]=hid[i][3]=hid[i][5]=hid[i][6]=2, hid[i][4]=5;
	tothid[0]=tothid[1]=13;
}

char State::getRandom(char side){
	assert(tothid[side]);
	char r = rand()%tothid[side]+1, choosed;
	for(choosed = 2; r>0; choosed++){
		r-=hid[side][choosed];
	}
	return 8|(side<<4)|choosed;
}

void State::move(Moves m){
	int startp = m.startx + 13*m.starty, endp = m.endx + 13*m.endy;
	board[endp]->kind = 0;	// eaten
	if(~board[startp]->kind&8)	// hidden
		board[startp]->kind = m.newchess;
	board[startp]->posx=m.endx;
	board[startp]->posy=m.endy;
	board[endp]=board[startp];
	board[startp]=&noMana;	// empty
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