#include "gp.h"
#include "function_set.c" 
#include "dataset.c"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <time.h>

/* 程序树的计算和呈现 evaluate and display */

float evaluate_node(Node* head,float* inputs){
    //if terminal case
    if (head->node_type!=0){
        if (head->node_type==1)  return inputs[head->index];
        else return head->value;
    }
    //if node case
	int childcount = head->fw->childcount;       
    int i=0;
    float temparr[childcount];  
    for (;i<childcount;++i){
        Node* current_node = head->children[i];      // current_node 会被赋值指针类型
        temparr[i] = evaluate_node(current_node,inputs);
    }
    return head->fw->function(temparr);
}
float evaluate_aeg(Node* head,float* inputs,float* constants){	//head is of abstract expression
    //if terminal case
    if (head->node_type!=0){
        if (head->node_type==1)  return inputs[head->index];
        else {	//node_type==3
        	return constants[head->index];
		}
    }
    //if node case
	int childcount = head->fw->childcount;       
    int i=0;
    float temparr[childcount];  
    for (;i<childcount;++i){
        Node* current_node = head->children[i];      // current_node 会被赋值指针类型
        temparr[i] = evaluate_aeg(current_node,inputs,constants);
    }
    return head->fw->function(temparr);
}
//indent 初始调用使用0。当然其他自然数也可以，无妨。
void display(Node *head,int indent){
    int i=0;
    for (;i<indent;++i)     printf("%s",symbol);
    if (head->node_type!=0){
        if (head->node_type==1)
            printf("p%d\n",head->index);
        else if (head->node_type==3) 
        	printf("c%d\n",head->index);
        else printf("%f\n",head->value);
    }
    else{       //non-terminal case
        printf("%s\n",head->fw->name);
        int count = head->fw->childcount;
        for (i=0;i<count;++i){
            Node* current_node = head->children[i];      // current_node 会被赋值指针类型
            display(current_node,indent+1);
        }
    }
    	if (indent==0)
			printf("\n"); 
}
void display_formula(Node *head){
	if (head->node_type!=0){
		if (head->node_type==1)
			printf(" p%d ",head->index);
		else if (head->node_type==3)
			printf(" c%d ",head->index);
		else	printf(" %.3f ",head->value);
	}
	else{
		int i;
		char* sym = "";
		switch(head->fw->name[0]){
			case 'a': sym="+";	break;
			case 's': 
                if (head->fw->name[1]=='u') sym="-";
                if (head->fw->name[1]=='i') sym="sin";
                if (head->fw->name[1]=='q') sym="square";
            	break;
			case 'm': sym="*";  break;
			case 'd': sym="/";	break;	
            case 'e': sym="^";  break;
            case 'p': sym="^";  break;
            case 'c':
				if (head->fw->name[1]=='u')	sym="cube";
				if (head->fw->name[1]=='o')	sym="cos";
			 	break;	
            case 'l': sym="ln"; break;	
            case 'q': sym="quart";break;
		}
		printf("(%s",sym);
		for (i=0;i<head->fw->childcount;++i){
			display_formula(head->children[i]);
		}
		printf(")");		
	}
}

/**树的随机生成
 * @PC: 变量的个数，paramnode count   @maxdepth: default-5,树的最大深度
 * @method: 生成方法(grow|full)       
 * @fpr,ppr: 仅在grow方法有意义。新建节点为函数型节点概率，不是函数节点时param node的概率
**/
Node* random_generation(int maxdepth,char* method,float fpr,float ppr){
	//default value
	if (maxdepth<0) maxdepth=5;
	if (method[0]!='f')	method="grow";
	if (fpr<0)	fpr=0.5;
	if (ppr<0)	ppr=0.4;
	
    if ( maxdepth==0 || ((method[0]=='g') && (random()>fpr)) ){ // generate terminal
        if (random()<ppr){  //param node, type=1
            Node *param = new_Node();
            init_node_type(param,1); init_node_terminal(param,randint(PC),-1);
            return param;
        }else{              //const node, type=2, random const (0,10)
            Node *const1 = new_Node();
            init_node_type(const1,2); init_node_terminal(const1,-1,randint(10)+random());
            return const1;
        }
    }
    else{       //generate function node
        int i;
        //choose a function randomly 
        int temp = randint(flist_use_count);
        Fwrapper* fw = &flist_use[temp];		//此处出过大错，未使用*fw而是fw 
        Node** children = new_children(fw->childcount);

        //单独对exp,pow做出生成上的限制，如第一个变量为变量，第二个变量为常量
        if (strcmp(fw->name,expw.name)==0){
            children[0] = random_generation(maxdepth-1,method,fpr,0); //常量，或没有变量 
            children[1] = random_generation(maxdepth-1,method,fpr,1); //变量，或没有常量 
        }
        else if (strcmp(fw->name,poww.name)==0){
            children[0] = random_generation(maxdepth-1,method,fpr,1); //变量，或没有常量 
            children[1] = random_generation(maxdepth-1,method,fpr,0); //常量，或没有变量 
        } 
		else{
			for (i=0;i<fw->childcount;++i){       
            children[i] = random_generation(maxdepth-1,method,fpr,ppr);   
        	}
		}
        
        Node *head = new_Node();
        init_node_type(head,0);
        init_node_function(head,fw,children);
        return head;
    }
}

//deepcopy()用于复制一棵树，不是简单地指针指向
Node* deepcopy(Node* head){
	int type = head->node_type;
	if (type==1 || type==2 || type==3){
		Node* root = new_Node();
		init_node_type(root,type);
		if (type==1)	init_node_terminal(root,head->index,-1);
		else if (type==2)	init_node_terminal(root,-1,head->value);
		else{
			root->index = head->index;
		}		
		return root;
	}
	else{		// a function node
		Node* root = new_Node();
		init_node_type(root,0);
		Node** children = new_children(head->fw->childcount);
		int i;
		for (i=0;i<head->fw->childcount;++i){
			children[i] = deepcopy(head->children[i]);
		}
		init_node_function(root,head->fw,children);
		return root;
	}
} 
void deepfree(Node* head){
    if (head->node_type!=0)
        free(head);
    else{
        int i;
        for (i=0;i<head->fw->childcount;++i)
            deepfree(head->children[i]);
        free(head->children);
        free(head);
    }
}
/*变异操作mutate,crossover，都是有概率发生的结构*/
//不能直接在原树修改。
// @prob_change默认值0.1; mutate最大层数暂定为3; 
//Q: free(t)之后，t指针还在

//任选一个节点，进行变异 
int cal_nodes(Node* head){
	if (head->node_type!=0)
		return 1;
	int count = head->fw->childcount, i=0, nodes=0;
	for (i=0;i<count;++i){
		nodes += cal_nodes(head->children[i]);
	}
	return nodes+1;
} 
int cal_constants(Node* head){
	if (head->node_type!=0){
		if (head->node_type==2)	return 1;	//a constant 
		else return 0;
	}
	int constants = 0,count = head->fw->childcount;
	int i;for (i=0;i<count;++i)
		constants += cal_constants(head->children[i]);
	return constants;
}

Node* get_subtree(Node* head, int node, int count){
	if (node==0)	return head;
	int i=0;
	int cnum = head->fw->childcount;
	
	count++;
	for (i=0;i<cnum;++i){
		int sub_nodes = cal_nodes(head->children[i]);
		if (sub_nodes+count>node)	{		//在这一子树里搜索 
			node -= count;	count=0;
			return get_subtree(head->children[i],node,count);
		}
		else{
			count += sub_nodes;
		}		
	}
} 
Node* replace_subtree(Node* head,Node* subtree, int node, int count){
	if (node==0){
			//display_formula(head);printf("original above\n");
		deepfree(head);
		head = deepcopy(subtree);	//mutate则记得释放随机树空间 
			//display_formula(head);printf("mutate above\n");
		return head;
	}	
	int i=0;
	int cnum = head->fw->childcount;
	
	count++;
	for (i=0;i<cnum;++i){
		int sub_nodes = cal_nodes(head->children[i]);
		if (sub_nodes+count>node)	{		//在这一子树里搜索 
			node -= count;	count=0;
			head->children[i] = replace_subtree(head->children[i],subtree,node,count);	//#
			break;	//# 阻止for循环继续修改下一个分支，退出即可 
		}
		else{
			count += sub_nodes;
		}		
	}
	return head;
} 
Node* mutate(Node* head){	
	int nodes = cal_nodes(head);
	int	randnode = randint(nodes);	//randnode 为待突变的节点编号
		//printf("\nnode\t%d\n",randnode);
	// 前序遍历，找到第randnode节点
	Node* root = deepcopy(head);
	Node* sub = random_generation(MAX_MUTATE_DEPTH,"grow",-1,-1);
	root = replace_subtree(root,sub,randnode,0);
	deepfree(sub);
		//display_formula(root);printf("\n");
	return root;
}
//随机选择两个节点，进行交换
Node* crossover(Node* t1,Node* t2){
	int nodes1 = cal_nodes(t1), nodes2 = cal_nodes(t2);
	int r1 = randint(nodes1), r2 = randint(nodes2);
	Node* sub = get_subtree(t2,r2,0); 
		//printf("\nr1\tr2\t%d\t%d\n",r1,r2);
	
	Node* root = deepcopy(t1);	
	root = replace_subtree(root,sub,r1,0);	
	return root;
} 


//stucture Abstract Expression Grammar
typedef struct Particle{
	float *constants,*velocity,*lbest;
	float fitness;	//fitness is lbest's fitness
}Particle;
typedef struct Pool{
	Particle** particles;
	int pool_count;
	
	float gbest;
}Pool;
typedef struct AEG{
	Node* head;		//全程不做变化 
	Node* a_head;	//abstract head expression
	float* constants;	//用指针表示数组，可以按需分配空间
	Pool* pool;
	
	int constant_count; 
	int nodes;	 
}AEG;
float* copy_array(float* a,float* b,int count){	//from b to a
	int i;
	if (a!=NULL)  free(a);
	a = (float*)malloc(sizeof(float)*count);
	for(i=0;i<count;++i)	a[i] = b[i];
	return a;
}
float* random_array(float* a,int count){	//generate random array
	if (a!=NULL)  free(a);
	a=(float*)malloc(sizeof(float)*count);
		
	int i; for (i=0;i<count;++i){
		a[i] = rand_constant();
	}
	return a;		
}
int assign_AEG(float* constants,Node* head,int ccount){		//返回当前ccount | 同时记录constants与修改原树为a_head 
	if (head->node_type==2){
		constants[ccount] =  head->value;
		head->node_type = 3;	head->index = ccount;
		ccount++; 
	} 
	else if (head->node_type==0) {
		int count = head->fw->childcount,i;
		for (i=0;i<count;++i)
			ccount = assign_AEG(constants,head->children[i],ccount); 
	} 
	return ccount; 
}
int assign_Sexp(Node* head,float* constants,int ccount){
	if (head->node_type==3){
		head->node_type = 2;
		head->value = constants[ccount++];
	} 
	else if (head->node_type==0) {
		int count = head->fw->childcount,i;
		for (i=0;i<count;++i)
			ccount = assign_Sexp(head->children[i],constants,ccount); 
	}		 
	return ccount; 
}
/*
void convert_to_AEG(AEG* aeg,Pool* pool2,float* constants){	//from with only head to a complete aeg //第二参数和第三参数输入一个即可 
	//aeg有可能已经有原来的值，故要注意是否释放空间 
	//pool有可能已申请空间，也可能没有,没有的就为它们申请 
	if (aeg->a_head!=NULL)	deepfree(aeg->a_head);	aeg->a_head = deepcopy(aeg->head);
	//calculate the number of constants in expression,assign aeg
	aeg->constant_count = cal_constants(aeg->head);
	aeg->nodes = cal_nodes(aeg->head);
	if (aeg->constants!=NULL)
		free(aeg->constants);
	aeg->constants = (float*)malloc(sizeof(float)*aeg->constant_count);
	assign_AEG(aeg->constants,aeg->a_head,0);
		//printf("Mid\n");
	//assign pool
	if (aeg->pool==NULL){
		aeg->pool = (Pool*)malloc(sizeof(Pool));
		aeg->pool->particles=NULL;	
	}  
	Pool* pool = aeg->pool;
	if (pool->particles==NULL){		//allcate pool
		pool->particles = (Particle**)malloc(sizeof(Particle*)*MAX_POOLSIZE);
		int i;for (i=0;i<MAX_POOLSIZE;++i){
			pool->particles[i] = (Particle*)malloc(sizeof(Particle));
			Particle* p = pool->particles[i];
			p->constants = p->lbest = p->velocity = NULL;
		}
		copy_array(pool->particles[0]->constants,aeg->constants,aeg->constant_count);
		pool->pool_count=1;	
	}	
	else if (pool2!=NULL) {	//replace pool	
		pool->pool_count = pool2->pool_count;
		int i;for (i=0;i<pool->pool_count;++i){
			copy_array(pool->particles[i]->constants,pool2->particles[i]->constants,aeg->constant_count);
			copy_array(pool->particles[i]->velocity,pool2->particles[i]->velocity,aeg->constant_count);
			copy_array(pool->particles[i]->lbest,pool2->particles[i]->lbest,aeg->constant_count);
			pool->particles[i]->fitness = pool2->particles[i]->fitness;
		}
	}
	else if (constants!=NULL){
		copy_array(pool->particles[pool->pool_count]->constants,constants,aeg->constant_count);
	} 
	
}
*/
AEG* convert_to_AEG2(AEG* aeg){	//from with only head to a complete aeg //用在程序中 
	if (aeg->a_head!=NULL)	deepfree(aeg->a_head);	aeg->a_head = deepcopy(aeg->head);
	//calculate the number of constants in expression,assign aeg
	aeg->constant_count = cal_constants(aeg->head);
	aeg->nodes = cal_nodes(aeg->head);
	if (aeg->constants!=NULL)		free(aeg->constants);
	aeg->constants = (float*)malloc(sizeof(float)*aeg->constant_count);
	assign_AEG(aeg->constants,aeg->a_head,0);
	//assign pool 
	/*修改处 
	Pool* pool = aeg->pool;
	pool->particles[0]->constants=copy_array(pool->particles[0]->constants,aeg->constants,aeg->constant_count);
	pool->pool_count=1;	*/	
	return aeg;
	
}
void convert_to_SExp(Node* head,float* constants){
	assign_Sexp(head,constants,0);
} 

/* Population 操作 */
typedef struct InfoTree{
	//Node* head;
	AEG* aeg;
	int island;
	float fitness;
	int weight;
}InfoTree;
typedef struct Population{
	int num;	//有多少个体 
	InfoTree** infotree;
}Population;

/* fitness function */
float score_function(Node* head,float** dataset,int data_num){
    float score=0.00,v=0.00;
    int i;
    for (i=0;i<data_num;++i){
        v = evaluate_node(head,dataset[i]);
        score += pow(fabs(dataset[i][2]-v),2);
    }
    return score/data_num;
}
float score_aeg(Node* head,float* constants,float** dataset,int data_num){
    float score=0.00,v=0.00;
    int i;
    for (i=0;i<data_num;++i){
        v = evaluate_aeg(head,dataset[i],constants);
        score += pow(fabs(dataset[i][2]-v),2);
    }
    return score/data_num;
}

int equal_aexp(Node* head1,Node* head2){
	if (head1->node_type!=head2->node_type)
		return 0;
	if (head1->node_type==0){
		if (strcmp(head1->fw->name,head2->fw->name)!=0)
			return 0;
		int i,count = head1->fw->childcount,result=1;
		for (i=0;i<count;++i)
			result = (result && equal_aexp(head1->children[i],head2->children[i]));
		return result;
	}
	if (head1->node_type==1){
		if (head1->index==head2->index)
			return 1;
		return 0;
	}
	if (head1->node_type==3)
		return 1;
	return 0;
}
void sort_pool(Pool* pool){	//insert_sort	//pre:所有fitness应该计算完成 
	int i,j;
	for(i=1;i<pool->pool_count;++i){
		Particle* temp = pool->particles[i];
			if (temp->fitness<=0){
				printf("\nError-Particle %d have no fitness\n",i);
				return;
			}
		for (j=i;j>0&&pool->particles[j-1]->fitness>temp->fitness;--j){
			pool->particles[j] = pool->particles[j-1];
		}
		pool->particles[j] = temp;
	}
} 

void insert_lambda(Population* pop,InfoTree* lambda,int location){	//lambda要有constants，fitness 	//与replace不同，包含合并过程 
	//不再合并pool，或者说，不再保留pool信息-对应的，优化算法中，所有pool都随机产生 
	int i;	AEG *aeg,*temp_aeg;	
	temp_aeg = lambda->aeg; 
	
	//init a_head,constants and aeg others
	temp_aeg = convert_to_AEG2(temp_aeg); 
	int r1 = temp_aeg->nodes;
	int ccount = temp_aeg->constant_count;
	
	/*修改处 
	if (temp_aeg->a_head==NULL){
		temp_aeg->a_head=deepcopy(temp_aeg->head);
	} 
	if (temp_aeg->constants==NULL)  	//#修改处 
		temp_aeg->constants = (float*)malloc(sizeof(float)*ccount);		
	assign_AEG(temp_aeg->constants,temp_aeg->a_head,0);
	*/ 
		//检查
		/*
		printf("Added to pop\n");
		printf("Fitness: %.2f\n",lambda->fitness);
		printf("Head\t");	display_formula(temp_aeg->head);
		printf("\nA_head\t");	display_formula(temp_aeg->a_head);
		printf("\nConstants\t");	for(i=0;i<ccount;++i)	printf("%.2f\t",temp_aeg->constants[i]);
		printf("\n\n**************************************\n");//*/	
		
	for(i=0;i<pop->num;++i){
		aeg = pop->infotree[i]->aeg;
		//not the homemorphic of all		
		if (r1!=aeg->nodes || !equal_aexp(temp_aeg->a_head,aeg->a_head)){
			continue;
		}
		//the homemorphic of one
		else{	//merge
				/*//
				printf("Find place %d to merge\n",i);
				printf("A_head\t");	display_formula(aeg->a_head);*/
		
			//单纯的替换或不替换constants,fitness
			if (lambda->fitness<pop->infotree[i]->fitness){
				aeg->constants = copy_array(aeg->constants,temp_aeg->constants,ccount);
				pop->infotree[i]->fitness = lambda->fitness;
			} 
			break; 
		}
	}
	if (i==pop->num){	//added to the last
		//added to the last, create pool
		InfoTree* it = pop->infotree[pop->num];
		it->fitness = lambda->fitness;	
		it->weight = lambda->weight;
		it->island = lambda->island;
		it->aeg->constant_count = ccount;
		if(it->aeg->head!=NULL)		deepfree(it->aeg->head);
		if(it->aeg->a_head!=NULL)		deepfree(it->aeg->a_head);
		it->aeg->head = deepcopy(temp_aeg->head);	
		it->aeg->a_head = deepcopy(temp_aeg->a_head);	
		//replace constants
		aeg = it->aeg;
		it->aeg->constants=copy_array(it->aeg->constants,lambda->aeg->constants,ccount);
		it->fitness = lambda->fitness;
			//
			/*
			printf("IN POP NUM\n");
			printf("Fitness: %.2f\n",it->fitness);
			printf("Head\t");	display_formula(aeg->head);
			printf("A_head\t");	display_formula(aeg->a_head);
			printf("Constants\t");	for(i=0;i<ccount;++i)	printf("%.2f\t",aeg->constants[i]);//*/ 
	
	} 
	//printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
} 

float* random_velocity(float* v,int count){	//#待完成 
	if(v!=NULL)  free(v);
	v=(float*)malloc(sizeof(float)*count);
	int i;for (i=0;i<count;++i){
		v[i] = rand_constant()/2;
	} 
	return v;
}
void free_particle(Particle* p){
	if(p->constants!=NULL) free(p->constants);
	if(p->lbest!=NULL) free(p->lbest);
	if(p->velocity!=NULL) free(p->velocity);
	if(p!=NULL) free(p); 
}
InfoTree* optimize_constant(InfoTree* it){
	
	AEG* aeg = it->aeg;	Pool* pool = it->aeg->pool;
		if (pool==NULL){
			printf("POOL EXMPTY IN OPTIMIZE_CONSTATN\n");
		}
	int i,j,k;
	int ccount;
	ccount=cal_constants(it->aeg->head);
	float lbest_fitness,gbest_fitness,cur_fitness;	
	float limit=1.0;	int looptime = 20;	//stop condition, 1.0 & 20(can be larger)
	float min_bound=-20.00,max_bound=20.00,max_velocity=5.00;	//boundary of particles' position and velocity, min and max
	float* gbest = NULL;	gbest_fitness=it->fitness;	//gbest and its fitness
	gbest=copy_array(gbest,it->aeg->constants,ccount);
	//fill pool with extra random vectors
	/*
	for (i=0;i<pool->pool_count;++i){
		if (pool->particles[i]->fitness<=0)	
			pool->particles[i]->fitness = score_aeg(aeg->a_head,pool->particles[i]->constants,DATA,DATANUM);
		pool->particles[i]->velocity = random_velocity(pool->particles[i]->velocity,ccount);
		pool->particles[i]->lbest=copy_array(pool->particles[i]->lbest,pool->particles[i]->constants,ccount);
	}
	*/ 
		//printf("enter IN OPTIMIZE_CONSTATN\n");
	pool->particles[0]->constants=copy_array(pool->particles[0]->constants,aeg->constants,ccount);
	pool->particles[0]->lbest=copy_array(pool->particles[0]->lbest,aeg->constants,ccount);
	pool->particles[0]->velocity = random_velocity(pool->particles[0]->velocity,ccount);
	pool->particles[0]->fitness = it->fitness;
	pool->pool_count=1;
	for (i=pool->pool_count;i<MAX_POOLSIZE;++i){
		pool->particles[i]->constants=random_array(pool->particles[i]->constants,ccount);
		pool->particles[i]->velocity = random_velocity(pool->particles[i]->velocity,ccount);
		pool->particles[i]->lbest=copy_array(pool->particles[i]->lbest,pool->particles[i]->constants,ccount);
		pool->particles[i]->fitness = score_aeg(aeg->a_head,pool->particles[i]->lbest,DATA,DATANUM);
		if (gbest_fitness>pool->particles[i]->fitness){			
			gbest_fitness=pool->particles[i]->fitness;
			gbest=copy_array(gbest,pool->particles[i]->lbest,ccount);
		} 
	}
	pool->pool_count = MAX_POOLSIZE;
		//检查particle各个参数是否存在
		/*
		for (i=0;i<pool->pool_count;++i){
			printf("FITNESS %.2f\tConstants: ",pool->particles[i]->fitness);
			for (j=0;j<aeg->constant_count;++j){
				printf("%.2f\t",pool->particles[i]->constants[j]);
			}
			printf("\n");
		}*/ 
		//printf("mid IN OPTIMIZE_CONSTATN\n");
	//iterate until santisfy a condition
	for (k=0;k<looptime&&gbest_fitness>limit;++k){
		for (i=0;i<MAX_POOLSIZE;++i){
			//update velocity
			Particle* p = pool->particles[i];
				
			float WL = 1.49;//0.25+(maxg-g)*1.0/maxg;	//local weight
			float WG = 1.49;//0.75+(maxg-g)*1.0/maxg;	//global weight
			float WV = 0.5;
			float r1=random(),r2=random();	
			for (j=0;j<ccount;++j){
					//printf("here0 IN OPTIMIZE_CONSTATN\n");
				float lnudge = (WL*r1*(p->lbest[j]-p->constants[j]));
					//printf("here1 IN OPTIMIZE_CONSTATN\n");
				float gnudge = (WG*r2*(gbest[j]-p->constants[j]));
					//printf("here2 IN OPTIMIZE_CONSTATN\n");
				float tv = (WV*(p->velocity[j]))+lnudge+gnudge;
					//printf("here3 IN OPTIMIZE_CONSTATN\n");
				if (tv>max_velocity)	tv=max_velocity;	//v boundary
				else if (tv<0-max_velocity)	tv=0-max_velocity;
				p->velocity[j] = tv;
				
				p->constants[j] = p->constants[j]+p->velocity[j];
					//printf("here4 IN OPTIMIZE_CONSTATN\n");
				//boundary limitation
				if (p->constants[j]<min_bound)	p->constants[j]=min_bound;
				if (p->constants[j]>max_bound)	p->constants[j]=max_bound;
			}
			//update lbest and gbest
			cur_fitness = score_aeg(aeg->a_head,p->constants,DATA,DATANUM);
			lbest_fitness = p->fitness;
			if (cur_fitness<lbest_fitness){
				p->lbest=copy_array(p->lbest,p->constants,ccount);
				p->fitness = cur_fitness;
				lbest_fitness = cur_fitness;
				if (cur_fitness<gbest_fitness){
					gbest=copy_array(gbest,p->constants,ccount);
					gbest_fitness = cur_fitness;
				}
			}
		} 
	}
		//printf("latter IN OPTIMIZE_CONSTATN\n");
	//set aeg->constants and fitness
		//printf("Gbest constants:\t");
		//for (i=0;i<ccount;++i)	printf("%.2f\t",gbest[i]);	printf("\n");
	aeg->constants=copy_array(aeg->constants,gbest,ccount);
	it->fitness = gbest_fitness;
	it->aeg->pool->pool_count=1;
	free(gbest);
	
	if (aeg->head!=NULL) deepfree(aeg->head);
	aeg->head=deepcopy(aeg->a_head);
	convert_to_SExp(aeg->head,aeg->constants);
		//printf("last IN OPTIMIZE_CONSTATN\n");
		/*
		printf("Optimized fitness %.2f\n",it->fitness);
		for (i=0;i<ccount;++i){
			printf("%.2f\t",aeg->constants[i]);
		}
		printf("\n");*/
	return it;	 
}

void add_random_individual(Population* outpop,int num){
	int K = num,i;
	for (i=0;i<K;++i){
		Node* tree;
		if (i<K/2)	 tree = random_generation(MAX_DEPTH,"full",-1,-1);
		else		 tree = random_generation(MAX_DEPTH,"grow",-1,-1);
		
		InfoTree* infotree = outpop->infotree[outpop->num];
			//display_formula(tree);
		
		if (infotree->aeg->head!=NULL) deepfree(infotree->aeg->head);	//所有空间默认已经分配，故每次都要释放已有空间。 
		infotree->aeg->head = tree;
		infotree->fitness = score_function(tree,DATA,DATANUM);
		infotree->island = 0;
		infotree->weight = -1;
		
		convert_to_AEG2(infotree->aeg);
		infotree->aeg->pool->particles[0]->fitness=infotree->fitness;
		outpop->num++; 		
			//printf("outpop->num\t%d\nfitness\t%f\n",outpop->num,outpop->infotree[outpop->num-1]->fitness); 
			
	}
}

int comp(const void*a,const void*b){     //qsort的比较函数
	InfoTree *i1 = *(InfoTree**)a, *i2 = *(InfoTree**)b;
	return(i1->fitness>i2->fitness)?1:-1;
}
void* sort_fitness(Population* pop){
	qsort(pop->infotree,pop->num,sizeof(InfoTree*),comp);  
}

	void show_population(Population* out);	//#待删除 

int weighted_complexity(Node* head,int depth){	//计算一棵树的复杂度 
	int complexity = 0,i;
	if (head->node_type==0){
		//a function node
		int weight = head->fw->weight;
		int count = head->fw->childcount;
		for (i=0;i<count;++i){
			complexity += weighted_complexity(head->children[i],weight*(depth+1));
		}
	}
	else{
		complexity = depth+1;
	}
	return complexity;	
}

InfoTree* population_pruning(Population* inpop,Population* outpop){
	//int max_island_count = 1;
	int i;
	if (inpop->num<=0){
		//initialize with random individuals
		add_random_individual(outpop,MAX_POPSIZE*5);
	}
	else{	
		add_random_individual(outpop,MAX_POPSIZE/10);		
	}
	sort_fitness(outpop);
		//show_population(outpop);
		/*
		printf("NUM: %d\n",outpop->num);
		for(i=0;i<outpop->num;++i){
			printf("%d\tFitness: %.2f\n",i,outpop->infotree[i]->fitness);
			display_formula(outpop->infotree[i]->aeg->head);	printf("\n");
			display_formula(outpop->infotree[i]->aeg->a_head);	printf("\n");
		} */
		
	inpop->num = 0;		//clear inpop
		
	//compute weight range
	int low=1000,high=0,range;
	for (i=0;i<outpop->num;++i){
		InfoTree* it = outpop->infotree[i];
		if (it->weight<0)	it->weight = weighted_complexity(it->aeg->head,0);
		int weight = it->weight;
		if (weight<low)	low = weight;
		if (weight>high)	high = weight;
			//检查weight 
			//if ((i+1)%10!=0)	printf("%d\t",weight);
			//else	printf("%d\n",weight);
	}
	range = (high-low);
	
	//assign island_counts[max_island_count],islands[max_island_count] 
	memset(island_counts,0,MAX_ISLAND_COUNT); 	
	for (i=0;i<outpop->num;++i){
		InfoTree* it = outpop->infotree[i];
		it->island = floor( (it->weight-low)*1.0/range *MAX_ISLAND_COUNT );	//对应到一个层级island,50 layers
			if (it->island>=MAX_ISLAND_COUNT)	it->island = MAX_ISLAND_COUNT-1;	//# 最高的分配到低一个层级 
		int island = it->island;
		island_counts[island]++;
		if (island_counts[island]<=MAX_ISLAND_SIZE){
			//将此个体放入inpop,设置islands[]
			insert_lambda(inpop,it,inpop->num);	//a- it无pool&inpop无pool b- it有pool,inpop有pool 
				//replace_lambda(inpop,it->head,it->fitness,it->island,it->weight,inpop->num);  
				//检查 
			inpop->num++;
			int count = island_counts[island];
			islands[island][island_counts[island]-1] = inpop->num-1;
		}
	}
		//检查islands,island_count
		/*
		int k;
		printf("Island Counts\n");
		for (j=0;j<MAX_ISLAND_COUNT;++j){
			printf("%d\t",island_counts[j]);
			if ((j+1)%10==0)	printf("\n");
		} 
		printf("Islands\n");
		for(j=0;j<MAX_ISLAND_COUNT;++j){
			for (k=0;k<island_counts[j];++k){
				printf("%d %.1f\t",islands[j][k],inpop->infotree[islands[j][k]]->fitness);
				if ((k+1)%5==0 )	printf("\n");
			}
			printf("\n");
		}	
		*/
	outpop->num = 0;
		
		//检查inpop
		/*
		printf("PR INPOP NUM: %d\n",inpop->num);
		for(i=0;i<inpop->num;++i){
			printf("%d\tFitness: %.2f\n",i,inpop->infotree[i]->fitness);
			display_formula(inpop->infotree[i]->aeg->head);	printf("\n");
			display_formula(inpop->infotree[i]->aeg->a_head);	printf("\n");
			printf("Constants: \t");int j;for (j=0;j<inpop->infotree[i]->aeg->constant_count;++j){
				printf("%.2f\t",inpop->infotree[i]->aeg->constants[j]);
			}
			printf("\n");
		}
		printf("\n&&&&&&&&&&&&&&&&&&&&&&&&&\n\n"); 
		//optimize constant
//		for(i=0;i<inpop->num;++i){
//			display_formula(optimize_constant(inpop->infotree[i]));	printf("\n");
//		}
		//*/
		
		 
	/*
	for (i=0;i<MAX_POPSIZE;++i){	//复制到inpop 
		//replace_lambda(inpop,outpop->infotree[i]->head,outpop->infotree[i]->fitness,i); 
	} */	
	//outpop->num = MAX_POPSIZE;
	//inpop->num=MAX_POPSIZE;
	return inpop->infotree[0];
} 
InfoTree* new_individual(){
	InfoTree* it = (InfoTree*)malloc(sizeof(InfoTree));
	//默认直接分配空间，每次赋值要先释放空间，再直接指针赋值 
	it->aeg = (AEG*)malloc(sizeof(AEG));
	it->aeg->head = it->aeg->a_head = NULL; it->aeg->constants = NULL;
		//it->aeg->pool = NULL;	
	it->aeg->pool = (Pool*)malloc(sizeof(Pool));
	it->aeg->pool->pool_count=0;
	it->aeg->pool->particles = (Particle**)malloc(sizeof(Particle*)*MAX_POOLMEM);
	int i;for(i=0;i<MAX_POOLMEM;++i){
		it->aeg->pool->particles[i] = (Particle*)malloc(sizeof(Particle));
		it->aeg->pool->particles[i]->constants=NULL;
		it->aeg->pool->particles[i]->velocity=NULL;
		it->aeg->pool->particles[i]->lbest=NULL;
	} 
	 
	
	return it;
}
InfoTree** new_infotree(int itnum){
	int i;
	InfoTree** its = (InfoTree**)malloc(sizeof(InfoTree*)*itnum);
	for(i=0;i<itnum;++i){
		its[i] = new_individual();
	}
	return its;	
}
Population* init_population_space(){	//必须在申请空间后，把原指针指向创建的指针，确实最后一句是不可的。 
	Population* apop = (Population*)malloc(sizeof(Population));
	apop->num=0;
	apop->infotree = new_infotree(MAX_POPSIZE*5);
	return apop;
}
void evolve_process(int Generation){
	int i,j;	InfoTree* champ;
	Population *in_population,*out_population;	
	in_population = init_population_space();	//初始化 
	out_population = init_population_space();
	
	population_pruning(in_population,out_population);
		//printf("first pruning continue\n");	//# 
	
	for(i=0;i<Generation;++i){
		//mutate and crossover for everyone
		for (j=0;j<in_population->num;++j){	
			//copy inpop to outpop first, then other operations.
			/*
			InfoTree* it = in_population->infotree[j];*/
			insert_lambda(out_population,in_population->infotree[j],out_population->num);	//有head/fitness，其他无，要创建 
				//replace_lambda(out_population,it->head,it->fitness,it->island,it->weight,out_population->num);
			out_population->num++;//*/ 
			
				//printf("1 OPTIMIEZED CONSTANT\n");
			in_population->infotree[j] = optimize_constant(in_population->infotree[j]);
				/*printf("ROUND %d  Optimiezed: %d\n",i,j);
				display_formula(in_population->infotree[j]->aeg->head);
				printf("\nfitness:%.2f\n",in_population->infotree[j]->fitness);*/
			insert_lambda(out_population,in_population->infotree[j],out_population->num);			
			out_population->num++;
			
				//printf("2 MUTATE\n");
			Node* root2 = mutate(in_population->infotree[j]->aeg->head);
				//display_formula(root2);	printf("\n");
			InfoTree* temp = (InfoTree*)malloc(sizeof(InfoTree));
			temp->aeg = (AEG*)malloc(sizeof(AEG));
			temp->island = temp->weight = -1;
			temp->fitness = score_function(root2,DATA,DATANUM);
				//printf("root2 fitness: %.2f\n",temp->fitness);
			temp->aeg->head = root2;	temp->aeg->a_head=NULL;	temp->aeg->pool=NULL;	temp->aeg->constants=NULL;
			insert_lambda(out_population,temp,out_population->num);
			deepfree(temp->aeg->head);	deepfree(temp->aeg->a_head);	free(temp->aeg->constants);	
			temp->aeg->head=NULL;	temp->aeg->a_head=NULL;	temp->aeg->constants=NULL;	
			out_population->num++;
				//printf("num\t%d\tfitness\t%f\n",out_population->num,out_population->infotree[out_population->num-1]->fitness);
					
			//crossover only the same island
				//printf("3 CROSSOVER\n");
			int island = in_population->infotree[j]->island;
			int r0;
			if (island_counts[island]>MAX_ISLAND_SIZE)	r0 = MAX_ISLAND_SIZE;
			else	r0 = island_counts[island];
			int k = randint(r0);
			k = islands[island][k];
			Node* root3 = crossover(in_population->infotree[j]->aeg->head,in_population->infotree[k]->aeg->head); 
			temp->aeg->head = root3;	temp->fitness = score_function(root3,DATA,DATANUM);
			insert_lambda(out_population,temp,out_population->num);
			deepfree(temp->aeg->head);	deepfree(temp->aeg->a_head);	free(temp->aeg->constants);
			out_population->num++;				
			
			free(temp->aeg);	free(temp);
				//printf("\n_______________%d_____________\n",j);
		}
			//检查outpop 
		population_pruning(in_population,out_population);
		champ = in_population->infotree[0];
		printf("Generation %d  ",i);
		printf("Champion\n");
		display_formula(champ->aeg->head);
		printf("\nfitenss: %.2f\tisland: %d\n",champ->fitness,champ->island);	
		printf("\n");
		
		if(champ->fitness<=LIMIT){
			printf("END\n");	break;
		}	
	}
	//display champion
		
}


/*
void test_flist_higher(float* inputs){
    Node* tree = fix_generation(2,1,&expw,"grow",-1,-1); 	//random_generation(2,1,"grow",-1,-1);  //
    printf("Answer: %f\n",evaluate_node(tree,inputs));
    display_formula(tree);  printf("\n");
    display(tree,0);	deepfree(tree);printf("\n");
      
}
*/
/*
void test_score_function(){
	//生成一个树，测试score；复制一下继续检验
	Node *tree = random_generation(2,3,"grow",-1,-1);
	float score = score_function(tree,data,200); 
	printf("score %f\n",score);
	
	score = score_function(tree,data,200); 
	printf("score %f\n",score);
	
	Node *tree2 = deepcopy(tree);
	score = score_function(tree2,data,200); 
	printf("score %f\n",score);	
}
*/
void test_new_operator(){
	int i;
	Node *dad = random_generation(2,"full",-1,-1);
	display_formula(dad);printf("\n");
	Node* mom = random_generation(2,"full",-1,-1);
	display_formula(mom);printf("\n"); 
	
	for (i=0;i<3;++i){
	Node* dad3 = mutate(dad);
	display_formula(dad3);printf("\ndad3 above\n"); 	
	}
	printf("\nCrossover\n");
	for (i=0;i<6;++i){
	Node* dad3 = crossover(dad,mom);
	display_formula(dad3);printf("\ndad3 above\n"); 	
	}
	
	/*
	Node* dad3 = deepcopy(dad);
	dad3 = mutate_subtree(dad3,4,0);
	display_formula(dad3);printf("\n");
	*/
}
void test_complexity(){
	int i;
	for (i=0;i<3;++i){
	
	Node* root = random_generation(i,"full",-1,-1);
	printf("Complexity:%d\n",weighted_complexity(root,0));
	display(root,0);
	
	}
}

Node* random_generation2(int maxdepth,char* method,float fpr,float ppr){
	//default value
	if (maxdepth<0) maxdepth=5;
	if (method[0]!='f')	method="grow";
	if (fpr<0)	fpr=0.5;
	if (ppr<0)	ppr=0.4;
	
    if ( maxdepth==0 || ((method[0]=='g') && (random()>fpr)) ){ // generate terminal
        if (random()<ppr){  //param node, type=1
            Node *param = new_Node();
            init_node_type(param,1); init_node_terminal(param,randint(PC),-1);
            	printf("param node %d\n",param->index);
            return param;
        }else{              //const node, type=2, random const (0,10)
            Node *const1 = new_Node();
            init_node_type(const1,2); init_node_terminal(const1,-1,randint(10)+random());
            	printf("const node %.2f\n",const1->value);
            return const1;
        }
    }
    else{       //generate function node
        int i;
        //choose a function randomly 
        //int temp = randint(flist_use_count);
        Fwrapper* fw = &squarew;//&flist_use[temp];		//此处出过大错，未使用*fw而是fw 
        Node** children = new_children(fw->childcount);
			printf("node name %s count %d\n",fw->name,fw->childcount);
		for (i=0;i<fw->childcount;++i){       
        children[i] = random_generation(maxdepth-1,method,fpr,ppr);   
    	}
        
        Node *head = new_Node();
        init_node_type(head,0);
        init_node_function(head,fw,children);
        return head;
    }
}
void test_module(){
	Node* tree = random_generation(2,"full",-1,-1);
	display(tree,2);
	display_formula(tree);
	/*
	InfoTree* it = (InfoTree*)malloc(sizeof(InfoTree));
	it->aeg =  (AEG*)malloc(sizeof(AEG));
	it->aeg->head = random_generation(3,"full",-1,-1);
	it->aeg->a_head=NULL; it->aeg->constants=NULL; 
	it->aeg->pool = (Pool*)malloc(sizeof(Pool));
	it->aeg->pool->particles = (Particle**)malloc(sizeof(Particle*)*MAX_POOLSIZE);
	int i;for (i=0;i<MAX_POOLSIZE;++i){
		it->aeg->pool->particles[i] = (Particle*)malloc(sizeof(Particle));
	}
	it->aeg->pool->pool_count=0;
	
	it->aeg = convert_to_AEG2(it->aeg);
	it->fitness = score_function(it->aeg->head,DATA,DATANUM);
	printf("Origin\n");
	display_formula(it->aeg->head);printf("\n");
	display_formula(it->aeg->a_head);printf("\n");
	printf("Fitness: %.2f\n",it->fitness);
	it = optimize_constant(it);
	printf("Fitness: %.2f\n",it->fitness);
	display_formula(it->aeg->head); */ 
	/*
	AEG* aeg = (AEG*)malloc(sizeof(AEG));
	aeg->a_head=aeg->head=NULL;	aeg->constants=NULL;
	aeg->head = random_generation(2,"grow",-1,-1);
	//aeg->a_head=deepcopy(aeg->head);
	aeg = convert_to_AEG2(aeg);
	printf("Head\t");	display_formula(aeg->head);
	printf("\na_Head\t");	display_formula(aeg->a_head);
	printf("\nConstants\t");
	int i;for (i=0;i<aeg->constant_count;++i)
		printf("%.2f\t",aeg->constants[i]);
	printf("\n");
	*/
	 
}

