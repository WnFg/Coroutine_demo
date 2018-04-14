#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ctime.h"

typedef struct Context
{
	void* rdi;
	void* rsi;
	void* rbp;
	void* rbx;
	void* rdx;
	void* rcx;
	void* rax;
	void* rip;
	void* rsp;
} Context;

typedef void (*Task)(void*);
// 事件
typedef void* Event;

typedef struct Coroutine
{
	struct Coroutine *next;
	int id;
	int stack_size;
	int flag;   // dead 0, runable 1, running 2, sleeping 3
	//int trapno;  // 系统调用号
	Context ct;
	Task start_fn;
	void* args;
	void* stack;
	Event* wait_event;
} Coroutine;

// 协程链表
Coroutine *chead, *ctail;
//调度器上下文
Context schedulerContext;

// 协程id
int id_cnt = 1;
Coroutine* runningCoroutine;

// 当前时间 
ULL now = 0;
// 时间事件链表
sleepEvent *shead, *stail;

// 协程状态
enum cStatus{
	DEAD = 0,
	RUNABLE,
	RUNNING,
	SLEEPING
};

// 系统调用号
enum SYS_CALL{
	NOCALL = -1,
	SLEEP = 1
};

int SET(Context *ct);
int GET(Context *ct);

void swapContext(Context* from, Context* to) {
	if (GET(from) == 0)
		SET(to);
}

void cotReadyToRun(Coroutine* cot) {
	cot->flag = RUNABLE; // runable
	if (chead == NULL) {
		chead = cot;
		ctail = cot;
	} else {
		ctail->next = cot;
		ctail = cot;
	}
	cot->next = NULL;
}

int createCoroutine(Task task, void* args) {
	Coroutine* cot = (Coroutine*)malloc(8192);
	cot->stack = cot + 1;
	cot->stack_size = 8192 - sizeof(*cot);
	cot->start_fn = task;
	cot->args = args;
	GET(&cot->ct); // 初始化上下文
	cot->ct.rip = task;
	cot->ct.rsp = cot->stack + cot->stack_size / 8;
	cot->ct.rdi = args;
	cot->id = id_cnt++;
	
	// 放入队列
	cotReadyToRun(cot);
	return cot->id;
}

// 放弃cpu，返回调度器
void cotYield(int flag, Event e) {
	runningCoroutine->flag = flag;
	if (e != NULL) {
		runningCoroutine->wait_event = e;
	}
	// 返回调度器
	swapContext(&runningCoroutine->ct, &schedulerContext);
}

// 协程退出
void cotExit() {
	cotYield(0, NULL);
}

// 唤醒阻塞在事件e上的协程
void cotWakeUp(Event e) {
	Coroutine* cur = chead;
	while (cur != NULL) {
		if (cur->flag == SLEEPING && cur->wait_event == e) {
			cur->flag = RUNABLE;
		}
		cur = cur->next;
	}
}

// 睡眠
void csleep(int t) {
	// 更新当前时间
	now = nsec();
	ULL wait_time = t + now;
	cotYield(SLEEPING, addSleepEvent(wait_time));
}

void scheduler() {
	for(;;) {
		Coroutine* last = NULL;
		Coroutine* cur = chead;
		while (cur != NULL) {
			if (cur->flag == RUNABLE) {
				cur->flag = RUNNING;
				runningCoroutine = cur;
				swapContext(&schedulerContext, &cur->ct);
				// clear
				Coroutine* r = cur;
				cur = cur->next;
				if (last != NULL) {
					last->next = cur;
				} else {
					chead = cur;
				}
				if (r->flag == SLEEPING) {
					// 放入队列尾部
					ctail->next = r;
					r->next = NULL;
					ctail = r;
				} else if (r->flag == DEAD) {
					free(r);
				}
			} else {
				last = cur;
				cur = cur->next;
			}
		}
		
		// 更新当前时间, 唤醒等待线程
		now = nsec();
		wake_up_now();
		//printf("%lld\n", now);
	}
}

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

int main()
{	
	createCoroutine(task1, NULL);
	createCoroutine(task2, NULL);
	scheduler();
	return 0;
}

ULL nsec(void) {
	struct timeval tv;

	if(gettimeofday(&tv, 0) < 0)
		return -1;
	return (ULL)tv.tv_sec*1000 + tv.tv_usec/1000;
}

sleepEvent* addSleepEvent(ULL wait_time) {
	sleepEvent* cur = shead; 
	while (cur != NULL && cur->sleep_time < wait_time) {
		cur = cur->next;
	}
	
	sleepEvent* new = (sleepEvent*)malloc(sizeof(sleepEvent));
	new->sleep_time = wait_time;
	if (cur == NULL) {
		if (stail != NULL) {
			stail->next = new;
			stail = new;
		} else {
			shead = new;
			stail = new;
		}
		new->next = NULL;
	} else {
		new->next = cur->next;
		cur->next = new;
	}
	return new;
}

void wake_up_now() {
	while (shead != NULL && shead->sleep_time <= now) {
		cotWakeUp(shead);
		void* r = shead;
		shead = shead->next;
		free(r);
	}
	if (shead == NULL) {
		stail = NULL;
	}
}
