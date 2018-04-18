#include "game.h"

State _;
deque<State> deque_;
Mana noMana, outMana;

// for simplicity, it is static now, but should be dynamic.
const int score_Chess[16]={0,0,30,60,30,80,30,30,0,1000,80,90,30,180,60,30};
const char chessName[2][8][3]={"暗","帅","马","炮","兵","车","士","相","暗","将","馬","砲","卒","車","仕","象"};

void State::show(){
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
		board[m.posy][m.posx]=&m;
	}
	else{
		if(i%13<2 || i%13>10 || i/13<2 || i/13>10)board[i/13][i%13]=&outMana;
		else board[i/13][i%13]=&noMana;
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
	tothid[side]--;
	hid[side][choosed]--;
	return 8|(side<<4)|choosed;
}

void State::move(Moves m){
	board[m.endy][m.endx]->posx = 0;		// eaten
	if(~board[m.endy][m.endx]->kind&8)		// hidden
		board[m.starty][m.startx]->kind = m.newchess;
	board[m.starty][m.startx]->posx=m.endx;
	board[m.starty][m.startx]->posy=m.endy;
	board[m.endy][m.endx]=board[m.starty][m.startx];
	board[m.starty][m.startx]=&noMana;		// empty
}

// Huge function, CAUTION!
// find out possible moves
void State::calcMoves(list<Moves>& L, char side){
	memset(capture, 0, sizeof capture);
	for(int i=0; i<16; i++)if(na[side][i].posx){
		Mana& m = na[side][i];
		switch(m.kind&7){
		case 1:{	//shuai
			char c, x=m.posx, y=m.posy;
			c=get(x-1, y);
			if(!c || (c>>4)==!side){	// c is empty, or c is an opponent move
				L.push_back(Moves{x,y,x-1,y,0});
				capture[side][x-1][y]++;
			}
			c=get(x+1, y);
			if(!c || (c>>4)==!side){	// c is empty, or c is an opponent move
				L.push_back(Moves{x,y,x+1,y,0});
				capture[side][x+1][y]++;
			}
			c=get(x, y-1);
			if(!c || (c>>4)==!side){	// c is empty, or c is an opponent move
				L.push_back(Moves{x,y,x,y-1,0});
				capture[side][x][y-1]++;
			}
			c=get(x, y+1);
			if(!c || (c>>4)==!side){	// c is empty, or c is an opponent move
				L.push_back(Moves{x,y,x,y+1,0});
				capture[side][x][y+1]++;
			}
		}break;
		case 2:{	//ma
			char c, x=m.posx, y=m.posy;
			c=get(x-1, y);
			if(!c){
				c=get(x-2, y-1);
				if(!c || (c>>4)==!side){	// c is empty, or c is an opponent move
					L.push_back(Moves{x,y,x-2,y-1,0});
					capture[side][x-2][y-1]++;
				}
				c=get(x-2, y+1);
				if(!c || (c>>4)==!side){	// c is empty, or c is an opponent move
					L.push_back(Moves{x,y,x-2,y+1,0});
					capture[side][x-2][y+1]++;
				}
			}
			c=get(x+1, y);
			if(!c){
				c=get(x+2, y-1);
				if(!c || (c>>4)==!side){	// c is empty, or c is an opponent move
					L.push_back(Moves{x,y,x+2,y-1,0});
					capture[side][x+2][y-1]++;
				}
				c=get(x+2, y+1);
				if(!c || (c>>4)==!side){	// c is empty, or c is an opponent move
					L.push_back(Moves{x,y,x+2,y+1,0});
					capture[side][x+2][y+1]++;
				}
			}
			c=get(x, y-1);
			if(!c){
				c=get(x-1, y-2);
				if(!c || (c>>4)==!side)	// c is empty, or c is an opponent move
					L.push_back(Moves{x,y,x-1,y-2,0});
				c=get(x+1, y-2);
				if(!c || (c>>4)==!side)	// c is empty, or c is an opponent move
					L.push_back(Moves{x,y,x+1,y-2,0});
			}
			c=get(x, y+1);
			if(!c){
				c=get(x-1, y+2);
				if(!c || (c>>4)==!side)	// c is empty, or c is an opponent move
					L.push_back(Moves{x,y,x-1,y+2,0});
				c=get(x+1, y+2);
				if(!c || (c>>4)==!side)	// c is empty, or c is an opponent move
					L.push_back(Moves{x,y,x+1,y+2,0});
			}
		}break;
		case 3:{ // pao
			char c, x, y=m.posy;
			for(x=m.posx-1; ; x--){
				c=get(x, y);
				if(c){// is a chess or out of board
					if(c&7){
						for(x--; !(c=get(x, y)); x--);
						if((c>>4)==!side)	// is a opponent chess
							L.push_back(Moves{m.posx,y,x,y,0});
					}// else out of board
					break;
				}
				else L.push_back(Moves{m.posx,y,x,y,0});
			}
			for(x=m.posx+1; ; x++){
				c=get(x, y);
				if(c){// is a chess or out of board
					if(c&7){
						for(x++; !(c=get(x, y)); x++);
						if((c>>4)==!side)	// is a opponent chess
							L.push_back(Moves{m.posx,y,x,y,0});
					}// else out of board
					break;
				}
				else L.push_back(Moves{m.posx,y,x,y,0});
			}
			x=m.posx;
			for(y=m.posy-1; ; y--){
				c=get(x, y);
				if(c){// is a chess or out of board
					if(c&7){
						for(y--; !(c=get(x, y)); y--);
						if((c>>4)==!side)	// is a opponent chess
							L.push_back(Moves{x,m.posy,x,y,0});
					}// else out of board
					break;
				}
				else L.push_back(Moves{x,m.posy,x,y,0});
			}
			for(y=m.posy+1; ; y++){
				c=get(x, y);
				if(c){// is a chess or out of board
					if(c&7){
						for(y++; !(c=get(x, y)); y++);
						if((c>>4)==!side)	// is a opponent chess
							L.push_back(Moves{x,m.posy,x,y,0});
					}// else out of board
					break;
				}
				else L.push_back(Moves{x,m.posy,x,y,0});
			}
		}break;
		case 4:{	//bin
			char c, x=m.posx, y=m.posy;
			c=get(x, y+side+side-1);
			if(!c || (c>>4)==!side)	// c is empty, or c is an opponent move
				L.push_back(Moves{x,y,x,y+side+side-1,0});
			if((!side && y<=7)||(side && y>=8)){
				c=get(x-1, y);
				if(!c || (c>>4)==!side)	// c is empty, or c is an opponent move
					L.push_back(Moves{x,y,x-1,y,0});
				c=get(x+1, y);
				if(!c || (c>>4)==!side)	// c is empty, or c is an opponent move
					L.push_back(Moves{x,y,x+1,y,0});
			}
		}break;
		case 5:{	//che
			char c, x, y=m.posy;
			for(x=m.posx-1; ; x--){
				c=get(x, y);
				if(c){// is a chess or out of board
					if((c>>4)==!side)	// is a opponent chess
						L.push_back(Moves{m.posx,y,x,y,0});
					break;
				}
				else L.push_back(Moves{m.posx,y,x,y,0});
			}
			for(x=m.posx+1; ; x++){
				c=get(x, y);
				if(c){// is a chess or out of board
					if((c>>4)==!side)	// is a opponent chess
						L.push_back(Moves{m.posx,y,x,y,0});
					break;
				}
				else L.push_back(Moves{m.posx,y,x,y,0});
			}
			x=m.posx;
			for(y=m.posy-1; ; y--){
				c=get(x, y);
				if(c){// is a chess or out of board
					if((c>>4)==!side)	// is a opponent chess
						L.push_back(Moves{x,m.posy,x,y,0});
					break;
				}
				else L.push_back(Moves{x,m.posy,x,y,0});
			}
			for(y=m.posy+1; ; y++){
				c=get(x, y);
				if(c){// is a chess or out of board
					if((c>>4)==!side)	// is a opponent chess
						L.push_back(Moves{x,m.posy,x,y,0});
					break;
				}
				else L.push_back(Moves{x,m.posy,x,y,0});
			}
		}break;
		case 6:{	//shi
			char c, x=m.posx, y=m.posy;
			if(~m.kind&8){			// hidden
				c=get(6, y+side+side-1);
				if(!c || (c>>4)==!side)
					L.push_back(Moves{x,y,6,y+side+side-1,0});
			}
			else{
				c=get(x-1, y-1);
				if(!c || (c>>4)==!side)	// c is empty, or c is an opponent move
					L.push_back(Moves{x,y,x-1,y-1,0});
				c=get(x+1, y-1);
				if(!c || (c>>4)==!side)	// c is empty, or c is an opponent move
					L.push_back(Moves{x,y,x+1,y-1,0});
				c=get(x-1, y+1);
				if(!c || (c>>4)==!side)	// c is empty, or c is an opponent move
					L.push_back(Moves{x,y,x-1,y+1,0});
				c=get(x+1, y+1);
				if(!c || (c>>4)==!side)	// c is empty, or c is an opponent move
					L.push_back(Moves{x,y,x+1,y+1,0});
			}
		}break;
		case 7:{	//xiang
			char c, x=m.posx, y=m.posy;
			if(!get(x-1, y-1)){
				c=get(x-2, y-2);
				if(!c || (c>>4)==!side)	// c is empty, or c is an opponent move
					L.push_back(Moves{x,y,x-2,y-2,0});
			}
			if(!get(x-1, y+1)){
				c=get(x-2, y+2);
				if(!c || (c>>4)==!side)	// c is empty, or c is an opponent move
					L.push_back(Moves{x,y,x-2,y+2,0});
			}
			if(!get(x+1, y-1)){
				c=get(x+2, y-2);
				if(!c || (c>>4)==!side)	// c is empty, or c is an opponent move
					L.push_back(Moves{x,y,x+2,y-2,0});
			}
			if(!get(x+1, y+1)){
				c=get(x+2, y+2);
				if(!c || (c>>4)==!side)	// c is empty, or c is an opponent move
					L.push_back(Moves{x,y,x+2,y+2,0});
			}
		}
		}
	}
}

// find out the score of nowstate, from the red side
int State::getScore(char side){
	int res=0;
	for(int i=0; i<16; i++){
		if(na[side][i].posx){	// exists
			res += score_Chess[na[side][i].kind&15];
		}
		else{					// eaten
			res -= score_Chess[na[side][i].kind&15];
		}
		if(na[!side][i].posx){	// exists
			res -= score_Chess[na[!side][i].kind&15];
		}
		else{					// eaten
			res += score_Chess[na[!side][i].kind&15];
		}
	}
	return res;
}

void State::deepCopy(){
	for(int i=0; i<2; i++)for(int j=0; j<16; j++)if(na[i][j].posx){
		board[na[i][j].posy][na[i][j].posx]=&na[i][j];
	}
}

int Moves::check(){
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
	int moved=m.check();
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

// search for the score of s after performed m
pair<int, Moves> Search(State s, Moves* m, char side, int depth, int alpha, int beta){
	s.deepCopy();
	if(m) s.move(*m);
	if(!depth){
		return make_pair(s.getScore(side),Moves());
	}
	list<Moves> L;
	s.calcMoves(L, side);
	Moves maxmove;
	//for(int i=0; i<16; i++){
		//int k=s.na[side][i].kind;
		for(auto nowm=L.begin(); nowm!=L.end(); nowm++){
			int nowscore=0, k=s.board[nowm->startx][nowm->starty]->kind;
			if(k&8)nowscore=-Search(s, &*nowm, !side, depth-1, -beta, -alpha).first;
			else{
				Moves nowmove = *nowm;
				for(int i=1; i<8; i++)if(s.hid[side][i]){
					nowmove.newchess=(side<<4)|8|i;
					nowscore-=s.hid[side][i]*Search(s, &nowmove, !side, depth-1, -beta, -alpha).first;
				}
				nowscore/=s.tothid[side];
			}
			if(alpha<nowscore){
				if(beta<=nowscore)return make_pair(nowscore, *nowm);
				maxmove=*nowm, alpha=nowscore;
			}
		}
	//}
	return make_pair(alpha,maxmove);
}

void aiDoIt(char side){
	time_t nowt=clock();
	auto result = Search(_, NULL, side, 4, -10000, 10000);
	printf("Cost %d ms.\n", clock()-nowt);
	Moves* nowStep=&(result.second);
	printf("%d (%d,%d)->(%d,%d)\nOperate ?", result.first, nowStep->startx, nowStep->starty, nowStep->endx, nowStep->endy);
	char operation[128];
	gets_s(operation);
	if(operation[0]=='y')moveAChess(*nowStep);
	system("pause");
}