#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>

int start=1;
int *recvbuf1;
int leftp,rightp;
int indexSaved;
int thisRank=-1;

MPI_Datatype arraytype;



void random_initialization(int *p, int N){
	int i;
	srand(time(NULL));
    
    for(i=0; i<N; i++){
		p[i] = rand()%100000000; 
	}
}



void verification(int *p, int N, int m, int k){
	int count=0, i;
	
	for(i=0; i<N; i++){
		if (p[i]<m)
			count++;
	}
	
	if ((count+1)==k)
		printf("\n\nswsto apotelesma!\n\n");
	else 
		printf("\n\nsfalma\n\n");
}

int compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}




int partition(int *p, int left, int right, int pivotValue, int rank){
	int storeIndex;
	int temp,i;
	int firstTimeSwap=1, enteredHere=0, enteredThere=0;
	
	//gia na svinei palies times twn processes
	thisRank=-1;
	
	storeIndex=left;
	
	for (i=left; i<=right; i++){
		if (p[i]<pivotValue){
			temp=p[storeIndex];
			p[storeIndex]=p[i];
			p[i]=temp;
			storeIndex++;	
			//metritis gia na vevaiwthw oti to storeIndex den tha kseperasei to right
			//giati tote den prepei na kanw swap
			enteredHere=1;
		}
		else if((p[i]==pivotValue)&&(firstTimeSwap)){
			temp=p[i];
			p[i]=p[right];	
			p[right]=temp;
			
			//prepei na elegxei kai to stoixeio pou itan right
			i--;
			
			//eisai to rank pou eixe ton trexoda pivot
			thisRank=rank;
			
			firstTimeSwap=0;
		}
		else{
			enteredThere=1;
		}
	}
	
	
	//an den exeis mono mikrotera i mono megalutera apo ton pivot boreis na kaneis swap
	//xwris na vgei stoixeio ektos pinaka
	if((enteredHere&&enteredThere)||(enteredHere&&(!firstTimeSwap))||(enteredThere&&(!firstTimeSwap)))
	{
		temp=p[right];
		p[right]=p[storeIndex];
		p[storeIndex]=temp;
	}
	//o trexwn pivot einai sto indexSaved
	indexSaved=storeIndex;
	
	return storeIndex;
}



int gselect(int *p, int left, int right, int k, int rank , int processes){
	int sendcount, recvcount, source, pivotDist, taskSize,  pivotNewIndex, i, offset=0;
	int size=right-left+1;
	int pivot, pivotIndex, pivotNewIndexChanged, fsubArraySize;
	MPI_Status stat2;
	taskSize=size/processes;
	
	
	source = 0;
	sendcount = taskSize;
	recvcount = taskSize;
	
	//arxikopoiisi
	if(start){
		rightp=right;
		leftp=left;
		right=taskSize-1;
	
		recvbuf1=(int *)malloc(taskSize*sizeof(int));
		pivotIndex=(leftp+rightp)/2;
		pivot=p[pivotIndex];
		MPI_Scatter(p, sendcount, MPI_INT, recvbuf1, recvcount, MPI_INT, source, MPI_COMM_WORLD);
	}
		

	//an den vriskomaste sti arxi vres ton pivot apo ta stoixeia pou exoun apomeinei
	if ((rank==source)&&(!start)){
		p=(int *)malloc((rightp-leftp+1)*sizeof(int));
		//an exeis xrisima stoixeia
		if (left<=right){
			memcpy (&p[0],&recvbuf1[left],(right-left+1)*sizeof(int));
			offset+=right-left+1;
		}
		for (i=1; i<processes; i++){
			MPI_Recv(&fsubArraySize, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &stat2);	
	
				if(fsubArraySize){
				MPI_Recv(&p[offset], fsubArraySize, MPI_INT, i, 1, MPI_COMM_WORLD, &stat2);		
				offset+=fsubArraySize; 
			}		
		}	
		pivot=p[offset/2];
		MPI_Bcast(&pivot, 1, MPI_INT, source, MPI_COMM_WORLD);
	}
	else if(!start){
		//an exeis xrisima stoixeia
		if (left<=right){
			fsubArraySize=right-left+1;

			MPI_Send(&fsubArraySize, 1, MPI_INT, source, 1, MPI_COMM_WORLD);
			
			MPI_Type_contiguous(fsubArraySize, MPI_INT, &arraytype);
			MPI_Type_commit(&arraytype);
			
			MPI_Send(&recvbuf1[left], 1, arraytype, source, 1,MPI_COMM_WORLD);		
			
			MPI_Type_free(&arraytype);				
		}
		else{
			fsubArraySize=0;
			MPI_Send(&fsubArraySize, 1, MPI_INT, source, 1, MPI_COMM_WORLD);
		}
		MPI_Bcast(&pivot, 1, MPI_INT, source, MPI_COMM_WORLD);
	}
	else
		start=0;


	
	
	pivotNewIndex=partition(recvbuf1, left, right, pivot, rank); 

	//prepei na afairw apo tous mikrous pinakes to diko tous left  
	pivotNewIndexChanged=pivotNewIndex-left;	
	
	MPI_Allreduce(&pivotNewIndexChanged, &pivotDist, 1, MPI_INT, MPI_SUM , MPI_COMM_WORLD); 
    
    
    pivotDist+=1;     	 
	
	
	if (pivotDist==k){
		return recvbuf1[indexSaved];
	}
	if (k<pivotDist){
		//afto isxuei gia ton p 
		rightp=(pivotDist+leftp-1)-1;
		//afto tha isxuei gia ta recbuf oxi gia olokliro to p
		return gselect(p, left, pivotNewIndex-1, k, rank, processes);
	}
	else{
		//afto isxuei gia ton p 
		leftp=(pivotDist+leftp-1)+1;
		//afto tha isxuei gia ta recbuf oxi gia olokliro to p
		if(rank==thisRank){
			return gselect(p, pivotNewIndex+1, right, k-pivotDist, rank, processes);
		}
		else {
			//prepei na eksetaseis kai to stoixeio pou vrisketai sto pivotNewIndex giati den einai o pivot
			return gselect(p, pivotNewIndex, right, k-pivotDist, rank, processes);
		}
	}
	
}



int main(int argc, char **argv){
	struct timeval tv, tv0, lapsed;
	int rank, processes, median, N, k;
	int *originalCopy, *p;	
	
	
	N=atoi(argv[1]);
	k=N/2+1;
	
	originalCopy=(int *)malloc(N*sizeof(int));
	p=(int *)malloc(N*sizeof(int));
	random_initialization(p, N);
	memcpy (originalCopy,p,N*sizeof(int));
	
	
	
	
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &processes);	
	
	gettimeofday(&tv0, NULL); 
	median=gselect(p, 0, N-1, k, rank, processes);
	gettimeofday(&tv, NULL);
			
	if(rank==thisRank){
		printf("\nmedian= %d\n", median);	
	}
	
	
	MPI_Finalize();
	
		

	if(tv.tv_usec<tv0.tv_usec){
		tv.tv_usec += 1000000;
		tv.tv_sec--;
	}
	
	lapsed.tv_usec=tv.tv_usec-tv0.tv_usec;
    lapsed.tv_sec=tv.tv_sec-tv0.tv_sec;
	
	if(rank==thisRank)
	printf("\ntime elapsed: %d, %d s\n", (int)lapsed.tv_sec, (int)lapsed.tv_usec); 
	
	
	if(rank==thisRank){
		verification(originalCopy, N, median, k);
	}
	
	return 0;
}
