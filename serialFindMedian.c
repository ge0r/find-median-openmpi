#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>



int compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}


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
		printf("\n\nsad face\n\n");
}


int partition(int *p, int left, int right, int pivotValue){
	int storeIndex;
	int temp,i;
	int firstTimeSwap=1;
	
	
	storeIndex=left;
	
	for (i=left; i<=right; i++){
		if (p[i]<pivotValue){
			temp=p[storeIndex];
			p[storeIndex]=p[i];
			p[i]=temp;
			storeIndex++;			
		}
		else if((p[i]==pivotValue)&&(firstTimeSwap)){
			temp=p[i];
			p[i]=p[right];	
			p[right]=temp;
			
			//prepei na elegxei kai to stoixeio pou itan right
			i--;
			 
			firstTimeSwap=0;
		}
	}
	
	temp=p[right];
	p[right]=p[storeIndex];
	p[storeIndex]=temp;
	
	return storeIndex;
}



int qselect(int *p, int left, int right, int k){
	
	int pivotIndex, pivotNewIndex, pivotDist;

	
	pivotIndex=(left+right)/2;
	
	pivotNewIndex=partition(p,left,right,p[pivotIndex]);

	pivotDist=pivotNewIndex-left+1; 

	
	
	if (pivotDist==k)
		return p[pivotNewIndex];
	else if (k<pivotDist)
		return qselect(p,left,pivotNewIndex-1,k);
	else
		return qselect(p,pivotNewIndex+1,right, k-pivotDist);
}



int main(int argc, char **argv){
	struct timeval tv, tv0, lapsed;
	int median, N, k;
	int *originalCopy, *p;	
	
	
	N=atoi(argv[1]);
	k=N/2+1;
	
	originalCopy=(int *)malloc(N*sizeof(int));
	p=(int *)malloc(N*sizeof(int));
	random_initialization(p, N);
	memcpy (originalCopy,p,N*sizeof(int));
	
	 
	

	gettimeofday(&tv0, NULL);
	median=qselect(p, 0, N-1, k);
	gettimeofday(&tv, NULL); 
			

	printf("\nmedian= %d\n", median);	
	
		
	
	
	

	if(tv.tv_usec<tv0.tv_usec){
		tv.tv_usec += 1000000;
		tv.tv_sec--;
	}
	
	lapsed.tv_usec=tv.tv_usec-tv0.tv_usec;
    lapsed.tv_sec=tv.tv_sec-tv0.tv_sec;
	
	printf("\ntime elapsed: %d, %d s\n", (int)lapsed.tv_sec, (int)lapsed.tv_usec); 
	

	verification(originalCopy, N, median,k);

	
	return 0;
}
