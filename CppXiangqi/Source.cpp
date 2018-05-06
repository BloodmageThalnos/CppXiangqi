#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <Windows.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <set>
#include <string>

using namespace std;

const int __CHESSNUM[]={0,2,2,2,2,2,5,1};
const char __CHESSNAM[2][8][3]={"��","܇","�R","��","��","��","��","��",
								"��","��","��","��","ʿ","��","��","˧",};

struct Node;

struct Mana{
	char px, py;												// λ��
	char kind;													// [0��1��][001-111������ʿ���˧]
	char flag;													// [0��1��][1����]
	int moves;													// ��һ��int��ѹ�����浱ǰ���ӿ��е������ƶ�

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
	int score;													// ����
	//char lastmove[2];											// ����ƶ�����

	// ��ȡ(x,y)λ���ϵ�����
	Mana& get(char x, char y){
		return ~bd[x][y]?ch[0][bd[x][y]]:noMana; 
	}

	// �����̱�ɿ���ʱ��ģ��
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
		for(int i=0; i<90; i++)									// �������
			bd[0][i]=-1;
		for(int i=0; i<32; i++)									// ��������
			bd[ch[0][i].px][ch[0][i].py]=i;
		for(int i=0; i<32; i++)
			calcMove(ch[0][i]);									// �����ƶ�
	}

	// �ƶ�һ���ӣ�����ǰ��ӵ�һ���ƶ�����newchess��ָ����ɵ�����
	void move(char sx, char sy, char ex, char ey, char newchess=0){
		get(ex, ey).eated();									// ���Ե���
		if(newchess){											// ���ӱ�����
			get(sx, sy).kind=newchess|8;
			hid[get(sx,sy).isred()][get(sx, sy).shape()]--;
			tothid[get(sx, sy).isred()]--;
		}

		lastmove[get(ex, ey).isred()]=bd[sx][sy];				// ����ƶ�����
		bd[ex][ey]=bd[sx][sy];									// �ı�����ָ��
		bd[sx][sy]=-1;

		for(int side=0; side<2; side++)for(int i=0; i<16; i++)
			if(!ch[side][i].eaten())
				updateMove(sx, sy, ex, ey, ch[side][i]);


		// ���³��������Ϣ
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
			for(char x=ch.x()-1; x>=0; x--)if(get(x,ch.y()).shape()){
				if(get(x,ch.y()).isred()^side)ch.addmove(x);
				break;
			}
			else ch.addmove(x);
			for(char x=ch.x()+1; x<=8; x++)if(get(x,ch.y()).shape()){
				if(get(x,ch.y()).isred()^side)ch.addmove(x);
				break;
			}
			else ch.addmove(x);
			for(char y=ch.y()-1; y>=0; y--)if(get(ch.x(),y).shape()){
				if(get(ch.x(),y).isred()^side)ch.addmove(y+16);
				break;
			}
			else ch.addmove(y+16);
			for(char y=ch.y()+1; y<=9; y++)if(get(ch.x(),y).shape()){
				if(get(ch.x(),y).isred()^side)ch.addmove(y+16);
				break;
			}
			else ch.addmove(y+16);
		}break;
		case 2:													// ��
		{

		}break;
		case 3:													// ��
		{

		}break;
		case 4:													// ʿ
		{

		}break;
		case 5:													// ��
		{

		}break;
		case 6:													// ��
		{

		}break;
		case 7:													// ˧
		{

		}break;
		}
	}

	int getScore(){
		return rand()%10000;
	}

	void doIt(char sx, char sy, char ex, char ey, vector<Node*>& v);

	void perfMove(int side, vector<Node*>& v);

	char getRandomHidden(char side){
		// assert(tothid[side]==0);
		int temp=rand()%tothid[side];
		for(char t=1;; t++)if((temp-=hid[side][t])<0)
			return t;											
	}
}_;

struct Node{
	State s;
	char sx, sy, ex, ey;
	vector<Node*> v;
	bool operator< (const Node& b) const{
		return s.score<b.s.score;
	}
};

void State::doIt(char sx, char sy, char ex, char ey, vector<Node*>& v){
	Node* temp = new Node{*this,sx,sy,ex,ey};
	temp->s.move(sx, sy, ex, ey);
	v.push_back(temp);
}

void State::perfMove(int side, vector<Node*>& v){
	for(int i=0; i<16; i++)if(!ch[side][i].eaten()){
		Mana& m=ch[side][i];
		switch(m.shape()){
		case 1:{
			for(char x=0; x<9; x++)if(m.moves&(1<<x))doIt(m.x(), m.y(), x, m.y(), v);
			for(char y=0; y<10; y++)if(m.moves&(1<<y+16))doIt(m.x(), m.y(), m.x(), y, v);
		}break;
		default:assert(true);
		}
	}
}


void wide_dfs(Node& root, int side, int depth){
	if(depth==0){
		root.s.score=root.s.getScore();									// ���㵱ǰ����÷�
		return;
	}
	vector<Node*>& Vec = root.v;
	root.s.perfMove(side, Vec);
	for(auto n:Vec){
		if(!n->s.get(n->ex,n->ey).shown()){
			State temp = n->s;
			int totscore = 0;
			for(int i=1; i<8; i++)if(n->s.hid[side][i]){
				temp.get(n->ex,n->ey).kind = 8+i;
				Node* add = new Node{temp,n->sx,n->sy,n->ex,n->ey};
				n->v.push_back(add);
				wide_dfs(*add, !side, depth-1);
				totscore+=add->s.score*n->s.hid[side][i];
			}
			root.s.score=totscore/n->s.tothid[side];
		}
		else{
			wide_dfs(*n, !side, depth-1);
			root.s.score=max(root.s.score, -n->s.score);
		}
	}
}

void showState(State s){
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xF0);
	for(int j=9; j>=0; j--){
		for(int i=0; i<=8; i++){
			if(s.get(i, j).isred())
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xFC);
			if(!s.get(i, j).shape())
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

int main(){
	freopen("Input.in", "r", stdin);
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
			cin>>sx>>sy>>ex>>ey>>newchess;
			_.move(sx, sy, ex, ey, newchess);
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
				
			}
		}
	}
}