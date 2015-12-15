#include "GoBoard.h"
#include <random>
#define FILL_BOARD_MAX 15
int GoBoard::fill_the_board_heuristic()
{
	for (int i = 0; i <FILL_BOARD_MAX; ++i)
	{
		int pos = rand()*board_size*board_size / (RAND_MAX + 1);
		int ai = I(pos);
		int aj = J(pos);
		bool found = true;
		if (get_board(ai, aj) != EMPTY)
			continue;
		for (int k = 0; k < 8; ++k)
		{

			int bi = ai + around_eight_i[k];
			int bj = aj + around_eight_j[k];
			if (on_board(bi, bj) && get_board(bi, bj) != EMPTY)
			{
				found = false;
				break;
			}
		}
		if(found )
			return pos;
	}
	return -1;
}

int  GoBoard::mogo_pattern_heuristic(int rival_pos,int color)
{
	int ai = I(rival_pos);
	int aj = J(rival_pos);
	int match[8];
	int match_number = 0;
	for (int i = 0; i < 8; i++) {
		int bi = ai + around_eight_i[i];
		int bj = aj + around_eight_j[i];
		if (get_board(bi, bj) == EMPTY && match_go_pattern(POS(bi, bj), color))
		{
			match[match_number++] = POS(bi, bj);
		}

	}
	if (match_number)
		return match[rand()*match_number / (RAND_MAX + 1)];
	else return -1;
}
bool GoBoard::match_go_pattern(int pos, int color)  //need to be modified and the points start at 1 not 0  or we implement according to the papter
{
	if (point == 1 || point == size || point == size2 || point == size*(size - 1) + 1) return false;  //filter corners.

	if (point > size && point % size && point <= size*(size - 1) && point % size != 1) {
		const int *vic = vicinity[point];
		for (int i = 1; i < 8; i += 2) {
			if (points[vic[i]]) {
				bool adj_color = points[vic[i]]->get_color();
				if (points[vic[i - 1]] && points[vic[i - 1]]->get_color() != adj_color) {
					if (points[vic[i + 2]] == 0 && points[vic[i + 6]] == 0) {
						if (points[vic[i + 1]] && points[vic[i + 1]]->get_color() != adj_color) {
							return true;
						}
						if (points[vic[i + 1]] == 0 && points[vic[i + 4]] == 0) {
							return true;
						}
					}
					if (points[vic[i + 2]] == 0 && points[vic[i + 4]] == 0 &&
						points[vic[i + 6]] && points[vic[i + 6]]->get_color() != adj_color) {
						return true;
					}
					if (points[vic[i + 6]] && points[vic[i + 6]]->get_color() == adj_color) {
						if ((points[vic[i + 2]] || points[vic[i + 4]] == 0 || points[vic[i + 4]]->get_color() == adj_color) &&
							(points[vic[i + 4]] || points[vic[i + 2]] == 0 || points[vic[i + 2]]->get_color() == adj_color)) {
							return true;
						}
					}
				}
				if (points[vic[i + 1]] && points[vic[i + 1]]->get_color() != adj_color) {
					if (points[vic[i + 2]] == 0 && points[vic[i + 6]] == 0) {
						if (points[vic[i - 1]] == 0 && points[vic[i + 4]] == 0) {
							return true;
						}
					}
					if (points[vic[i + 4]] == 0 && points[vic[i + 6]] == 0 &&
						points[vic[i + 2]] && points[vic[i + 2]]->get_color() != adj_color) {
						return true;
					}
				}
				if (points[vic[i + 2]] && points[vic[i + 6]] && points[vic[i + 2]]->get_color() != adj_color &&
					points[vic[i + 6]]->get_color() != adj_color &&
					(points[vic[i + 4]] == 0 || points[vic[i + 4]]->get_color() == adj_color) &&
					(points[vic[i + 3]] == 0 || points[vic[i + 3]]->get_color() == adj_color) &&
					(points[vic[i + 5]] == 0 || points[vic[i + 5]]->get_color() == adj_color)) {
					return true;
				}
				if (adj_color != side && points[vic[i - 1]] && points[vic[i - 1]]->get_color() == side &&
					points[vic[i + 2]] == 0 && points[vic[i + 4]] == 0 && points[vic[i + 6]] == 0 &&
					points[vic[i + 1]] && points[vic[i + 1]]->get_color() == adj_color) {
					return true;
				}
				if (adj_color != side && points[vic[i + 1]] && points[vic[i + 1]]->get_color() == side &&
					points[vic[i + 2]] == 0 && points[vic[i + 4]] == 0 && points[vic[i + 6]] == 0 &&
					points[vic[i - 1]] && points[vic[i - 1]]->get_color() == adj_color) {
					return true;
				}
			}
		}
	}
	else {
		for (int i = 1; i < 8; i += 2) {
			const int *vic = vicinity[point];
			if (vic[i]) {
				if (points[vic[i]]) {
					if (vic[i + 2] && points[vic[i + 2]] && points[vic[i + 2]]->get_color() != points[vic[i]]->get_color() &&
						(vic[i + 6] == 0 || points[vic[i + 6]] == 0 || points[vic[i + 6]]->get_color() != points[vic[i]]->get_color())) {
						return true;
					}
					if (vic[i + 6] && points[vic[i + 6]] && points[vic[i + 6]]->get_color() != points[vic[i]]->get_color() &&
						(vic[i + 2] == 0 || points[vic[i + 2]] == 0 || points[vic[i + 2]]->get_color() != points[vic[i]]->get_color())) {
						return true;
					}
					if (points[vic[i]]->get_color() == side) {
						if ((vic[i + 1] && points[vic[i + 1]] && points[vic[i + 1]]->get_color() != side)) {
							return true;
						}
						if ((vic[i - 1] && points[vic[i - 1]] && points[vic[i - 1]]->get_color() != side)) {
							return true;
						}
					}
					if (points[vic[i]]->get_color() != side) {
						if (vic[i + 1] && points[vic[i + 1]] && points[vic[i + 1]]->get_color() == side) {
							if (vic[i + 2] == 0 || points[vic[i + 2]] == 0 || points[vic[i + 2]]->get_color() == side) {
								return true;
							}
							if (vic[i + 2] && points[vic[i + 2]] && points[vic[i + 2]]->get_color() != side
								&& vic[i + 6] && points[vic[i + 6]] && points[vic[i + 6]]->get_color() == side) {
								return true;
							}
						}
						if (vic[i - 1] && points[vic[i - 1]] && points[vic[i - 1]]->get_color() == side) {
							if (vic[i + 6] == 0 || points[vic[i + 6]] == 0 || points[vic[i + 6]]->get_color() == side) {
								return true;
							}
							if (vic[i + 6] && points[vic[i + 6]] && points[vic[i + 6]]->get_color() != side &&
								vic[i + 2] && points[vic[i + 2]] && points[vic[i + 2]]->get_color() == side) {
								return true;
							}
						}
					}
				}
				else if ((vic[i + 6] && points[vic[i + 6]] && vic[i - 1] && points[vic[i - 1]] &&
					points[vic[i + 6]]->get_color() != points[vic[i - 1]]->get_color())
					|| (vic[i + 2] && points[vic[i + 2]] && vic[i + 1] && points[vic[i + 1]] &&
						points[vic[i + 2]]->get_color() != points[vic[i + 1]]->get_color())) {
					return true;
				}
			}
		}
	}
	return false;
}