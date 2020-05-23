#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>

key_t msgqidUp, msgqidDown;

struct msgbuff {
    long mtype;
    char mtext[70];
};

int main() {
    //Client code

    int mypid;
    mypid = getpid();
    struct msgbuff UpMessage, DownMessage;
    int rec_val, send_val;

    msgqidUp = msgget(12613, IPC_CREAT | 0644);
    if (msgqidUp == -1) {
        perror("Error in creating UP queue.\n");
        exit(-1);
    }
    printf("UP msgqid = %d\n", msgqidUp);

    msgqidDown = msgget(12614, IPC_CREAT | 0644);
    if (msgqidDown == -1) {
        perror("Error in creating DOWN queue.\n");
        exit(-1);
    }
    printf("DOWN msgqid = %d\n", msgqidDown);

    UpMessage.mtype = mypid%10000;
    printf("My type: %d\n", mypid%10000);

    while(1) {
        printf("\nPlease enter message to be sent to server:\n");
        scanf("%s", UpMessage.mtext);

        send_val = msgsnd(msgqidUp, &UpMessage, 256, !IPC_NOWAIT);
        if (send_val == -1) {
            perror("Error in sending to UP queue.\n");
        }
        else {
            printf("Message sent to UP queue: %s\n", UpMessage.mtext);
        }


        rec_val = msgrcv(msgqidDown, &DownMessage, 256, (mypid%10000), !IPC_NOWAIT);
        if (rec_val == -1) {
            perror("Error in receiving from DOWN queue.\n");
        }
        else {
            printf("Message received from DOWN queue: %s\n", DownMessage.mtext);
        }
    }
    return 0;
}