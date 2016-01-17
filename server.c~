#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <pthread.h>
#define BUFFER_LENGTH 128
typedef struct FileNode
{
	char* filename;
	struct FileNode* next;
}FileNode;
typedef struct LineNode
{
	int count;
	int linenumber;
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
LineNode* linelist; 
int lineSearch(char keyword[], char line[])	//searches the given line and finds how many exact keyword matches in the line
{
    //make it char* so that char comparison is easier
    char *str = &line[0];
    char *key = &keyword[0];
    size_t keylength = strlen(key);

	//number of occurances
    size_t count = 0;
    char *ptr = str;

	//as long as there are keywords left find them
    while ((ptr = strstr(ptr, key)) != NULL)
    {
    	//look ahead and compare if both end and start of the keyword is whitespace
        char *q = ptr + keylength;
        if (ptr == str || isblank((unsigned char) *(ptr - 1)))
        {
            if (*q == '\n' || isblank((unsigned char) *q))
            {
            	count++;
            }
        }
        ptr = q;
    }

    return count;
}
int linecontains(char* line, char* keyword)
{

	char* result = strstr(line,keyword);//If line contains, return 

	if(result == NULL)
		return 0;

	return 1;

}
void assignThread(char keyword[],FileNode* node)
{
	char word[BUFFER_LENGTH]; 
	int linenumber=0;//
	FILE *fp = fopen(node->filename,"r");
	LineNode* list;
	int index = 0;
	while(1)
	{
	    int ch;
		//Read line
		while((ch = fgetc(fp)) != '\n' && ch != -1 && index < BUFFER_LENGTH)
		{
			word[index++] = ch;
		}
		word[index] = '\0';
		
		index = 0;
		linenumber++;
		
		if (word[0] != '\0')
		{
			if (linecontains(word,keyword) == 1)
			{
				if(list==NULL)
				{
					list=(LineNode*)malloc(sizeof(struct LineNode));
					list->count=lineSearch(keyword,word);
					list->linenumber=linenumber;
					list->next=NULL;
				}
				else
				{
					LineNode* curr=list;
					while(curr->next!=NULL)
						curr=curr->next;
					LineNode* newLine=(LineNode*)malloc(sizeof(struct LineNode));
					newLine->count=lineSearch(keyword, word);
					newLine->linenumber=linenumber;
					curr->next=newLine;
					newLine->next=NULL;
				}
			}
		}
					
		//When filereader sees end of the document, it will be stopped 
		if(feof(fp))
		   break;
	}
	list=linelist;
	fclose(fp);
	//childrenAmountOperation(DEC);
	/*
	LineNode* someth=list;
	int totalcount;
	while(someth!=NULL)
		totalcount=totalcount+list->count;
		
	someth=list;
	fprintf(stdout, "< \"%s\" > ",node->filename);
	fprintf(stdout, "[ \"%d\" ]: ",totalcount);
	while(someth!=NULL)
	{
		//Prints as much as how many times word occurs on a single line.
		int i=0;
		for(i=0;i<list->count;i++)
			fprintf(stdout, " \"%d\" ",someth->linenumber);
	}*/
}
void* InitiateThread(void* temp)
{
	struct Request* final;
	final=temp;
	assignThread(final->keyword, final->filelist);
	pthread_exit(NULL);
    	return NULL;
}
int main(int argc, char **argv)
{
	char request_queue_name[128];
	mqd_t request;
	int send;
	int clients=0; //Since maximum 5 clients are allowed, we have to count them.
	char *bufptr;
	struct mq_attr mq_attr;
	memset(&mq_attr, '\0', sizeof(mq_attr));
	int buflen;

	struct Request *iPtr;
	//Basic check whether you use <req-qname> or not
	if(argc != 2)
	{
		perror("PLEASE USE: server <sqname>\n");
		exit(1);
	}
	
	//A message queue with specified name is opened.
	strcpy(request_queue_name,argv[1]);
	request = mq_open(request_queue_name, O_RDWR | O_CREAT, 0666, NULL);
	
	if(request == -1)
	{
		perror("SERVER: Cannot open the message queue\n");
		exit(1);
	}

	mq_getattr(request, &mq_attr);
	buflen = mq_attr.mq_msgsize;
	bufptr = (char *) malloc(buflen); 

	fprintf(stdout, "SERVER: Message que opened with the id: %d\n",(int)request);
	pid_t pid;
	while(1)
	{	
		if(clients<=5)
		{
			send = mq_receive(request, (char*) bufptr, buflen, NULL);
			if(send==-1)
			{
				perror("recieve to message queue failed\n");
				exit(1);
			}
			else{
				printf("SERVER: Client with m.queue name %s did a new request with keyword %s",iPtr->reply_queue_name,iPtr->keyword);
				clients++;
				iPtr = (struct Request *) bufptr;
				pid=fork();
				//pid<0 indicates that we cannot create a requested child process
				if (pid < 0)
				{
					exit(EXIT_FAILURE);
				}
				//pid=0 indicates we have succesfully created our child process.
				else if(pid == 0)
				{
					
					char* word=iPtr->keyword;
					int count=0; //count of files to create sufficient # of threads.
					int k;
					FileNode* curr;
					curr=iPtr->filelist;
					while(curr!=NULL)
					{
						count++;
						curr=curr->next;
					}
					struct Request temp;
					strcpy(temp.keyword, word);
					temp.filelist=iPtr->filelist;
					pthread_t threadnumber[count]; //Array of threads
					int i;
					for(i=0; i<count;i++)
					{
						pthread_create(&threadnumber[i], NULL, &InitiateThread, (void *)&temp);
						temp.filelist=temp.filelist->next;
					}
					i=0;
					for(i=0; i<count;i++)
						pthread_join(threadnumber[i],NULL);
					}
			}
		}
	}
	struct Reply tobesent; 
	tobesent.linelist=linelist;
	mqd_t reply = mq_open(iPtr->reply_queue_name, O_RDWR);
	if(reply == -1)
	{
		perror("cannot open client message queue\n");
		exit(1);
	}
	//Sending data to client via reply queue
	send=mq_send(reply, (char*) &tobesent, sizeof(struct Reply), 0);
	//If it cannot send the data
	if(send==-1)
	{
		perror("send to message queue failed\n");
		exit(1);
	}
	mq_close(request);
	fprintf(stdout, "SERVER: Server exit....\n");
	return 0;
}
