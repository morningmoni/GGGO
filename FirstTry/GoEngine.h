#pragma once
#include "uctNode.h"
#include <time.h>
#include "GoBoard.h"
#include "constants.h"
#include <windows.h>

class GoEngine {
public:
	GoBoard * go_board;
	
	int games;
	int rivalMovei, rivalMovej;
	uctNode *root;
	clock_t fin_clock;
	CRITICAL_SECTION cs;


	GoEngine(GoBoard *b);
	uctNode* treePolicy(GoBoard * temp_board);
	void uctSearch(int *pos, int color, int *moves, int num_moves);
	static unsigned  __stdcall ThreadFunc(void * p);// originally static
	int POS(int i, int  j) { return ((i)* go_board->board_size + (j)); }
	int  I(int pos) {return ((pos) / go_board->board_size);}
	int  J(int pos) { return ((pos) % go_board->board_size); }
	uctNode* expand(uctNode* curNode, int* moves, int num_moves);
	uctNode* bestchild(uctNode* curNode, int c);
	bool cmpLess(const uctNode *a, const uctNode *b){return a->score < b->score;}
	bool cmpMore(const uctNode* a, const uctNode *b){return a->score > b->score;}

	void calScore(uctNode* tmp, int c);
	int defaultPolicy(GoBoard* temp,int color);

	void backup(uctNode* v, int reward);
	void init_GGGO();
	void generate_move(int *i, int *j, int color);
	void aiMove(int *pos, int color, int *moves, int num_moves);
	void place_free_handicap(int handicap);
};