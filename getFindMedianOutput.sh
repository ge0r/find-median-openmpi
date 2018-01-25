#! /bin/bash


SAVE_FILE=medianOutput;

rm -f $SAVE_FILE;

i=6;
count=0;

while [ $i -le 26 ];do
	powN=$i;
	N=$((2**$powN));
	echo -e "\n\n----------------N = $N---------------- " >> $SAVE_FILE;
	echo -e "\nserial output: " >> $SAVE_FILE;
	echo -e "\n\n$count trexw 4 fores to ./serialFindMedian.out $N\n";
	
	for m in {1..4};do
		echo -e "\nserialFindMedian: " >> $SAVE_FILE;
		./serialFindMedian.out $N>> $SAVE_FILE;
		echo -e "\n" >> $SAVE_FILE;
	done
	
	j=0;
	i=$(($i+4));
	echo -e "\n\nparallel output: " >> $SAVE_FILE;
	
	while [ $j -le 6 ];do
		powP=$j;
		P=$((2**$powP));
		j=$(($j+1));
		echo -e "\n--------P = $P--------\n" >> $SAVE_FILE;
		echo -e "\n\n$count trexw 4 fores to mpirun -np $P findMedian.out $N ";
	
		for m in {1..4};do
			echo -e "\nfindMedian:" >> $SAVE_FILE;
			mpirun -np $P findMedian.out $N >> $SAVE_FILE;
			echo -e "\n" >> $SAVE_FILE;
		done
		
		echo -e "\n\n$count trexw 4 fores to mpirun -np $P alternativeFindMedian.out $N";
		
		for m in {1..4};do
			echo -e "\nalternativefindMedian:" >> $SAVE_FILE;
			mpirun -np $P alternativeFindMedian.out $N >> $SAVE_FILE;
			echo -e "\n" >> $SAVE_FILE;
			#min trekseis me 32 kai anw processes gia ta 2^26 data
			count=$(($count+1));
		done
		
		if [ $i -eq 30 ];then 
			if [ $j -eq 5 ];then
				j=7;
			fi;
		fi;
	done
done



#get the mean times

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
};' < $SAVE_FILE;

