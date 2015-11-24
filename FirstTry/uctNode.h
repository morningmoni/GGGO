#pragma once

#include "ConstantVariables.h"
#include "vector"

class uctNode{
public:
	//�����֧���ܹ����˶�����
	int play;
	//�����֧�º�-��Ϊ������
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