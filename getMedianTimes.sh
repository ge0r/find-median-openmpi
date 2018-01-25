#! /bin/bash

rm -f meanTimes;

awk 'BEGIN {countS=5; countP=0; serialTime=0;};
/N/{split($3, a, "-"); Ni=a[1];};
/serial output/{sRow=NR-6; countS=0;}; 
(NR-sRow)==12 && (countS<4) {
	split($3, a, ","); 
	secs=a[1]*1000000; usecs=$4; serialTime+=secs+usecs; 
	if (countS==3) {
		serialTime/=4; 
		usecs=serialTime%1000000; 
		secs=(serialTime-usecs)/1000000;
		usecs= usecs "us";
		secs= secs "s";
		printf("N=%-8d         serialTime=         %-3s %-8s \n",Ni,secs,usecs) >>"meanTimes";
	}
	sRow=NR;countS++;
};
/P/{split($3, a, "-"); Pi=a[1];};
/findMedian/ {pRow=NR;};
(NR-pRow)==4 {
	split($3, a, ","); 
	secs=a[1]*1000000; usecs=$4; parallelTime+=secs+usecs;
	countP++;	 
	if (countP==4) {
		parallelTime/=4; 
		usecs=parallelTime%1000000; 
		secs=(parallelTime-usecs)/1000000;
		usecs= usecs "us";
		secs= secs "s";
		printf("N=%-8d  P=%-2s   parallelTime=       %-3s %-8s \n",Ni,Pi,secs,usecs) >>"meanTimes";
		parallelTime=0;	
	}
	else if (countP==8) {
		parallelTime/=4; 
		usecs=parallelTime%1000000; 
		secs=(parallelTime-usecs)/1000000;
		usecs= usecs "us";
		secs= secs "s";
		printf("N=%-8d  P=%-2s   alternativePtime=   %-3s %-8s \n",Ni,Pi,secs,usecs) >>"meanTimes";	
		countP=0;
	}
};' < medianOutput;
