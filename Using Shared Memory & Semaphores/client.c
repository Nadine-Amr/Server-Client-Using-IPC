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


struct shmem *shmaddr;
struct shmem {
    pid_t pid;
    char mtext[70];
};

void handler(int signum) {
    printf("Detaching shared memory in client.\n");
    shmdt(shmaddr);
    exit(-1);
}

union Semun
{
    int val;                /* value for SETVAL */
    struct semid_ds *buf;   /* buffer for IPC_STAT & IPC_SET */
    ushort *array;          /* array for GETALL & SETALL */
    struct seminfo *__buf;  /* buffer for IPC_INFO */
    void *__pad;
};

int create_sem(int key)
{
    union Semun semun;

    int sem = semget(key, 1, 0666|IPC_CREAT);

    if(sem == -1)
    {
        perror("Error in create sem");
        exit(-1);
    }
    
    return sem;
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

int main() {
    //Client code

    signal(SIGINT, handler);
    signal(SIGUSR1, received);

    int sem1 = create_sem(32770);

    int shmid = shmget(32779, sizeof(struct shmem), IPC_CREAT|0644);
    if(shmid == -1)
    {
        perror("Error in creating shared memory.\n");
        exit(-1);
    }
    else 
    {
        printf("Shared memory created with ID = %d\n", shmid);
    }

    shmaddr = shmat(shmid, (void *)0, 0);
    if(shmaddr == -1)
    {
        perror("Error in attaching to shared memory in client.\n");
        exit(-1);
    }
    else
    {
        printf("Shared memory attached at address %x in client.\n", shmaddr);
    }

    while(1) {
        printf("\nPlease enter message to be converted:\n");
        char toBeConverted[70];
        scanf("%s", toBeConverted);


        down(sem1);
        pid_t serverPID = shmaddr->pid;

        shmaddr->pid = getpid();
        strcpy(shmaddr->mtext, toBeConverted);
        printf("Message written to shared memory: %s\n", shmaddr->mtext);

        kill(serverPID, SIGUSR1);

        RECEIVED_MSG = 0;
        while (!RECEIVED_MSG) {
            sleep(20);
        }

        printf("Message read from shared memory: %s, written by: %d\n\n", shmaddr->mtext, shmaddr->pid);
        up(sem1);
    }
    return 0;
}