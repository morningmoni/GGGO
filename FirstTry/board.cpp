#include "board.h"
#include "ConstantVariables.h"
#include "stdio.h"
#include "math.h"
#include "set"
#include "queue"
#include "time.h"
#include "bitset"
#include "uctNode.h"
#include "vector"
#include "algorithm"
#include "cmath"


bool board::checkColor()
{
	return color;
}

std::bitset<SIZE*SIZE> board::copy(std::bitset<SIZE*SIZE> &a)
{
	std::bitset<SIZE*SIZE> tmp;
	for (int i = 0; i < SIZE*SIZE; ++i)
	{
		if (a[i]) tmp.set(i);
		else tmp.reset(i);
	}
	return tmp;
}

//pos:AI选择要下的位置,-1表示pass lastPos:上一步用户下的位置,-1表示上一步没有下
void board::aiMoveGreedy(int &pos, int lastPos, int rivalPos)
{
	std::bitset<SIZE*SIZE> storeBlack = copy(black);
	std::bitset<SIZE*SIZE> storeWhite = copy(white);
	int b = 0, w = 0, bScore = 0, wScore = 0;
	calcGame(b, w, bScore, wScore);
	int tmpScore = bScore + wScore;
	int storePos = -1;
	for (int i = 0; i < SIZE*SIZE; ++i)
	{
		if (color)
		{
			bool flag = placeWhite(i%SIZE, i / SIZE);
			if (!flag) continue;
			bScore = 0;
			wScore = 0;
			calcGame(b, w, bScore, wScore);
			if (bScore + wScore<tmpScore)
			{
				tmpScore = bScore + wScore;
				storePos = i;
			}
		}
		else
		{
			bool flag = placeBlack(i%SIZE, i / SIZE);
			if (!flag) continue;
			bScore = 0;
			wScore = 0;
			calcGame(b, w, bScore, wScore);
			if (bScore + wScore>tmpScore)
			{
				tmpScore = bScore + wScore;
				storePos = i;
			}
		}
		black = copy(storeBlack);
		white = copy(storeWhite);
	}
	pos = storePos;
}

//pos:AI选择要下的位置,-1表示pass lastPos:上一步用户下的位置,-1表示上一步没有下
void board::aiMoveGreedy2(int &pos, int lastPos, int rivalPos)
{
	std::bitset<SIZE*SIZE> storeBlack = copy(black);
	std::bitset<SIZE*SIZE> storeWhite = copy(white);
	int b = 0, w = 0, bScore = 0, wScore = 0;
	calcGame(b, w, bScore, wScore);
	int tmpScore = b - w;
	int storePos = -1;
	for (int i = 0; i < SIZE*SIZE; ++i)
	{
		if (lastPos != -1 && i == lastPos) continue;
		if (color)
		{
			bool flag = placeWhite(i%SIZE, i / SIZE);
			if (!flag) continue;
			bScore = 0;
			wScore = 0;
			b = 0;
			w = 0;
			calcGame(b, w, bScore, wScore);
			if (b - w<tmpScore)
			{
				tmpScore = b - w;
				storePos = i;
			}
		}
		else
		{
			bool flag = placeBlack(i%SIZE, i / SIZE);
			if (!flag) continue;
			bScore = 0;
			wScore = 0;
			b = 0;
			w = 0;
			calcGame(b, w, bScore, wScore);
			if (b - w>tmpScore)
			{
				tmpScore = b - w;
				storePos = i;
			}
		}
		black = copy(storeBlack);
		white = copy(storeWhite);
	}
	pos = storePos;
}

void board::aiMove(int &pos, int lastPos, int rivalPos)
{
	++step;
	/*if (step < STARTMEDIAN)
	{
		aiMoveStart(pos, lastPos, rivalPos);
	}
	else if (step<MEDIANMINMAX)
	{
		aiMoveGreedy2(pos, lastPos, rivalPos);
	}
	else
	{
		aiMoveMonteCarlo(pos, lastPos, rivalPos);
	}*/
	aiMoveMonteCarlo(pos, lastPos, rivalPos);
	lastPosition = pos;
}

//true-black,white-white,user
board::board(bool c)
{
	color = c;
	srand(time(NULL));
	step = 0;
	lastPosition = -1;
}

board::board(const board& b)
{
	color = b.color;
	step = b.step;
	lastPosition = b.lastPosition;
	for (int i = 0; i < SIZE*SIZE; ++i)
	{
		if (b.black[i]) black.set(i);
		else black.reset(i);
		if (b.white[i]) white.set(i);
		else white.reset(i);
	}
}

int board::min(int a, int b)
{
	return a < b ? a : b;
}

int board::max(int a, int b)
{
	return a>b ? a : b;
}

int board::calcDisImpact(int d)
{
	int t = IMPACT;
	for (int i = 0; i < d; ++i)
	{
		t >>= 1;
	}
	return t;
}

//计算当前盘面上黑白势力
bool board::calcGame(int &b, int &w, int &bScore, int &wScore, bool show)
{
	int **extra = new int*[SIZE];
	for (int i = 0; i < SIZE; ++i)
	{
		extra[i] = new int[SIZE];
		for (int j = 0; j < SIZE; ++j) extra[i][j] = 0;//i-x,j-y
	}
	for (int d = 0; d < SIZE*SIZE; ++d)
	{
		if (!black[d] && !white[d]) continue;
		int x = d % SIZE;
		int y = d / SIZE;
		for (int i = max(x - IMPACTDIS, 0); i <= min(x + IMPACTDIS, SIZE - 1); ++i)
		{
			for (int j = max(y - IMPACTDIS, 0); j <= min(y + IMPACTDIS, SIZE - 1); ++j)
			{
				if (abs(x - i) + abs(y - j)>IMPACTDIS) continue;
				int dis = checkDistance(x, y, i, j);
				if (dis == -1 || dis>IMPACTDIS) continue;
				int ppos = i + j*SIZE;
				if (black[d])
				{
					if (black[ppos] && ppos != d) extra[j][i] += calcDisImpact(dis)/SAMECOLOR;
					else if (white[ppos] && ppos != d) extra[j][i] += calcDisImpact(dis) / DIFFERENTCOLOR;
					else extra[j][i] += calcDisImpact(dis);
				}
				else
				{
					if (white[ppos] && ppos != d) extra[j][i] -= calcDisImpact(dis) / SAMECOLOR;
					else if (black[ppos] && ppos != d) extra[j][i] -= calcDisImpact(dis) / DIFFERENTCOLOR;
					else extra[j][i] -= calcDisImpact(dis);
				}
			}
		}
	}
	if (show)
	{
		printf("----------------\n    ");
		for (int i = 0; i < SIZE; ++i)
		{
			if (i / 10 == 0) printf("%d   ", i);
			else printf("%d  ", i);
		}
		printf("\n");
		for (int i = 0; i < SIZE; ++i)
		{
			if (i / 10 == 0) printf("%d   ", i);
			else printf("%d  ", i);
			for (int j = 0; j < SIZE; ++j)
			{
				//printf("%d ", extra[i][j]);
				//if (abs(extra[i][j]) / 10 == 0) printf("%d  ", extra[i][j]);
				//else printf("%d ", extra[i][j]);
				if (extra[i][j] >= 10) printf("%d  ", extra[i][j]);
				else if (extra[i][j] >= 0) printf("%d   ", extra[i][j]);
				else if (extra[i][j]>-10) printf("%d  ", extra[i][j]);
				else printf("%d ", extra[i][j]);
			}
			printf("\n");
		}
		printf("----------------\n");
	}	
	for (int i = 0; i < SIZE; ++i)
	{
		for (int j = 0; j < SIZE; ++j)
		{
			if (extra[i][j]>BLACKEDGE)
			{
				++b;
				bScore += extra[i][j];
			}
			else if (extra[i][j] < WHITEEDGE)
			{
				++w;
				wScore += extra[i][j];
			}
		}
		delete []extra[i];
	}
	delete[]extra;
	//printf("%d %d\n", bScore, wScore);
	return true;
}

void board::showGame()
{
	int w = 0;
	int b = 0;
	int bScore = 0;
	int wScore = 0;
	bool flag = calcGame(b, w, bScore, wScore, true);
	if (!flag)
	{
		printf("Something wrong with the board\n");
	}
	else
	{
		printf("Black: %d\tWhite: %d\n", b, w);
	}
}

int board::checkDistance(int x0, int y0, int x1, int y1)
{
	return abs(x0 - x1) + abs(y0 - y1);
}

//计算棋盘上两点间的最小距离，遇到棋子挡路要绕道，出现错误时返回-1
//int board::checkDistance(int x0, int y0, int x1, int y1)
//{
//	if (x0 < 0 || x0 >= SIZE || y0 < 0 || y0 >= SIZE) return -1;
//	if (x1 < 0 || x1 >= SIZE || y1 < 0 || y1 >= SIZE) return -1;
//	if (x0 == x1 && y0 == y1) return 0;
//	int pos0 = x0 + y0*SIZE;
//	int pos1 = x1 + y1*SIZE;
//	std::set<int> reached;
//	std::queue<int> toreach;
//	toreach.push(pos0);
//	int ans = 0;
//	int level = 1;
//	while (!toreach.empty())
//	{
//		for (int i = 0; i < level; ++i)
//		{
//			int tmp = toreach.front();
//			toreach.pop();
//			reached.insert(tmp);
//			if (tmp == pos1) return ans;
//			if (tmp%SIZE>0 && reached.find(tmp - 1) == reached.end() && (tmp - 1 == pos1 || (!black[tmp - 1] && !white[tmp - 1])))
//			{
//				toreach.push(tmp - 1);
//			}
//			if (tmp%SIZE<SIZE-1 && reached.find(tmp + 1) == reached.end() && (tmp + 1 == pos1 || (!black[tmp + 1] && !white[tmp + 1])))
//			{
//				toreach.push(tmp + 1);
//			}
//			if (tmp / SIZE>0 && reached.find(tmp - SIZE) == reached.end() && (tmp - SIZE == pos1 || (!black[tmp - SIZE] && !white[tmp - SIZE])))
//			{
//				toreach.push(tmp - SIZE);
//			}
//			if (tmp / SIZE<SIZE-1 && reached.find(tmp + SIZE) == reached.end() && (tmp + SIZE == pos1 || (!black[tmp + SIZE] && !white[tmp + SIZE])))
//			{
//				toreach.push(tmp + SIZE);
//			}
//		}
//		++ans;
//		level = toreach.size();
//		if (ans>IMPACTDIS) return -1;
//	}
//	return 0;
//}

//黑棋落子，需要考虑棋子是否重叠，是否为别人的穴，考虑连环劫，应用bfs
bool board::placeBlack(int x, int y)
{
	if (x < 0 || x >= SIZE || y < 0 || y >= SIZE) return false;
	int pos = x + y*SIZE;
	if (black[pos] || white[pos]) return false;
	//检查是否是连环劫，先下子，然后检查上下左右的气
	bool flag = false;
	black.set(pos);
	if (x != 0 && !calcLiberty(x-1,y))
	{
		take(x - 1, y);
		flag = true;
	}
	if (x != SIZE - 1 && !calcLiberty(x + 1, y))
	{
		take(x + 1, y);
		flag = true;
	}
	if (y != 0 && !calcLiberty(x, y - 1))
	{
		take(x, y - 1);
		flag = true;
	}
	if (y != SIZE-1 && !calcLiberty(x, y + 1))
	{
		take(x, y + 1);
		flag = true;
	}
	if (flag) return true;
	black.reset(pos);

	//判断是否是四角且穴
	if (x == 0 && y == 0 && white[pos + 1] && white[pos + SIZE]) return false;
	if (x == SIZE - 1 && y == SIZE - 1 && white[pos - 1] && white[pos - SIZE]) return false;
	if (x == 0 && y == SIZE - 1 && white[pos + 1] && white[pos - SIZE]) return false;
	if (x == SIZE-1 && y == 0 && white[pos - 1] && white[pos + SIZE]) return false;
	//判断是否是四边且穴
	if (x == 0 && y != 0 && y != SIZE - 1 && white[pos + 1] && white[pos - SIZE] && white[pos + SIZE]) return false;
	if (x == SIZE-1 && y != 0 && y != SIZE - 1 && white[pos - 1] && white[pos - SIZE] && white[pos + SIZE]) return false;
	if (y == 0 && x != 0 && x != SIZE - 1 && white[pos - 1] && white[pos + 1] && white[pos + SIZE]) return false;
	if (y == SIZE-1 && x != 0 && x != SIZE - 1 && white[pos - 1] && white[pos + 1] && white[pos - SIZE]) return false;
	//判断中间是否为穴
	if (x != 0 && x != SIZE - 1 && y != 0 && y != SIZE - 1 && white[pos - 1] && white[pos + 1] && white[pos - SIZE] && white[pos + SIZE]) return false;
	black.set(pos);
	return true;
}

//白棋落子，需要考虑棋子是否重叠，是否为别人的穴，考虑连环劫
bool board::placeWhite(int x, int y)
{
	if (x < 0 || x >= SIZE || y < 0 || y >= SIZE) return false;
	int pos = x + y*SIZE;
	if (black[pos] || white[pos]) return false;
	//检查是否是连环劫，先下子，然后检查上下左右的气
	bool flag = false;
	white.set(pos);
	if (x != 0 && !calcLiberty(x - 1, y))
	{
		take(x - 1, y);
		flag = true;
	}
	if (x != SIZE - 1 && !calcLiberty(x + 1, y))
	{
		take(x + 1, y);
		flag = true;
	}
	if (y != 0 && !calcLiberty(x, y - 1))
	{
		take(x, y - 1);
		flag = true;
	}
	if (y != SIZE - 1 && !calcLiberty(x, y + 1))
	{
		take(x, y + 1);
		flag = true;
	}
	if (flag) return true;
	white.reset(pos);

	//判断是否是四角且穴
	if (x == 0 && y == 0 && black[pos + 1] && black[pos + SIZE]) return false;
	if (x == SIZE - 1 && y == SIZE - 1 && black[pos - 1] && black[pos - SIZE]) return false;
	if (x == 0 && y == SIZE - 1 && black[pos + 1] && black[pos - SIZE]) return false;
	if (x == SIZE - 1 && y == 0 && black[pos - 1] && black[pos + SIZE]) return false;
	//判断是否是四边且穴
	if (x == 0 && y != 0 && y != SIZE - 1 && black[pos + 1] && black[pos - SIZE] && black[pos + SIZE]) return false;
	if (x == SIZE - 1 && y != 0 && y != SIZE - 1 && black[pos - 1] && black[pos - SIZE] && black[pos + SIZE]) return false;
	if (y == 0 && x != 0 && x != SIZE - 1 && black[pos - 1] && black[pos + 1] && black[pos + SIZE]) return false;
	if (y == SIZE - 1 && x != 0 && x != SIZE - 1 && black[pos - 1] && black[pos + 1] && black[pos - SIZE]) return false;
	//判断中间是否为穴
	if (x != 0 && x != SIZE - 1 && y != 0 && y != SIZE - 1 && black[pos - 1] && black[pos + 1] && black[pos - SIZE] && black[pos + SIZE]) return false;
	white.set(pos);
	return true;
}

void board::show()
{
	printf("o代表黑子，x代表白子，.代表没有棋子。\n  ");
	for (int i = 0; i < SIZE; ++i)
	{
		if (i / 10 == 0)
		{
			printf("%d  ", i);
		}
		else
		{
			printf("%d ", i);
		}
	}
	printf("\n");
	for (int i = 0; i < SIZE*SIZE; ++i)
	{
		if (i%SIZE == 0)
		{
			if ((i/SIZE) / 10 == 0)
			{
				printf("%d ", i / SIZE);
			}
			else
			{
				printf("%d", i / SIZE);
			}
		}
		if (black[i]) printf("o  ");
		else if (white[i]) printf("x  ");
		else printf(".  ");
		if (!((i + 1) % SIZE)) printf("\n");
	}
}

//计算棋盘上某一位置的棋子所在区域有没有气，0表示没有气，即被围死了，-1表示该位置没有棋子，应用bfs
int board::calcLiberty(int x, int y)
{
	if (x < 0 || x >= SIZE || y < 0 || y >= SIZE) return -1;
	int pos = y*SIZE + x;
	if (!black[pos] && !white[pos]) return -1;
	std::queue<int> blocks;
	std::set<int> liberty;
	std::set<int> usedBlocks;
	if (black[pos])
	{
		blocks.push(pos);
		while (!blocks.empty())
		{
			int tmp = blocks.front();
			blocks.pop();
			usedBlocks.insert(tmp);
			int tmpx = tmp % SIZE;
			int tmpy = tmp / SIZE;
			if (tmpx != 0)
			{
				if (black[tmp - 1] && usedBlocks.find(tmp - 1) == usedBlocks.end()) blocks.push(tmp - 1);
				else if (!white[tmp - 1] && !black[tmp - 1]) liberty.insert(tmp - 1);
			}
			if (tmpx != SIZE - 1)
			{
				if (black[tmp + 1] && usedBlocks.find(tmp + 1) == usedBlocks.end()) blocks.push(tmp + 1);
				else if (!white[tmp + 1] && !black[tmp + 1]) liberty.insert(tmp - 1);
			}
			if (tmpy != 0)
			{
				if (black[tmp - SIZE] && usedBlocks.find(tmp - SIZE) == usedBlocks.end()) blocks.push(tmp - SIZE);
				else if (!white[tmp - SIZE] && !black[tmp - SIZE]) liberty.insert(tmp - 1);
			}
			if (tmpy != SIZE-1)
			{
				if (black[tmp + SIZE] && usedBlocks.find(tmp + SIZE) == usedBlocks.end()) blocks.push(tmp + SIZE);
				else if (!white[tmp + SIZE] && !black[tmp + SIZE]) liberty.insert(tmp - 1);
			}
			if (liberty.size() > 1) return 2;
			else if (liberty.size() == 1) return 1;
		}
		return 0;
	}
	else
	{
		blocks.push(pos);
		while (!blocks.empty())
		{
			int tmp = blocks.front();
			blocks.pop();
			usedBlocks.insert(tmp);
			int tmpx = tmp % SIZE;
			int tmpy = tmp / SIZE;
			if (tmpx != 0)
			{
				if (white[tmp - 1] && usedBlocks.find(tmp - 1) == usedBlocks.end()) blocks.push(tmp - 1);
				else if (!black[tmp - 1] && !white[tmp - 1]) liberty.insert(tmp - 1);
			}
			if (tmpx != SIZE - 1)
			{
				if (white[tmp + 1] && usedBlocks.find(tmp + 1) == usedBlocks.end()) blocks.push(tmp + 1);
				else if (!black[tmp + 1] && !white[tmp + 1]) liberty.insert(tmp - 1);
			}
			if (tmpy != 0)
			{
				if (white[tmp - SIZE] && usedBlocks.find(tmp - SIZE) == usedBlocks.end()) blocks.push(tmp - SIZE);
				else if (!black[tmp - SIZE] && !white[tmp - SIZE]) liberty.insert(tmp - 1);
			}
			if (tmpy != SIZE - 1)
			{
				if (white[tmp + SIZE] && usedBlocks.find(tmp + SIZE) == usedBlocks.end()) blocks.push(tmp + SIZE);
				else if (!black[tmp + SIZE] && !white[tmp + SIZE]) liberty.insert(tmp - 1);
			}
			if (liberty.size() > 1) return 2;
			else if (liberty.size() == 1) return 1;
		}
		return 0;
	}
	return -1;
}

//计算棋盘上某一位置的棋子所在区域有几口气，0表示没有气，即被围死了，-1表示该位置没有棋子，应用bfs
int board::calcLibertyComplex(int x, int y)
{
	if (x < 0 || x >= SIZE || y < 0 || y >= SIZE) return -1;
	int pos = y*SIZE + x;
	if (!black[pos] && !white[pos]) return -1;
	std::queue<int> blocks;
	std::set<int> liberty;
	std::set<int> usedBlocks;
	if (black[pos])
	{
		blocks.push(pos);
		while (!blocks.empty())
		{
			int tmp = blocks.front();
			blocks.pop();
			usedBlocks.insert(tmp);
			int tmpx = tmp % SIZE;
			int tmpy = tmp / SIZE;
			if (tmpx != 0)
			{
				if (black[tmp - 1] && usedBlocks.find(tmp - 1) == usedBlocks.end()) blocks.push(tmp - 1);
				else if (!white[tmp - 1] && !black[tmp - 1]) liberty.insert(tmp - 1);
			}
			if (tmpx != SIZE - 1)
			{
				if (black[tmp + 1] && usedBlocks.find(tmp + 1) == usedBlocks.end()) blocks.push(tmp + 1);
				else if (!white[tmp + 1] && !black[tmp + 1]) liberty.insert(tmp + 1);
			}
			if (tmpy != 0)
			{
				if (black[tmp - SIZE] && usedBlocks.find(tmp - SIZE) == usedBlocks.end()) blocks.push(tmp - SIZE);
				else if (!white[tmp - SIZE] && !black[tmp - SIZE]) liberty.insert(tmp - SIZE);
			}
			if (tmpy != SIZE - 1)
			{
				if (black[tmp + SIZE] && usedBlocks.find(tmp + SIZE) == usedBlocks.end()) blocks.push(tmp + SIZE);
				else if (!white[tmp + SIZE] && !black[tmp + SIZE]) liberty.insert(tmp + SIZE);
			}
		}
		return liberty.size();
	}
	else
	{
		blocks.push(pos);
		while (!blocks.empty())
		{
			int tmp = blocks.front();
			blocks.pop();
			usedBlocks.insert(tmp);
			int tmpx = tmp % SIZE;
			int tmpy = tmp / SIZE;
			if (tmpx != 0)
			{
				if (white[tmp - 1] && usedBlocks.find(tmp - 1) == usedBlocks.end()) blocks.push(tmp - 1);
				else if (!black[tmp - 1] && !white[tmp - 1]) liberty.insert(tmp - 1);
			}
			if (tmpx != SIZE - 1)
			{
				if (white[tmp + 1] && usedBlocks.find(tmp + 1) == usedBlocks.end()) blocks.push(tmp + 1);
				else if (!black[tmp + 1] && !white[tmp + 1]) liberty.insert(tmp + 1);
			}
			if (tmpy != 0)
			{
				if (white[tmp - SIZE] && usedBlocks.find(tmp - SIZE) == usedBlocks.end()) blocks.push(tmp - SIZE);
				else if (!black[tmp - SIZE] && !white[tmp - SIZE]) liberty.insert(tmp - SIZE);
			}
			if (tmpy != SIZE - 1)
			{
				if (white[tmp + SIZE] && usedBlocks.find(tmp + SIZE) == usedBlocks.end()) blocks.push(tmp + SIZE);
				else if (!black[tmp + SIZE] && !white[tmp + SIZE]) liberty.insert(tmp + SIZE);
			}
		}
		return liberty.size();
	}
	return -1;
}

//取子
bool board::take(int x, int y)
{
	if (x < 0 || x >= SIZE || y < 0 || y >= SIZE) return false;
	int pos = x + y*SIZE;
	if (!black[pos] && !white[pos]) return false;
	std::queue<int> blocks;
	std::set<int> usedBlocks;
	std::set<int>::iterator it;
	if (black[pos])
	{
		blocks.push(pos);
		while (!blocks.empty())
		{
			int tmp = blocks.front();
			blocks.pop();
			usedBlocks.insert(tmp);
			int tmpx = tmp % SIZE;
			int tmpy = tmp / SIZE;
			if (tmpx != 0)
			{
				if (black[tmp - 1] && usedBlocks.find(tmp - 1) == usedBlocks.end()) blocks.push(tmp - 1);
			}
			if (tmpx != SIZE - 1)
			{
				if (black[tmp + 1] && usedBlocks.find(tmp + 1) == usedBlocks.end()) blocks.push(tmp + 1);
			}
			if (tmpy != 0)
			{
				if (black[tmp - SIZE] && usedBlocks.find(tmp - SIZE) == usedBlocks.end()) blocks.push(tmp - SIZE);
			}
			if (tmpy != SIZE - 1)
			{
				if (black[tmp + SIZE] && usedBlocks.find(tmp + SIZE) == usedBlocks.end()) blocks.push(tmp + SIZE);
			}
		}
		for (it = usedBlocks.begin(); it != usedBlocks.end(); ++it)
		{
			black.reset(*it);
		}
	}
	else
	{
		blocks.push(pos);
		while (!blocks.empty())
		{
			int tmp = blocks.front();
			blocks.pop();
			usedBlocks.insert(tmp);
			int tmpx = tmp % SIZE;
			int tmpy = tmp / SIZE;
			if (tmpx != 0)
			{
				if (white[tmp - 1] && usedBlocks.find(tmp - 1) == usedBlocks.end()) blocks.push(tmp - 1);
			}
			if (tmpx != SIZE - 1)
			{
				if (white[tmp + 1] && usedBlocks.find(tmp + 1) == usedBlocks.end()) blocks.push(tmp + 1);
			}
			if (tmpy != 0)
			{
				if (white[tmp - SIZE] && usedBlocks.find(tmp - SIZE) == usedBlocks.end()) blocks.push(tmp - SIZE);
			}
			if (tmpy != SIZE - 1)
			{
				if (white[tmp + SIZE] && usedBlocks.find(tmp + SIZE) == usedBlocks.end()) blocks.push(tmp + SIZE);
			}
		}
		for (it = usedBlocks.begin(); it != usedBlocks.end(); ++it)
		{
			white.reset(*it);
		}
	}
	return true;
}
/*
	0	1
	2	3
*/
void board::aiMoveStart(int &pos, int lastPos, int rivalPos)
{
	int b0, b1, b2, b3;
	int w0, w1, w2, w3;
	cornerCnt(0, 0, CORNERSIZE, CORNERSIZE, b0, w0);
	cornerCnt(SIZE-CORNERSIZE, 0, SIZE, CORNERSIZE, b1, w1);
	cornerCnt(0, SIZE-CORNERSIZE, CORNERSIZE, SIZE, b2, w2);
	cornerCnt(SIZE - CORNERSIZE, SIZE - CORNERSIZE, SIZE, SIZE, b3, w3);
	//printf("%d %d\n", b0, w0);
	//printf("%d %d\n", b1, w1);
	//printf("%d %d\n", b2, w2);
	//printf("%d %d\n", b3, w3);
	if (b0 == 0 && w0 == 0)
	{
		pos = CORNERSIZE - 1 + SIZE*(CORNERSIZE - 1);
		return;
	}
	else if (b1 == 0 && w1 == 0)
	{
		pos = SIZE - CORNERSIZE + SIZE*(CORNERSIZE - 1);
		return;
	}
	else if (b2 == 0 && w2 == 0)
	{
		pos = CORNERSIZE - 1 + SIZE*(SIZE-CORNERSIZE);
		return;
	}
	else if (b3 == 0 && w3 == 0)
	{
		pos = SIZE - CORNERSIZE + SIZE*(SIZE - CORNERSIZE);
		return;
	}
	else
	{
		aiMoveGreedy2(pos, lastPos, rivalPos);
		return;
	}
}

void board::cornerCnt(int x0, int y0, int x1, int y1, int &b, int &w)
{
	b = 0;
	w = 0;
	int pos = -1;
	//printf("x0=%d y0=%d x1=%d y1=%d\n",x0,y0,x1,y1);
	for (int j = y0; j < y1; ++j)
	{
		for (int i = x0; i < x1; ++i)
		{
			pos = i + j*SIZE;
			//printf("pos:%d i:%d j:%d\n",pos,i,j);
			if (black[pos]) ++b;
			else if (white[pos]) ++w;
		}
	}
}

void board::aiMoveMinMax(int &pos, int lastPos, int rivalPos)
{
	std::bitset<SIZE*SIZE> storeBlack = copy(black);
	std::bitset<SIZE*SIZE> storeWhite = copy(white);
	std::bitset<SIZE*SIZE> processBlack;
	std::bitset<SIZE*SIZE> processWhite;
	int b = 0, w = 0, bScore = 0, wScore = 0;
	calcGame(b, w, bScore, wScore);
	int tmpScore = b - w;
	int storePos = -1;
	int ttmpScore = tmpScore;
	int storeSecondScore = tmpScore;
	for (int i = 0; i < SIZE*SIZE; ++i)
	{
		if (lastPos != -1 && i == lastPos) continue;
		if (color)
		{
			bool flag = placeWhite(i%SIZE, i / SIZE);
			if (!flag) continue;
			int ix = i%SIZE;
			int iy = i / SIZE;
			processBlack = copy(black);
			processWhite = copy(white);
			bScore = 0;
			wScore = 0;
			b = 0;
			w = 0;
			calcGame(b, w, bScore, wScore);
			tmpScore = b - w;
			ttmpScore = tmpScore;
			int sstorePos = -1;
			for (int xx = max(0, ix - MINMAXRANGE); xx < min(SIZE, ix + MINMAXRANGE); ++xx)
			{
				for (int yy = max(0, iy - MINMAXRANGE); yy < min(SIZE, iy + MINMAXRANGE); ++yy)
				{
					int pp = xx + SIZE*yy;
					if (pp == i) continue;
					bool fflag = placeBlack(xx, yy);
					if (!fflag) continue;
					bScore = 0;
					wScore = 0;
					b = 0;
					w = 0;
					calcGame(b, w, bScore, wScore);
					if (b - w > ttmpScore)
					{
						ttmpScore = b - w;
					}
					black = copy(processBlack);
					white = copy(processWhite);
				}
			}
			if (ttmpScore < storeSecondScore)
			{
				storeSecondScore = ttmpScore;
				storePos = i;
			}			
		}
		else
		{
			bool flag = placeBlack(i%SIZE, i / SIZE);
			if (!flag) continue;
			int ix = i%SIZE;
			int iy = i / SIZE;
			processBlack = copy(black);
			processWhite = copy(white);
			bScore = 0;
			wScore = 0;
			b = 0;
			w = 0;
			calcGame(b, w, bScore, wScore);
			tmpScore = b - w;
			ttmpScore = tmpScore;
			int sstorePos = -1;
			for (int xx = max(0, ix - MINMAXRANGE); xx < min(SIZE, ix + MINMAXRANGE); ++xx)
			{
				for (int yy = max(0, iy - MINMAXRANGE); yy < min(SIZE, iy + MINMAXRANGE); ++yy)
				{
					int pp = xx + SIZE*yy;
					if (pp == i) continue;
					bool fflag = placeWhite(xx, yy);
					if (!fflag) continue;
					bScore = 0;
					wScore = 0;
					b = 0;
					w = 0;
					calcGame(b, w, bScore, wScore);
					if (b - w < ttmpScore)
					{
						ttmpScore = b - w;
					}
					black = copy(processBlack);
					white = copy(processWhite); 
				}
			}
			if (ttmpScore > storeSecondScore)
			{
				storeSecondScore = ttmpScore;
				storePos = i;
			}
		}
		black = copy(storeBlack);
		white = copy(storeWhite);
	}
	if (storePos == -1)
	{
		aiMoveGreedy2(pos, lastPos, rivalPos);
	}
	else
	{
		pos = storePos;
	}	
}

//该位置是否可下c色，判断是否是四角四边中间，判断周围有没有对手棋子，有的话判断下子能否提子，能的话ok，不能的话说明下子保证无提子，下子，判断气，再复位
bool board::available(int pos, bool c, int lastPos)
{
	if (pos < 0 || pos >= SIZE*SIZE) return false;
	if (pos == lastPos) return false;
	if (black[pos] || white[pos]) return false;
	if (c)
	{
		if (pos == 0)
		{
			if (white[pos + 1] && calcLiberty((pos + 1) % SIZE, (pos + 1) / SIZE) == 1) return true;
			if (white[pos + SIZE] && calcLiberty((pos + SIZE) % SIZE, (pos + SIZE) / SIZE) == 1) return true;
		}
		else if (pos == SIZE - 1)
		{
			if (white[pos - 1] && calcLiberty((pos - 1) % SIZE, (pos - 1) / SIZE) == 1) return true;
			if (white[pos + SIZE] && calcLiberty((pos + SIZE) % SIZE, (pos + SIZE) / SIZE) == 1) return true;
		}
		else if (pos == SIZE*SIZE-SIZE)
		{
			if (white[pos + 1] && calcLiberty((pos + 1) % SIZE, (pos + 1) / SIZE) == 1) return true;
			if (white[pos - SIZE] && calcLiberty((pos - SIZE) % SIZE, (pos - SIZE) / SIZE) == 1) return true;
		}
		else if (pos == SIZE*SIZE - 1)
		{
			if (white[pos - 1] && calcLiberty((pos - 1) % SIZE, (pos - 1) / SIZE) == 1) return true;
			if (white[pos - SIZE] && calcLiberty((pos - SIZE) % SIZE, (pos - SIZE) / SIZE) == 1) return true;
		}
		else if (pos > 0 && pos < SIZE - 1)
		{
			if (white[pos + 1] && calcLiberty((pos + 1) % SIZE, (pos + 1) / SIZE) == 1) return true;
			if (white[pos - 1] && calcLiberty((pos - 1) % SIZE, (pos - 1) / SIZE) == 1) return true;
			if (white[pos + SIZE] && calcLiberty((pos + SIZE) % SIZE, (pos + SIZE) / SIZE) == 1) return true;
		}
		else if (pos>SIZE*SIZE - SIZE && pos < SIZE*SIZE - 1)
		{
			if (white[pos + 1] && calcLiberty((pos + 1) % SIZE, (pos + 1) / SIZE) == 1) return true;
			if (white[pos - 1] && calcLiberty((pos - 1) % SIZE, (pos - 1) / SIZE) == 1) return true;
			if (white[pos - SIZE] && calcLiberty((pos - SIZE) % SIZE, (pos - SIZE) / SIZE) == 1) return true;
		}
		else if (pos%SIZE == 0 && pos / SIZE>0 && pos / SIZE < SIZE - 1)
		{
			if (white[pos + 1] && calcLiberty((pos + 1) % SIZE, (pos + 1) / SIZE) == 1) return true;
			if (white[pos - SIZE] && calcLiberty((pos - SIZE) % SIZE, (pos - SIZE) / SIZE) == 1) return true;
			if (white[pos + SIZE] && calcLiberty((pos + SIZE) % SIZE, (pos + SIZE) / SIZE) == 1) return true;
		}
		else if (pos%SIZE == SIZE - 1 && pos / SIZE>0 && pos / SIZE < SIZE - 1)
		{
			if (white[pos - 1] && calcLiberty((pos - 1) % SIZE, (pos - 1) / SIZE) == 1) return true;
			if (white[pos - SIZE] && calcLiberty((pos - SIZE) % SIZE, (pos - SIZE) / SIZE) == 1) return true;
			if (white[pos + SIZE] && calcLiberty((pos + SIZE) % SIZE, (pos + SIZE) / SIZE) == 1) return true;
		}
		else
		{
			if (white[pos + 1] && calcLiberty((pos + 1) % SIZE, (pos + 1) / SIZE) == 1) return true;
			if (white[pos - 1] && calcLiberty((pos - 1) % SIZE, (pos - 1) / SIZE) == 1) return true;
			if (white[pos - SIZE] && calcLiberty((pos - SIZE) % SIZE, (pos - SIZE) / SIZE) == 1) return true;
			if (white[pos + SIZE] && calcLiberty((pos + SIZE) % SIZE, (pos + SIZE) / SIZE) == 1) return true;
		}
		black.set(pos);
		int liberty = calcLiberty(pos%SIZE, pos / SIZE);
		black.reset(pos);
		//printf("check: x=%d y=%d liberty=%d\n", pos%SIZE, pos / SIZE, liberty);
		return liberty != 0;
	}
	else
	{
		if (pos == 0)
		{
			if (black[pos + 1] && calcLiberty((pos + 1) % SIZE, (pos + 1) / SIZE) == 1) return true;
			if (black[pos + SIZE] && calcLiberty((pos + SIZE) % SIZE, (pos + SIZE) / SIZE) == 1) return true;
		}
		else if (pos == SIZE - 1)
		{
			if (black[pos - 1] && calcLiberty((pos - 1) % SIZE, (pos - 1) / SIZE) == 1) return true;
			if (black[pos + SIZE] && calcLiberty((pos + SIZE) % SIZE, (pos + SIZE) / SIZE) == 1) return true;
		}
		else if (pos == SIZE*SIZE - SIZE)
		{
			if (black[pos + 1] && calcLiberty((pos + 1) % SIZE, (pos + 1) / SIZE) == 1) return true;
			if (black[pos - SIZE] && calcLiberty((pos - SIZE) % SIZE, (pos - SIZE) / SIZE) == 1) return true;
		}
		else if (pos == SIZE*SIZE - 1)
		{
			if (black[pos - 1] && calcLiberty((pos - 1) % SIZE, (pos - 1) / SIZE) == 1) return true;
			if (black[pos - SIZE] && calcLiberty((pos - SIZE) % SIZE, (pos - SIZE) / SIZE) == 1) return true;
		}
		else if (pos > 0 && pos < SIZE - 1)
		{
			if (black[pos + 1] && calcLiberty((pos + 1) % SIZE, (pos + 1) / SIZE) == 1) return true;
			if (black[pos - 1] && calcLiberty((pos - 1) % SIZE, (pos - 1) / SIZE) == 1) return true;
			if (black[pos + SIZE] && calcLiberty((pos + SIZE) % SIZE, (pos + SIZE) / SIZE) == 1) return true;
		}
		else if (pos>SIZE*SIZE - SIZE && pos < SIZE*SIZE - 1)
		{
			if (black[pos + 1] && calcLiberty((pos + 1) % SIZE, (pos + 1) / SIZE) == 1) return true;
			if (black[pos - 1] && calcLiberty((pos - 1) % SIZE, (pos - 1) / SIZE) == 1) return true;
			if (black[pos - SIZE] && calcLiberty((pos - SIZE) % SIZE, (pos - SIZE) / SIZE) == 1) return true;
		}
		else if (pos%SIZE == 0 && pos / SIZE>0 && pos / SIZE < SIZE - 1)
		{
			if (black[pos + 1] && calcLiberty((pos + 1) % SIZE, (pos + 1) / SIZE) == 1) return true;
			if (black[pos - SIZE] && calcLiberty((pos - SIZE) % SIZE, (pos - SIZE) / SIZE) == 1) return true;
			if (black[pos + SIZE] && calcLiberty((pos + SIZE) % SIZE, (pos + SIZE) / SIZE) == 1) return true;
		}
		else if (pos%SIZE == SIZE - 1 && pos / SIZE>0 && pos / SIZE < SIZE - 1)
		{
			if (black[pos - 1] && calcLiberty((pos - 1) % SIZE, (pos - 1) / SIZE) == 1) return true;
			if (black[pos - SIZE] && calcLiberty((pos - SIZE) % SIZE, (pos - SIZE) / SIZE) == 1) return true;
			if (black[pos + SIZE] && calcLiberty((pos + SIZE) % SIZE, (pos + SIZE) / SIZE) == 1) return true;
		}
		else
		{
			if (black[pos + 1] && calcLiberty((pos + 1) % SIZE, (pos + 1) / SIZE) == 1) return true;
			if (black[pos - 1] && calcLiberty((pos - 1) % SIZE, (pos - 1) / SIZE) == 1) return true;
			if (black[pos - SIZE] && calcLiberty((pos - SIZE) % SIZE, (pos - SIZE) / SIZE) == 1) return true;
			if (black[pos + SIZE] && calcLiberty((pos + SIZE) % SIZE, (pos + SIZE) / SIZE) == 1) return true;
		}
		white.set(pos);
		int liberty = calcLiberty(pos%SIZE, pos / SIZE);
		white.reset(pos);
		return liberty != 0;
	}
	return false;
}

//黑白双方随机下棋，返回这盘随机下的棋最终的结果，正为黑
int board::autoRun()
{
	std::bitset<SIZE*SIZE> storeBlack = copy(black);
	std::bitset<SIZE*SIZE> storeWhite = copy(white);
	bool passBlack = false;
	bool passWhite = false;
	int iterstep = 0;
	int maxStep = MAXSTEP - step > 10 ? MAXSTEP - step : 10;
	if (color)
	{
		while ((!passBlack || !passWhite) && iterstep < maxStep)
		{
			++iterstep;
			bool flagBlack = true;
			bool flagWhite = true;
			for (int i = 0; i < TRYTIME; ++i)
			{
				int ppos = rand()*SIZE*SIZE / (RAND_MAX + 1);
				flagWhite = placeWhite(ppos%SIZE, ppos / SIZE);
				if (flagWhite) break;
			}
			passWhite = !flagWhite;
			for (int i = 0; i < TRYTIME; ++i)
			{
				int ppos = rand()*SIZE*SIZE / (RAND_MAX + 1);
				flagBlack = placeBlack(ppos%SIZE, ppos / SIZE);
				if (flagBlack) break;
			}
			passBlack = !flagBlack;
		}
	}
	else
	{
		while ((!passBlack || !passWhite) && iterstep < maxStep)
		{
			++iterstep;
			bool flagBlack = true;
			bool flagWhite = true;
			for (int i = 0; i < TRYTIME; ++i)
			{
				int ppos = rand()*SIZE*SIZE / (RAND_MAX + 1);
				flagBlack = placeBlack(ppos%SIZE, ppos / SIZE);
				if (flagBlack) break;
			}
			passBlack = !flagBlack;
			for (int i = 0; i < TRYTIME; ++i)
			{
				int ppos = rand()*SIZE*SIZE / (RAND_MAX + 1);
				flagWhite = placeWhite(ppos%SIZE, ppos / SIZE);
				if (flagWhite) break;
			}
			passWhite = !flagWhite;
		}
	}
	int bScore = 0;
	int wScore = 0;
	int b = 0;
	int w = 0;
	calcGame(b, w, bScore, wScore, false);
	//showGame();
	//show();
	return b - w;
	//return 0;
}

void board::aiMoveMonteCarlo(int &pos, int lastPos, int rivalPos)
{
	if (rivalPos == -1)
	{
		aiMoveGreedy2(pos, lastPos, rivalPos);
		return;
	}
	int bScore = 0;
	int wScore = 0;
	int b = 0;
	int w = 0;
	calcGame(b, w, bScore, wScore, false);
	std::bitset<SIZE*SIZE> storeBlack = copy(black);
	std::bitset<SIZE*SIZE> storeWhite = copy(white);
	int storeStep = step;
	uctNode* root = new uctNode(rivalPos, color, NULL);
	int games = 0;
	getAvailableMonteCarloMove(root, games);	
	while (games < MAXGAMES)
	{
		uctNode *tmp = root;
		while (tmp->play>1 && tmp->nextMove.size() > 0 && tmp->opened)
		{
			//printf("%d %d result:%d play:%d\n",tmp->pos%SIZE, tmp->pos/SIZE, tmp->playResult, tmp->play);
			if (tmp != root)
			{
				if (!tmp->color)
				{
					placeBlack(tmp->pos%SIZE, tmp->pos / SIZE);
				}
				else
				{
					placeWhite(tmp->pos%SIZE, tmp->pos / SIZE);
				}
			}
			for (int ii = 0; ii < tmp->nextMove.size(); ++ii)
			{
				uctNode *tt = tmp->nextMove[ii];
				tt->score = (tt->playResult + 0.0) / tt->play + sqrtf(2*log(games)/tt->play);
			}
			sort(tmp->nextMove.begin(), tmp->nextMove.end(), cmpLess);
			if (!tmp->color)
			{
				tmp = tmp->nextMove[tmp->nextMove.size() - 1];
			}
			else
			{
				tmp = tmp->nextMove[0];
			}
		}
		//printf("x=%d y=%d\n", tmp->pos%SIZE, tmp->pos / SIZE);
		if (tmp->opened && tmp->nextMove.size() == 0)
		{
			//printf("ddddd\n");
			black = copy(storeBlack);
			white = copy(storeWhite);
			step = storeStep;
			break;
		}
		getAvailableMonteCarloMove(tmp, games);
		black = copy(storeBlack);
		white = copy(storeWhite);
		step = storeStep;
	}
	for (int ii = 0; ii < root->nextMove.size(); ++ii)
	{
		uctNode *tt = root->nextMove[ii];
		tt->score = (tt->playResult + 0.0) / tt->play + sqrtf(2 * log(games) / tt->play);
	}
	sort(root->nextMove.begin(), root->nextMove.end(), cmpLess);
	if (!color)
	{
		uctNode *tmpNode = root->nextMove[root->nextMove.size() - 1];
		if (tmpNode->playResult / tmpNode->play > b - w)
		{
			pos = tmpNode->pos;
		}
		else
		{
			pos = -1;
		}
	}
	else
	{
		uctNode *tmpNode = root->nextMove[0];
		if (tmpNode->playResult / tmpNode->play < b - w)
		{
			pos = tmpNode->pos;
		}
		else
		{
			pos = -1;
		}
	}
	delete root;
	if (pos == -1)
	{
		aiMoveGreedy2(pos, lastPos, rivalPos);
	}
}

//寻找蒙特卡洛方法的可选点，thisColor为当前选择可下的棋子的颜色，与上一颜色相反
void board::getAvailableMonteCarloMove(uctNode *root, int &games)
{
	root->opened = true;
	int x = root->pos%SIZE;
	int y = root->pos / SIZE;
	std::bitset<SIZE*SIZE> storeBlack = copy(black);
	std::bitset<SIZE*SIZE> storeWhite = copy(white);
	int storeStep = step;
	for (int i = max(0, x - MONTECARLORANGE); i < min(SIZE, x + MONTECARLORANGE); ++i)
	{
		for (int j = max(0, y - MONTECARLORANGE); j < min(SIZE, y + MONTECARLORANGE); ++j)
		{
			int p = i + j*SIZE;
			if (available(p, !root->color, root->pos))
			{
				//printf("x=%d y=%d i=%d j=%d\n",x,y,i,j);
				uctNode *next = new uctNode(p, !root->color, root);
				root->addPos(next);
				if (root->color)
					placeBlack(p%SIZE, p / SIZE);
				else
					placeWhite(p%SIZE, p / SIZE);
				++step;
				int r = autoRun();
				++games;
				next->result(r);
				black = copy(storeBlack);
				white = copy(storeWhite);
				step = storeStep;
			}
		}
	}
}