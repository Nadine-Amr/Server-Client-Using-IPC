#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/shm.h>
#include <sys/sem.h>

int RECEIVED_MSG = 0;
void received(int signum) {
    RECEIVED_MSG = 1;
}

int shmid, sem1;
struct shmem {
    pid_t pid;
    char mtext[70];
};

union Semun
{
    int val;                /* value for SETVAL */
    struct semid_ds *buf;   /* buffer for IPC_STAT & IPC_SET */
    ushort *array;          /* array for GETALL & SETALL */
    struct seminfo *__buf;  /* buffer for IPC_INFO */
    void *__pad;
};

int create_sem(int key, int initial_value)
{
    union Semun semun;

    int sem = semget(key, 1, 0666|IPC_CREAT);

    if(sem == -1)
    {
        perror("Error in create sem");
        exit(-1);
    }

    semun.val = initial_value;  /* initial value of the semaphore, Binary semaphore */
    if(semctl(sem, 0, SETVAL, semun) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }
    
    return sem;
}

void destroy_sem(int sem)
{
    if(semctl(sem, 0, IPC_RMID) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }
}

void down(int sem)
{
    struct sembuf p_op;

    p_op.sem_num = 0;
    p_op.sem_op = -1;
    p_op.sem_flg = !IPC_NOWAIT;

    if(semop(sem, &p_op, 1) == -1)
    {
        perror("Error in down()");
        exit(-1);
    }
}

void up(int sem)
{
    struct sembuf v_op;

    v_op.sem_num = 0;
    v_op.sem_op = 1;
    v_op.sem_flg = !IPC_NOWAIT;

    if(semop(sem, &v_op, 1) == -1)
    {
        perror("Error in up()");
        exit(-1);
    }
}

/*convert upper case to lower case or vise versa */
void conv(char *msg) {
    int size = strlen(msg);
    for (int i = 0; i < size; i++) {
        if (islower(msg[i])) {
            msg[i] = toupper(msg[i]);
        }
        else if (isupper(msg[i])) {
            msg[i] = tolower(msg[i]);
        }
    }
}

void handler(int signum) {
    printf("Destroying shared memory and semaphore in server.\n");
    shmctl(shmid, IPC_RMID, (struct shmid_ds*)0);
    destroy_sem(sem1);
    exit(-1);
}

int main() {
    //Server code

    signal(SIGINT, handler);
    signal(SIGUSR1, received);

    sem1 = create_sem(32770, 0);
    shmid = shmget(32779, sizeof(struct shmem), IPC_CREAT|0644);

    if(shmid == -1)
    {
        perror("Error in creating shared memory.\n");
        exit(-1);
    }
    else 
    {
        printf("Shared memory created with ID = %d\n", shmid);
    }

    struct shmem *shmaddr = shmat(shmid, (void *)0, 0);
    if(shmaddr == -1)
    {
        perror("Error in attaching to shared memory in server.\n");
        exit(-1);
    }
    else
    {
        printf("Shared memory attached at address %x in server.\n\n", shmaddr);
    }

    shmaddr->pid = getpid();
    strcpy(shmaddr->mtext, "Server started successfully.\n");
    up(sem1);


    while(1) {

        printf("Waiting for conversion requests.\n\n");

        RECEIVED_MSG = 0;
        while (!RECEIVED_MSG) {
            sleep(20);
        }

        pid_t clientPID = shmaddr->pid;
        printf("\nRead from shared memory: %s, written by: %d\n", shmaddr->mtext, clientPID);

        shmaddr->pid = getpid();
        conv(shmaddr->mtext);
        printf("Message written to shared memory: %s\n", shmaddr->mtext);

        kill(clientPID, SIGUSR1);
    }

    return 0;
}