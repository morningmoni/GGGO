/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * This is Brown, a simple go program.                           *
 *                                                               *
 * Copyright 2003 and 2004 by Gunnar Farneb鋍k.                  *
 *                                                               *
 * Permission is hereby granted, free of charge, to any person   *
 * obtaining a copy of this file gtp.c, to deal in the Software  *
 * without restriction, including without limitation the rights  *
 * to use, copy, modify, merge, publish, distribute, and/or      *
 * sell copies of the Software, and to permit persons to whom    *
 * the Software is furnished to do so, provided that the above   *
 * copyright notice(s) and this permission notice appear in all  *
 * copies of the Software and that both the above copyright      *
 * notice(s) and this permission notice appear in supporting     *
 * documentation.                                                *
 *                                                               *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY     *
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE    *
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR       *
 * PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS. IN NO      *
 * EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN THIS  *
 * NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR    *
 * CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING    *
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF    *
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT    *
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS       *
 * SOFTWARE.                                                     *
 *                                                               *
 * Except as contained in this notice, the name of a copyright   *
 * holder shall not be used in advertising or otherwise to       *
 * promote the sale, use or other dealings in this Software      *
 * without prior written authorization of the copyright holder.  *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math.h"
#include "brown.h"
#include <fstream>
#include "algorithm"
#include "vector"
#include "uctNode.h"
#include "windows.h"
#include "string"
#include <time.h>
using namespace std;
/* The GTP specification leaves the initial board size and komi to the
 * discretion of the engine. We make the uncommon choices of 6x6 board
 * and komi -3.14.
 */
int board_size = 13;
float komi = -3.14;

/* Board represented by a 1D array. The first board_size*board_size
 * elements are used. Vertices are indexed row by row, starting with 0
 * in the upper left corner.
 */
static int board[MAX_BOARD * MAX_BOARD];

/* Stones are linked together in a circular list for each string. */
static int next_stone[MAX_BOARD * MAX_BOARD];

/* Storage for final status computations. */
static int final_status[MAX_BOARD * MAX_BOARD];

/* Point which would be an illegal ko recapture. */
static int ko_i, ko_j;

/* Offsets for the four directly adjacent neighbors. Used for looping. */
static int deltai[4] = {-1, 1, 0, 0};
static int deltaj[4] = {0, 0, -1, 1};

/* Macros to convert between 1D and 2D coordinates. The 2D coordinate
 * (i, j) points to row i and column j, starting with (0,0) in the
 * upper left corner.
 */
#define POS(i, j) ((i) * board_size + (j))
#define I(pos) ((pos) / board_size)
#define J(pos) ((pos) % board_size)

/* Macro to find the opposite color. */
#define OTHER_COLOR(color) (WHITE + BLACK - (color))

void
init_brown()
{
  int k;
  int i, j;

  /* The GTP specification leaves the initial board configuration as
   * well as the board configuration after a boardsize command to the
   * discretion of the engine. We choose to start with up to 20 random
   * stones on the board.
   */
  clear_board();
  for (k = 0; k < 20; k++) {
    int color = rand() % 2 ? BLACK : WHITE;
    generate_move(&i, &j, color);
    play_move(i, j, color);
  }
}

void
clear_board()
{
  memset(board, 0, sizeof(board));
}

int
board_empty()
{
  int i;
  for (i = 0; i < board_size * board_size; i++)
    if (board[i] != EMPTY)
      return 0;

  return 1;
}

int
get_board(int i, int j)
{
  return board[i * board_size + j];
}

/* Get the stones of a string. stonei and stonej must point to arrays
 * sufficiently large to hold any string on the board. The number of
 * stones in the string is returned.
 */
int
get_string(int i, int j, int *stonei, int *stonej)
{
  int num_stones = 0;
  int pos = POS(i, j);
  do {
    stonei[num_stones] = I(pos);
    stonej[num_stones] = J(pos);
    num_stones++;
    pos = next_stone[pos];
  } while (pos != POS(i, j));

  return num_stones;
}

static int
pass_move(int i, int j)
{
  return i == -1 && j == -1;
}

static int
on_board(int i, int j)
{
  return i >= 0 && i < board_size && j >= 0 && j < board_size;
}

int
legal_move(int i, int j, int color)
{
  int other = OTHER_COLOR(color);

  /* Pass is always legal. */
  if (pass_move(i, j))
    return 1;

  /* Already occupied. */
  if (get_board(i, j) != EMPTY)
  {
		return 0;
  }

  /* Illegal ko recapture. It is not illegal to fill the ko so we must
   * check the color of at least one neighbor.
   */
  if (i == ko_i && j == ko_j
      && ((on_board(i - 1, j) && get_board(i - 1, j) == other)
	  && (on_board(i + 1, j) && get_board(i + 1, j) == other)
    && (on_board(i, j - 1) && get_board(i, j - 1) == other)
    && (on_board(i, j + 1) && get_board(i, j + 1) == other) ))
    return 0;

  return 1;
}

/* Does the string at (i, j) have any more liberty than the one at
 * (libi, libj)?
 */
static int
has_additional_liberty(int i, int j, int libi, int libj)
{
  int pos = POS(i, j);
  do {
    int ai = I(pos);
    int aj = J(pos);
    int k;
    for (k = 0; k < 4; k++) {
      int bi = ai + deltai[k];
      int bj = aj + deltaj[k];
      if (on_board(bi, bj) && get_board(bi, bj) == EMPTY
	  && (bi != libi || bj != libj))
      return 1;
    }

    pos = next_stone[pos];
  } while (pos != POS(i, j));

  return 0;
}

/* Does (ai, aj) provide a liberty for a stone at (i, j)? */
static int
provides_liberty(int ai, int aj, int i, int j, int color)
{
  /* A vertex off the board does not provide a liberty. */
  if (!on_board(ai, aj))
    return 0;

  /* An empty vertex IS a liberty. */
  if (get_board(ai, aj) == EMPTY)
    return 1;

  /* A friendly string provides a liberty to (i, j) if it currently
   * has more liberties than the one at (i, j).
   */
  if (get_board(ai, aj) == color)
    return has_additional_liberty(ai, aj, i, j);

  /* An unfriendly string provides a liberty if and only if it is
   * captured, i.e. if it currently only has the liberty at (i, j).
   */
  return !has_additional_liberty(ai, aj, i, j);
}

/* Is a move at (i, j) suicide for color? */
static int suicide(int i, int j, int color)
{
  int k;
  for (k = 0; k < 4; k++)
  {
	  if (provides_liberty(i + deltai[k], j + deltaj[k], i, j, color))
		  return 0;
  }
    

  return 1;
}

/* Remove a string from the board array. There is no need to modify
 * the next_stone array since this only matters where there are
 * stones present and the entire string is removed.
 */
static int
remove_string(int i, int j)
{
  int pos = POS(i, j);
  int removed = 0;
  do {
    board[pos] = EMPTY;
    removed++;
    pos = next_stone[pos];
  } while (pos != POS(i, j));
  return removed;
}

/* Do two vertices belong to the same string. It is required that both
 * pos1 and pos2 point to vertices with stones.
 */
static int
same_string(int pos1, int pos2)
{
  int pos = pos1;
  do {
    if (pos == pos2)
      return 1;
    pos = next_stone[pos];
  } while (pos != pos1);

  return 0;
}

/* Play at (i, j) for color. No legality check is done here. We need
 * to properly update the board array, the next_stone array, and the
 * ko point.
 */
void play_move(int i, int j, int color)
{
  if (!on_board(i,j) || get_board(i,j)!=EMPTY)
  {
    //mylog("play move error");
    return;
  }
  int pos = POS(i, j);
  int captured_stones = 0;
  int k;

  /* Reset the ko point. */
  ko_i = -1;
  ko_j = -1;

  /* Nothing more happens if the move was a pass. */
  if (pass_move(i, j))
  {
    return;
  }

  /* If the move is a suicide we only need to remove the adjacent
   * friendly stones.
   */
  if (suicide(i, j, color))
  {
    for (k = 0; k < 4; k++)
    {
      int ai = i + deltai[k];
      int aj = j + deltaj[k];
      if (on_board(ai, aj) && get_board(ai, aj) == color)
        remove_string(ai, aj);
    }
    return;
  }

  /* Not suicide. Remove captured opponent strings. */
  for (k = 0; k < 4; k++)
  {
    int ai = i + deltai[k];
    int aj = j + deltaj[k];
    if (on_board(ai, aj) && get_board(ai, aj) == OTHER_COLOR(color) && !has_additional_liberty(ai, aj, i, j))
      captured_stones += remove_string(ai, aj);
  }

  /* Put down the new stone. Initially build a single stone string by
   * setting next_stone[pos] pointing to itself.
   */
  board[pos] = color;
  next_stone[pos] = pos;

  /* If we have friendly neighbor strings we need to link the strings
   * together.
   */
  for (k = 0; k < 4; k++)
  {
    int ai = i + deltai[k];
    int aj = j + deltaj[k];
    int pos2 = POS(ai, aj);
    /* Make sure that the stones are not already linked together. This
     * may happen if the same string neighbors the new stone in more
     * than one direction.
     */
    if (on_board(ai, aj) && board[pos2] == color && !same_string(pos, pos2))
    {
      /* The strings are linked together simply by swapping the the
       * next_stone pointers.
       */
      int tmp = next_stone[pos2];
      next_stone[pos2] = next_stone[pos];
      next_stone[pos] = tmp;
    }
  }

  /* If we have captured exactly one stone and the new string is a
   * single stone it may have been a ko capture.
   */
  if (captured_stones == 1 && next_stone[pos] == pos)
  {
    int ai, aj;
    /* Check whether the new string has exactly one liberty. If so it
     * would be an illegal ko capture to play there immediately. We
     * know that there must be a liberty immediately adjacent to the
     * new stone since we captured one stone.
     */
    for (k = 0; k < 4; k++) {
      ai = i + deltai[k];
      aj = j + deltaj[k];
      if (on_board(ai, aj) && get_board(ai, aj) == EMPTY)
      {
        break;
      }
    }

    if (!has_additional_liberty(i, j, ai, aj)) {
      ko_i = ai;
      ko_j = aj;
    }
  }
}

/* Generate a move. */
void generate_move(int *i, int *j, int color)
{
	int moves[MAX_BOARD * MAX_BOARD];
	int num_moves = 0;
	int ai, aj;
	int k;

	memset(moves, 0, sizeof(moves));
	for (ai = 0; ai < board_size; ai++)
	{
		for (aj = 0; aj < board_size; aj++)
		{
			/* Consider moving at (ai, aj) if it is legal and not suicide. */
			if (legal_move(ai, aj, color) && !suicide(ai, aj, color))
			{
      			/* Further require the move not to be suicide for the opponent... */
      			if (!suicide(ai, aj, OTHER_COLOR(color)))
					moves[num_moves++] = POS(ai, aj);
				else
				{
    				 /* ...however, if the move captures at least one stone,
					  * consider it anyway.
    				*/
      				for (k = 0; k < 4; k++)
					{
      					int bi = ai + deltai[k];
      					int bj = aj + deltaj[k];
      					if (on_board(bi, bj) && get_board(bi, bj) == OTHER_COLOR(color))
						{
      						moves[num_moves++] = POS(ai, aj);
      						break;
      					}
						if (get_board(bi, bj) && get_board(bi, bj) == color && checkLiberty(bi, bj) == 1)
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
  ofstream outfile1("loglog.txt", ios_base::app);
  outfile1 << "genmove\t";
  outfile1 << *i << " " << *j;
  outfile1 << "\r\n";
  outfile1.close();
}

/* Set a final status value for an entire string. */
static void set_final_status_string(int pos, int status)
{
  int pos2 = pos;
  do
  {
    final_status[pos2] = status;
    pos2 = next_stone[pos2];
  } while (pos2 != pos);
}

/* Compute final status. This function is only valid to call in a
 * position where generate_move() would return pass for at least one
 * color.
 *
 * Due to the nature of the move generation algorithm, the final
 * status of stones can be determined by a very simple algorithm:
 *
 * 1. Stones with two or more liberties are alive with territory.
 * 2. Stones in atari are dead.
 *
 * Moreover alive stones are unconditionally alive even if the
 * opponent is allowed an arbitrary number of consecutive moves.
 * Similarly dead stones cannot be brought alive even by an arbitrary
 * number of consecutive moves.
 *
 * Seki is not an option. The move generation algorithm would never
 * leave a seki on the board.
 *
 * Comment: This algorithm doesn't work properly if the game ends with
 *          an unfilled ko. If three passes are required for game end,
 *          that will not happen.
 */
void
compute_final_status(void)
{
  int i, j;
  int pos;
  int k;

  for (pos = 0; pos < board_size * board_size; pos++)
    final_status[pos] = UNKNOWN;

  for (i = 0; i < board_size; i++)
    for (j = 0; j < board_size; j++)
      if (get_board(i, j) == EMPTY)
	for (k = 0; k < 4; k++) {
	  int ai = i + deltai[k];
	  int aj = j + deltaj[k];
	  if (!on_board(ai, aj))
	    continue;
	  /* When the game is finished, we know for sure that (ai, aj)
           * contains a stone. The move generation algorithm would
           * never leave two adjacent empty vertices. Check the number
           * of liberties to decide its status, unless it's known
           * already.
	   *
	   * If we should be called in a non-final position, just make
	   * sure we don't call set_final_status_string() on an empty
	   * vertex.
	   */
	  pos = POS(ai, aj);
	  if (final_status[pos] == UNKNOWN) {
	    if (get_board(ai, aj) != EMPTY) {
	      if (has_additional_liberty(ai, aj, i, j))
		set_final_status_string(pos, ALIVE);
	      else
		set_final_status_string(pos, DEAD);
	    }
	  }
	  /* Set the final status of the (i, j) vertex to either black
           * or white territory.
	   */
	  if (final_status[POS(i, j)] == UNKNOWN) {
	    if ((final_status[pos] == ALIVE) ^ (get_board(ai, aj) == WHITE))
	      final_status[POS(i, j)] = BLACK_TERRITORY;
	    else
	      final_status[POS(i, j)] = WHITE_TERRITORY;
	  }
	}
}

int
get_final_status(int i, int j)
{
  return final_status[POS(i, j)];
}

void
set_final_status(int i, int j, int status)
{
  final_status[POS(i, j)] = status;
}

/* Valid number of stones for fixed placement handicaps. These are
 * compatible with the GTP fixed handicap placement rules.
 */
int
valid_fixed_handicap(int handicap)
{
  if (handicap < 2 || handicap > 9)
    return 0;
  if (board_size % 2 == 0 && handicap > 4)
    return 0;
  if (board_size == 7 && handicap > 4)
    return 0;
  if (board_size < 7 && handicap > 0)
    return 0;

  return 1;
}

/* Put fixed placement handicap stones on the board. The placement is
 * compatible with the GTP fixed handicap placement rules.
 */
void
place_fixed_handicap(int handicap)
{
  int low = board_size >= 13 ? 3 : 2;
  int mid = board_size / 2;
  int high = board_size - 1 - low;

  if (handicap >= 2) {
    play_move(high, low, BLACK);   /* bottom left corner */
    play_move(low, high, BLACK);   /* top right corner */
  }

  if (handicap >= 3)
    play_move(low, low, BLACK);    /* top left corner */

  if (handicap >= 4)
    play_move(high, high, BLACK);  /* bottom right corner */

  if (handicap >= 5 && handicap % 2 == 1)
    play_move(mid, mid, BLACK);    /* tengen */

  if (handicap >= 6) {
    play_move(mid, low, BLACK);    /* left edge */
    play_move(mid, high, BLACK);   /* right edge */
  }

  if (handicap >= 8) {
    play_move(low, mid, BLACK);    /* top edge */
    play_move(high, mid, BLACK);   /* bottom edge */
  }
}

/* Put free placement handicap stones on the board. We do this simply
 * by generating successive black moves.
 */
void
place_free_handicap(int handicap)
{
  int k;
  int i, j;

  for (k = 0; k < handicap; k++) {
    generate_move(&i, &j, BLACK);
    play_move(i, j, BLACK);
  }
}






/////////////////////
int minmin(int a, int b){return (a < b) ? a : b; }
int maxmax(int a, int b){return (a>b) ? a : b;}



int calcDisImpact(int d)
{
	int t = IMPACT;
	for (int i = 0; i < d; ++i)
	{
		t >>= 1;
	}
	return t;
}

int checkDistance(int x0, int y0, int x1, int y1)
{
	return abs(x0 - x1) + abs(y0 - y1);
}



void calcGame(int *b, int *w, int *bScore, int *wScore)
{
	int **extra = new int*[board_size];  // SIZE to board size
	for (int i = 0; i < board_size; ++i)
	{
		extra[i] = new int[board_size];
		for (int j = 0; j < board_size; ++j) extra[i][j] = 0;//i-x,j-y
	}
	for (int d = 0; d < board_size*board_size; ++d)
	{
		if (board[d] == EMPTY) continue; //black and white changed into board[d]
		int x = I(d);    //d%SIZE into  J(d)
		int y = J(d);   //D /SIZE into I(d)
		for (int i = maxmax(x - IMPACTDIS, 0); i <= minmin(x + IMPACTDIS, board_size - 1); ++i)
		{
			for (int j = maxmax(y - IMPACTDIS, 0); j <= minmin(y + IMPACTDIS, board_size - 1); ++j)
			{
				if (abs(x - i) + abs(y - j)>IMPACTDIS) continue;
				int dis = checkDistance(x, y, i, j);
				if (dis == -1 || dis>IMPACTDIS) continue;
				int ppos = POS(i,j);
				if (board[d] == BLACK)    //black[d] -> board
				{
					if ( board[ppos] == BLACK  && ppos != d) extra[j][i] += calcDisImpact(dis)/SAMECOLOR;
					else if (board[ppos] == WHITE && ppos != d) extra[j][i] += calcDisImpact(dis) / DIFFERENTCOLOR;
					else extra[j][i] += calcDisImpact(dis);
				}
				else
				{
					if (board[ppos] == WHITE && ppos != d) extra[j][i] -= calcDisImpact(dis) / SAMECOLOR;
					else if (board[ppos] == BLACK && ppos != d) extra[j][i] -= calcDisImpact(dis) / DIFFERENTCOLOR;
					else extra[j][i] -= calcDisImpact(dis);
				}
			}
		}
	}

	for (int i = 0; i < board_size; ++i)
	{
		for (int j = 0; j < board_size; ++j)
		{
			if (extra[i][j]>BLACKEDGE)
			{
				++(*b);
				*bScore += extra[i][j];
			}
			else if (extra[i][j] < WHITEEDGE)
			{
				++(*w);
				*wScore += extra[i][j];
			}
		}
		delete []extra[i];
	}
	delete[]extra;
	//printf("%d %d\n", bScore, wScore);
}

void aiMoveGreedy2(int *pos, int color,int * moves,int num_moves)
{
  int * store_board = new int [board_size*board_size];
  int * store_next_stone = new int [board_size*board_size];
  for (int i = 0;i<board_size*board_size;++i)
  {
      store_board[i] = board[i];
      store_next_stone[i] = next_stone[i];
  }
  int storeko_i = ko_i;
  int storeko_j = ko_j;
  int storeStep = step;

	int b = 0, w = 0, bScore = 0, wScore = 0;
	calcGame(&b, &w, &bScore, &wScore);
	int tmpScore = b - w;
	int storePos = -1;
	for (int i = 0;i<num_moves;++i)
  {
      play_move(I(moves[i]),J(moves[i]),color);

      bScore = 0;
      wScore = 0;
      b = 0;
      w = 0;
      calcGame(&b, &w, &bScore, &wScore);
      if (color == BLACK && b - w>tmpScore)
      {
          tmpScore = b - w;
          storePos = moves[i];
      }
  		if (color == WHITE && b-w<tmpScore)
  		{
  			tmpScore = b-w;
  			storePos = moves[i];
  		}
      for(int ii = 0;ii<board_size*board_size;++ii)
      {
        board[ii] = store_board[ii];
        next_stone[ii] = store_next_stone[ii];
      }
      step = storeStep;
      ko_i = storeko_i;
      ko_j = storeko_j;
  }
	(*pos) = storePos;
  if ((*pos) == POS(ko_i,ko_j))
  {
    *pos = -1;
  }
  delete []store_board;
  delete []store_next_stone;
}
/*
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 2
 * End:
 */

int autoRun2(int color)
{
  if (color!=BLACK && color!=WHITE) return 0;

  int moves1[MAX_BOARD * MAX_BOARD];
  int num_moves1 = 0;
  int moves2[MAX_BOARD * MAX_BOARD];
  int num_moves2 = 0;
  int ai, aj;
  int k;

  memset(moves1, 0, sizeof(moves1));
  memset(moves2, 0, sizeof(moves2));
  for (ai = 0; ai < board_size; ai++){
    for (aj = 0; aj < board_size; aj++) {
      /* Consider moving at (ai, aj) if it is legal and not suicide. */
      if (legal_move(ai, aj, color) && !suicide(ai, aj, color)) {
        /* Further require the move not to be suicide for the opponent... */
        if (!suicide(ai, aj, OTHER_COLOR(color)))
          moves1[num_moves1++] = POS(ai, aj);
        else {
        /* ...however, if the move captures at least one stone,
               * consider it anyway.
         */
          for (k = 0; k < 4; k++) {
            int bi = ai + deltai[k];
            int bj = aj + deltaj[k];
            if (on_board(bi, bj) && get_board(bi, bj) == OTHER_COLOR(color)) {
              moves1[num_moves1++] = POS(ai, aj);
              break;
            }
          }
        }
      }
    }
  }

  for (ai = 0; ai < board_size; ai++){
    for (aj = 0; aj < board_size; aj++) {
      /* Consider moving at (ai, aj) if it is legal and not suicide. */
      if (legal_move(ai, aj, OTHER_COLOR(color)) && !suicide(ai, aj, OTHER_COLOR(color))) {
        /* Further require the move not to be suicide for the opponent.color.. */
        if (!suicide(ai, aj, color))
          moves2[num_moves2++] = POS(ai, aj);
        else {
        /* ...however, if the move captures at least one stone,
               * consider it anyway.
         */
          for (k = 0; k < 4; k++) {
            int bi = ai + deltai[k];
            int bj = aj + deltaj[k];
            if (on_board(bi, bj) && get_board(bi, bj) == color) {
              moves2[num_moves2++] = POS(ai, aj);
              break;
            }
          }
        }
      }
    }
  }

  bool pass1 = false;
  bool pass2 = false;
  int maxStep = min(num_moves1,num_moves2)/2;
  int iterstep = 0;
  while ((!pass1 || !pass2) && iterstep<maxStep)
  {
    ++iterstep;
    int ppos;
    bool flag1 = true;
    bool flag2 = true;
    for (int i=0; i<TRYTIME; ++i)
    {
      ppos = rand()%num_moves1;
      flag1 = available(I(ppos), J(ppos), color);
      if (flag1)
      {
        play_move(I(ppos), J(ppos), color);
          break;
      }
    }
    pass1 = !flag1;

    for (int i=0; i<TRYTIME; ++i)
    {
      ppos = rand()%num_moves2;
      flag2 = available(I(ppos), J(ppos), color);
      if (flag2)
      {
        play_move(I(ppos), J(ppos), color);
          break;
      }
    }
    pass2 = !flag2;

    if (iterstep%CLEARTIME == 0)
    {
      memset(moves1, 0, sizeof(moves1));
      memset(moves2, 0, sizeof(moves2));
      for (ai = 0; ai < board_size; ai++){
        for (aj = 0; aj < board_size; aj++) {
          /* Consider moving at (ai, aj) if it is legal and not suicide. */
          if (legal_move(ai, aj, color) && !suicide(ai, aj, color)) {
            /* Further require the move not to be suicide for the opponent... */
            if (!suicide(ai, aj, OTHER_COLOR(color)))
              moves1[num_moves1++] = POS(ai, aj);
            else {
            /* ...however, if the move captures at least one stone,
                   * consider it anyway.
             */
              for (k = 0; k < 4; k++) {
                int bi = ai + deltai[k];
                int bj = aj + deltaj[k];
                if (on_board(bi, bj) && get_board(bi, bj) == OTHER_COLOR(color)) {
                  moves1[num_moves1++] = POS(ai, aj);
                  break;
                }
              }
            }
          }
        }
      }

      for (ai = 0; ai < board_size; ai++){
        for (aj = 0; aj < board_size; aj++) {
          /* Consider moving at (ai, aj) if it is legal and not suicide. */
          if (legal_move(ai, aj, OTHER_COLOR(color)) && !suicide(ai, aj, OTHER_COLOR(color))) {
            /* Further require the move not to be suicide for the opponent.color.. */
            if (!suicide(ai, aj, color))
              moves2[num_moves2++] = POS(ai, aj);
            else {
            /* ...however, if the move captures at least one stone,
                   * consider it anyway.
             */
              for (k = 0; k < 4; k++) {
                int bi = ai + deltai[k];
                int bj = aj + deltaj[k];
                if (on_board(bi, bj) && get_board(bi, bj) == color) {
                  moves2[num_moves2++] = POS(ai, aj);
                  break;
                }
              }
            }
          }
        }
      }
    }
  }
  int bScore = 0;
  int wScore = 0;
  int b = 0;
  int w = 0;
  calcGame(&b, &w, &bScore, &wScore);
  return b-w;
}





int autoRun(int color)
{
	if (color!=BLACK && color!=WHITE) return 0;
	bool passBlack = false;
	bool passWhite = false;
	int iterstep = 0;
	int maxStep = MAXSTEP - step > 10 ? MAXSTEP - step : 10;
	if (color==BLACK)
	{
		while ((!passBlack || !passWhite) && iterstep < maxStep)
		{
			++iterstep;
			 bool flagBlack = true;
			bool flagWhite = true;
			for (int i = 0; i < TRYTIME; ++i)
			{
				int ppos = rand()%(board_size*board_size);
				flagBlack = available(I(ppos), J(ppos), BLACK);
				if (flagBlack)
				{
					play_move(I(ppos), J(ppos), BLACK);
					break;
				}
			}
			passBlack = !flagBlack;
			for (int i = 0; i < TRYTIME; ++i)
			{
				int ppos = rand()%(board_size*board_size);
				flagWhite = available(I(ppos), J(ppos), WHITE);
				if (flagWhite)
				{
					play_move(I(ppos), J(ppos), WHITE);
					break;
				}
			}
			passWhite = !flagWhite;
		}
	}
	else
	{
		//ofstream outfile1  ("loglog.txt");
		//outfile1<<"o";
		//outfile1.close();
		while ((!passBlack || !passWhite) && iterstep < maxStep)
		{
			++iterstep;
			bool flagBlack = false;
			bool flagWhite = false;
			for (int i = 0; i < TRYTIME; ++i)
			{
				int ppos = rand()%(board_size*board_size);
				flagWhite = available(I(ppos), J(ppos), WHITE);
				if (flagWhite)
				{
					play_move(I(ppos), J(ppos), WHITE);
					break;
				}
			}
			passWhite = !flagWhite;
			for (int i = 0; i < TRYTIME; ++i)
			{
				int ppos = rand()%(board_size*board_size);
				flagBlack = available(I(ppos), J(ppos), BLACK);
				if (flagBlack)
				{
					play_move(I(ppos), J(ppos), BLACK);
					break;
				}
			}
			passBlack = !flagBlack;
		}
	}
	//ofstream outfile2  ("loglog.txt");
	//outfile2<<"k";
	//outfile2.close();
	int bScore = 0;
	int wScore = 0;
	int b = 0;
	int w = 0;
	calcGame(&b, &w, &bScore, &wScore);
	return b-w;
	//return rand()*100%(board_size*board_size)-112;
}

bool available(int i, int j, int color)
{
	//return (on_board(i,j)==1) && (legal_move(i,j,color)==1) && (suicide(i,j,color)==0);
	if (!on_board(i,j) || get_board(i,j) != EMPTY) return false;
	if (legal_move(i,j,color) && !suicide(i,j,color))
	{
		if (!suicide(i,j, OTHER_COLOR(color)))
			return true;
		else
		{
			for (int k=0; k<4; ++k)
			{
				int bi = i+deltai[k];
				int bj = j+deltaj[k];
				if (on_board(bi, bj) && get_board(bi, bj) == OTHER_COLOR(color)) {
					return true;
				}
				if (get_board(bi, bj) && get_board(bi, bj) == color && checkLiberty(bi, bj) == 1)
				{
					return true;
				}
			}
		}
	}
	return false;
}

void getAvailableMonteCarloMove(uctNode *root, int *games)
{
  root->opened = true;
  int x = I(root->pos);
  int y = J(root->pos);
  int * store_board = new int [board_size*board_size];
  int * store_next_stone = new int [board_size*board_size];
  for (int i = 0;i<board_size*board_size;++i)
  {
    store_board[i] = board[i];
    store_next_stone[i] = next_stone[i];
  }
  int storeStep = step;
  int storeko_i = ko_i;
  int storeko_j = ko_j;

  for (int i = max(0, x - MONTECARLORANGE); i < min(board_size, x + MONTECARLORANGE); ++i)
  {
    for (int j = max(0, y - MONTECARLORANGE); j < min(board_size, y + MONTECARLORANGE); ++j)
    {
      if (available(i,j,OTHER_COLOR(root->color)))
      {
        uctNode *next = new uctNode(POS(i,j), OTHER_COLOR(root->color), root);
        root->addPos(next);
        play_move(i, j, OTHER_COLOR(root->color));
        ++step;
        int r = autoRun(root->color);
        ++(*games);
        next->result(r);
        for(int ii = 0;ii<board_size*board_size;++ii)
        {
          board[ii] = store_board[ii];
          next_stone[ii] = store_next_stone[ii];
        }
        step = storeStep;
        ko_i = storeko_i;
        ko_j = storeko_j;
      }
    }
  }
  delete []store_board;
  delete []store_next_stone;
}

void aiMoveMonteCarlo(int *pos, int color,int *moves,int num_moves)
{
  if (rivalMovei==-1 || rivalMovej==-1)
  {
    aiMoveGreedy2(pos,color,moves,num_moves);
    return;
  }
  srand(time(NULL));
  int bScore = 0;
  int wScore = 0;
  int b = 0;
  int w = 0;
  calcGame(&b, &w, &bScore, &wScore);

  int * store_board = new int [board_size*board_size];
  int * store_next_stone = new int [board_size*board_size];
  for (int i = 0;i<board_size*board_size;++i)
  {
    store_board[i] = board[i];
    store_next_stone[i] = next_stone[i];
  }
  int storeStep = step;
  int storeko_i = ko_i;
  int storeko_j = ko_j;

  uctNode* root = new uctNode(POS(rivalMovei,rivalMovej), OTHER_COLOR(color), NULL);
  int games = 0;
  getAvailableMonteCarloMove(root, &games);
  while (games < MAXGAMES)
  {
    uctNode *tmp = root;
    while (tmp->play>1 && tmp->nextMove.size() > 0 && tmp->opened)
    {
      if (tmp!=root)
      {
        play_move(I(tmp->pos), J(tmp->pos), tmp->color);
      }
      for (int ii = 0; ii < tmp->nextMove.size(); ++ii)
      {
        uctNode *tt = tmp->nextMove[ii];
        tt->score = (tt->playResult + 0.0) / tt->play + sqrtf(2*log(games)/tt->play);
      }
      sort(tmp->nextMove.begin(), tmp->nextMove.end(), cmpLess);
      if (tmp->color==WHITE)
      {
        tmp = tmp->nextMove[tmp->nextMove.size() - 1];
      }
      else
      {
        tmp = tmp->nextMove[0];
      }
    }
    if (tmp->opened && tmp->nextMove.size() == 0)
    {
      break;
    }
    getAvailableMonteCarloMove(tmp, &games);
    for(int ii = 0;ii<board_size*board_size;++ii)
    {
      board[ii] = store_board[ii];
      next_stone[ii] = store_next_stone[ii];
    }
    step = storeStep;
    ko_i = storeko_i;
    ko_j = storeko_j;
  }
  for(int ii = 0;ii<board_size*board_size;++ii)
  {
    board[ii] = store_board[ii];
    next_stone[ii] = store_next_stone[ii];
  }
  step = storeStep;
  ko_i = storeko_i;
  ko_j = storeko_j;
  if ( root->nextMove.size() > 0)
  {
    for (int ii = 0; ii < root->nextMove.size(); ++ii)
    {
      uctNode *tt = root->nextMove[ii];
      tt->score = (tt->playResult + 0.0) / tt->play + sqrtf(2 * log(games) / tt->play);
    }
    if (color==BLACK)
    {
      uctNode *tmpNode = root->nextMove[root->nextMove.size() - 1];
      //mylog("black:",tmpNode->playResult / tmpNode->play);
      if (tmpNode->playResult / tmpNode->play > b - w)
      {
        (*pos) = tmpNode->pos;
      }
      else
      {
        (*pos) = -1;
      }
    }
    else
    {
      uctNode *tmpNode = root->nextMove[0];
      mylog("white:",tmpNode->playResult / tmpNode->play);
      if (tmpNode->playResult / tmpNode->play < b - w)
      {
        (*pos) = tmpNode->pos;
      }
      else
      {
        (*pos) = -1;
      }
    }
  }
  else
  {
    (*pos) = -1;
  }
  delete root;
  delete []store_board;
  delete []store_next_stone;
  if ((*pos) == -1 || (*pos)==POS(ko_i,ko_j))
  {
    aiMoveGreedy2(pos, color, moves, num_moves);
  }
}

bool cmpLess(const uctNode *a, const uctNode *b)
{
  return a->score < b->score;
}

bool cmpMore(const uctNode* a, const uctNode *b)
{
  return a->score > b->score;
}

void mylog(const char str[])
{
  ofstream outfile1("loglog.txt",ios_base::app);
  outfile1<<str;
  outfile1<<"\r\n";
  outfile1.close();
}

void mylog(int num1, int num2)
{
  ofstream outfile1("lognum.txt",ios_base::app);
  outfile1<<num1<<" "<<num2;
  outfile1<<"\r\n";
  outfile1.close();
}

void mylog(int num)
{
  ofstream outfile1("loglog.txt",ios_base::app);
  outfile1<<num;
  outfile1<<"\r\n";
  outfile1.close();
}

void mylog(const char str[], int num)
{
  ofstream outfile1("loglog.txt",ios_base::app);
  outfile1<<str;
  outfile1<<num;
  outfile1<<"\r\n";
  outfile1.close();
}

void show_game()
{
  ofstream outfile1("log_game.txt", ios_base::app);
  outfile1<<"----------------------------\r\n";
  outfile1<<"board\r\n";
  outfile1<<"   ";
  for (int i=0; i<board_size; ++i)
  {
    outfile1<<i;
    if (i/10==0)
      outfile1<<"  ";
    else
      outfile1<<" ";
  }
  outfile1<<"\t     ";

  for (int i=0; i<board_size; ++i)
  {
    outfile1<<i;
    if (i/10==0)
      outfile1<<"     ";
    else
      outfile1<<"    ";
  }

  outfile1<<"\r\n";
  for (int i=0; i<board_size; ++i)
  {
    outfile1<<i;
    if (i/10==0)
      outfile1<<"  ";
    else
      outfile1<<" ";
    for (int j=0; j<board_size; ++j)
    {
      if (get_board(i,j)==BLACK)
        outfile1<<"B";
      else if (get_board(i,j)==WHITE)
        outfile1<<"W";
      else
        outfile1<<"O";
      outfile1<<"  ";
    }

    outfile1<<"\t";
    outfile1<<i;
    if (i/10==0)
      outfile1<<"  ";
    else
      outfile1<<" ";
    for (int j=0; j<board_size; ++j)
    {
      int t = next_stone[POS(i,j)];
      int ui = I(t);
      int uy = J(t);
      if (ui/10==0)
        outfile1<<" ";
      outfile1<<ui;
      outfile1<<",";
      outfile1<<uy;
      if (uy/10==0)
        outfile1<<"  ";
      else
        outfile1<<" ";

    }
    outfile1<<"\r\n";
  }
  outfile1<<"\r\n";
  outfile1.close();
}

//Start here
int generate_legal_moves(int* moves, int color)
{
	int num_moves = 0;
	int ai, aj;
	int k;

	memset(moves, 0, sizeof(moves));
	for (ai = 0; ai < board_size; ai++)
	{
		for (aj = 0; aj < board_size; aj++)
		{
			/* Consider moving at (ai, aj) if it is legal and not suicide. */
			if (legal_move(ai, aj, color) && !suicide(ai, aj, color))
			{
				/* Further require the move not to be suicide for the opponent... */
				if (!suicide(ai, aj, OTHER_COLOR(color)))
					moves[num_moves++] = POS(ai, aj);
				else
				{
					/* ...however, if the move captures at least one stone,
					* consider it anyway.
					*/
					for (k = 0; k < 4; k++)
					{
						int bi = ai + deltai[k];
						int bj = aj + deltaj[k];
						if (on_board(bi, bj) && get_board(bi, bj) == OTHER_COLOR(color))
						{
							moves[num_moves++] = POS(ai, aj);
							break;
						}
						if (get_board(bi, bj) && get_board(bi, bj) == color && checkLiberty(bi, bj) == 1)
						{
							moves[num_moves++] = POS(ai, aj);
							break;
						}
					}
				}
			}
		}
	}
	return num_moves;
}

uctNode* expand(uctNode* curNode, int* moves, int num_moves)
{
	for (int i = 0; i < num_moves; ++i)
	{
		bool flag = true;
		for (int j=0; j<curNode->nextMove.size(); ++j)
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

uctNode* bestchild(uctNode* curNode, int c, int games)
{
	calScore(curNode, games);
	sort(curNode->nextMove.begin(), curNode->nextMove.end(), cmpLess);
	if (curNode->color==BLACK)
		return curNode->nextMove[0];
	else
		return curNode->nextMove[curNode->nextMove.size()-1];
}

uctNode* treePolicy(uctNode* v, int games)
{
	uctNode* curNode = v;
	int* moves = new int[MAX_BOARD * MAX_BOARD]; //available moves
	int num_moves;	//available moves_count
	while (curNode->nextMove.size() > 0 || !curNode->lastMove) //while not leaf node, or is root
	{
		if (curNode->pos != POS(rivalMovei, rivalMovej))
		{
			play_move(I(curNode->pos), J(curNode->pos), curNode->color);
		}
		num_moves = generate_legal_moves(moves, OTHER_COLOR(curNode->color));
		if (num_moves != curNode->nextMove.size()) //not fully expanded
		{
			uctNode* tmp = expand(curNode, moves, num_moves);
			delete[]moves;
			return tmp;
		}			
		else
			curNode = bestchild(curNode, 1, games);
	}
	delete[]moves;
	return curNode;
}

int defaultPolicy(int color)
{
	return autoRun(color);
}

void backup(uctNode* v, int reward)
{
	v->result(reward);
}
void uctSearch(int *pos, int color, int *moves, int num_moves)
{
	if (rivalMovei == -1 || rivalMovej == -1)
	{
		aiMoveGreedy2(pos, color, moves, num_moves);
		return;
	}
	srand(time(NULL));
	int * store_board = new int[board_size*board_size];
	int * store_next_stone = new int[board_size*board_size];
	for (int i = 0; i<board_size*board_size; ++i)
	{
		store_board[i] = board[i];
		store_next_stone[i] = next_stone[i];
	}
	int storeStep = step;
	int storeko_i = ko_i;
	int storeko_j = ko_j;

	int games = 0;
	uctNode* root = new uctNode(POS(rivalMovei, rivalMovej), OTHER_COLOR(color), NULL);
	int reward = 0;

	while (games < MAXGAMES)
	{
		uctNode* chosenNode = treePolicy(root,games);
		if (!chosenNode)
			break;
		play_move(I(chosenNode->pos), J(chosenNode->pos), chosenNode->color);
		reward = defaultPolicy(OTHER_COLOR(chosenNode->color));
		backup(chosenNode, reward);
		++games;
		for(int ii = 0;ii<board_size*board_size;++ii)
		{
			board[ii] = store_board[ii];
			next_stone[ii] = store_next_stone[ii];
		}
		step = storeStep;
		ko_i = storeko_i;
		ko_j = storeko_j;
	}
	if (root->nextMove.size()>0)
	{
		uctNode* resNode = bestchild(root, 0, games); //final result
		*pos = resNode->pos;
	}
	else
	{
		*pos = -1;
	}
	

  delete root;
  delete []store_board;
  delete []store_next_stone;
  if ((*pos) == -1 || (*pos)==POS(ko_i,ko_j) || !legal_move(I(*pos),J(*pos),color))
  {
    aiMoveGreedy2(pos, color, moves, num_moves);
  }
}

void calScore(uctNode* tmp, int games)
{
  for (int ii = 0; ii < tmp->nextMove.size(); ++ii)
  {
    uctNode *tt = tmp->nextMove[ii];
	if (tt->play == 0)
		tt->score = 0;
	else
		tt->score = (tt->playResult + 0.0) / tt->play + sqrt(2*log(games)/tt->play);
  }
}

int checkLiberty(int i, int j)
{
	if (!on_board(i, j))
		return -1;
	int color = get_board(i, j);
	if (color == EMPTY)
		return -1;
	int other = OTHER_COLOR(color);
	int ai;
	int aj;
	int pos = POS(i, j);
	int pos1 = pos;
	int ans = 0;
	do{
		ai = I(pos1);
		aj = J(pos1);
		for (int k = 0; k < 4; ++k)
		{
			int bi = ai + deltai[k];
			int bj = aj + deltaj[k];
			if (on_board(bi, bj) && get_board(bi, bj) == other)
				++ans;
		}
		pos1 = next_stone[pos1];
	} while (pos1 != pos);
	return ans;
}

void aiMove(int *pos, int color,int *moves,int num_moves)
{
  //aiMoveGreedy2(pos,color,moves,num_moves);
  //aiMoveMonteCarlo(pos,color,moves,num_moves);
  uctSearch(pos,color,moves,num_moves);
}