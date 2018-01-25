#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>

int start=1;
int *subArray, *subArraySizes, *offsets;
int leftp,rightp;
int thisRank;
int source=0;
int leftp,rightp;

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

	return storeIndex;
}



int gselect(int *p, int left, int right, int k, int rank , int processes){
	int pivotDist, pivotNewIndex, i;
	int pSize, modu;
	int pivot, pivotIndex, pivotNewIndexChanged, mySize, myOldSize;

		
	if(!start){
		//mazepse ola ta stoixeia pou apemeinan ston p
		mySize=right-left+1;
		MPI_Gather(&mySize, 1, MPI_INT, subArraySizes, 1, MPI_INT, source, MPI_COMM_WORLD);
	    
	    if(rank==source){
			offsets[0]=0;
			for( i=1; i<processes; i++){
				if (subArraySizes[i]<0){
					//min steileis tipota kai min allakseis to offset
					subArraySizes[i]=0;
				}
				else{
					offsets[i]= subArraySizes[i-1]+offsets[i-1];
				}
			}
       }
       //to twrino megethos tou p
       pSize=rightp-leftp+1;
       MPI_Gatherv(&subArray[left], mySize, MPI_INT, &p[0], subArraySizes, offsets, MPI_INT, source, MPI_COMM_WORLD);
       mySize=pSize/processes;
	}
	else{
		//trexei prwti fora ara arxikopoiw ta stoixeia
		pSize=right-left+1;
		mySize=pSize/processes;
		rightp=right;
		leftp=left;
		
		if (rank==source){
			//borei na xreiastei na pareis processes-1 stoixeia parapanw 
			subArray=(int *)malloc((mySize+processes)*sizeof(int));
		}
		else{
			subArray=(int *)malloc(mySize*sizeof(int));
		}
		
		if(rank==source){
			subArraySizes=(int *)malloc(processes*(sizeof(int)));
			offsets=(int *)malloc(processes*(sizeof(int)));
		}

		start=0;
	}
	
		
	left=0;
    right=mySize-1;

	
	if(rank==source){
		srand(time(NULL));
		pivotIndex=rand()%pSize;
		pivot=p[pivotIndex];	
	}
	

	//moirase ta stoixeia pou einai ston p kai steile kai ton pivot
	MPI_Scatter(p, mySize, MPI_INT, subArray, mySize, MPI_INT, source, MPI_COMM_WORLD);	
	MPI_Bcast(&pivot, 1, MPI_INT, source, MPI_COMM_WORLD);
	
	//ta stoixeia borei na min moirastoun ola opote mazepse sto rank osa perisepsan
    modu=pSize%processes;
    if ((modu)&&(rank==source)){
	   myOldSize=mySize;
	   mySize+=modu;
	   for(i=0; i<modu; i++){
		   subArray[myOldSize+i]=p[pSize-modu+i];
	   }
	   right=mySize-1;
	}
	
	
	//vres posa mikrotera stoixeia apo ton pivot uparxoun se kathe subArray
	pivotNewIndex=partition(subArray, left, right, pivot, rank);
	
	//afto borei na thelei apla delete
	pivotNewIndexChanged=pivotNewIndex-left;
	
	//vres tin apostasi tou pivot apo tin arxi tou pinaka
	MPI_Allreduce(&pivotNewIndexChanged, &pivotDist, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	pivotDist++;
	
	
	if (pivotDist==k){
		return subArray[pivotNewIndex];
	}
	else if (k<pivotDist){
		rightp=(pivotDist+leftp-1)-1;
		return gselect(p, left, pivotNewIndex-1, k, rank, processes);
	}
	else{
		leftp=(pivotDist+leftp-1)+1;
		if(rank==thisRank){
			return gselect(p, pivotNewIndex+1, right, k-pivotDist, rank, processes);
		}
		else {
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
