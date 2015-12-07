#include "GoEngine.h"
#include <time.h>
#include <stdlib.h>
#include <process.h>
#include <windows.h>
#include "algorithm"
#include<vector>
#include "publicFunc.h"
#include <fstream>
#include "GoBoard.h"
using namespace std;
GoEngine::~GoEngine(){}

GoEngine::GoEngine(GoBoard * b) {
	go_board = b;
}

uctNode* GoEngine::expand(uctNode* curNode, int* moves, int num_moves)
{
	for (int i = 0; i < num_moves; ++i)
	{
		bool flag = true;
		for (int j = 0; j<curNode->nextMove.size(); ++j)
		{
			if (moves[i] == curNode->nextMove[j]->pos)
			{
				flag = false;
				break;
			}
		}
		if (flag)//TODO
		{
			uctNode* nextchosenNode = new uctNode(moves[i], OTHER_COLOR(curNode->color), curNode);
			curNode->addPos(nextchosenNode);
			return nextchosenNode;
		}
	}
	return NULL; //indicates error
}


void GoEngine::calScore(uctNode* tmp, int c)
{
	if (tmp->color == BLACK)
	{
		for (int ii = 0; ii < tmp->nextMove.size(); ++ii)
		{
			uctNode *tt = tmp->nextMove[ii];
			if (tt->play == 0)
			{
				tt->score = 0;
				continue;
			}
			tt->score = (tt->playResult + 0.0) / tt->play - c*sqrt(2 * log(tmp->play) / tt->play);
		}
	}
	else
	{
		for (int ii = 0; ii < tmp->nextMove.size(); ++ii)
		{
			uctNode *tt = tmp->nextMove[ii];
			if (tt->play == 0)
			{
				tt->score = 0;
				continue;
			}
			tt->score = (tt->playResult + 0.0) / tt->play + c*sqrt(2 * log(tmp->play) / tt->play);
		}
	}
}


uctNode* GoEngine::bestchild(uctNode* curNode, int c)
{
	calScore(curNode, c);
	sort(curNode->nextMove.begin(), curNode->nextMove.end(), cmpLess);
	if (curNode->color == BLACK)
		return curNode->nextMove[0];
	else
		return curNode->nextMove[curNode->nextMove.size() - 1];
}

/*uctNode* GoEngine::treePolicy(GoBoard * temp_board)
{
	uctNode* curNode = root;

	int* moves = new int[GoBoard::board_size*GoBoard::board_size]; //available moves
	int num_moves;	//available moves_count
	EnterCriticalSection(&cs);
	temp_board = go_board->copy_board();
	while (curNode->nextMove.size() > 0 || !curNode->lastMove) //while not leaf node, or is root
	{
		if (curNode->pos != POS(rivalMovei, rivalMovej))
		{
			go_board->play_move(I(curNode->pos), J(curNode->pos), curNode->color);
		}
		num_moves = go_board->generate_legal_moves(moves, OTHER_COLOR(curNode->color));
		if (num_moves != curNode->nextMove.size()) //not fully expanded
		{
			uctNode* tmp = expand(curNode, moves, num_moves);
			delete[]moves;
			LeaveCriticalSection(&cs);
			return tmp;
		}
		else
			curNode = bestchild(curNode, 1);
	}
	delete[]moves;
	LeaveCriticalSection(&cs);
	return curNode;
}*/
uctNode* GoEngine::treePolicy(uctNode* v, int games)
{
	uctNode* curNode = v;
	int* moves = new int[MAX_BOARD * MAX_BOARD]; //available moves
	int num_moves;	//available moves_count
	while (curNode->nextMove.size() > 0 || !curNode->lastMove) //while not leaf node, or is root
	{
		if (curNode->pos != POS(rivalMovei, rivalMovej))
		{
			go_board->play_move(I(curNode->pos), J(curNode->pos), curNode->color);
		}
		num_moves = go_board->generate_legal_moves(moves, OTHER_COLOR(curNode->color));
		if (num_moves != curNode->nextMove.size()) //not fully expanded
		{
			uctNode* tmp = expand(curNode, moves, num_moves);
			delete[]moves;
			return tmp;
		}
		else
			curNode = bestchild(curNode, 1);
	}
	delete[]moves;
	return curNode;
}

int GoEngine::defaultPolicy(GoBoard * temp,int color)
{
	return temp->autoRun(color);
}



void GoEngine::backup(uctNode* v, int reward)
{
	v->result(reward);
}


/*unsigned __stdcall GoEngine::ThreadFunc(void * p)
{
	int seed = GetCurrentThreadId()*time(NULL);
	srand(seed);
	int reward = 0;
	GoEngine *engine = (GoEngine *) p;
	while (engine->games < MAXGAMES && clock()-engine->fin_clock> MAXTIME )
	{
		GoBoard * temp_board = NULL;

		uctNode* chosenNode = engine->treePolicy( temp_board);
		if (!chosenNode)
			break;
		temp_board->play_move(engine->I(chosenNode->pos), engine->J(chosenNode->pos), chosenNode->color);
		reward = engine->defaultPolicy(temp_board,OTHER_COLOR(chosenNode->color));
		EnterCriticalSection(&engine->cs);
		engine->backup(chosenNode, reward);
		++engine->games;
		LeaveCriticalSection(&engine->cs);
		delete temp_board;
	}
	return 0;
}*/
void GoEngine::uctSearch(int *pos, int color, int *moves, int num_moves)
{

	srand(time(NULL));
	int * store_board = new int[GoBoard::board_size*GoBoard::board_size];
	int * store_next_stone = new int[GoBoard::board_size*GoBoard::board_size];
	for (int i = 0; i<GoBoard::board_size*GoBoard::board_size; ++i)
	{
		store_board[i] = go_board->board[i];
		store_next_stone[i] = go_board->next_stone[i];
	}
	int storeStep = go_board->step;
	int storeko_i = go_board->ko_i;
	int storeko_j = go_board->ko_j;
	//GoBoard* store = go_board->copy_board();
	int games = 0;
	uctNode* root = new uctNode(POS(rivalMovei, rivalMovej), OTHER_COLOR(color), NULL);
	int reward = 0;

	while (games < MAXGAMES)
	{
		uctNode* chosenNode = treePolicy(root, games);
		if (!chosenNode)
			break;
		go_board->play_move(I(chosenNode->pos), J(chosenNode->pos), chosenNode->color);
		reward = defaultPolicy(go_board,OTHER_COLOR(chosenNode->color));
		backup(chosenNode, reward);
		++games;
		for (int ii = 0; ii<GoBoard::board_size*GoBoard::board_size; ++ii)
		{
			go_board->board[ii] = store_board[ii];
			go_board->next_stone[ii] = store_next_stone[ii];
		}
		go_board->step = storeStep;
		go_board->ko_i = storeko_i;
		go_board->ko_j = storeko_j;
		//go_board = store->copy_board();
	}
	if (root->nextMove.size()>0)
	{
		uctNode* resNode = bestchild(root, 0); //final result
		*pos = resNode->pos;
	}
	else
	{
		*pos = -1;
	}


	delete root;
	//delete store;
	delete[]store_board;
	delete[]store_next_stone;
	if ((*pos) == POS(go_board->ko_i, go_board->ko_j) || !go_board->legal_move(I(*pos), J(*pos), color))
	{
		(*pos) = -1;
	}
}

void GoEngine::aiMove(int *pos, int color, int *moves, int num_moves)
{
	aiMovePreCheck(pos, color, moves, num_moves);
	if (*pos==-1)
		uctSearch(pos, color, moves, num_moves);
}


/* Generate a move. */
void GoEngine::generate_move(int *i, int *j, int color)
{
	int moves[MAX_BOARD * MAX_BOARD];
	int num_moves = 0;
	int ai, aj;
	int k;

	memset(moves, 0, sizeof(moves));
	for (ai = 0; ai < GoBoard::board_size; ai++)
	{
		for (aj = 0; aj < GoBoard::board_size; aj++)
		{
			/* Consider moving at (ai, aj) if it is legal and not suicide. */
			if (go_board->legal_move(ai, aj, color) && !go_board->suicide(ai, aj, color))
			{
				/* Further require the move not to be suicide for the opponent... */
				if (!go_board->suicide(ai, aj, OTHER_COLOR(color)))
					moves[num_moves++] = POS(ai, aj);
				else
				{
					/* ...however, if the move captures at least one stone,
					* consider it anyway.
					*/
					for (k = 0; k < 4; k++)
					{
						int bi = ai + go_board->deltai[k];
						int bj = aj + go_board->deltaj[k];
						if (go_board->on_board(bi, bj) && go_board->get_board(bi, bj) == OTHER_COLOR(color))
						{
							moves[num_moves++] = POS(ai, aj);
							break;
						}
						if (go_board->get_board(bi, bj) && go_board->get_board(bi, bj) == color && go_board->checkLiberty(bi, bj) == 1)
						{
							moves[num_moves++] = POS(ai, aj);
							break;
						}
					}
				}
			}
		}
	}
	/* Choose one of the considered moves randomly with uniform
	* distribution. (Strictly speaking the moves with smaller 1D
	* coordinates tend to have a very slightly higher probability to be
	* chosen, but for all practical purposes we get a uniform
	* distribution.)
	*/
	if (num_moves > 0)
	{
		int temp_pos = -1;
		aiMove(&temp_pos, color, moves, num_moves);
		if (temp_pos == -1)
		{
			(*i) = -1;
			(*j) = -1;
		}
		else
		{
			*i = I(temp_pos);
			*j = J(temp_pos);
		}
	}
	else
	{
		*i = -1;
		*j = -1;
		return;
	}
}


/* Put free placement handicap stones on the board. We do this simply
* by generating successive black moves.
*/
void GoEngine::place_free_handicap(int handicap)
{
	int k;
	int i, j;

	for (k = 0; k < handicap; k++) {
		generate_move(&i, &j, BLACK);
		go_board->play_move(i, j, BLACK);
	}
}

void GoEngine::aiMovePreCheck(int *pos, int color, int *moves, int num_moves)
{
	*pos = -1;
	for (int i = 0; i < GoBoard::board_size; ++i)
	{
		for (int j = 0; j < GoBoard::board_size; ++j)
		{
			if (!go_board->on_board(i, j) || go_board->get_board(i, j) != OTHER_COLOR(color))
				continue;
			if (go_board->checkLiberty(i, j) == 1)
			{
				int ppos = go_board->findALiberty(i, j);
				if (go_board->available(I(ppos), J(ppos), color))
				{
					*pos = ppos;
					return;
				}
			}
		}
	}

	int * store_board = new int[GoBoard::board_size*GoBoard::board_size];
	int * store_next_stone = new int[GoBoard::board_size*GoBoard::board_size];
	for (int i = 0; i<GoBoard::board_size*GoBoard::board_size; ++i)
	{
		store_board[i] = go_board->board[i];
		store_next_stone[i] = go_board->next_stone[i];
	}
	int storeStep = go_board->step;
	int storeko_i = go_board->ko_i;
	int storeko_j = go_board->ko_j;
	int other = OTHER_COLOR(color);
	for (int i = max(0, rivalMovei - PRECHECKRANGE); i <= (GoBoard::board_size - 1, rivalMovei + PRECHECKRANGE); ++i)
	{
		for (int j = max(0, rivalMovej - PRECHECKRANGE); j <= (GoBoard::board_size - 1, rivalMovej + PRECHECKRANGE); ++j)
		{
			if (go_board->on_board(i, j) && go_board->get_board(i, j) == color)
			{
				if (go_board->checkLiberty(i, j) == 2)
				{
					int pos0 = POS(i, j);
					int pos1 = pos0;
					do{
						int ai = I(pos1);
						int aj = J(pos1);
						for (int k = 0; k < 4; ++k)
						{
							int bi = ai + go_board->deltai[k];
							int bj = aj + go_board->deltaj[k];
							if (go_board->on_board(bi, bj) && go_board->get_board(bi, bj) == EMPTY && go_board->available(bi, bj, color))
							{
								go_board->play_move(bi, bj, color);
								int tmpLiberty = go_board->checkLiberty(bi, bj);
								if (tmpLiberty > 3)
								{
									*pos = POS(bi, bj);
									for (int ii = 0; ii<GoBoard::board_size*GoBoard::board_size; ++ii)
									{
										go_board->board[ii] = store_board[ii];
										go_board->next_stone[ii] = store_next_stone[ii];
									}
									go_board->step = storeStep;
									go_board->ko_i = storeko_i;
									go_board->ko_j = storeko_j;
									return;
								}
								for (int ii = 0; ii<GoBoard::board_size*GoBoard::board_size; ++ii)
								{
									go_board->board[ii] = store_board[ii];
									go_board->next_stone[ii] = store_next_stone[ii];
								}
								go_board->step = storeStep;
								go_board->ko_i = storeko_i;
								go_board->ko_j = storeko_j;
							}
						}
						pos1 = go_board->next_stone[pos1];
					} while (pos1 != pos0);
				}
				else if (go_board->checkLiberty(i, j) == 1)
				{
					int pos0 = POS(i, j);
					int pos1 = pos0;
					do{
						int ai = I(pos1);
						int aj = J(pos1);
						for (int k = 0; k < 4; ++k)
						{
							int bi = ai + go_board->deltai[k];
							int bj = aj + go_board->deltaj[k];
							if (go_board->on_board(bi, bj) && go_board->get_board(bi, bj) == EMPTY && go_board->available(bi, bj, color))
							{
								go_board->play_move(bi, bj, color);
								int tmpLiberty = go_board->checkLiberty(i, j);
								if (go_board->get_board(i, j) == EMPTY || tmpLiberty > 2)
								{
									*pos = POS(bi, bj);
									for (int ii = 0; ii<GoBoard::board_size*GoBoard::board_size; ++ii)
									{
										go_board->board[ii] = store_board[ii];
										go_board->next_stone[ii] = store_next_stone[ii];
									}
									go_board->step = storeStep;
									go_board->ko_i = storeko_i;
									go_board->ko_j = storeko_j;
									delete[]store_board;
									delete[]store_next_stone;
									return;
								}
								for (int ii = 0; ii<GoBoard::board_size*GoBoard::board_size; ++ii)
								{
									go_board->board[ii] = store_board[ii];
									go_board->next_stone[ii] = store_next_stone[ii];
								}
								go_board->step = storeStep;
								go_board->ko_i = storeko_i;
								go_board->ko_j = storeko_j;
							}
						}
						pos1 = go_board->next_stone[pos1];
					} while (pos1 != pos0);
				}
			}
			if (go_board->on_board(i, j) && go_board->get_board(i, j) == other)
			{
				if (go_board->checkLiberty(i, j) == 2)
				{
					int storei[2];
					int storej[2];
					int kk = 0;
					for (int k = 0; k< 4; ++k)
					{
						int bi = i + go_board->deltai[k];
						int bj = j + go_board->deltaj[k];
						if (go_board->on_board(bi, bj) && go_board->get_board(bi, bj) == EMPTY && go_board->available(bi, bj, color) && !go_board->suicideLike(bi, bj, color))
						{
							storei[kk] = bi;
							storej[kk] = bj;
							++kk;
						}
					}
					if (kk == 1)
					{
						*pos = POS(storei[0], storej[0]);
						return;
					}
					if (kk == 0)
						continue;
					for (int k = 0; k < 2; ++k)
					{
						go_board->play_move(storei[k], storej[k], color);
						/*for (int kkk = 0; kkk < 4; ++kkk)
						{
						int aai = storei[k] - deltai[kkk];
						int aaj = storej[k] - deltaj[kkk];
						if (aai==i || aaj==j)
						continue;
						if (on_board(aai, aaj) && get_board(aai, aaj) == other && !same_string(POS(i, j), POS(aai, aaj)) && checkLiberty(aai, aaj) == 1)
						{
						*pos = POS(storei[k], storej[k]);
						for (int ii = 0; ii<board_size*board_size; ++ii)
						{
						board[ii] = store_board[ii];
						next_stone[ii] = store_next_stone[ii];
						}
						step = storeStep;
						ko_i = storeko_i;
						ko_j = storeko_j;
						delete[]store_board;
						delete[]store_next_stone;
						return;
						}
						}*/
						go_board->play_move(storei[1 - k], storej[1 - k], other);
						if (go_board->checkLiberty(i, j) <= 2)
						{
							*pos = POS(storei[k], storej[k]);
							for (int ii = 0; ii<GoBoard::board_size*GoBoard::board_size; ++ii)
							{
								go_board->board[ii] = store_board[ii];
								go_board->next_stone[ii] = store_next_stone[ii];
							}
							go_board->step = storeStep;
							go_board->ko_i = storeko_i;
							go_board->ko_j = storeko_j;
							delete[]store_board;
							delete[]store_next_stone;
							return;
						}
						for (int ii = 0; ii<GoBoard::board_size*GoBoard::board_size; ++ii)
						{
							go_board->board[ii] = store_board[ii];
							go_board->next_stone[ii] = store_next_stone[ii];
						}
						go_board->step = storeStep;
						go_board->ko_i = storeko_i;
						go_board->ko_j = storeko_j;
					}
				}
			}
		}
	}
	delete[]store_board;
	delete[]store_next_stone;

	*pos = -1;
}