#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */ 
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <pthread.h>    /* POSIX Threads */
#include <string>     /* String handling */
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <queue>

using namespace std;
#define NUM_THREAD 50
#define pp pair<float,string>
void *print_message_function (void* ptr);
float getMold(const vector<int>& vec);
float getSimilarity(const vector<int>& lhs, const vector<int>& rhs);
priority_queue<pp> frequency;//存取每個文件計算完的平均詞頻

typedef struct str_thdata
{
    string thread_no;
    string message;
    vector<string>word;
    int tid;
    map<string,int> m;
    vector<int> wordF;//存放詞頻向量
} thdata;


pthread_t worker[NUM_THREAD];
thdata data[NUM_THREAD];
set<string> store;
int dataNum=0; //data 個數

int main(int argc, char**argv)
{
	string inFile=argv[1];
	string line;
	clock_t start_time,end_time;
	start_time=clock();
	
	//讀檔
	ifstream in(inFile);
	//
	
	int i=0;
	
	while(getline(in, line)){
		
		//thread_no
		string d=" ";    		  		
   		size_t pos=0;
   		string p=line;
   		string ss;
   		bool inIf=true;
    		if((pos=p.find(d))!= string::npos){
    		
    			ss=p.substr(0,pos);
    			data[i].thread_no = ss;
    			inIf=false;
    		}
    		if(inIf==true)
			data[i].thread_no = line;
		
		//文件
		getline(in, line);
		
		 //先將標點符號轉成空白
  		for(int x=0;x<line.length();x++){
     	
    			if(line[x]==','||line[x]=='.'||line[x]==';'||line[x]=='!'||line[x]=='?'||line[x]==':'||int(line[x])==39||line[x]=='"'||line[x]=='-'||int(line[x])==96){//英文會使用得標點符號種類    	    				   		
    				line[x]=' '; //轉成空白
    		
    			}
    		}	
    		
		//去除多餘的空白
		string s=line;
		 
   		for(int x=0;x<s.length();x++){
    	
    			if(s[x]==' '&&s[x+1]==' '){
    				string::iterator it=s.begin()+x;
    				s.erase(it);
    			}
   		 }
   		 
   		line=s;
   		
   		//切割字串
   		
   		bool letter;
   		pos=0;
   		d=" ";
   		p=line;
    		while((pos=p.find(d))!= string::npos){
    		
    			ss=p.substr(0,pos);
    			letter=true;//檢查有沒有其他字元
    			
    			for(int x=0;x<ss.length();x++){
    				
    				if(int(ss[x])<97||int(ss[x])>122){ //有其他字元
    					
    					letter=false; 
    				
    					break;
    				}
    			
    			}
    		
    			if(letter==true){	
    			
    				data[i].word.push_back(ss); //把這個詞放進word
    				
    				store.insert(ss);
    			}
    			p.erase(0, pos + d.length());
    			
    		}
    		
    		letter=true;
    		if(p.length()>0){
    			for(int x=0;x<p.length();x++){
    				if(int(p[x])<97||int(p[x])>122){
    					letter=false;
    					break;
    				}
    			}
    			if(letter==true){
    				data[i].word.push_back(p);
    				store.insert(p);
    			}
    		}
  		
    		for(const auto &str : data[i].word){
    		   			
    			data[i].m[str]++;//加入這個資料的 map m   					
   		}
   		
		data[i].message = line; //存入文件				
				
		i++;	
		dataNum++;			
	}//while end
	
	
	//創建thread
	for(int x=0;x<i;x++)
	{
		data[x].tid=x;
		pthread_create (&worker[x], NULL,  print_message_function,(void *) &data[x]); //算出這個文件的詞頻向量
		
		cout<<"[Main thread]: create TID:"<<worker[data[x].tid]<<", DocID:"<<data[x].thread_no<<endl;
		sleep(1);									
		
	}	
	
	sleep(1);
	for(int j=0; j<dataNum;j++){
		pthread_join(worker[j],NULL);
	}
	
	
	cout<<"[Main thread] KeyDocID:"<<frequency.top().second<<" Highest Average Cosine: "<<frequency.top().first<<endl;
	
	for(int j=0;j<dataNum;j++){
		pthread_detach(worker[j]);
	}
	
	//CPU Time
	end_time=clock();
	double cputime=((double)(end_time-start_time))*1000000/CLOCKS_PER_SEC;
	cout<<"[Main thread] CPU Time:"<<cputime<<"ms"<<endl;
	//
	
	
	return 0;
}



void *print_message_function ( void* ptr )
{
	
	thdata *datas;            
   	datas = (thdata *) ptr;
   	clock_t start_time,end_time; //計算CPU time
   	start_time =clock();
 
   	//把詞頻向量放進這個資料的vector
   	for(auto it=store.begin();it!=store.end();++it){
   		datas->wordF.push_back(datas->m[*it]); 
   	}
   		
    
	//列印出詞頻向量
	cout<<"[TID="<<worker[datas->tid]<<"] DocID:"<<datas->thread_no<<" ";
	cout<<"[";
	auto it=store.begin();
	auto it_end=store.end();
	it_end--;
	for(;it!=it_end;++it){
		cout<<datas->m[*it]<<",";
		
	}
	cout<<datas->m[*it_end]<<"]"<<endl;

    //求餘弦相似係數
    
    float avg=0;
    for(int x=0;x<dataNum;x++){
    	if(x!=datas->tid){
 		vector<int> reg;
 	
 		//將data[x]的map佔存在reg
    		for(auto it=store.begin();it!=store.end();++it){
   			reg.push_back(data[x].m[*it]); 
   		}
   		
    		float cos_val=getSimilarity(datas->wordF,reg);
    		avg+=cos_val;
    		cout<<"[TID="<<worker[datas->tid]<<"] cosine("<<datas->thread_no<<","<<data[x].thread_no<<")="<<cos_val<<endl;
    		reg.clear();
    	}
    }
       
   avg/=dataNum;
   cout<<"[TID="<<worker[datas->tid]<<"] Avg_cosine: "<<avg<<endl;
   frequency.push({avg,datas->thread_no}); //將最後這個data的cos平均存進queue
   avg=0;
   
   //CPU Time
   end_time=clock();
   double cputime=((double)(end_time-start_time))*1000000/CLOCKS_PER_SEC;
   cout<<"[TID="<<worker[datas->tid]<<"] CPU Time: "<<cputime<<"ms"<<endl;
   //
   
   sleep(5);
   pthread_exit(0); 
} 


//求cos
float getMold(const vector<int>& vec)
{
	int n = vec.size();
	float sum =0.0;
	for(int i=0;i<n;++i){
		sum+=vec[i]*vec[i];
	}
	return sqrt(sum);
}

float getSimilarity(const vector<int>& lhs, const vector<int>& rhs)
{
	int n=lhs.size();
	assert(n==rhs.size());
	float tmp=0.0;
	for(int i=0;i<n;++i){
		tmp+=lhs[i]*rhs[i];
	}
	return tmp/(getMold(lhs)*getMold(rhs));
}
	
	
	
	
	
	
	


