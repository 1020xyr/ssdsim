#include <cstdio>
#include<cstring>
#include<cstdlib>
#include<cmath>
#include <iostream>
#include <string>

//0.035156 0 1998976 24 0	


const long long int kArraySize = 10000000000;
const int kStartPoint = 10000;
const int kEndPoint = 500000-10000;

int main(int argc, char const *argv[]) {
	char trace_name[30];
	strcpy(trace_name,argv[1]);
	FILE* input = fopen(trace_name,"r");
	FILE* output = fopen("result/stat.txt","a+");
	double time;
	int dev,size,ope;
	long long int lsn;
	double time_interval,read_ratio; 
	double read_size=0,write_size=0,read_num=0,write_num=0,update_write_size=0;
	double start_time,end_time;
	int* array = new int[kArraySize];			//38G
	memset(array,0,sizeof(array));

	int cnt = 0;
	while (fscanf(input, "%lf %d %lld %d %d", &time, &dev, &lsn, &size, &ope) != EOF) {
		if(ope==1){		//read
			read_num++;
			read_size += size;
		}
		else{			//write
			write_num++;
			write_size += size;
			if(lsn > kArraySize){
				printf("small size\n");
				while(1){}
			}
			for(int i=0;i<size;i++){
				array[lsn+i]++;
			}
		}
		cnt++;
		if(cnt==kStartPoint){
			start_time = time;
		}
		if(cnt==kEndPoint){
			end_time = time;
		}				
	}
	for(long long int i = 0;i<kArraySize;i++){
		if(array[i]>1){
			update_write_size += array[i] - 1;
		}
	}
	read_ratio = read_num / (read_num + write_num);
	time_interval = (end_time - start_time) / (kEndPoint - kStartPoint);
	fprintf(output, "******%s statistics******\n", trace_name);
	fprintf(output, "read number=%.lf, write number=%.lf\n", read_num, write_num);
	fprintf(output, "read request size=%.lf, write request size=%.lf\n",read_size, write_size);
	fprintf(output, "read ratio=%.2lf\n", read_ratio);
	fprintf(output, "read average request size =%.1lf\n", read_size / read_num);
	fprintf(output, "write average request size=%.1lf\n", write_size / write_num);
	fprintf(output, "update write size=%.lf\n", update_write_size);
	fprintf(output, "update write ratio=%.2lf\n", update_write_size / write_size);
	fprintf(output, "time interval=%.1lfms\n\n", time_interval);
}
