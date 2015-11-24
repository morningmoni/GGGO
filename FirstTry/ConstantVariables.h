#pragma once

#include "math.h"
#include "uctNode.h"

#define SIZE 15
#define IMPACT 32
#define IMPACTDIS int(log2(IMPACT))
#define AIMOVEMAX 10
#define SAMECOLOR 4
#define DIFFERENTCOLOR 2
#define STARTMEDIAN 5
#define CORNERSIZE 5
#define MINMAXRANGE 3
#define BLACKEDGE 4
#define WHITEEDGE -BLACKEDGE
#define MEDIANMINMAX 50
#define TRYTIME 100
#define MAXSTEP 60
#define TIMELIMIT 2
#define MAXGAMES 600
#define MONTECARLORANGE 4

bool cmpLess(const uctNode *a, const uctNode *b);
bool cmpMore(const uctNode *a, const uctNode *b);