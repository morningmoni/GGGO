#pragma once

#include "ConstantVariables.h"
#include "vector"

class uctNode{
public:
	//�����֧���ܹ����˶����̣�play=blackWin+whiteWin
	int play;
	//�����֧�º����ܹ�Ӯ�˶�����
	int blackWin;
	//�����֧�°����ܹ�Ӯ�˶�����
	int whiteWin;
	double score;
	int pos;
	std::vector<uctNode> nextMove;
};