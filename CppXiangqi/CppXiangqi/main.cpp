#include "main.h"

// show startpage here or pretend as if you have a startpage.
// return non-zero to continue, zero to quit
int showStartPage(){
	puts("Hello!");
	system("pause");
	return true;
}

// main game process
void Game(){
	if( !showStartPage() )return;
	initEverything();
	while(true){
		showPanel();
		switch(read()){
		// move a chess
		case 1:{
			puts("Input startx,starty,endx,endy,newstate.");
			char sx=read(), sy=read(), ex=read(), ey=read(), nw=read();
			if(!moveAChess(Moves{sx, sy, ex, ey, nw}))goto dataerror;
		}	break;
		case 2:{
			redoLastmove();
		}	break;
		case 3:{

		}	break;
		case 4:{

		}	break;
		default:dataerror:
			puts("Operation error. You may try again.\n");
			fflush(stdin);
			system("pause");
			continue;
		}
	}
}

int main(){
	Game();
	return 0;
}