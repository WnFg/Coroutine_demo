#ifndef __CTIME_H
#define __CTIME_H
#define ULL unsigned long long
typedef struct sleepEvent
{
	struct sleepEvent *next;
	ULL sleep_time;
}sleepEvent;

ULL nsec(void);

sleepEvent* addSleepEvent(ULL wait_time);
	
void wake_up_now();
#endif

