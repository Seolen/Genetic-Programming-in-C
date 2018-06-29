#include "gp.h"
#include "gp.c" 
#include "function_set.c"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
 

int main(){
	//基本定义 
	
    srand((int)time(0));    //随机数种子
    
    //初始化 
    set_data(400); 	
	MAX_DEPTH = 4;  LIMIT=0.99;
    
    //flist_use = flist_primary;    flist_use_count=7;
	flist_use = flist_basic;    flist_use_count=4;
	//flist_use = flist_extend;    flist_use_count=8;
	//flist_use = flist_big;    flist_use_count=6;
	assign_flist(); 
	
	//test_new_operator();
	//test_population();
	//test_complexity();
 	//evolve_process(30);
 	//printf("END EVOLVE\n");
 	test_module();
    
    
    while(1);
    return 0;
}
