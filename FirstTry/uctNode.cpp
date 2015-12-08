#include "uctNode.h"
#include <fstream>

uctNode* uctNode::copy()
{
	uctNode* tmp = new uctNode(pos, color, lastMove);
	tmp->play = play;
	tmp->playResult = playResult;
	tmp->score = score;
	tmp->opened = opened;
	for (int i = 0; i < nextMove.size(); ++i)
	{
		tmp->addPos(nextMove[i]);
	}
	return tmp;
}

uctNode::uctNode(int p, int c, uctNode* last)
{
	play = 0;
	playResult = 0;
	pos = p;
	color = c;
	score = 0.0;
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