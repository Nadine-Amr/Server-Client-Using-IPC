#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>

key_t msgqidUp, msgqidDown;

struct msgbuff {
    long mtype;
    char mtext[70];
};

void handler(int signum) {
    int rem_msg;

    rem_msg = msgctl(msgqidUp, IPC_RMID, (struct msqid_ds *) 0);
    if (rem_msg == -1) {
        perror("Error in deleting UP queue.\n");
    }
    else {
        printf("UP queue deleted.\n");
    }

    rem_msg = msgctl(msgqidDown, IPC_RMID, (struct msqid_ds *) 0);
    if (rem_msg == -1) {
        perror("Error in deleting DOWN queue.\n");
    }
    else {
        printf("DOWN queue deleted.\n");
    }
    exit(-1);
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

int main() {
    // Server code
    signal(SIGINT, handler);
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

    while(1) {
        rec_val = msgrcv(msgqidUp, &UpMessage, 256, 0, !IPC_NOWAIT);

        if (rec_val == -1) {
            perror("Error in receiving from UP queue.\n");
        }
        else {
            printf("\nMessage received from UP queue: %s, with type: %ld\n", UpMessage.mtext, UpMessage.mtype);

            conv(UpMessage.mtext);

            DownMessage.mtype = UpMessage.mtype;
            strcpy(DownMessage.mtext, UpMessage.mtext);

            send_val = msgsnd(msgqidDown, &DownMessage, 256, !IPC_NOWAIT);

            if (send_val == -1) {
                perror("Error in sending to DOWN queue.\n");
            }
            else {
                printf("Message sent to DOWN queue: %s, with type: %ld\n", DownMessage.mtext, DownMessage.mtype);
            }
        }
    }
    return 0;
}