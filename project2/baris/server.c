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
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <ctype.h>
#include  <sys/types.h>
#define sm_name "/shared_mem" 
#define SIZE 256
//A structure that holds the necessary information to pass through threads.
struct thread_data{
   char* keyword;
   char* filename;
   char* result;
};

//Simple method checks whether a word is in a particular line or not. 
int linecontains(char* line, char* keyword)
{

	char* result = strstr(line,keyword);

	if(result == NULL)
		return 0;

	return 1;

}

//Method that finds & returns the number of keywords in a particular line
int lineSearch(char line[], char keyword[])	
{
	//make it char* so that char comparison becomes easier
	char *str = &line[0];
	char *key = &keyword[0];
	size_t keylength = strlen(key);

	//number of occurances
	size_t count = 0;
	char *ptr = str;

	//if there are keywords, we have to find number of occurence
	while (linecontains(ptr,key))
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
int AssignThread(char* filename, char* keyword, char* result)
{
	//Open given file with filename taken
	FILE *fp = fopen (filename, "r" );
	
	//Line number
	int linenumber=0;
	int lines[4096];
	//Occurence of keyword in whole documents
	int occur=0;
	int index = 0;
	
	//Char array to get a single line
	char word[SIZE];
	
	while(1)
	{
	    int ch;
		//Read single line
		while((ch = fgetc(fp)) != '\n' && ch != -1 && index < SIZE)
		{
			word[index++] = ch;
		}
		word[index] = '\0';
		
		index = 0;
		linenumber++;
		//If a line is not an empty line
		if (word[0] != '\0')
		{
			//If you see an occurrence
			if (lineSearch(word,keyword) > 0)
			{
				//Increase number of occurrences
				occur=occur+lineSearch(word,keyword);
				//Store the line number
				int j=0;
				for(j=0;j<lineSearch(word,keyword);j++)
				{
					lines[j]=linenumber;
				}
					
			}
		}	
		//When filereader sees end of the document, it will be stopped 
		if(feof(fp))
		   break;
	}
	//Save the filename,occurence number and occurence lines in wanted manner.
	strcat(result, "<");
	strcat(result, filename);
	strcat(result, ">");
	strcat(result, " ");
	strcat(result, "[");
	char* occurtostr;
	sprintf(occurtostr, "%d", occur);
	strcat(result, occurtostr);
	strcat(result, "]");
	strcat(result, " ");
	for(index=0;index<occur;index++)
	{
		sprintf(occurtostr, "%d", lines[index]);
		strcat(result, occurtostr);
	}
}

//Member function to initialize a thread, since threads are not available to be used directly.
void *InitializeThread(void *threadarg)	//thread passing function
{
	struct thread_data *my_data;
	my_data = (struct thread_data *) threadarg;
	//Call the function that does the "real" work.
    	AssignThread(my_data->filename,my_data->keyword, my_data->result);    
    	pthread_exit(NULL);
    	return NULL;
}
int main(int argc, char *argv[])
{
	//We get the name of request channel
	char* sqname=argv[1];
	//Message queue's that we have to implement. 
	mqd_t mq, c_mq;
	int n;
	char allocator[SIZE+1];
	memset(allocator,0,sizeof(allocator));
	char *bufptr;
	int buflen;

	struct mq_attr attr = {0}; 

	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = sizeof(allocator);
	attr.mq_curmsgs = 0;
	
	//If we cannot initialize server because of command line parameter missing.
	if(argc != 2)
	{
		perror("PLEASE USE: server <sqname>\n");
		exit(1);
	} 
	//Open the request message queue
	mq = mq_open(sqname, O_RDWR | O_CREAT, 0666, &attr);
	//Error situation.
	if(mq == -1)
	{
		perror("SERVER: Cannot open the message queue\n");
		exit(1);
	}

	fprintf(stdout, "SERVER: Message que opened with the id: %d\n",(int)mq);
	//Wait for data that will come from the client, server works infinetely..
	while(1)
	{
		n = mq_receive(mq, allocator, sizeof(allocator), NULL);
		if(n==-1)
		{
			perror("recieve to message queue failed\n");
			exit(1);
		}
		
		int n;
		//We have the data taken from client, now we need to assign a child to work on it.
		int pidID = fork();
		if(pidID < 0)
		{
			perror("SERVER: fork failed: ");
			exit(1);
		}
		else if(pidID>0)
		{
			//If parent...
			return;
		}
		//Means pidID=0
		else
		{
			fprintf(stdout,"SERVER: A new child ( %d )\n", getpid());
			fprintf(stderr,"our files are received by SERVER as: %s\n",allocator);
			//We send data in a form TTT YYY ZZZ GGG etc. So, " " is our delimiter to use strtok()
			char* request_queue_name;
			const char s[2] = " ";
			request_queue_name=strtok(bufptr,s);
			printf("%s\n", request_queue_name);
			char* keyword;
			keyword=strtok(bufptr,s);
			printf("%s\n", keyword);
			int numberoffiles;
			char* somenumber;
			somenumber=strtok(bufptr,s);
			numberoffiles=atoi(somenumber);
			printf("%d\n", numberoffiles);
			char inputs[numberoffiles];
			int i=0;
			for(i=0;i<numberoffiles;i++)
				inputs[i]=*(strtok(bufptr,s));
			char* reply_queue_name=strtok(bufptr,s);
			/*
			NOW WE HAVE!
			request_queue_name
			keyword
			(int)numberoffiles
			inputs[] where name of inputs are hold
			reply_queue_name
			*/
			struct thread_data thread_data_array[numberoffiles]; //array to pass parameters to thread
			for(i=0;i<numberoffiles;i++)
			{
				thread_data_array[i].keyword=keyword;
				thread_data_array[i].filename=&(inputs[i]);
				strcpy(thread_data_array[i].result,"");
			}
			pthread_t threads[numberoffiles];
			for(i=0;i<numberoffiles;i++)
			{
				pthread_create(&threads[i],NULL, &InitializeThread, (void *)&thread_data_array[i]);
			}
			for(i=0;i<numberoffiles;i++)
			{
				pthread_join(threads[i], NULL);
				sleep(1);
			}
			char* item;
			for(i=0;i<numberoffiles;i++)
			{
				strcat(item,thread_data_array[i].result);
				strcat(item," ");
			}
			c_mq=mq_open(reply_queue_name, O_RDWR);
			mq_getattr(c_mq, &attr);
			int buflen = attr.mq_msgsize;	
			item = (char *) malloc(buflen); 
			n = mq_send(mq, (char*) &item, strlen(item)+1, 0);
			if(n==-1)
			{
				perror("send to reply queue failed\n");
				exit(1);
			}
			//mq_close(c_mq);
			//mq_unlink(reply_queue_name);
			//exit(1);
		}
		
	}
	wait();
	mq_close(mq);
	fprintf(stdout, "SERVER: Server exit....\n");
	return 0;
}
