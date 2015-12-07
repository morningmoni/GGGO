#pragma once
#include"constants.h"
class GoBoard {
public:
	int * board;
	int * next_stone;
	static int board_size;
	static float komi;
	static int final_status[MAX_BOARD * MAX_BOARD];
	int ko_i;
	int ko_j;
	int step;

	int handicap;
	static int deltai[4];
	static int deltaj[4];

	GoBoard();
	~GoBoard();

	GoBoard * copy_board();
	int  I(int pos);
	int  J(int pos);
	int POS(int i, int  j);
	int get_string(int i, int j, int *stonei, int *stonej);
	void play_move( int i, int j, int color);
	int pass_move(int i, int j);
	int on_board(int i, int j);
	int get_board( int i, int j);
	int board_empty();
	void clear_board();
	int suicide(int i, int j, int color);
	int provides_liberty(int ai, int aj, int i, int j, int color);	//some static functions tranfered to not static
	int has_additional_liberty(int i, int j, int libi, int libj);
	int remove_string(int i, int j);
	int same_string(int pos1, int pos2);
	int legal_move(int i, int j, int color);
	int generate_legal_moves(int* moves, int color);
	int checkLiberty(int i, int j);
	int autoRun(int color);
	bool available(int i, int j, int color);
	void calcGame(int *b, int *w, int *bScore, int *wScore);
	void compute_final_status(void);
	int get_final_status(int i, int j);
	void set_final_status(int i, int j, int status);
	int valid_fixed_handicap(int handicap);
	void place_fixed_handicap(int handicap);
	void set_final_status_string(int pos, int status);
	int findALiberty(int i, int j);
	int suicideLike(int i, int j, int color);
};