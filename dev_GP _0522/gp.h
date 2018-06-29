#pragma once 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <time.h>


#define randint(x) (rand()%x)    //���������������main()Ҫ�� srand((int)time(0));
#define random() (rand()/(double)(RAND_MAX))    //0-1���С��
#define rand_constant() (randint(10)+random())    //[0,10)�������������PSO 

typedef float (*Func)(float* values); /*����ָ�룬���ڽṹ���Ա*/
int MAX_DEPTH;
#define PC 2

#define MAX_MUTATE_DEPTH 2
#define MAX_POPSIZE 200

#define MAX_ALLOW_FITNESS 20000
#define MAX_ISLAND_COUNT 10
#define MAX_ISLAND_SIZE 20

	int island_counts[MAX_ISLAND_COUNT];	
	int islands[MAX_ISLAND_COUNT][MAX_ISLAND_SIZE];		//ÿ���㼶��Ӧ�ĸ�����inpop�±� 
	
#define MAX_POOLSIZE 20 //�洢�ռ� 
#define MAX_POOLMEM  40	//���ٿռ� 

#define INF 10000
#define MAXNUM 1000000
#define MAXEXP 30
#define MAXIDX 100

//stop evolve condition
float LIMIT;

char* symbol = "--";

/* 1. basic structure */
typedef struct Fwrapper{
    Func function;  //�����������ݣ����ù��캯��
    char* name;
    int childcount;
    int weight;
}Fwrapper;
Fwrapper addw,subw,mulw,divw,sinw,cosw,expw,poww,logw,squarew,cubew,quartw,sqrtw;
Fwrapper flist_basic[4],flist_higher[5],flist_primary[7];	//��ר�Ÿ�ֵһ�� 
Fwrapper flist_extend[8],flist_big[6];
int flist_basic_count = 4,flist_higher_count=5,flist_primary_count=7;
int flist_extend_count=8,flist_big_count=6;
//flist used in main()
// Fwrapper* flist_use = flist_basic;
// int flist_use_count = 4;
//Fwrapper* flist_use = flist_primary;
//int flist_use_count = 9;
 Fwrapper* flist_use;// = flist_extend;
 int flist_use_count;// = 7;

typedef struct Node{
    int node_type;  // 0 for Node, 1 for paramnode, 2 for constnode
    struct Node **children;     //һ��������
    Fwrapper* fw;   
    
    int index;      //paramnode
    float value;      //constnode
}Node;

/* 2. function set part */
Node* new_Node();
Node** new_children(int childcount);
void assign_flist();

/* 3. dataset part */
float** DATA;	// #0422
int DATANUM;
float hidden_1(float a,float b);
float** build_sets(int num);	//# ,float (*hidden_func)(float a,float b)
void dataset_test();
void data_test();

/* 4. evolve process */
int evolve(int pc,int maxdepth,int popsize,int maxgen,int maxwaitgen,float limit,
            float mutation_rate,float crossover_rate,float pnew
			);	//# float (*hidden_func)(float a,float b)
void evolve_process(int Generation);
void test_evolve();
void test_flist_higher(float* inputs); 
void test_new_operator();
void test_population(); 
void test_complexity();
void test_module(); 
