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

struct Node;
struct Mana;
struct State;

struct Mana{
	char px, py;												// 位置
	char kind;													// [0暗1明][001-111车马炮士相兵帅]
	char flag;													// [0黑1红][1被吃]
	int moves;													// 用一个int来压缩保存当前棋子可行的所有移动

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
}noMana;

struct State{
	char bd[9][10];												// 场面
	Mana ch[2][16];												// 32颗棋子
	char hid[2][8];												// 每方剩余暗子数量
	char tothid[2];												// 每方暗子总量
	int score;													// 分数

	// 获取(x,y)位置上的棋子
	Mana& get(char x, char y){
		return ~bd[x][y]?ch[0][bd[x][y]]:noMana; 
	}

	// 检查(x,y)位置是否有棋子，等价于get(x,y).shape()，效率较高
	bool has(char x, char y){
		return ~bd[x][y];
	}

	// 将棋盘变成开局时的模样
	void init(){
		ch[0][0]=Mana{0,0,1,2};
		ch[0][1]=Mana{8,0,1,2};
		ch[0][2]=Mana{1,0,2,2};
		ch[0][3]=Mana{7,0,2,2};
		ch[0][4]=Mana{2,0,5,2};
		ch[0][5]=Mana{6,0,5,2};
		ch[0][6]=Mana{3,0,4,2};
		ch[0][7]=Mana{5,0,4,2};
		ch[0][8]=Mana{4,0,15,2};
		ch[0][9]=Mana{1,2,3,2};
		ch[0][10]=Mana{7,2,3,2};
		ch[0][11]=Mana{0,3,6,2};
		ch[0][12]=Mana{2,3,6,2};
		ch[0][13]=Mana{4,3,6,2};
		ch[0][14]=Mana{6,3,6,2};
		ch[0][15]=Mana{8,3,6,2};
		ch[1][0]=Mana{0,9,1,0};
		ch[1][1]=Mana{8,9,1,0};
		ch[1][2]=Mana{1,9,2,0};
		ch[1][3]=Mana{7,9,2,0};
		ch[1][4]=Mana{2,9,5,0};
		ch[1][5]=Mana{6,9,5,0};
		ch[1][6]=Mana{3,9,4,0};
		ch[1][7]=Mana{5,9,4,0};
		ch[1][8]=Mana{4,9,15,0};
		ch[1][9]=Mana{1,7,3,0};
		ch[1][10]=Mana{7,7,3,0};
		ch[1][11]=Mana{0,6,6,0};
		ch[1][12]=Mana{2,6,6,0};
		ch[1][13]=Mana{4,6,6,0};
		ch[1][14]=Mana{6,6,6,0};
		ch[1][15]=Mana{8,6,6,0};
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
		get(ex, ey).eated();									// 被吃的子
		get(sx, sy).px=ex;
		get(sx, sy).py=ey;										// 修改棋子坐标

		bd[ex][ey]=bd[sx][sy];									// 改变棋盘指针
		bd[sx][sy]=-1;

		for(int side=0; side<2; side++)for(int i=0; i<16; i++)
			if(!ch[side][i].eaten())
				updateMove(sx, sy, ex, ey, ch[side][i]);

		// 更新场面相关信息
	}

	void flushOut(char sx, char sy, char newchess){
		get(sx, sy).kind=newchess|8;
		hid[get(sx, sy).isred()][get(sx, sy).shape()]--;
		tothid[get(sx, sy).isred()]--;
	}

	// 更新当前棋子的合法移动
	void updateMove(char sx, char sy, char ex, char ey, Mana& ch){
		calcMove(ch);
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
			if(y<=6 && !has(x, y+1)){
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

		}break;
		case 7:													// 帅
		{

		}break;
		}
	}

	// 计算当前场面的评价得分
	int getScore(bool side){
		// 只算一边
		const static int sc[]={0,100,60,60,50,30,20,50000};
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
				ret+=__COUNT_B1(m.moves)*pw[m.shape()];
										// 否则计算该子对棋盘的控制能力。
			}
		}
		return ret;
	}

	void doIt(char sx, char sy, char ex, char ey, vector<Node*>& v);

	void perfMove(int side, vector<Node*>& v);

	char getRandomHidden(char side){
		// assert(tothid[side]!=0);
		int temp=rand()%tothid[side];
		for(char t=1;; t++)if((temp-=hid[side][t])<0)
			return t;											
	}
};

struct Node{
	// Node 表示搜索时的一个节点，构成一棵搜索树。
	State s;								// 该节点对应的场面
	char sx, sy, ex, ey;					// 记录此节点的上一步。我觉得记录在state中更好。
	vector<Node*> v;						// 孩子们
	Node* to;								// 大儿子
	bool operator< (const Node& b) const{	// 用于sort
		return s.score<b.s.score;
	}
}__;

State& _=__.s;

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

void State::doIt(char sx, char sy, char ex, char ey, vector<Node*>& v){
	Node* temp = new Node{*this,sx,sy,ex,ey};
	temp->s.move(sx, sy, ex, ey);
	v.push_back(temp);
}

void State::perfMove(int side, vector<Node*>& v){
	for(int i=0; i<16; i++)if(!ch[side][i].eaten()){
		Mana& m=ch[side][i];
		switch(m.shape()){
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

void wide_dfs(Node& root, bool side, int depth){
	void showState(State s);
	if(depth==0){
		root.s.score=root.s.getScore(side)-root.s.getScore(!side);	// 计算当前场面得分
		//printf("Score = %d.\n", root.s.score);
		return;
	}
	auto& Vec = root.v;
	root.s.perfMove(side, Vec);
	root.s.score=-0x3f3f3f3f;
	for(auto n:Vec){
		if(!n->s.get(n->ex,n->ey).shown()){
			//State temp = n->s;
			int totscore = 0;
			for(int i=1; i<8; i++)if(n->s.hid[side][i]){
				Node* add = new Node{n->s,n->sx,n->sy,n->ex,n->ey};
				add->s.flushOut(n->ex, n->ey, i);
				add->s.calcMove(n->s.get(n->ex, n->ey));
				n->v.push_back(add);
				wide_dfs(*add, !side, depth-1);
				totscore+=add->s.score*n->s.hid[side][i];
			}
			n->s.score=totscore/n->s.tothid[side];
			n->to=(Node*)-1;
		}
		else{
			wide_dfs(*n, !side, depth-1);
		}
		if(root.s.score+n->s.score<0){
			root.s.score=-n->s.score;
			root.to=n;
		}
	}
}

void show_dfs(Node& root, int showd, int depth=0, string s=""){
	if(depth<showd){

	}
	else{
		Node* a=&root;
		for(; a->v.size(); a=a->to){
			if(!~(int)a->to){
			//if(!a->s.get(a->ex,a->ey).shown()){
				for(auto b:a->v){
					show_dfs(*b, 0, 0, s+showMove(*b)+"暗"+__CHESSNAM[a->s.get(a->ex, a->ey).isred()][a->s.get(a->ex, a->ey).shape()]+"->"+__CHESSNAM[b->s.get(b->ex, b->ey).isred()][b->s.get(b->ex, b->ey).shape()]+"  ");
				}
				return;
			}
			else if(a->sx||a->sy||a->ex||a->ey)
				s+=showMove(*a)+__CHESSNAM[a->s.get(a->ex, a->ey).isred()][a->s.get(a->ex, a->ey).shape()]+"  ";
		}
		cout<<s<<"得分："<<a->s.score<<endl;
	}
}

void show_result(Node& root){
	printf("(%d,%d)->(%d,%d) %d\n", root.to->sx, root.to->sy, root.to->ex, root.to->ey, root.to->s.score);
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
			}
		}
		else if(command=="compute"){
			cin>>command;
			if(command=="red"){
				time_t i=clock();
				wide_dfs(__, 0, 2);
				printf("%d\n", (int)(clock()-i));
			}
			show_result(__);
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