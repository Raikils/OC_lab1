#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include "trialfuncs.h"

#define MAX_SOFT_TRIE 5

int function_type = 0;

void f_func(int x,int fd[2]);
void g_func(int x,int fd[2]);
int choose_function_type();
void manager(int x);

int main()
{
    printf("To stop the program press e\n");
    printf("To start the program enter a number\n");
    function_type = choose_function_type();
    printf("To change function type press t\n");
    while(true) {
        int x = 0;
        char s[100];
        scanf("%s", &s);
        if (strlen(s) == 1 && s[0] == 'e') break;
        if (strlen(s) == 1 && s[0] == 't') {
            function_type = choose_function_type();
            continue;
        }
        int start = 0;
        bool wrong_input = false;
        if (s[0] == '-') start++;
        for (int i = start; i < strlen(s); i++) {
             if (s[i] >= '0' && s[i] <= '9') x += (s[i] - '0') * pow(10, (strlen(s) - i - 1));
             else{ wrong_input = true; break;}
        }
        if (wrong_input) continue;
        if (start) x *= -1;
        manager(x);
    }
    return 0;
}

void manager(int x)
{
    int fd[2][2];
        pipe(fd[0]);
        int id1 = fork();
        if (id1 == 0) {
            f_func(x, fd[0]);
        } else {
            pipe(fd[1]);
            int id2 = fork();
            if (id2 == 0) {
            g_func(x, fd[1]);
        } 
    else 
    {    
    close(fd[1][1]);
    close(fd[0][1]);
    fd_set master, tset;
    int nfd, f_status = -1, g_status = -1;
    int f_result, g_result;
    char tc;
    bool cancellation_proposed = false;
    bool time_out1 = true;
    FD_ZERO(&tset);
    FD_SET(STDIN_FILENO, &tset);
    FD_SET(fd[0][0], &tset);
    FD_SET(fd[1][0], &tset);
    nfd = fd[1][0]+1;
    master = tset;
    struct timeval *t;
    struct timeval t1 = {20, 0};
    t = &t1;
    while(true) {
        select(nfd, &master, NULL, NULL, t);
        time_out1 = true;
        if (FD_ISSET(STDIN_FILENO, &master)) {
            time_out1 = false;
            read(STDIN_FILENO, &tc,1);
            if (!cancellation_proposed && tc == 'e') {
                cancellation_proposed = true;
                printf("Please confirm that computation should be stopped y(es, stop)/n(ot yet)[n]\n");
                t1.tv_sec = 5;
                t = &t1;
            }
            if (cancellation_proposed && tc == 'y') {
                kill(id2, SIGSTOP);
                kill(id1, SIGSTOP);
                return;
            }
            if (cancellation_proposed && tc == 'n') {
                cancellation_proposed = false;
                printf("Computation continued\n");
            }
        }
        if (f_status == -1 && FD_ISSET(fd[0][0], &master)) {
            time_out1 = false;
            read(fd[0][0], &f_status, sizeof(int));
            if (f_status == 0) {
                read(fd[0][0], &f_result, sizeof(int));
            } else {
            }
        }
        if (g_status == -1 && FD_ISSET(fd[1][0], &master)) {
            time_out1 = false;
            read(fd[1][0], &g_status, sizeof(int));
            if (g_status == 0) {
                read(fd[1][0], &g_result, sizeof(int));
            } else {
            }
        }
        if (!cancellation_proposed && time_out1) {
            break;
        }
        if (cancellation_proposed && time_out1) {
            cancellation_proposed = false;
            printf("action is not confirmed within <time>. proceeding...\n");
            t1.tv_sec = 20;
            t = &t1;
        }
        if (f_status != -1 && g_status != -1) break;
        else master = tset;
    }
    FD_ZERO(&tset);
    FD_ZERO(&master);
    wait(NULL);
    if (!cancellation_proposed && time_out1) {
        printf("run out of time\n");
    } else if (!f_status && !g_status) {
        printf("Result: %d\n", f_result + g_result);
    } else {
        printf("Computetion fails\n");
        switch (f_status) {
            case 0: printf("Function f had result %d", f_result);
            case 1: printf("Function f had soft fail %d times", MAX_SOFT_TRIE);
            case 2: printf("Function f had hard fail");
        }
        printf("\n");
        switch (g_status) {
            case 0: printf("Function g had result %d", g_result);
            case 1: printf("Function g had soft fail %d times", MAX_SOFT_TRIE);
            case 2: printf("Function g had hard fail");
        }
        printf("\n");
    }
    kill(id2, SIGSTOP);
    }
    kill(id1, SIGSTOP);
    }
}

void f_func(int x,int fd[2])
{
    close(fd[0]);
    compfunc_status_t status;
    int result, stat;
    switch (function_type) {
        case 0: status = trial_f_imul(x, &result);
        case 1: status = trial_f_fmul(x, &result);
        case 2: status = trial_f_imin(x, &result);
        case 3: status = trial_f_fmin(x, &result);
        case 4: status = trial_f_and(x, &result);
        case 5: status = trial_f_or(x, &result);
    }
    if (status == 0) {
        stat = 0;
        write(fd[1], &stat, sizeof(int));
        write(fd[1], &result, sizeof(int));
    } else {
        if (status == 1) {
            for (int i = 0; i < MAX_SOFT_TRIE - 1; i++) {
                switch (function_type) {
                case 0: status = trial_f_imul(x, &result);
                case 1: status = trial_f_fmul(x, &result);
                case 2: status = trial_f_imin(x, &result);
                case 3: status = trial_f_fmin(x, &result);
                case 4: status = trial_f_and(x, &result);
                case 5: status = trial_f_or(x, &result);
                }
                if (status == 0) break;
            }
            if (status == 0) {
                stat = 0;
                write(fd[1], &stat, sizeof(int));
                write(fd[1], &result, sizeof(int));
            } else {
                stat = 1;
                write(fd[1], &stat, sizeof(int));
            }
        } else {
            stat = 2;
            write(fd[1], &stat, sizeof(int));
        }
    }
    exit(0);
}

void g_func(int x,int fd[2])
{
    close(fd[0]);
    compfunc_status_t status;
    int result, stat;
    switch (function_type) {
        case 0: status = trial_g_imul(x, &result);
        case 1: status = trial_g_fmul(x, &result);
        case 2: status = trial_g_imin(x, &result);
        case 3: status = trial_g_fmin(x, &result);
        case 4: status = trial_g_and(x, &result);
        case 5: status = trial_g_or(x, &result);
    }
    if (status == 0) {
        int stat = 0;
        write(fd[1], &stat, sizeof(int));
        write(fd[1], &result, sizeof(int));
    } else {
        if (status == 1) {
            for (int i = 0; i < MAX_SOFT_TRIE - 1; i++) {
                switch (function_type) {
                case 0: status = trial_g_imul(x, &result);
                case 1: status = trial_g_fmul(x, &result);
                case 2: status = trial_g_imin(x, &result);
                case 3: status = trial_g_fmin(x, &result);
                case 4: status = trial_g_and(x, &result);
                case 5: status = trial_g_or(x, &result);
                }
                if (status == 0) break;
            }
            if (status == 0) {
                stat = 0;
                write(fd[1], &stat, sizeof(int));
                write(fd[1], &result, sizeof(int));
            } else {
                stat = 1;
                write(fd[1], &stat, sizeof(int));
            }
        } else {
            stat = 2;
            write(fd[1], &stat, sizeof(int));
        }
    }
    exit(0);
}

int choose_function_type()
{
    printf("Choose functio type(imul, fmul, imin,fmin,and,or)\n");
    while(true) {
        char s[100];
        scanf("%s", &s);
        if (!strcmp(s, "imul")) {
            return 0;
        }
        if (!strcmp(s, "fmul")) {
            return 1;
        }
        if (!strcmp(s, "imin")) {
            return 2;
        }
        if (!strcmp(s, "fmin")) {
            return 3;
        }
        if (!strcmp(s, "and")) {
            return 4;
        }
        if (!strcmp(s, "or")) {
            return 5;
        }
    }  
}