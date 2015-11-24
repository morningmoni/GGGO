#pragma once

#include "ConstantVariables.h"
#include "vector"

class uctNode{
public:
	//这个分支下总共下了多少盘
	int play;
	//这个分支下黑-白为多少子
	int playResult;
	bool color;
	int pos;
	bool opened;
	float score;
	std::vector<uctNode*> nextMove;
	uctNode* lastMove;
	uctNode(int p, bool c, uctNode *last);
	~uctNode();
	void addPos(uctNode* p);
	void result(int r);
};