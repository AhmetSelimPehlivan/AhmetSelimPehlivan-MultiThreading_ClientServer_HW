#include <stdio.h>
#include <pthread.h>
#include <sys/msg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#define NOT_READY -1
#define FILLED 0
#define TAKEN 1
#define NUM_THREAD 10

struct mesg_buffer {
   long mesg_type;
   char mesg_text[1];
   pid_t mesg_pid;
} message;

struct Memory {
	int matrix1[50];
	int matrix2[50];
	int matrixResult[50];
};

void *thread(void* args) {
	// Run Code
    key_t ShmKEY;
    int ShmID, i;
	int pid =((int*)args)[0];
	int msgid =((int*)args)[2];
    struct Memory *ShmPTR;
		if((ShmKEY = ftok("server.c", pid)) == -1){
			perror("ftok shared");
			exit(1);
		}
		if((ShmID = shmget(ShmKEY, 1024, IPC_CREAT | 0666)) == -1){
			perror("shmget");
			exit(1);
		}
		
		if((ShmPTR = (struct Memory *) shmat(ShmID, NULL, 0)) == NULL){
			perror("shmat");
			exit(1);
		}
		int matrix1ColSize = ShmPTR->matrix1[1];
		int matrix1RowSize = ShmPTR->matrix1[0];
		ShmPTR->matrixResult[0] = matrix1RowSize;
		ShmPTR->matrixResult[1] = matrix1RowSize;
		for(int i=0;i<matrix1RowSize;i++)    
			for(int j=0;j<matrix1RowSize;j++)
				for(int k=0;k<matrix1ColSize;k++){
					ShmPTR->matrixResult[((i*matrix1RowSize)+j)+2]+=ShmPTR->matrix1[((i*matrix1ColSize)+k)+2] * ShmPTR->matrix2[((k*matrix1RowSize)+j)+2];
  				}
		  message.mesg_text[0]='2'; // Ready Massagemessage is 2 that meanings matrix is multiplied succesfully
		if((msgsnd(msgid, &message, sizeof(message), 0)) == -1){	// send message to client
				perror("msgsnd");
				exit(1);
		}
		
		shmdt((void *) ShmPTR);
		shmctl(ShmID, IPC_RMID, NULL);
    return 0;
}

int main(int argc, char const *argv[])
{
    pthread_t tid[NUM_THREAD];
    key_t key;
    int msgid,i=0;
    while(i!=NUM_THREAD) {
         // ftok to generate unique key
		if((key=ftok("client.c",'b')) == -1){
			perror("key");
			exit(1);   
		}

		if((msgid = msgget(key, 0666 | IPC_CREAT)) == -1){
			perror("msgid");
			exit(1);
		}
		// msgrcv to receive message
		if((msgrcv(msgid, &message, sizeof(message), 1, 0)) == -1){
			perror("msgrcv");
			exit(1);
		}
		else{
			if(message.mesg_text[0] == '1'){
				int arr[3] = {message.mesg_pid,atoi(message.mesg_text),msgid};
				if ((pthread_create(&tid[i++],NULL,thread,(void *)arr)) != 0) {
					perror("thcreate");
					exit(1);
				}
			}
		}
   }
    return 0;
}
