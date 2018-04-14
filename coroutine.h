#ifndef __COROUTINE_H
#define __COROUTINE_H
typedef void (*Task)(void*);
int createCoroutine(Task task, void* args);
void csleep(int sec);
void cotExit();
#endif
