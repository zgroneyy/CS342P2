#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_LENGTH 81
//Line node lists that holds whole line
typedef struct LineNode
{
	char line[BUFFER_LENGTH];
	struct LineNode* next;
}LineNode;

//A struct for files, keeps a tempfile to be used after.
typedef struct FileNode
{
	char* filename;
	FILE* tempfile;
	struct FileNode* next;
}FileNode;

//Method check whether line contains keyword or not
int linecontains(char* line, char* keyword)
{

	char* result = strstr(line,keyword);//If line contains, return 

	if(result == NULL)
		return 0;

	return 1;

}

//A method that writes linked list into the file
void writeOutputFile(char* filename, LineNode* outputlist)
{
	FILE* output = fopen(filename, "w+");
	//A simple loop to print out our Intersection linked list to .txt file
	while(outputlist!=NULL)
	{
		fprintf(output,"%s\n",outputlist->line);
		outputlist=outputlist->next;
	}
	fclose(output);
}
//Bubblesort for the linked list.
void bubblesort(LineNode **head) 
{
    int pass = 0; // True if no swaps were made in a pass

    // Silly control statement
	if (*head == NULL || (*head)->next == NULL) 
		return;

	while (!pass) {
	   LineNode **pivot = head; // "source" of the pointer to the current node in the list struct                             
	   LineNode *nd = *head;// local iterator pointer
	   LineNode *nx = (*head)->next;
	   pass = 1;
	   //As long as we have elements to sort, this should be continued.
	   while (nx) {
		  //If first is bigger than second, swap.
		  if (strcmp(nd->line, nx->line) > 0) {
			 nd->next = nx->next;
			 nx->next = nd;
			 *pivot = nx;
			 pass = 0;
		  }
		  pivot = &nd->next;
		  nd = nx;
		  nx = nx->next;
	   }
	}
}
/*
Function assignChild
Called just after fork(), works in child process
Read data from given input line by line, if line contains keyword simply prints it to tempfile in wanted style.
If line does not contains keyword, ignores the line and goes until the end of input.
*/
void assignChild(char* inputFile,char* keyword,FILE* tempfile)
{
	FILE *fp = fopen (inputFile, "r" );
	
	char word[BUFFER_LENGTH]; 
	int linenumber=0;//
	
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
				fprintf(tempfile,"%s: %s (%d)\n",inputFile,word,linenumber);
			}
		}
					
		//When filereader sees end of the document, it will be stopped 
		if(feof(fp))
		   break;
	}
	fclose(fp);
	//childrenAmountOperation(DEC);
	exit(0);
}

int main(int argc, char* argv[])
{
	clock_t start = clock();

	//argv[0] reserved for program name, so argv[1] will be our keyword
	char* keyword=argv[1];
	int N;
	
	//N will be entered command line as a char*, needed to be typecast into an integer.
	N=atoi(argv[2]);
	
	//Initialization of first node of list of files.
	FileNode* head; 
	head=(FileNode*)malloc(sizeof(struct FileNode));
	head->filename=argv[3];
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
		int j=3+i;
		newFile->filename=argv[j];
		newFile->tempfile = NULL;
		curr->next=newFile;
		newFile->next=NULL;
		curr=curr->next;
	}

	FileNode* curr2;
	curr2=head;
	
	//Here, main process forks N times into separate children. 
	pid_t pid;
	while(curr2!=NULL)
	{
		curr2->tempfile = tmpfile();
		
		pid = fork();
		//Process id of children cannot be less than 0; if it could, well there is an error.
		if (pid < 0)
		{
			exit(EXIT_FAILURE);
		}
		//pid=0 indicates we have succesfully created our child process.
		else if(pid == 0)
		{
			//I'm child and here to work.
			assignChild(curr2->filename,keyword,curr2->tempfile);
			exit(0);
		}
		
		curr2=curr2->next;
	}
	//We have to wait until we close all child processes. 
	while(wait(NULL) > 0);
	
	curr2 = head;
	char word[BUFFER_LENGTH];
	//Dummy is created as NOT a pointer to escape from malloc. Dummy is used as list of lines in a tempfile. 
	LineNode dummy;
	LineNode* cur = &dummy;
	
	while (curr2 != NULL)
	{
		FILE* tempfile = curr2->tempfile;
		rewind(tempfile);
		int index = 0;
		
		while (1)
		{
		    int ch;
			//Read line char by char
			while((ch = fgetc(tempfile)) != '\n' && ch != -1 && index < BUFFER_LENGTH)
			{
				word[index++] = ch;
			}
			word[index] = '\0';
			index = 0;
			//If you see an empty line
			if (word[0] != '\0')
			{
				//printf("%s\n",word);
				LineNode* newnode=(LineNode*)malloc(sizeof(struct LineNode));
				strcpy(newnode->line,word);
				cur->next=newnode;
				newnode->next=NULL;
				cur=cur->next;
			}
			//End of tempfile.
			if (feof(tempfile))
				break;
		}
		//Close tempfile and go through File's linked list. 
		fclose(curr2->tempfile);
		curr2->tempfile = NULL;
		curr2 = curr2->next;
	}
	//SORT
	bubblesort(&dummy.next);
	writeOutputFile(argv[N + 3],dummy.next);
	clock_t end = clock();
	float seconds = (float)(end - start) / CLOCKS_PER_SEC;
	printf ("Program lasts: %f\n", seconds);
}
