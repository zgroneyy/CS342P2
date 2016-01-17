#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include <errno.h> 
#include <pthread.h>
#define BUFFER_LENGTH 128
//Struct to keep the names of files to be processed
typedef struct FileNode
{
	char* filename;
	struct FileNode* next;
}FileNode;
typedef struct LineNode
{
	char line[BUFFER_LENGTH];
	struct LineNode* next;
}LineNode;
struct Reply
{
	LineNode* linelist;
};

//Struct the keep the data to be sent to server. 
struct Request
{
	FileNode* filelist;
	char keyword[64];
	char reply_queue_name[32];
};
int main(int argc, char **argv)
{
	char request_queue_name[128]; //request queue name
	char keyword[128]; //keyword 
	int N; //number of files
	int send;//a basic check integer whether message is send succesfully or not. 
	mqd_t reply, request; //queues
	
	char *bufptr;
	struct mq_attr mq_attr;
	int buflen;
	
	strcpy(request_queue_name, argv[1]);
	strcpy(keyword, argv[2]);
	N = atoi(argv[3]);
	if(N>10)
	{
		printf("%s", "You have written too many files");
		return 0;
	}
	//Files will be hold in a linked list structure, we will send pointer of head of this structure
	//when we send it to Server. 
	FileNode* head; 
	head=(FileNode*)malloc(sizeof(struct FileNode));
	head->filename=argv[4];
	head->next=NULL;
	
	//A FileNode pointer to go through the list.
	FileNode* curr;
	curr=head;
	//Here, we are getting the names of input files from commandline.
	int i;
	for(i=1;i<N;i++)
	{
		FileNode* newFile=(FileNode*)malloc(sizeof(struct FileNode));
		//argv[0],argv[1] & argv[2] are separated from other attributes. Should start with 3 and go through 3+N-1
		int j=4+i;
		newFile->filename=argv[j];
		curr->next=newFile;
		newFile->next=NULL;
		curr=curr->next;
	}
	
	//A queue that will be used in data sending, called "request"
	request=mq_open(request_queue_name, O_RDWR);
	if(request == -1)
	{
		perror("cannot open the request queue\n");
		exit(1);
	}
	
	//create unique reply queue name by using getpid() 
	char reply_queue_name[200] = "/repq_";
	char pidstr[10];
	sprintf(pidstr, "%d", getpid());
	strcat(reply_queue_name,pidstr);

	//Create a reply queue to be sent to server and get the processed data back
	reply = mq_open(reply_queue_name, O_RDWR | O_CREAT, 0666, NULL);
	if(reply == -1)
	{
		perror("cannot open the reply queue\n");
		exit(1);
	}
	//assign attributes of reply_queue. Whenever we get a reply, we need these attributes. 
	mq_getattr(request, &mq_attr);
	buflen = mq_attr.mq_msgsize;
	bufptr = (char *) malloc(buflen); 
	//We create a package to be sent to the server, includes list of files, keyword and reply queue name
	struct Request item; 
	item.filelist=head;
	strcpy(item.keyword, keyword);
	strcpy(item.reply_queue_name, reply_queue_name);
	
	//Sending data to server via request queue
	send=mq_send(request, (char*) &item, sizeof(struct Request), 0);
	//If it cannot send the data
	if(send==-1)
	{
		perror("send to message queue failed\n");
		exit(1);
	}
	//
	struct Reply *iPtr;
	while(1)
	{
		//try to receive
		send = mq_receive(reply, (char*) bufptr, buflen, NULL);
		//If it cannot send data successfully
		if(send==-1)
		{
			perror("recieve to client message queue failed\n");
			exit(1);
		}
		iPtr = (struct Reply *) bufptr;
	}
	while(iPtr->linelist!=NULL)
		printf("%s\n", iPtr->linelist->line);
	mq_close(request);
	mq_unlink(reply_queue_name);
	return 0;
}
