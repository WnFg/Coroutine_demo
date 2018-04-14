#include "coroutine.h"
#include <stdio.h>

void task1(void* args) {
	int i = 0;
	for (; i < 20; i++) {
		printf("i am task1\n\n");
		csleep(1000);
	}
	cotExit();
}

void task2(void* args) {
	int i = 0;
	for (; i < 20; i++) {
		printf("i am task2\n\n");
		csleep(1000);
	}
	cotExit();
}



void coroutineMain(void* args)
{
	// 创建两个协程task1, task2
	createCoroutine(task1, NULL);
	createCoroutine(task2, NULL);
	cotExit();
}
