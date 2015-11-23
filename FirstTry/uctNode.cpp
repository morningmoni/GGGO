#include "board.h"
#include "uctNode.h"
#include "ConstantVariables.h"

uctNode::uctNode(int p, bool c, uctNode* last)
{
	play = 0;
	playResult = 0;
	pos = p;
	color = c;
	lastMove = last;
	opened = false;
}

void uctNode::addPos(uctNode* p)
{
	nextMove.push_back(p);
}

uctNode::~uctNode()
{
	for (int i = 0; i < nextMove.size(); ++i)
	{
		delete nextMove[i];
	}
}

void uctNode::result(int r)
{
	uctNode *p = this;
	while (p)
	{
		++(p->play);
		p->playResult += r;
		p = p->lastMove;
	}
}