#include "GoBoard.h"
#include <random>
void GoBoard::try_to_save_by_eat(int i, int j, int *saves, int &saves_number)// find the group's adjacent groups and check whether we can eat it
{
	int color = get_board(i, j);
	int ai;
	int aj;
	int pos = POS(i, j);
	int pos1 = pos;
	bool checked[MAX_BOARD*MAX_BOARD];
	for (int i = 0; i < board_size*board_size; ++i)
	{
		checked[i] = false;
	}
	do {
		ai = I(pos1);
		aj = J(pos1);
		for (int k = 0; k < 4; ++k)
		{
			int bi = ai + deltai[k];
			int bj = aj + deltaj[k];
			if (!on_board(bi, bj))
				continue;
			if (get_board(bi, bj) == OTHER_COLOR(color))
			{
				int move = find_one_Liberty_for_atari2(bi, bj, checked);
				if (move == -1)
					continue;
				if (available(I(move), J(move), color))
				{
					saves[saves_number++] = move;
				}
			}

		}
		pos1 = next_stone[pos1];

	} while (pos1 != pos);

}
int GoBoard::last_atari_heuristic( int color)
{
	if (rival_move_i == -1)
		return -1;

	int saves[169];
	int saves_number = 0;
	for (int i = 0; i < 4; ++i) {									//check whether there is a atari
		int neighbor_i = rival_move_i + deltai[i];
		int neighbor_j = rival_move_j + deltaj[i];
		if (!on_board(neighbor_i, neighbor_j))
			continue;
		if (get_board(neighbor_i, neighbor_j) == color)
		{

			int move = find_one_Liberty_for_atari(neighbor_i, neighbor_j); // only one liberty means in atari
			if (move == -1)
				continue;
			try_to_save_by_eat(neighbor_i, neighbor_j, saves, saves_number);
			/*try to save by  escape, that is to check the "move" provides more liberty*/
			int libs = 0;										
			for (int j = 0; j < 4 && libs<2; ++j)
			{
				int ne_lib_i = I(move) + deltai[j];				//check the liberty's near by provides more liberty 
				int ne_lib_j = J(move) + deltaj[j];
				if (!on_board(ne_lib_i, ne_lib_j))
					continue;
				if (get_board(ne_lib_i, ne_lib_j) == OTHER_COLOR(color))
					continue;
				if (get_board(ne_lib_i, ne_lib_j) == EMPTY)
				{
					libs++;
					continue;
				}
				if (get_board(ne_lib_i, ne_lib_j) == color)
				{
					libs += checkLiberty(ne_lib_i, ne_lib_j) - 1;
				}
			}
			if (libs > 1 && available(I(move), J(move), color))
			{
				saves[saves_number++] = move;
			}
		}

	}
	if (saves_number)
		return  saves[rand()*saves_number / (RAND_MAX + 1)];
	else return -1;
}