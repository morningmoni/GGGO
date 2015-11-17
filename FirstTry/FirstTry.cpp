// FirstTry.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "board.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"

int _tmain(int argc, _TCHAR* argv[])
{
	board *bb = NULL;
	int t1 = 0, t2 = 0, t = 0;
	int iteratetimes = 100;
	t1 = time(NULL);
	for (int i = 0; i < iteratetimes; ++i)
	{
		bb = new board(true);
		bb->autoRun();
		delete bb;
	}
	//bb = new board(true);
	//int t1 = time(NULL);
	//bb->autoRun();
	//int t2 = time(NULL);
	//printf("time = %d\n", t2 - t1);
	printf("time=%f\n", (time(NULL) -t1 + 0.0) / iteratetimes);
	bb = new board(true);
	bb->autoRun();
	bb->showGame();
	bb->show();
	system("pause");
	return 0;
}

/*
int _tmain(int argc, _TCHAR* argv[])
{
	printf("命令如下:\nnew black-以黑棋开局\nnew white以白棋开局\nplace x y-在x,y位置放棋子,x为横坐标,从左到右依次变大,y为纵坐标,从上当下依次变大,从0开始\n");
	printf("pass-跳过一步\nend-结束当前棋局\n");
	board *bb = NULL;
	try
	{
		char msg[50];
		while (1)
		{
			scanf("%s", msg);
			if (strcmp(msg, "\n")==0) continue;
			if (strcmp(msg, "new") == 0)
			{
				scanf("%s", msg);
				if (strcmp(msg, "black")==0)
				{
					if (bb)
					{
						bb->show();
						delete bb;
						bb = NULL;
						printf("强制结束上一局\n");
					}
					bb = new board(true);
					bb->show();
				}
				else if (strcmp(msg, "white")==0)
				{
					if (bb)
					{
						bb->show();
						delete bb;
						bb = NULL;
						printf("强制结束上一局\n");
					}
					bb = new board(false);
					int pos = -1;
					bb->aiMove(pos, -1);
					if (pos != -1)
					{
						bb->placeBlack(pos%SIZE, pos / SIZE);
						printf("Computer place %d %d\n", pos%SIZE, pos / SIZE);
						bb->show();
					}
				}
				else
				{
					printf("Invalid input\n");
					continue;
				}
			}
			else
			{
				if (!bb)
				{
					printf("Invalid input\n");
					continue;
				}
				if (strcmp(msg, "end")==0)
				{
					bb->show();
					bb->showGame();
					delete bb;
					bb = NULL;
					break;
				}
				else if (strcmp(msg, "place")==0)
				{
					int x = -1, y = -1;
					scanf("%d%d", &x, &y);
					if (bb->checkColor())
					{
						bool flag = bb->placeBlack(x, y);
						if (!flag)
						{
							printf("Invalid position\n");
							continue;
						}
						int pos = -1;
						bb->aiMove(pos, x + y*SIZE);
						if (pos != -1)
						{
							bb->placeWhite(pos%SIZE, pos / SIZE);
							printf("Computer place %d %d\n", pos%SIZE, pos / SIZE);
						}
						else
						{
							printf("Computer pass\n");
						}
					}
					else
					{
						bool flag = bb->placeWhite(x, y);
						if (!flag)
						{
							printf("Invalid position\n");
							continue;
						}
						int pos = -1;
						bb->aiMove(pos, x + y*SIZE);
						if (pos != -1)
						{
							bb->placeBlack(pos%SIZE, pos / SIZE);
							printf("Computer place %d %d\n", pos%SIZE, pos / SIZE);
						}
						else
						{
							printf("Computer pass\n");
						}
					}
					bb->show();
				}
				else if (strcmp(msg, "pass")==0)
				{
					if (bb->checkColor())
					{
						int pos = -1;
						bb->aiMove(pos, -1);
						if (pos != -1)
						{
							bb->placeWhite(pos%SIZE, pos / SIZE);
							printf("Computer place %d %d\n", pos%SIZE, pos / SIZE);
						}
						else
						{
							printf("Computer pass\n");
						}
					}
					else
					{
						int pos = -1;
						bb->aiMove(pos, -1);
						if (pos != -1)
						{
							bb->placeBlack(pos%SIZE, pos / SIZE);
							printf("Computer place %d %d\n", pos%SIZE, pos / SIZE);
						}
						else
						{
							printf("Computer pass\n");
						}
					}
					bb->show();
				}
				else
				{
					printf("Invalid Input\n");
					continue;
				}
			}
		}
		delete bb;
		bb = NULL;
	}
	catch (int)
	{
		if (bb)
		{
			delete bb;
			bb = NULL;
		}
		printf("Something Wrong!\n");
	}
	system("pause");
	return 0;
}
*/
