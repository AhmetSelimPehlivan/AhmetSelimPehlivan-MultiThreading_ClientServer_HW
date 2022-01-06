#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/msg.h>

struct mesg_buffer {
    long mesg_type;
    char mesg_text[1];
    pid_t mes_pid;
} message;

struct Memory {
    int matrix1[50];
    int matrix2[50];
    int matrixResult[50];
};

void print(int matrix[50]){ // Print matrix by its size
        printf("%d %d",matrix[0],matrix[1]);
    for (int i = 0; i < matrix[0]*matrix[1]; i++){
        if (i%matrix[1]==0)
            printf("\n");
        printf("%d ",matrix[i+2]);
    }
}
int main(int argc, char const *argv[])
{
    FILE* fp;
    key_t key;
    key_t ShmKEY;
    int ShmID;
    int msgid;
    int * ShmResult;
    char FilePath1[50],FilePath2[50];
    int matrix1[100],matrix2[100];
    int matrix1Len=0,matrix2Len=0;
    struct Memory *ShmPTR;
   
	printf("First File Path : \n");
    scanf("%s",FilePath1);
	fp = fopen (FilePath1, "r");    //Get File name and put values into matrix1 local array
    int next=0,i=0;
        while (fscanf(fp, "%d",&matrix1[i++])!=-1)
            matrix1Len=i;
	fclose(fp);

	printf("Second File Path : \n");
    i=0;
    scanf("%s",FilePath2);          //Get File name and put values into matrix2 local array
	fp = fopen (FilePath2, "r");
        while (fscanf(fp, "%d",&matrix2[i++])!=-1)
            matrix2Len=i;
	fclose(fp);
    
    if(!(matrix1[0]==matrix2[1] && matrix1[1]==matrix2[0])){    // Control First 2 values of matrix arrays because of propert multiplications of matrix
        perror("Matrix Size");
        exit(1);
    }

    message.mes_pid= getpid();
    if((key=ftok("client.c",'b')) == -1){   // ftok to generate unique key
		perror("key");
		exit(1);   
	}
    // msgget creates a message queue
    // and returns identifier
    if((msgid = msgget(key, 0666 | IPC_CREAT)) == -1){
        perror("msgid");
        exit(1);
    }
    message.mesg_type = 1;
    message.mesg_text[0]='1'; // Ready Massage

    // msgsnd to send message
    if((msgsnd(msgid, &message, sizeof(message), 0)) == -1){
        perror("msgsnd");
        exit(1);
    }

    //
    if((ShmKEY =ftok("server.c",message.mes_pid)) == -1){
        perror("ftok shared");
        exit(1);
    }
        
    if((ShmID = shmget(ShmKEY,1024,0666 | IPC_CREAT)) == -1){
        perror("shmget");
        exit(1);
    }
    printf("shmKey is %d\n",ShmID);
    if((ShmPTR = (struct Memory *) shmat(ShmID,NULL,0)) == NULL){
            perror("shmat");
            exit(1);
    }
    //Put matrixes to our shared matrix in shared memory structure
    for (int i = 0; i < matrix1Len; i++)
        ShmPTR->matrix1[i]=matrix1[i];
    for (int i = 0; i < matrix2Len; i++)
        ShmPTR->matrix2[i]=matrix2[i];
            

    if((msgrcv(msgid, &message, sizeof(message), 1, 0)) == -1){
        perror("msgrcv");
        exit(1);
    }
    if (message.mesg_text[0] == '2') // message is succesfull getted
        print(ShmPTR->matrixResult); // get result matrix from server in this step

    return 0;
}