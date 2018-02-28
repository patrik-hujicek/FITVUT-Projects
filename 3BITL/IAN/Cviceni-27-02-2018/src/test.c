#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>


void sigalarm_handler(int);
void register_sigalarm_handler(void);
void get_sig_registered_mask(sigset_t *);
void report_registered_signals(sigset_t *);

#define ALARM_INTERVAL 10u
#define SLEEP_INTERVAL (ALARM_INTERVAL * 2u)

#define LOG_FILE 	"/root/Cviceni-27-02-2018/logs/test.log"
#define SCRIPT_FILE "/root/Cviceni-27-02-2018/scripts/ping.sh"

#define SAVE(X) if((X) == -1)	\
					printf("(%s) failed! [%s]\n", #X, strerror(errno));

int main(char **argv, int argc) {

	int rc = 0;
	char cmd[512];
	unsigned int time_left = SLEEP_INTERVAL;
	time_t mytime = time(NULL);
	char *time_str = ctime(&mytime);
	sigset_t sigset;

	printf("Start - time: %s", time_str);	

	report_registered_signals(&sigset);
	register_sigalarm_handler();
	report_registered_signals(&sigset);
	
	printf("Arming the periodic alarm for [%u]s - done\n", ALARM_INTERVAL);
	SAVE(alarm(ALARM_INTERVAL))
	
	remove(LOG_FILE);

	do {
		snprintf(cmd, sizeof(cmd) - 1, "/bin/bash %s >> %s 2>&1", SCRIPT_FILE, LOG_FILE);
		printf("Executing: [%s], ret: [%d]\n", cmd, system(cmd));

		printf("Going to sleep for [%u]s\n", time_left);
		time_left = sleep(time_left);
		
		printf("Woken up ...\n", time_left);
	} while (time_left);	

	printf("End - time: %s", time_str);	
	return 0;
}

void sigalarm_handler(int signum) {
	sigset_t block_sigmask, old_sigmask;
	time_t mytime;
	char * time_str;

	SAVE(sigfillset(&block_sigmask))
	SAVE(sigprocmask(SIG_SETMASK, &block_sigmask, &old_sigmask))

	mytime = time(NULL);
	time_str = ctime(&mytime);

	printf("\nGot [%s] signal, signum[%u] time: %s", strsignal(signum), signum, time_str);
	register_sigalarm_handler();
	printf("Rearming for [%u]s, ret[%u] - done\n", ALARM_INTERVAL, alarm(ALARM_INTERVAL));	
	SAVE(sigprocmask(SIG_SETMASK, &old_sigmask, NULL))
}

void register_sigalarm_handler(void) {
	printf("Registering SIGALRM handler - %s\n", (signal(SIGALRM, sigalarm_handler) == SIG_ERR) ? strerror(errno) : "done");
}

void report_registered_signals(sigset_t *mask) {
	unsigned short int i;

	
	SAVE(sigemptyset(mask))
	get_sig_registered_mask(mask);

	printf("Registered signals: ");

	for(i = 1; i < NSIG; i++) 
		if(sigismember(mask, i)) 
			printf("%d [%s] ", i, strsignal(i));
	printf("\n");
}

void get_sig_registered_mask(sigset_t *mask) {
	struct sigaction sa;
	unsigned short int i;

	for(i = 1; i < NSIG; i++) {

		if(sigaction(i, NULL, &sa) == -1 && errno == EINVAL)
			continue;

		if(sa.sa_handler != SIG_IGN && sa.sa_handler != SIG_DFL)
			sigaddset(mask, i);		
	}
}
