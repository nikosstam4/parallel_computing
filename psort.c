#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <omp.h>


#define MAXCHARSIZE 50                                                              
typedef char STRING[MAXCHARSIZE];

typedef struct {
	int startNode, endNode;
}EDGE;

FILE *f;
	
void getS(int set[],EDGE *graph, int edges);
void printSet(int set[]);
int noDuplicates(int set[], int element);
int removeNodeFromS(int set[], EDGE *graph);
void insertAtEnd(int set[],int element);
int hasNoIncomingEdges(int node,EDGE *graph,int removedNode);
void removeEdge(int removedNode, EDGE *graph, int set[]);

	

int main(int argc, char *argv[]){

//Variables Declaration
	STRING numberOfThreads,fileName;
	struct timeval t1,t2;
    double time1,time2;

//File read into memory
	
	STRING string_size,string_edges;
	int edges,size,num_threads;
	STRING temp1, temp2;

	strcpy(fileName, argv[1]); 
	strcpy(numberOfThreads, argv[2]);
	num_threads = atoi(numberOfThreads);
	omp_set_num_threads(num_threads);

    f = fopen(fileName, "r");
    if (f == NULL)
    {
        printf("File failed to open.\n");
        return 0;
    }
    if (numberOfThreads == NULL)
    {
    	printf("Give number of threads.\n");
    	return 0;
	}
    fseek(f,0,SEEK_END);
    if (ftell(f) == 0)
    {
        printf("Cannot sort: The graph is empty.\n");
        return 0;
    }
    
    rewind(f);
    
    if (fscanf(f,"%s %s %s", string_size, string_size, string_edges))
    size = atoi(string_size);
    edges = atoi(string_edges);
    printf("Size is %d x %d with %d edges.\n",size,size,edges);
    
    EDGE *graph = (EDGE *)malloc(sizeof(EDGE)*edges);
  		
   	int line = 0,i = 0;
   	while(i < edges)
   	{
   		if(fscanf(f,"%d %d", &graph[i].startNode, &graph[i].endNode));
   		//printf("%d %d\n", graph[i].startNode, graph[i].endNode);
   		i++;
   	}
   	
   	graph[i].startNode = -1;
   	graph[i].endNode = -1;
   	
    fclose(f);
    
    gettimeofday(&t1, NULL);
    time1 = t1.tv_sec + 1e-6 * t1.tv_usec;
    
    //SET S
    int setS[size],setL[edges],removedNode;
    setS[0] = -1;
    setL[0] = -1;
 
	//create set S
	getS(setS,graph,edges);
	//printSet(setS); 	
 	
 	#pragma omp parallel
 	#pragma omp single
 	#pragma omp parallel for 
	for(int k=0; k<edges; k++)
	{
		#pragma omp task
		{
			//process node and remove edges of removed node from graph
			removedNode = removeNodeFromS(setS, graph);
			//printSet(setS);
		
			//printf("%d\n", removedNode);
			removeEdge(removedNode, graph, setS);
				
			//insert processed node at L
			insertAtEnd(setL,removedNode); 
		}
	#pragma omp taskwait
		
	}

	gettimeofday(&t2, 0);
    time2 = t2.tv_sec + 1e-6 * t2.tv_usec;
    
    double elapsed = time2 - time1;
    printf(" Time elapsed is: %lf seconds\n", elapsed); 
    
	printSet(setL);
    
  return 0;  
  
}//end main

void getS(int set[],EDGE *graph, int edges)
{
	int sizeS = 0;
	int flag;
	
	#pragma omp parallel for
	for(int i=0; i<edges; i++)
	{
		flag = 0;
		for(int j=0; j<edges; j++)
		{
			if(graph[i].startNode == graph[j].endNode)	//has incoming edges
			{
				flag = 1;
			}
		}
		if(flag == 0)
		{
			if(noDuplicates(set, graph[i].startNode))
			{
				set[sizeS+1] = -1;
				set[sizeS] = graph[i].startNode;
				sizeS++;
			}		
		}		
	}
}

void printSet(int set[])
{
	int i = 0;
	printf("This is set L: \n");

	while(set[i] != -1)
	{
		printf("%d,",set[i]);
		i++;
	}
	printf("\n");
}

int noDuplicates(int set[], int element)
{
	int i = 0;
	while(set[i] != -1)
	{
		if(set[i] == element)
		{
			return 0;
		}
		i++;
	}
	return 1;
}

int removeNodeFromS(int set[], EDGE *graph)
{
	int i=0,node;
	
	node = set[0];
	
	#pragma omp parallel
	#pragma omp single
	while(set[i] != -1)
	{
		#pragma omp task
		{
			set[i] = set[i+1];
			i++;
		}
	}

	return node;
}

void removeEdge(int removedNode, EDGE *graph, int set[])
{		
	int i=0;

	while(graph[i].startNode != -1 && graph[i].endNode != -1)
	{
		if(graph[i].startNode == removedNode)
		{
			int j = i;
			if(hasNoIncomingEdges(graph[j].endNode, graph, removedNode))
				{
					insertAtEnd(set, graph[j].endNode);
				}
			while(graph[j].startNode != -1 && graph[j].endNode != -1)
			{			
				graph[j].startNode = graph[j+1].startNode;
				graph[j].endNode = graph[j+1].endNode;
				j++;	
			}
			i--;
		}
		i++;
	}
	
}

void insertAtEnd(int set[],int element)
{
	int i=0;
	
	while(set[i] != -1)
	{
		i++;
	}
	set[i] = element;
	set[i+1] = -1;
}
					
int hasNoIncomingEdges(int node,EDGE *graph,int removedNode)
{
	int i=0,count=0,position;
	while(graph[i].startNode != -1 && graph[i].endNode != -1)
	{

		if(graph[i].endNode == node)
		{
			position = i;
			count++;
		}
		i++;
	}
	
	if(count == 1 && graph[position].startNode == removedNode)
	{
		return 1;
	}
	else
	{
		return 0;
	}	
}


