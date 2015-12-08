#ifndef UCTNODE_H_INCLUDED
#define UCTNODE_H_INCLUDED

#include "vector"

class uctNode{
public:
	int play;
	int playResult;
	int color;
	int pos;
	bool opened;
	float score;
	std::vector<uctNode*> nextMove;
	uctNode* lastMove;
	uctNode(int p, int c, uctNode *last);
	~uctNode();
	void addPos(uctNode* p);
	void result(int r);
	uctNode* copy();
};

#endif