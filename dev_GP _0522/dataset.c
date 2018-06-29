#include "gp.h"


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <time.h>


/*** 数据集构造与评价 ***/
// 模型构造 
float hidden_function(float x,float y){
//    return x*x+2*y+3*x+5;
//	  return 5*x+pow(y,3);
//    return 20.9*x+15.7*y;                     //linear f1
//    return 20.4*pow(x,2)+5.2*pow(y,2)-30.5;    //two f2	在双二阶情况下无法预测 
//    return 20.4*pow(x,3)+3.7*pow(x,2)-pow(y,2) ;   //three f3
//    return 20.4*pow(x,4)+6.5*pow(x,2)+3.9*pow(x,2)+6.3*x-pow(y,4)+3*pow(y,2)+42.6 ;       //four f4
//    return  pow(3.4,x)+pow(4.2,y)-8.8;                //exp f4
//    return  pow(x,4)+pow(y,5)+10      ;              //pow f5

//	  return 3.0+(2.13*log(x));
//	  return 2.0-(2.1*(cos(x))*sin(y));
//	  return 2.0-(2.1*(cos(x))*sin(y)); 

//	return 1.57+24.3*x;		//p1
//	return 0.23+14.2*(x+y);		//再次计算 
//	return -5.41+(4.9*(2*x-0.5*y));
//	return -2.3+(0.13*sin(x));	//p4
//	return 3.0+(2.13*log(x));
//	
//	return 213-(213*pow(-0.5,x));
//	return sqrt(x)+sqrt(y)+1;		//p7
	return 12.0-(6.0*cos(x)+3.0*sin(y));
}
//num表示数据集的大小,默认200
float** build_sets(int num){
    if (num<50 || num>1000) num=200;
    float** rows = (float**)malloc(sizeof(float*)*num);
    float x,y;	int i;
    for (i=0;i<num;++i){
        x = randint(20);
        y = randint(20);
        rows[i] = (float*)malloc(sizeof(float)*3);  //3为x,y,answer；后续修改:score_function
        rows[i][0] = x; rows[i][1] = y; rows[i][2] = hidden_function(x,y); 
    }
    return rows;
}
void set_data(int num){		// #0422
	DATA = build_sets(num);
	DATANUM = num;
}


/* 测试构造区*/
void dataset_test(){
    float** rows = build_sets(50);
    int i,j;
    for(i=0;i<50;++i){
        for (j=0;j<3;++j)
            printf("%f\t",rows[i][j]);
        printf("\n");
    }
}
void data_test(){
    int i,j;
    for(i=0;i<200;++i){
        for (j=0;j<3;++j)
            printf("%f\t",DATA[i][j]);
        printf("\n");
    }
}
