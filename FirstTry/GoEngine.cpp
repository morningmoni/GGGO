#include "GoEngine.h"
#include <time.h>
#include <stdlib.h>
#include <process.h>
#include <windows.h>
#include "algorithm"
#include<vector>
using namespace std;
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

uctNode* GoEngine::treePolicy(GoBoard * temp_board)
{
	uctNode* curNode = root;

	int* moves = new int[go_board->board_size2]; //available moves
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
}

int GoEngine::defaultPolicy(GoBoard * temp,int color)
{
	return temp->autoRun(color);
}



void GoEngine::backup(uctNode* v, int reward)
{
	v->result(reward);
}


unsigned __stdcall GoEngine::ThreadFunc(void * p)
{
	int seed = GetCurrentThreadId()*time(NULL);
	srand(seed);
	int reward = 0;
	GoEngine *engine = (GoEngine *) p;
	while (engine->games < MAXGAMES && clock()-engine->fin_clock> MAXTIME )
	{
		GoBoard * temp_board;

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
}
void GoEngine::uctSearch(int *pos, int color, int *moves, int num_moves)
{

	srand(time(NULL));
	//int games = 0; games and reward is a parameter of class GoEngine
	games = 0;
	uctNode* root = new uctNode(POS(rivalMovei, rivalMovej), OTHER_COLOR(color), NULL);
	//int reward = 0;
	fin_clock = clock();
	HANDLE handles[THREAD_NUM];

	InitializeCriticalSection(&cs);
	for (int i = 0; i < THREAD_NUM; i++) {
		handles[i] = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, (void *)this, 0, NULL);
	}
	WaitForMultipleObjects(THREAD_NUM, handles, TRUE, INFINITE);
	DeleteCriticalSection(&cs);
	for (int i = 0; i < THREAD_NUM; i++) CloseHandle(handles[i]);


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
/*	if ((*pos) == -1 || (*pos) == POS(ko_i, ko_j) || !legal_move(I(*pos), J(*pos), color))
	{
		aiMoveGreedy2(pos, color, moves, num_moves);
	}*/ 
	//greedy not implemented
}


void GoEngine::aiMove(int *pos, int color, int *moves, int num_moves)
{
	//aiMoveGreedy2(pos,color,moves,num_moves);
	//aiMoveMonteCarlo(pos,color,moves,num_moves);
	//aiMovePreCheck(pos, color, moves, num_moves);
	//if (*pos==-1)
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
	for (ai = 0; ai < go_board->board_size; ai++)
	{
		for (aj = 0; aj < go_board->board_size; aj++)
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
	}
	/*ofstream outfile1("loglog.txt", ios_base::app);
	outfile1 << "genmove\t";
	outfile1 << *i << " " << *j;
	outfile1 << "\r\n";
	outfile1.close();*/   ///I include fstream.h but no use?
}



void GoEngine::init_GGGO()
{
	int k;
	int i, j;

	/* The GTP specification leaves the initial board configuration as
	* well as the board configuration after a boardsize command to the
	* discretion of the engine. We choose to start with up to 20 random
	* stones on the board.
	*/
	go_board->clear_board();
	for (k = 0; k < 20; k++) {
		int color = rand() % 2 ? BLACK : WHITE;
		generate_move(&i, &j, color);
		go_board->play_move(i, j, color);
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