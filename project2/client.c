/*
** Özgür Öney
** CS342 Project 2
** Assoc. Prof. Özcan Öztürk
** 21101821
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include <errno.h> 
#define SIZE 256
int main(int argc, char **argv)
{

	mqd_t mq, c_mq;
	int buflen;
	int n;
	char* temp;
	char allocator[SIZE+1];
	//memset(allocator,0,sizeof(allocator));//a basic allocator for bufptr, to prevent segmentation fault comes from string manipulation.
	struct mq_attr attr = {0}; 

	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = sizeof(allocator);
	attr.mq_curmsgs = 0;
	//a line of parameters to be send to server. " " will be the delimiter.
	char *bufptr = &allocator[0];
	//Getting necessary information into a char* buffer
	temp=argv[1];
	strcpy(allocator, temp); //request queue name
	strcat(allocator, " ");
	temp=argv[2];
	strcat(allocator, temp); //keyword
	strcat(allocator, " ");
	temp=argv[3];
	strcat(allocator, temp);//number of files
	strcat(allocator, " ");
	int i;
	int another=atoi(argv[3]);
	int count=0;
	//Getting input file names from command line and add it to the our line.
	for(i=0; i<another;i++)
	{
		strcat(allocator, " ");
		count=4+i;
		strcat(allocator, argv[count]);
	}
	//Open message queue
	char *sqname=argv[1];
	//We are writer process as client.
	mq = mq_open(sqname, O_WRONLY);
	if(mq == -1)
	{
		perror("cannot open the message queue\n");
		exit(1);
	}
	//create unique client queue name
	char client_queue_name[200] = "/cqueue_";
	char pidstr[10];
	sprintf(pidstr, "%d", getpid());
	strcat(client_queue_name,pidstr);
	
	//Open reply queue
	c_mq = mq_open(client_queue_name, O_CREAT|O_RDWR, 0644, &attr);
	if(c_mq == -1)
	{
		perror("cannot open the client message queue\n");
		exit(1);
	}
	//add name of reply queue to the char buffer
	strcat(allocator," ");
	strcat(allocator,client_queue_name);
	//NOW WE HAVE <request_queue_name keyword #offiles inp1 inp2 ... reply_queue_name> structure
	//Send data 
	n = mq_send(mq, allocator, sizeof(allocator), 0);
	//If cannot send
	if(n==-1)
	{
		perror("send to message queue failed\n");
		exit(1);
	}
	
	fprintf(stderr, "successful mq_send from client %d\n",getpid());
	//wait for a reply from server
	while(1)
	{
		n = mq_receive(c_mq, (char*) bufptr, buflen, NULL);
		if(n==-1)
		{
			perror("recieve to client message queue failed\n");
			exit(1);
		}
		else
		{
			printf("Message received.");
			printf("%s", bufptr);
		}	
	}
	fprintf(stderr,"Successfull end");
	mq_close(mq);
	mq_unlink(client_queue_name);
	

	return 0;
}
