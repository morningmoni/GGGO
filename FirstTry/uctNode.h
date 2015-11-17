#pragma once

#include "ConstantVariables.h"
#include "vector"

class uctNode{
public:
	//这个分支下总共下了多少盘，play=blackWin+whiteWin
	int play;
	//这个分支下黑棋总共赢了多少盘
	int blackWin;
	//这个分支下白棋总共赢了多少盘
	int whiteWin;
	double score;
	int pos;
	std::vector<uctNode> nextMove;
};