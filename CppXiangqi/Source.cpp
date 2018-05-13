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
const char __CHESSNAM[2][8][3]={"��","܇","�R","��","��","��","��","��",
								"��","��","��","��","ʿ","��","��","˧",
								};

inline int __COUNT_B1(int t){
	// ����t�ж�������1��������
	register int r=0;
	for(; t; t&=t-1)r++;
	return r;
}

struct Node;
struct Mana;
struct State;

struct Mana{
	char px, py;												// λ��
	char kind;													// [0��1��][001-111������ʿ���˧]
	char flag;													// [0��1��][1����]
	int moves;													// ��һ��int��ѹ�����浱ǰ���ӿ����ƶ�

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
	char bd[9][10];												// ����
	Mana ch[2][16];												// 32������
	char hid[2][8];												// ÿ��ʣ�వ������
	char tothid[2];												// ÿ����������
	//int score;												// ����

	// ��ȡ(x,y)λ���ϵ�����
	Mana& get(char x, char y){
		return ~bd[x][y]?ch[0][bd[x][y]]:noMana; 
	}

	// ���(x,y)λ���Ƿ������ӣ��ȼ���get(x,y).shape()��Ч�ʽϸ�
	bool has(char x, char y) const{
		return ~bd[x][y];
	}

	// �����̱�ɿ���ʱ��ģ��
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
		for(int i=0; i<90; i++)									// �������
			bd[0][i]=-1;
		for(int i=0; i<32; i++)									// ��������
			bd[ch[0][i].px][ch[0][i].py]=i;
		for(int i=0; i<32; i++)
			calcMove(ch[0][i]);									// �����ƶ�
		for(int i=0; i<2; i++){
			hid[i][1]=hid[i][2]=hid[i][3]=hid[i][4]=hid[i][5]=2;
			hid[i][6]=5;
			tothid[i]=15;
		}
	}

	// �ƶ�һ���ӣ�����ǰ��ӵ�һ���ƶ�����newchess��ָ����ɵ�����
	void move(char sx, char sy, char ex, char ey){
		get(ex, ey).eated();									// ���Ե���
		get(sx, sy).px=ex;
		get(sx, sy).py=ey;										// �޸���������

		bd[ex][ey]=bd[sx][sy];									// �ı�����ָ��
		bd[sx][sy]=-1;

		for(int side=0; side<2; side++)for(int i=0; i<16; i++)
			if(!ch[side][i].eaten())
				updateMove(sx, sy, ex, ey, ch[side][i]);

		// ���³��������Ϣ
	}

	// ����һ����
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

	// ���µ�ǰ���ӵĺϷ��ƶ�
	void updateMove(char sx, char sy, char ex, char ey, Mana& ch){
		calcMove(ch);
	}

	// ���㵱ǰ���ӵĺϷ��ƶ�
	void calcMove(Mana& ch){
		ch.moves=0;
		bool side = ch.isred();
		switch(ch.shape()){
		case 1:													// ��
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
		case 2:													// ��
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
		case 3:													// ��
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
		case 4:													// ʿ
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
				}
			}
			else{
				char x=ch.isred()?1:8, y=4;
				if(!has(x, y) || (get(x, y).isred()^side))
					ch.addmove(side+(ch.x()==5)*2);
			}
		}break;
		case 5:													// ��
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
		case 6:													// ��
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
				if(y!=9 && (!has(x, y+1) || (get(x, y+1).isred()^side))){
					ch.addmove(1);
				}
			}
			else{
				if(y && (!has(x, y-1) || (get(x, y-1).isred()^side))){
					ch.addmove(0);
				}
			}
		}break;
		case 7:													// ��˧
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

	// ���㵱ǰ��������۵÷�
	int getScore(bool side) const{
		// ֻ��һ��
		const static int sc[]={0,120,60,600,50,30,20,50000};
		const static int pw[]={0,6,4,5,4,3,4,2};
		int ret=0;
		
		int mid=0;
		for(int i=1; i<8; i++)mid+=sc[i]*hid[side][i];
		mid/=tothid[side];
		for(int i=0; i<16; i++){
			Mana m=ch[side][i];
			if(m.eaten()){
				if(m.shown())
					ret-=sc[m.shape()];	// ������Ե����ӣ��ͼ�ȥ���ӵ�����
				else ret-=mid;			// ����Ե��ǰ��ӣ��ͼ�ȥƽ������
			}
			else{
				ret+=sc[m.shape()];		// ���������б�׷���ϸ��ӵ�����
				ret+=__COUNT_B1(m.moves)*pw[m.shape()];
										// ���Ӷ����̵Ŀ���������
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
	// Node ��ʾ����ʱ��һ���ڵ㣬����һ����������
	//State s;								// �ýڵ��Ӧ�ĳ��档̫ռ�ڴ�ɾ��֮��
	char sx, sy, ex, ey;					// ��¼�˽ڵ����һ�����Ҿ��ü�¼��state�и��á�
	//vector<Node*> v;						// ������
	//Node* to;								// �����
	int score;								// �˽ڵ������
	bool operator<(const Node& b) const;
	void show() const{
		printf("(%d,%d)->(%d,%d) %d\n", sx, sy, ex, ey, score);
	}
	short hash() const{
		return sx+sy*9+ex*90+ey*810;
	}
}__;

int MAGIC_SCORE[8100];

bool Node::operator< (const Node& b) const{	// ����sort
	return MAGIC_SCORE[hash()]>MAGIC_SCORE[b.hash()];	
											// ע����ţ���������
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
		root.score=s.getScore(side)-s.getScore(!side);	// ���㵱ǰ����÷�
														//printf("Score = %d.\n", root.s.score);
		return;
	}
	vector<Node> Vec;
	Node* b;
	s.perfMove(side, Vec);
	root.score=-0xfffff;
	for(auto n:Vec){
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
		root.score=s.getScore(side)-s.getScore(!side);	// ���㵱ǰ����÷�
		//printf("Score = %d.\n", root.score);
		return;
	}

	vector<Node> Vec;
	s.perfMove(side, Vec);
	sort(Vec.begin(), Vec.end());
	Node* b=&Vec[], &best=*b;							// �ҵ���õ���һ��

	State temp=s;
	temp.move(best.sx, best.sy, best.ex, best.ey);
	if(!temp.get(best.ex, best.ey).shown()){
		int totscore = 0;
		for(int i=1; i<7; i++)if(s.hid[side][i]){
			temp.flushOut(best.ex, best.ey, i);
			temp.calcMove(temp.get(best.ex, best.ey));
			wide_dfs(best, temp, !side, depth-1, -beta, -alpha);
			totscore+=best.score*s.hid[side][i];
		}
		best.score=totscore/s.tothid[side];
	}
	else{
		wide_dfs(best, temp, !side, depth-1, -beta, -alpha);
	}
	root.score=-best.score;

	for(auto& n:Vec)if((&n)!=(&best)){					// ������������
		State temp=s;
		temp.move(n.sx, n.sy, n.ex, n.ey);
		if(!temp.get(n.ex, n.ey).shown()){
			int totscore = 0;
			for(int i=1; i<7; i++)if(s.hid[side][i]){
				temp.flushOut(n.ex, n.ey, i);
				temp.calcMove(temp.get(n.ex, n.ey));
				wide_dfs(n, temp, !side, depth-1, -alpha-1, -alpha);
				totscore+=n.score*s.hid[side][i];
			}
			n.score=totscore/s.tothid[side];
		}
		else{
			wide_dfs(n, temp, !side, depth-1, -alpha-1, -alpha);
		}

		int now=-n.score;
		if(now>alpha && now<beta){
			if(!temp.get(n.ex, n.ey).shown()){
				int totscore = 0;
				for(int i=1; i<7; i++)if(s.hid[side][i]){
					temp.flushOut(n.ex, n.ey, i);
					temp.calcMove(temp.get(n.ex, n.ey));
					wide_dfs(n, temp, !side, depth-1, -beta, -alpha);
					totscore+=n.score*s.hid[side][i];
				}
				n.score=totscore/s.tothid[side];
			}
			else{
				wide_dfs(n, temp, !side, depth-1, -beta, -alpha);
			}
			now=-n.score;
		}

		if(now>=root.score){
			root.score=now;
			b=&n;
			if(now>=alpha){
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
			}
		}
		else if(command=="compute"){
			int side;
			cin>>command;
			if(command=="red") side=1;
			else if(command=="black") side=0;

			time_t i=clock();
			wide_dfs(__, _, side, 4, -100000, 100000);
			printf("cost %d ms\n", (int)(clock()-i));
			i=clock();
			//old_wide_dfs(__, _, side, 3);
			printf("cost %d ms\n", (int)(clock()-i));
			show_result(__);
		}
		else if(command=="debug"){
			cin>>command;
			if(command=="mana"){
				for(int i=0; i<32; i++){
					Mana& ti=_.ch[0][i];
					if(ti.eaten())continue;
					printf("%s %s%s�� (%d.%d)\n\t", ti.isred()?"��":"��", ti.shown()?"��":"��", __CHESSNAM[ti.isred()][ti.shape()], ti.x(), ti.y());
					for(int i=0; i<32; i++){
						printf(ti.moves&(1<<i)?"1":"0");
						if(i%8==7)putchar(' ');
					}
					puts("");
				}
				for(int i=0; i<32; i++){
					Mana& ti=_.ch[0][i];
					if(!ti.eaten())continue;
					printf("�����ԣ�%s %s%s\n", ti.isred()?"��":"��", ti.shown()?"��":"��", __CHESSNAM[ti.isred()][ti.shape()]);
				}
			}
			else if(command=="dfs"){
				//debug_dfs(__);
			}
		}
	}
}


// ��˧�������滹û��
