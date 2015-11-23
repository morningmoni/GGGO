#pragma once

#include "ConstantVariables.h"
#include <bitset>
#include "uctNode.h"

class board{
public:
	//color记录对手使用的颜色，ai使用与color相反的颜色
	bool color;
	int step;
	int lastPosition;
	bool take(int x, int y);
	int checkDistance(int x0, int y0, int x1, int y1);
	bool calcGame(int &b, int &w, int &bScore, int &wScore, bool show = false);
	int calcDisImpact(int d);
	int min(int a, int b);
	int max(int a, int b);
	std::bitset<SIZE*SIZE> copy(std::bitset<SIZE*SIZE> &a);
	void aiMoveGreedy(int &pos, int lastPos, int rivalPos);
	void aiMoveGreedy2(int &pos, int lastPos, int rivalPos);
	void aiMoveStart(int &pos, int lastPos, int rivalPos);
	void aiMoveMinMax(int &pos, int lastPos, int rivalPos);
	void cornerCnt(int x0, int y0, int x1, int y1, int &b, int &w);
	bool checkColor();
	board(bool c);
	board(const board& b);
	void aiMove(int &pos, int lastPos, int rivalPos);
	void showGame();
	std::bitset<SIZE*SIZE> black;
	std::bitset<SIZE*SIZE> white;
	bool placeBlack(int x, int y);
	bool placeWhite(int x, int y);
	int calcLiberty(int x, int y);
	int calcLibertyComplex(int x, int y);
	void show();
	int autoRun();
	void aiMoveMonteCarlo(int &pos, int lastPos, int rivalPos);
	bool available(int pos, bool c, int lastPos);
	void getAvailableMonteCarloMove(uctNode *root, int &games);
};