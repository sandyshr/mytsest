#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "thpool.h"
#include "scanner.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


TargetList *my_target_list;

void ping_check(void *arg) {
    //TargetList *target_list = (TargetList *) arg;
    // for (int i = 0; i < target_list->target_count; i++) {
        Target *target = (Target *)arg;
        if (target->status == UNKNOWN) {
            char cmd[256];
            sprintf(cmd, "ping -c 1 -w 1 %s > /dev/null", inet_ntoa(target->ip_addr));
            FILE *fp = popen(cmd, "r");
            if (fp == NULL) {
                printf("Failed to execute command for %s\n", inet_ntoa(target->ip_addr));
                //continue;
            }
            int ret = pclose(fp);
            if (ret == 0) {
				pthread_mutex_lock(&mutex);
                target->status = UP;
                printf("%s is UP\n", inet_ntoa(target->ip_addr));
				    pthread_mutex_unlock(&mutex);

            } else {
				    pthread_mutex_lock(&mutex);

                target->status = DOWN;
                printf("%s is DOWN\n", inet_ntoa(target->ip_addr));
				    pthread_mutex_unlock(&mutex);

            }
        }
    //}
}


int main(int argc, char* argv[]){
    char ip_input[BUF_SIZE] = "192.168.127.128/26";
    my_target_list = get_target_list(ip_input);

    puts("Making threadpool with 4 threads");
    threadpool thpool = thpool_init(8);

    puts("Adding tasks to threadpool");
    for (int i = 0; i < my_target_list->target_count; i++){
        thpool_add_work(thpool, ping_check, &(my_target_list->targets[i]));
    }

    thpool_wait(thpool);
    puts("Killing threadpool");
    thpool_destroy(thpool);
    print_target_list(my_target_list);

    return 0;
}
