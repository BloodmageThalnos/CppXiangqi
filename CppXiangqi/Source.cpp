#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <Windows.h>
#include <time.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>
#include <set>

using namespace std;

const int __CHESSNUM[]={0,2,2,2,2,2,5,1};
const char __CHESSNAM[2][8][3]={"暗","車","馬","炮","仕","象","卒","将",
								"　","车","马","炮","士","相","兵","帅",
								};

inline int __COUNT_B1(int t){
	// 计算t中二进制下1的数量。
	register int r=0;
	for(; t; t&=t-1)r++;
	return r;
}

char abs(char c){ return c>=0?c:-c; }

struct Node;
struct Mana;
struct State;

struct Mana{
	char px, py;												// 位置
	char kind;													// [0暗1明][001-111车马炮士相兵帅]
	char flag;													// [0黑1红][1被吃]
	int moves;													// 用一个int来压缩保存当前棋子可行移动

	char x(){ return px; }
	char y(){ return py; }
	bool isred(){ return flag&2; }
	char shape(){ return kind&7; }
	bool shown(){ return kind&8; }
	bool eaten(){ return flag&1; }
	void eated(){
		flag|=1;
		moves=0;
	}
	void addmove(char t){
		moves |= (1<<t);
	}
	void remmove(char t){
		moves -= (moves&(1<<t));
	}
	void remmove(char t1, char t2){
		moves -= (moves&((1<<t1)|(1<<t2)));
	}
	void setmove(char t, bool x){
		moves = (moves | (1<<t)) ^ (!x<<t);
	}
	bool hasmove(char t){
		return moves&(1<<t);
	}
}noMana;

struct State{
	char bd[9][10];												// 场面
	Mana ch[2][16];												// 32颗棋子
	char hid[2][8];												// 每方剩余暗子数量
	char tothid[2];												// 每方暗子总量
	// int score;												// 分数

	// 获取(x,y)位置上的棋子
	Mana& get(char x, char y){
		return ~bd[x][y]?ch[0][bd[x][y]]:noMana; 
	}

	// 检查(x,y)位置是否有棋子，等价于get(x,y).shape()，效率较高
	bool has(char x, char y) const{
		return ~bd[x][y];
	}

	// 将棋盘变成开局时的模样
	void init(){
		ch[1][0]=Mana{0,0,1,2};
		ch[1][1]=Mana{8,0,1,2};
		ch[1][2]=Mana{1,0,2,2};
		ch[1][3]=Mana{7,0,2,2};
		ch[1][4]=Mana{2,0,5,2};
		ch[1][5]=Mana{6,0,5,2};
		ch[1][6]=Mana{3,0,4,2};
		ch[1][7]=Mana{5,0,4,2};
		ch[1][8]=Mana{4,0,15,2};
		ch[1][9]=Mana{1,2,3,2};
		ch[1][10]=Mana{7,2,3,2};
		ch[1][11]=Mana{0,3,6,2};
		ch[1][12]=Mana{2,3,6,2};
		ch[1][13]=Mana{4,3,6,2};
		ch[1][14]=Mana{6,3,6,2};
		ch[1][15]=Mana{8,3,6,2};
		ch[0][0]=Mana{0,9,1,0};
		ch[0][1]=Mana{8,9,1,0};
		ch[0][2]=Mana{1,9,2,0};
		ch[0][3]=Mana{7,9,2,0};
		ch[0][4]=Mana{2,9,5,0};
		ch[0][5]=Mana{6,9,5,0};
		ch[0][6]=Mana{3,9,4,0};
		ch[0][7]=Mana{5,9,4,0};
		ch[0][8]=Mana{4,9,15,0};
		ch[0][9]=Mana{1,7,3,0};
		ch[0][10]=Mana{7,7,3,0};
		ch[0][11]=Mana{0,6,6,0};
		ch[0][12]=Mana{2,6,6,0};
		ch[0][13]=Mana{4,6,6,0};
		ch[0][14]=Mana{6,6,6,0};
		ch[0][15]=Mana{8,6,6,0};
		for(int i=0; i<90; i++)									// 场面清空
			bd[0][i]=-1;
		for(int i=0; i<32; i++)									// 摆上棋子
			bd[ch[0][i].px][ch[0][i].py]=i;
		for(int i=0; i<32; i++)
			calcMove(ch[0][i]);									// 计算移动
		for(int i=0; i<2; i++){
			hid[i][1]=hid[i][2]=hid[i][3]=hid[i][4]=hid[i][5]=2;
			hid[i][6]=5;
			tothid[i]=15;
		}
	}

	// 移动一颗子，如果是暗子第一次移动，在newchess中指定变成的明子
	void move(char sx, char sy, char ex, char ey){
		bool test=has(ex, ey);
		if(test){
			get(ex, ey).eated();								// 被吃的子
		}

		get(sx, sy).px=ex;
		get(sx, sy).py=ey;										// 修改棋子坐标

		bd[ex][ey]=bd[sx][sy];									// 改变棋盘指针
		bd[sx][sy]=-1;

		for(int i=0; i<32; i++)if(!ch[0][i].eaten()){			// 更新棋子移动
				if(&ch[0][i]==&get(ex, ey))
					calcMove(ch[0][i]);
				else{
					(ch[0][i].isred()^get(ex, ey).isred()) ?
						updateMove(sx, sy, ex, ey, ch[0][i]) :
						updateMove2(sx, sy, ex, ey, ch[0][i]);
					//calcMove(ch[0][i]);
				}
			}
	}

	// 翻开一颗子
	void flushOut(char x, char y, char newchess){
		Mana& m=get(x, y);
		if(m.kind>8){
			hid[m.isred()][m.shape()]++;
			tothid[m.isred()]++;
		}
		m.kind=newchess|8;
		assert(hid[m.isred()][newchess]>0);
		assert(tothid[m.isred()]>0);
		hid[m.isred()][newchess]--;
		tothid[m.isred()]--;
	}

	// 更新当前棋子的合法移动
	void updateMove(const char sx, const char sy, const char ex, const char ey, Mana& ch){
		char x=ch.x(), y=ch.y();
		const bool test=true;
		bool side=ch.isred();
		switch(ch.shape()){
		case 6:													// 兵
		{
			if(x==sx && y==(side?sy-1:sy+1)){
				ch.addmove(side);
			}
			else if(x==ex && y==(side?ey-1:ey+1)){
				ch.setmove(side, test);
			}
			if(side?y>=5:y<=4){
				if(y==sy){
					if(x==sx+1)ch.addmove(2);
					if(x==sx-1)ch.addmove(3);
				}
				if(y==ey){
					if(x==ex+1)ch.setmove(2,test);
					if(x==ex-1)ch.setmove(3,test);
				}
			}
		}break;
		case 7:													// 将帅
		{
			if(sx>=3 && sx<=5 && (side?sy<=2:sy>=7)){
				if(sx==x && sy+1==y)ch.addmove(0);
				else if(sx==x && sy-1==y)ch.addmove(1);
				else if(sx+1==x && sy==y)ch.addmove(2);
				else if(sx-1==x && sy==y)ch.addmove(3);
			}
			if(ex>=3 && ex<=5 && (side?ey<=2:ey>=7)){
				if(ex==x && ey+1==y)ch.setmove(0, test);
				else if(ex==x && ey-1==y)ch.setmove(1, test);
				else if(ex+1==x && ey==y)ch.setmove(2, test);
				else if(ex-1==x && ey==y)ch.setmove(3, test);
			}
		}break;
		case 1:													// 车
		{
			if(sy==y || ey==y){
				ch.moves&=0xFFC00;
				for(char nx=x-1; nx>=0; nx--)if(has(nx, y)){
					if(get(nx, y).isred()^side)ch.addmove(nx);
					break;
				}
				else ch.addmove(nx);
				for(char nx=x+1; nx<=8; nx++)if(has(nx, y)){
					if(get(nx, y).isred()^side)ch.addmove(nx);
					break;
				}
				else ch.addmove(nx);
			}
			if(sx==x || ex==x){
				ch.moves&=0x3FF;
				for(char ny=y-1; ny>=0; ny--)if(has(x, ny)){
					if(get(x, ny).isred()^side)ch.addmove(ny+10);
					break;
				}
				else ch.addmove(ny+10);
				for(char ny=y+1; ny<=9; ny++)if(has(x, ny)){
					if(get(x, ny).isred()^side)ch.addmove(ny+10);
					break;
				}
				else ch.addmove(ny+10);
			}
		}break;
		case 2:													// 马
		{
			if(x+1==sx && sx!=8 && y==sy){						// 马腿移开的情况
				if(y!=9 && (!has(x+2, y+1) || (get(x+2, y+1).isred()^side))){
					ch.addmove(1);
				}
				if(y!=0 && (!has(x+2, y-1) || (get(x+2, y-1).isred()^side))){
					ch.addmove(7);
				}
			}
			else if(x-1==sx && sx!=0 && y==sy){
				if(y!=0 && (!has(x-2, y-1) || (get(x-2, y-1).isred()^side))){
					ch.addmove(5);
				}
				if(y!=9 && (!has(x-2, y+1) || (get(x-2, y+1).isred()^side))){
					ch.addmove(3);
				}
			}
			else if(y+1==sy && sy!=9 && x==sx){
				if(x!=0 && (!has(x-1, y+2) || (get(x-1, y+2).isred()^side))){
					ch.addmove(2);
				}
				if(x!=8 && (!has(x+1, y+2) || (get(x+1, y+2).isred()^side))){
					ch.addmove(0);
				}
			}
			else if(y-1==sy && sy!=0 && x==sx){
				if(x!=8 && (!has(x+1, y-2) || (get(x+1, y-2).isred()^side))){
					ch.addmove(6);
				}
				if(x!=0 && (!has(x-1, y-2) || (get(x-1, y-2).isred()^side))){
					ch.addmove(4);
				}
			}
			else if(x+2==sx){
				if(!has(x+1, y)){
					if(y+1==sy)ch.addmove(1);
					else if(y-1==sy)ch.addmove(7);
				}
			}
			else if(y-2==sy){
				if(!has(x, y-1)){
					if(x+1==sx)ch.addmove(6);
					else if(x-1==sx)ch.addmove(4);
				}
			}
			else if(x-2==sx){
				if(!has(x-1, y)){
					if(y-1==sy)ch.addmove(5);
					else if(y+1==sy)ch.addmove(3);
				}
			}
			else if(y+2==sy){
				if(!has(x, y+1)){
					if(x+1==sx)ch.addmove(0);
					else if(x-1==sx)ch.addmove(2);
				}
			}
			if(x+1==ex && ex!=8 && y==ey){						// 马腿被别
				ch.remmove(1, 7);
			}
			else if(x-1==ex && ex!=0 && y==ey){
				ch.remmove(3, 5);
			}
			else if(y+1==ey && ey!=9 && x==ex){
				ch.remmove(0, 2);
			}
			else if(y-1==ey && ey!=0 && x==ex){
				ch.remmove(4, 6);
			}
			else if(x+2==ex){
				if(!has(x+1, y)){
					if(y+1==ey)ch.setmove(1, test);
					else if(y-1==ey)ch.setmove(7, test);
				}
			}
			else if(y-2==ey){
				if(!has(x, y-1)){
					if(x+1==ex)ch.setmove(6, test);
					else if(x-1==ex)ch.setmove(4, test);
				}
			}
			else if(x-2==ex){
				if(!has(x-1, y)){
					if(y-1==ey)ch.setmove(5, test);
					else if(y+1==ey)ch.setmove(3, test);
				}
			}
			else if(y+2==ey){
				if(!has(x, y+1)){
					if(x-1==ex)ch.setmove(2, test);
					else if(x+1==ex)ch.setmove(0, test);
				}
			}
		}break;
		case 3:													// 炮
		{
			if(y==sy || y==ey){
				ch.moves&=0xFFC00;
				for(char x=ch.x()-1; x>=0; x--)if(has(x, y)){
					for(char x2=x-1; x2>=0; x2--)if(has(x2, y)){
						if(get(x2, y).isred()^side)ch.addmove(x2);
						break;
					}
					break;
				}
				else ch.addmove(x);
				for(char x=ch.x()+1; x<=8; x++)if(has(x, y)){
					for(char x2=x+1; x2<=8; x2++)if(has(x2, y)){
						if(get(x2, y).isred()^side)ch.addmove(x2);
						break;
					}
					break;
				}
				else ch.addmove(x);
			}
			if(x==sx || x==ex){
				ch.moves&=0x3FF;
				for(char y=ch.y()-1; y>=0; y--)if(has(x, y)){
					for(char y2=y-1; y2>=0; y2--)if(has(x, y2)){
						if(get(x, y2).isred()^side)ch.addmove(y2+10);
						break;
					}
					break;
				}
				else ch.addmove(y+10);
				for(char y=ch.y()+1; y<=9; y++)if(has(x, y)){
					for(char y2=y+1; y2<=9; y2++)if(has(x, y2)){
						if(get(x, y2).isred()^side)ch.addmove(y2+10);
						break;
					}
					break;
				}
				else ch.addmove(y+10);
			}
		}break;
		case 4:													// 士
		{
			if(ch.shown()){
				if((x==sx+1||x==sx-1)&&(y==sy+1||y==sy-1)){
					ch.addmove((x+1==sx)+(y+1==sy)*2);
				}
				if((x==ex+1||x==ex-1)&&(y==ey+1||y==ey-1)){
					ch.setmove((x+1==ex)+(y+1==ey)*2, test);
				}
			}
			else{
				if(sx==4 && sy==(ch.isred()?1:8)){
					ch.addmove(side*2+(ch.x()==3));
				}
				else if(ex==4 && ey==(ch.isred()?1:8)){
					ch.setmove(side*2+(ch.x()==3), test);
				}
			}
		}break;
		case 5:													// 相
		{
			if(	((x==sx-1||x==sx+1) && (y==sy-1||y==sy+1))		// 相眼
				|| ((x==sx-2||x==sx+2) && (y==sy-2||y==sy+2))
				|| ((x==ex-1||x==ex+1) && (y==ey-1||y==ey+1))
				|| ((x==ex-2||x==ex+2) && (y==ey-2||y==ey+2))){
				ch.moves=0;
				if(y>1){
					if(x>1 && !has(x-1, y-1) && (!has(x-2, y-2) || (get(x-2, y-2).isred()^side))){
						ch.addmove(0);
					}
					if(x<8 && !has(x+1, y-1) && (!has(x+2, y-2) || (get(x+2, y-2).isred()^side))){
						ch.addmove(1);
					}
				}
				if(y<8){
					if(x>1 && !has(x-1, y+1) && (!has(x-2, y+2) || (get(x-2, y+2).isred()^side))){
						ch.addmove(2);
					}
					if(x<8 && !has(x+1, y+1) && (!has(x+2, y+2) || (get(x+2, y+2).isred()^side))){
						ch.addmove(3);
					}
				}
			}
		}break;
		}
	}
	void updateMove2(const char sx, const char sy, const char ex, const char ey, Mana& ch){
		char x=ch.x(), y=ch.y();
		const bool test=false;
		bool side=ch.isred();
		switch(ch.shape()){
		case 6:													// 兵
		{
			if(x==sx && y==(side?sy-1:sy+1)){
				ch.addmove(side);
			}
			else if(x==ex && y==(side?ey-1:ey+1)){
				ch.setmove(side, test);
			}
			if(side?y>=5:y<=4){
				if(y==sy){
					if(x==sx+1)ch.addmove(2);
					if(x==sx-1)ch.addmove(3);
				}
				if(y==ey){
					if(x==ex+1)ch.setmove(2, test);
					if(x==ex-1)ch.setmove(3, test);
				}
			}
		}break;
		case 7:													// 将帅
		{
			if(sx>=3 && sx<=5 && (side?sy<=2:sy>=7)){
				if(sx==x && sy+1==y)ch.addmove(0);
				else if(sx==x && sy-1==y)ch.addmove(1);
				else if(sx+1==x && sy==y)ch.addmove(2);
				else if(sx-1==x && sy==y)ch.addmove(3);
			}
			if(ex>=3 && ex<=5 && (side?ey<=2:ey>=7)){
				if(ex==x && ey+1==y)ch.setmove(0, test);
				else if(ex==x && ey-1==y)ch.setmove(1, test);
				else if(ex+1==x && ey==y)ch.setmove(2, test);
				else if(ex-1==x && ey==y)ch.setmove(3, test);
			}
		}break;
		case 1:													// 车
		{
			if(sy==y || ey==y){
				ch.moves&=0xFFC00;
				for(char nx=x-1; nx>=0; nx--)if(has(nx, y)){
					if(get(nx, y).isred()^side)ch.addmove(nx);
					break;
				}
				else ch.addmove(nx);
				for(char nx=x+1; nx<=8; nx++)if(has(nx, y)){
					if(get(nx, y).isred()^side)ch.addmove(nx);
					break;
				}
				else ch.addmove(nx);
			}
			if(sx==x || ex==x){
				ch.moves&=0x3FF;
				for(char ny=y-1; ny>=0; ny--)if(has(x, ny)){
					if(get(x, ny).isred()^side)ch.addmove(ny+10);
					break;
				}
				else ch.addmove(ny+10);
				for(char ny=y+1; ny<=9; ny++)if(has(x, ny)){
					if(get(x, ny).isred()^side)ch.addmove(ny+10);
					break;
				}
				else ch.addmove(ny+10);
			}
		}break;
		case 2:													// 马
		{
			if(x+1==sx && sx!=8 && y==sy){						// 马腿移开的情况
				if(y!=9 && (!has(x+2, y+1) || (get(x+2, y+1).isred()^side))){
					ch.addmove(1);
				}
				if(y!=0 && (!has(x+2, y-1) || (get(x+2, y-1).isred()^side))){
					ch.addmove(7);
				}
			}
			else if(x-1==sx && sx!=0 && y==sy){
				if(y!=0 && (!has(x-2, y-1) || (get(x-2, y-1).isred()^side))){
					ch.addmove(5);
				}
				if(y!=9 && (!has(x-2, y+1) || (get(x-2, y+1).isred()^side))){
					ch.addmove(3);
				}
			}
			else if(y+1==sy && sy!=9 && x==sx){
				if(x!=0 && (!has(x-1, y+2) || (get(x-1, y+2).isred()^side))){
					ch.addmove(2);
				}
				if(x!=8 && (!has(x+1, y+2) || (get(x+1, y+2).isred()^side))){
					ch.addmove(0);
				}
			}
			else if(y-1==sy && sy!=0 && x==sx){
				if(x!=8 && (!has(x+1, y-2) || (get(x+1, y-2).isred()^side))){
					ch.addmove(6);
				}
				if(x!=0 && (!has(x-1, y-2) || (get(x-1, y-2).isred()^side))){
					ch.addmove(4);
				}
			}
			else if(x+2==sx){
				if(!has(x+1, y)){
					if(y+1==sy)ch.addmove(1);
					else if(y-1==sy)ch.addmove(7);
				}
			}
			else if(y-2==sy){
				if(!has(x, y-1)){
					if(x+1==sx)ch.addmove(6);
					else if(x-1==sx)ch.addmove(4);
				}
			}
			else if(x-2==sx){
				if(!has(x-1, y)){
					if(y-1==sy)ch.addmove(5);
					else if(y+1==sy)ch.addmove(3);
				}
			}
			else if(y+2==sy){
				if(!has(x, y+1)){
					if(x+1==sx)ch.addmove(0);
					else if(x-1==sx)ch.addmove(2);
				}
			}
			if(x+1==ex && ex!=8 && y==ey){						// 马腿被别
				ch.remmove(1, 7);
			}
			else if(x-1==ex && ex!=0 && y==ey){
				ch.remmove(3, 5);
			}
			else if(y+1==ey && ey!=9 && x==ex){
				ch.remmove(0, 2);
			}
			else if(y-1==ey && ey!=0 && x==ex){
				ch.remmove(4, 6);
			}
			else if(x+2==ex){
				if(!has(x+1, y)){
					if(y+1==ey)ch.setmove(1, test);
					else if(y-1==ey)ch.setmove(7, test);
				}
			}
			else if(y-2==ey){
				if(!has(x, y-1)){
					if(x+1==ex)ch.setmove(6, test);
					else if(x-1==ex)ch.setmove(4, test);
				}
			}
			else if(x-2==ex){
				if(!has(x-1, y)){
					if(y-1==ey)ch.setmove(5, test);
					else if(y+1==ey)ch.setmove(3, test);
				}
			}
			else if(y+2==ey){
				if(!has(x, y+1)){
					if(x-1==ex)ch.setmove(2, test);
					else if(x+1==ex)ch.setmove(0, test);
				}
			}
		}break;
		case 3:													// 炮
		{
			if(y==sy || y==ey){
				ch.moves&=0xFFC00;
				for(char x=ch.x()-1; x>=0; x--)if(has(x, y)){
					for(char x2=x-1; x2>=0; x2--)if(has(x2, y)){
						if(get(x2, y).isred()^side)ch.addmove(x2);
						break;
					}
					break;
				}
				else ch.addmove(x);
				for(char x=ch.x()+1; x<=8; x++)if(has(x, y)){
					for(char x2=x+1; x2<=8; x2++)if(has(x2, y)){
						if(get(x2, y).isred()^side)ch.addmove(x2);
						break;
					}
					break;
				}
				else ch.addmove(x);
			}
			if(x==sx || x==ex){
				ch.moves&=0x3FF;
				for(char y=ch.y()-1; y>=0; y--)if(has(x, y)){
					for(char y2=y-1; y2>=0; y2--)if(has(x, y2)){
						if(get(x, y2).isred()^side)ch.addmove(y2+10);
						break;
					}
					break;
				}
				else ch.addmove(y+10);
				for(char y=ch.y()+1; y<=9; y++)if(has(x, y)){
					for(char y2=y+1; y2<=9; y2++)if(has(x, y2)){
						if(get(x, y2).isred()^side)ch.addmove(y2+10);
						break;
					}
					break;
				}
				else ch.addmove(y+10);
			}
		}break;
		case 4:													// 士
		{
			if(ch.shown()){
				if((x==sx+1||x==sx-1)&&(y==sy+1||y==sy-1)){
					ch.addmove((x+1==sx)+(y+1==sy)*2);
				}
				if((x==ex+1||x==ex-1)&&(y==ey+1||y==ey-1)){
					ch.setmove((x+1==ex)+(y+1==ey)*2, test);
				}
			}
			else{
				if(sx==4 && sy==(ch.isred()?1:8)){
					ch.addmove(side*2+(ch.x()==3));
				}
				else if(ex==4 && ey==(ch.isred()?1:8)){
					ch.setmove(side*2+(ch.x()==3), test);
				}
			}
		}break;
		case 5:													// 相
		{
			if(((x==sx-1||x==sx+1) && (y==sy-1||y==sy+1))		// 相眼
				|| ((x==sx-2||x==sx+2) && (y==sy-2||y==sy+2))
				|| ((x==ex-1||x==ex+1) && (y==ey-1||y==ey+1))
				|| ((x==ex-2||x==ex+2) && (y==ey-2||y==ey+2))){
				ch.moves=0;
				if(y>1){
					if(x>1 && !has(x-1, y-1) && (!has(x-2, y-2) || (get(x-2, y-2).isred()^side))){
						ch.addmove(0);
					}
					if(x<8 && !has(x+1, y-1) && (!has(x+2, y-2) || (get(x+2, y-2).isred()^side))){
						ch.addmove(1);
					}
				}
				if(y<8){
					if(x>1 && !has(x-1, y+1) && (!has(x-2, y+2) || (get(x-2, y+2).isred()^side))){
						ch.addmove(2);
					}
					if(x<8 && !has(x+1, y+1) && (!has(x+2, y+2) || (get(x+2, y+2).isred()^side))){
						ch.addmove(3);
					}
				}
			}
		}break;
		}
	}

	// 计算当前棋子的合法移动
	void calcMove(Mana& ch){
		ch.moves=0;
		bool side = ch.isred();
		switch(ch.shape()){
		case 1:													// 车
		{
			for(char x=ch.x()-1; x>=0; x--)if(has(x,ch.y())){
				if(get(x,ch.y()).isred()^side)ch.addmove(x);
				break;
			}
			else ch.addmove(x);
			for(char x=ch.x()+1; x<=8; x++)if(has(x,ch.y())){
				if(get(x,ch.y()).isred()^side)ch.addmove(x);
				break;
			}
			else ch.addmove(x);
			for(char y=ch.y()-1; y>=0; y--)if(has(ch.x(),y)){
				if(get(ch.x(),y).isred()^side)ch.addmove(y+10);
				break;
			}
			else ch.addmove(y+10);
			for(char y=ch.y()+1; y<=9; y++)if(has(ch.x(),y)){
				if(get(ch.x(),y).isred()^side)ch.addmove(y+10);
				break;
			}
			else ch.addmove(y+10);
		}break;
		case 2:													// 马
		{
			char x=ch.x(), y=ch.y();
			if(x<=6 && !has(x+1, y)){
				if(y!=9 && (!has(x+2, y+1) || (get(x+2, y+1).isred()^side))){
					ch.addmove(1);
				}
				if(y!=0 && (!has(x+2, y-1) || (get(x+2, y-1).isred()^side))){
					ch.addmove(7);
				}
			}
			if(y>=2 && !has(x, y-1)){
				if(x!=8 && (!has(x+1, y-2) || (get(x+1, y-2).isred()^side))){
					ch.addmove(6);
				}
				if(x!=0 && (!has(x-1, y-2) || (get(x-1, y-2).isred()^side))){
					ch.addmove(4);
				}
			}
			if(x>=2 && !has(x-1, y)){
				if(y!=0 && (!has(x-2, y-1) || (get(x-2, y-1).isred()^side))){
					ch.addmove(5);
				}
				if(y!=9 && (!has(x-2, y+1) || (get(x-2, y+1).isred()^side))){
					ch.addmove(3);
				}
			}
			if(y<=7 && !has(x, y+1)){
				if(x!=0 && (!has(x-1, y+2) || (get(x-1, y+2).isred()^side))){
					ch.addmove(2);
				}
				if(x!=8 && (!has(x+1, y+2) || (get(x+1, y+2).isred()^side))){
					ch.addmove(0);
				}
			}
		}break;
		case 3:													// 炮
		{
			char x=ch.x(), y=ch.y();
			for(char x=ch.x()-1; x>=0; x--)if(has(x, y)){
				for(char x2=x-1; x2>=0; x2--)if(has(x2, y)){
					if(get(x2, y).isred()^side)ch.addmove(x2);
					break;
				}
				break;
			}
			else ch.addmove(x);
			for(char x=ch.x()+1; x<=8; x++)if(has(x, y)){
				for(char x2=x+1; x2<=8; x2++)if(has(x2, y)){
					if(get(x2, y).isred()^side)ch.addmove(x2);
					break;
				}
				break;
			}
			else ch.addmove(x);
			for(char y=ch.y()-1; y>=0; y--)if(has(x, y)){
				for(char y2=y-1; y2>=0; y2--)if(has(x, y2)){
					if(get(x, y2).isred()^side)ch.addmove(y2+10);
					break;
				}
				break;
			}
			else ch.addmove(y+10);
			for(char y=ch.y()+1; y<=9; y++)if(has(x, y)){
				for(char y2=y+1; y2<=9; y2++)if(has(x, y2)){
					if(get(x, y2).isred()^side)ch.addmove(y2+10);
					break;
				}
				break;
			}
			else ch.addmove(y+10);
		}break;
		case 4:													// 士
		{
			if(ch.shown()){
				char x=ch.x(), y=ch.y();
				if(y!=0){
					if(x!=0 && (!has(x-1, y-1) || (get(x-1, y-1).isred()^side))){
						ch.addmove(0);
					}
					if(x!=8 && (!has(x+1, y-1) || (get(x+1, y-1).isred()^side))){
						ch.addmove(1);
					}
				}
				if(y!=9){
					if(x!=0 && (!has(x-1, y+1) || (get(x-1, y+1).isred()^side))){
						ch.addmove(2);
					}
					if(x!=8 && (!has(x+1, y+1) || (get(x+1, y+1).isred()^side))){
						ch.addmove(3);
					}
				}		//shujuku zuomixing de buding.
			}
			else{
				char y=ch.isred()?1:8, x=4;
				if(!has(x, y) || (get(x, y).isred()^side))
					ch.addmove(side*2+(ch.x()==3));
			}
		}break;
		case 5:													// 相
		{
			char x=ch.x(), y=ch.y();
			if(y>1){
				if(x>1 && !has(x-1, y-1) && (!has(x-2, y-2) || (get(x-2, y-2).isred()^side))){
					ch.addmove(0);
				}
				if(x<8 && !has(x+1, y-1) && (!has(x+2, y-2) || (get(x+2, y-2).isred()^side))){
					ch.addmove(1);
				}
			}
			if(y<8){
				if(x>1 && !has(x-1, y+1) && (!has(x-2, y+2) || (get(x-2, y+2).isred()^side))){
					ch.addmove(2);
				}
				if(x<8 && !has(x+1, y+1) && (!has(x+2, y+2) || (get(x+2, y+2).isred()^side))){
					ch.addmove(3);
				}
			}
		}break;
		case 6:													// 兵
		{
			char x=ch.x(), y=ch.y();
			if(side?y>=5:y<=4){
				if(x && (!has(x-1, y) || (get(x-1, y).isred()^side))){
					ch.addmove(2);
				}
				if(x!=8 && (!has(x+1, y) || (get(x+1, y).isred()^side))){
					ch.addmove(3);
				}
			}
			if(side){
				if(y!=9 && (!has(x, y+1) || !get(x, y+1).isred())){
					ch.addmove(1);
				}
			}
			else{
				if(y && (!has(x, y-1) || get(x, y-1).isred())){
					ch.addmove(0);
				}
			}
		}break;
		case 7:													// 将帅
		{
			char x=ch.x(), y=ch.y();
			if((side?y:y!=7) && (!has(x, y-1) || (get(x, y-1).isred()^side))){
				ch.addmove(0);
			}
			if((side?y!=2:y!=9) && (!has(x, y+1) || (get(x, y+1).isred()^side))){
				ch.addmove(1);
			}
			if(x!=3 && (!has(x-1, y) || (get(x-1, y).isred()^side))){
				ch.addmove(2);
			}
			if(x!=5 && (!has(x+1, y) || (get(x+1, y).isred()^side))){
				ch.addmove(3);
			}
		}break;
		}
	}

	// 计算当前场面的评价得分
	int getScore(bool side) const{
		// 只算一边
		const static int sc[]={0,120,60,60,50,30,20,50000};
		const static int pw[]={0,6,4,5,4,3,4,2};
		int ret=0;
		
		int mid=0;
		for(int i=1; i<8; i++)mid+=sc[i]*hid[side][i];
		mid/=tothid[side];
		for(int i=0; i<16; i++){
			Mana m=ch[side][i];
			if(m.eaten()){
				if(m.shown())
					ret-=sc[m.shape()];	// 如果被吃掉的子，就减去该子的子力
				else ret-=mid;			// 如果吃的是暗子，就减去平均子力
			}
			else{
				ret+=sc[m.shape()];		// 由于明暗有别，追加上该子的子力
				ret+=__COUNT_B1(m.moves)*pw[m.shape()];
										// 该子对棋盘的控制能力。
			}
		}
		return ret;
	}

	void doIt(char sx, char sy, char ex, char ey, vector<Node>& v);

	void perfMove(int side, vector<Node>& v);

	char getRandomHidden(char side){
		// assert(tothid[side]!=0);
		int temp=rand()%tothid[side];
		for(char t=1;; t++)if((temp-=hid[side][t])<0)
			return t;											
	}
};

struct Node{
	// Node 表示搜索时的一个节点，构成一棵搜索树。
	//State s;								// 该节点对应的场面。太占内存删除之。
	char sx, sy, ex, ey;					// 记录此节点的上一步。我觉得记录在state中更好。
	//vector<Node*> v;						// 孩子们
	//Node* to;								// 大儿子
	int score;								// 此节点的评分
	bool operator<(const Node& b) const;
	void show() const{
		printf("(%d,%d)->(%d,%d) %d\n", sx, sy, ex, ey, score);
	}
	short hash() const{
		return sx+sy*9+ex*90+ey*810;
	}
}__;

int MAGIC_SCORE[8100];
const int inf=100000;

bool Node::operator< (const Node& b) const{	// 用于sort
	return MAGIC_SCORE[hash()]>MAGIC_SCORE[b.hash()];	
											// 注意符号，倒序排序
}

void updateMagic(const Node& b){
	MAGIC_SCORE[b.hash()]=MAGIC_SCORE[b.hash()]*0.7 + b.score;
}

State _;

string showMove(Node& n){
	stringstream s;
	s<<"("<<(int)n.sx<<","<<(int)n.sy<<")->("<<(int)n.ex<<","<<(int)n.ey<<") ";
	return s.str();
}

void showState(State s){
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xF0);
	for(int j=9; j>=0; j--){
		for(int i=0; i<=8; i++){
			if(s.get(i, j).isred())
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xFC);
			if(!s.has(i, j))
				printf(__CHESSNAM[0][8]);
			else if(!s.get(i, j).shown())
				printf(__CHESSNAM[0][0]);
			else
				printf(__CHESSNAM[s.get(i, j).isred()][s.get(i, j).shape()]);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xF0);
			if(i<8)putchar(' ');
		}
		puts("");
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}

void State::doIt(char sx, char sy, char ex, char ey, vector<Node>& v){
	v.push_back(Node{sx,sy,ex,ey});
}

void State::perfMove(int side, vector<Node>& v){
	for(int i=0; i<16; i++)if(!ch[side][i].eaten()){
		Mana& m=ch[side][i];
		switch(m.shape()){
		case 6:case 7:{
			if(m.moves&1)doIt(m.x(), m.y(), m.x(), m.y()-1, v);
			if(m.moves&2)doIt(m.x(), m.y(), m.x(), m.y()+1, v);
			if(m.moves&4)doIt(m.x(), m.y(), m.x()-1, m.y(), v);
			if(m.moves&8)doIt(m.x(), m.y(), m.x()+1, m.y(), v);
		}break;
		case 1:case 3:{
			for(char x=0; x<9; x++)if(m.moves&(1<<x))doIt(m.x(), m.y(), x, m.y(), v);
			for(char y=0; y<10; y++)if(m.moves&(1<<y+10))doIt(m.x(), m.y(), m.x(), y, v);
		}break;
		case 2:{
			if(m.moves&1)doIt(m.x(), m.y(), m.x()+1, m.y()+2, v);
			if(m.moves&2)doIt(m.x(), m.y(), m.x()+2, m.y()+1, v);
			if(m.moves&4)doIt(m.x(), m.y(), m.x()-1, m.y()+2, v);
			if(m.moves&8)doIt(m.x(), m.y(), m.x()-2, m.y()+1, v);
			if(m.moves&16)doIt(m.x(), m.y(), m.x()-1, m.y()-2, v);
			if(m.moves&32)doIt(m.x(), m.y(), m.x()-2, m.y()-1, v);
			if(m.moves&64)doIt(m.x(), m.y(), m.x()+1, m.y()-2, v);
			if(m.moves&128)doIt(m.x(), m.y(), m.x()+2, m.y()-1, v);
		}break;
		case 4:{
			if(m.moves&1)doIt(m.x(), m.y(), m.x()-1, m.y()-1, v);
			if(m.moves&2)doIt(m.x(), m.y(), m.x()+1, m.y()-1, v);
			if(m.moves&4)doIt(m.x(), m.y(), m.x()-1, m.y()+1, v);
			if(m.moves&8)doIt(m.x(), m.y(), m.x()+1, m.y()+1, v);
		}break;
		case 5:{
			if(m.moves&1)doIt(m.x(), m.y(), m.x()-2, m.y()-2, v);
			if(m.moves&2)doIt(m.x(), m.y(), m.x()+2, m.y()-2, v);
			if(m.moves&4)doIt(m.x(), m.y(), m.x()-2, m.y()+2, v);
			if(m.moves&8)doIt(m.x(), m.y(), m.x()+2, m.y()+2, v);
		}break;
		default:;
		}
	}
}

void old_wide_dfs(Node& root, State& s, bool side, int depth){
	if(depth==0){
		//showState(s);
		root.score=s.getScore(side)-s.getScore(!side);	// 计算当前场面得分
		//printf("Score = %d.\n", root.s.score);
		return;
	}
	vector<Node> Vec;
	Node* b;
	s.perfMove(side, Vec);
	root.score=-0xfffff;
	for(auto& n:Vec){
		State temp=s;
		temp.move(n.sx, n.sy, n.ex, n.ey);
		if(!temp.get(n.ex, n.ey).shown()){
			int totscore = 0;
			for(int i=1; i<7; i++)if(s.hid[side][i]){
				temp.flushOut(n.ex, n.ey, i);
				temp.calcMove(temp.get(n.ex, n.ey));
				old_wide_dfs(n, temp, !side, depth-1);
				totscore+=n.score*s.hid[side][i];
			}
			n.score=totscore/s.tothid[side];
		}
		else{
			old_wide_dfs(n, temp, !side, depth-1);
		}
		if(root.score+n.score<0){
			root.score=-n.score;
			b=&n;
		}
	}
	if(depth>=3)b->show();
}

void wide_dfs(Node& root, State& s, bool side, int depth, int alpha, int beta){
	if(depth==0){
		//showState(s);
		root.score=s.getScore(side)-s.getScore(!side);	// 计算当前场面得分
		//printf("Score = %d.\n", root.score);
		return;
	}

	vector<Node> Vec;
	s.perfMove(side, Vec);
	//sort(Vec.begin(), Vec.end());
	Node* b;

	root.score=-0xfffff;

	for(auto& n:Vec){
		State temp=s;
		temp.move(n.sx, n.sy, n.ex, n.ey);
		if(!temp.get(n.ex, n.ey).shown()){
			int totscore = 0;
			for(int i=1; i<7; i++)if(s.hid[side][i]){
				temp.flushOut(n.ex, n.ey, i);
				temp.calcMove(temp.get(n.ex, n.ey));
				wide_dfs(n, temp, !side, depth-1, -beta, inf);
				totscore+=n.score*s.hid[side][i];
			}
			n.score=totscore/s.tothid[side];
		}
		else{
			wide_dfs(n, temp, !side, depth-1, -beta, -alpha);
		}

		int now=-n.score;
		if(now>root.score){
			root.score=now;
			b=&n; 
			if(now>alpha){
				alpha=now;
				if(now>=beta)break;
			}
		}
	}
	if(depth>=3)b->show();
}

void show_result(Node& root){
}

int main(){
	//freopen("Input.in", "r", stdin);
	string command;
	while(cin>>command){
		if(command=="start"){
			_.init();
		}
		else if(command=="show"){
			showState(_);
		}
		else if(command=="move"){
			int sx, sy, ex, ey, newchess;
			cin>>sx>>sy>>ex>>ey;
			_.move(sx, sy, ex, ey);
			if(!_.get(ex, ey).shown()){
				cin>>newchess;
				if(!newchess)newchess=_.getRandomHidden(_.get(ex,ey).isred());
				_.flushOut(ex, ey, newchess);
				_.calcMove(_.get(ex, ey));
			}
		}
		else if(command=="compute"){
			int side;
			cin>>command;
			if(command=="red") side=1;
			else if(command=="black") side=0;

			time_t i=clock();
			old_wide_dfs(__, _, side, 3);
			printf("old_wide cost %d ms\n", (int)(clock()-i));
			show_result(__);
			i=clock();
			wide_dfs(__, _, side, 3, -inf, inf);
			printf("wide cost %d ms\n", (int)(clock()-i));
		}
		else if(command=="debug"){
			cin>>command;
			if(command=="mana"){
				for(int i=0; i<32; i++){
					Mana& ti=_.ch[0][i];
					if(ti.eaten())continue;
					printf("%s %s%s： (%d.%d)\n\t", ti.isred()?"红":"黑", ti.shown()?"明":"暗", __CHESSNAM[ti.isred()][ti.shape()], ti.x(), ti.y());
					for(int i=0; i<32; i++){
						printf(ti.moves&(1<<i)?"1":"0");
						if(i%8==7)putchar(' ');
					}
					puts("");
				}
				for(int i=0; i<32; i++){
					Mana& ti=_.ch[0][i];
					if(!ti.eaten())continue;
					printf("（被吃）%s %s%s\n", ti.isred()?"红":"黑", ti.shown()?"明":"暗", __CHESSNAM[ti.isred()][ti.shape()]);
				}
			}
			else if(command=="dfs"){
				//debug_dfs(__);
			}
		}
	}
}


// 将帅不能照面还没做
