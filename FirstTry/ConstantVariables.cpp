#include "ConstantVariables.h"
#include "uctNode.h"

bool cmpLess(const uctNode* a, const uctNode *b)
{
	return a->playResult*b->play < b->playResult*a->play;
}
bool cmpMore(const uctNode* a, const uctNode *b)
{
	return a->playResult*b->play > b->playResult*a->play;
}