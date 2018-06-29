#pragma once 
#include "gp.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <time.h>

/* init structure */
void init_Fwrapper(Fwrapper *fw,char *name,int count,int weight){
	fw->name = (char*)malloc(sizeof(char)*15);  //memory-- fw是有限且固定的，可以暂不释放
    strcpy(fw->name,name);
    fw->childcount = count;
    fw->weight = weight;
}



void init_node_type(Node* node,int node_type_v){
    node->node_type = node_type_v;        
}
void init_node_function(Node* node,Fwrapper* fw,Node **children_v){	
    node->fw = fw;
    node->children = children_v;
}
void init_node_terminal(Node* node,int index,float value){
    if (index==-1)
        node->value = value;
    else    node->index = index;
}

/* 构造一棵树 */
/* memory-- 所有的Node类型，在一轮评估结束且下一轮已复制必要的个体后，全部递归释放空间 */
Node* new_Node(){
	return (Node*)malloc(sizeof(Node));         
}
Node** new_children(int childcount){
	Node **children = (Node**)malloc(sizeof(Node*)*childcount);  
	return children;
}


/* 函数构造 */
/*
weight:
	+ - abs 01		* / square cube quart 02
	sqrt 03		log exp 04
	cos sin 05	tan tanh 06
	max min 07		
*/
float add_func(float *values){
    return values[0]+values[1];
}
void use_add(){
    addw.function = add_func;
    init_Fwrapper(&addw,"add",2,1);  
}
float sub_func(float *values){
    return values[0]-values[1];
}
void use_sub(){
    subw.function = sub_func;
    init_Fwrapper(&subw,"substract",2,1);  
}
float mul_func(float *values){
	if (values[0]*values[1]>MAXNUM)	return 1.0;
    return values[0]*values[1];
}
void use_mul(){
    mulw.function = mul_func;
    init_Fwrapper(&mulw,"multiply",2,2);  
}
float div_func(float *values){
    if ((fabs(values[1])<0.01) || (fabs(values[0]/values[1])>INF))
        return 1.0;
    return values[0]/values[1];
}
void use_div(){
    divw.function = div_func;
    init_Fwrapper(&divw,"division",2,2);  
}
float sin_func(float *values){
    return sin(values[0]);
}
void use_sin(){
    sinw.function = sin_func;
    init_Fwrapper(&sinw,"sin",1,5);  
}
float cos_func(float *values){
    return cos(values[0]);
}
void use_cos(){
    cosw.function = cos_func;
    init_Fwrapper(&cosw,"cos",1,5);  
}
float exp_func(float *values){
    //a^x,a>0 and a!=1
    if (values[0]<0 || fabs(values[0]-1)<0.01 )    return 1.00;    
    //not too large
    if (values[0]>MAXIDX || values[1]>MAXEXP || pow(values[0],values[1])>MAXNUM)
    	return 1.00;
    return pow(values[0],values[1]);
}
void use_exp(){
    expw.function = exp_func;
    init_Fwrapper(&expw,"exponential",2,4);  
}
float pow_func(float *values){
    //x^a,x>0 (eg. x^(1/2) )
    if (values[0]<0 || fabs(values[0]-0)<0.01)    return 1.00;  
	if (values[0]>MAXIDX || values[1]>MAXEXP || pow(values[0],values[1])>MAXNUM)
    	return 1.00;  
    return pow(values[0],values[1]);
}
void use_pow(){
    poww.function = pow_func;
    init_Fwrapper(&poww,"power",2,4);  
}
float log_func(float *values){
    if (values[0]<1 || fabs(values[0]-1)<0.01)    return 0.00;    
    return log(values[0]);
}
void use_log(){
    logw.function = log_func;
    init_Fwrapper(&logw,"logarithm",1,4);  
}
//square,cube,quart
float square_func(float *values){
    if (values[0]*values[0]>MAXNUM)    return 1.00;    
    return values[0]*values[0];
}
void use_square(){
    squarew.function = square_func;
    init_Fwrapper(&squarew,"square",1,2);  
}
float cube_func(float *values){
    if (pow(values[0],2)>MAXNUM || pow(values[0],3)>MAXNUM)    return 1.00;    
    return pow(values[0],3);
}
void use_cube(){
    cubew.function = cube_func;
    init_Fwrapper(&cubew,"cube",1,2);  
}
float quart_func(float *values){
    if (pow(values[0],2)>MAXNUM || pow(values[0],3)>MAXNUM || pow(values[0],4)>MAXNUM)    return 1.00;    
    return pow(values[0],4);
}
void use_quart(){
    quartw.function = quart_func;
    init_Fwrapper(&quartw,"quart",1,2);  
}
float sqrt_func(float *values){
    if (values[0]<=0)    return 0;
	return sqrt(values[0]);    
}
void use_sqrt(){
    sqrtw.function = sqrt_func;
    init_Fwrapper(&sqrtw,"sqrt",1,3);  
}


void assign_flist_basic(){
	flist_basic[0]=addw;	flist_basic[1]=subw;	flist_basic[2]=mulw;	flist_basic[3]=divw;
}
void assign_flist_higher(){
	flist_higher[0]=sinw;  flist_higher[1]=cosw;  flist_higher[2]=logw;  flist_higher[3]=expw;    flist_higher[4]=poww;
}
void assign_flist_primary(){
	flist_primary[0]=addw;	flist_primary[1]=subw;	flist_primary[2]=mulw;	flist_primary[3]=divw;
	flist_primary[4]=sinw;  flist_primary[5]=cosw;  flist_primary[6]=logw;  
}
void assign_flist_extend(){
	flist_extend[0]=addw;	flist_extend[1]=subw;	flist_extend[2]=mulw;	flist_extend[3]=divw;
	flist_extend[4]=cubew;	flist_extend[5]=quartw;	flist_extend[6]=squarew;	flist_extend[7]=sqrtw;
}
void assign_flist_big(){
	flist_big[0]=addw;	flist_big[1]=subw;	flist_big[2]=mulw;	flist_big[3]=divw;
	//flist_big[4]=cubew;	flist_big[5]=quartw;	flist_big[6]=squarew;	flist_big[7]=sqrtw;
	flist_big[4]=poww;	flist_big[5]=expw;		
}
void assign_flist(){
	use_add();  use_sub();  use_mul();  use_div();
	use_sin();  use_cos();  use_log();  use_exp(); use_pow();
	use_square();	use_cube();	use_quart(); use_sqrt();
	assign_flist_basic(); 	//四则运算 
	assign_flist_higher(); 	//几个高级运算 
	assign_flist_primary(); //四则+三角+log 
	assign_flist_extend();	//常用-四则+二三四次
	assign_flist_big();		//常用扩展- +指数幂函 
} 


