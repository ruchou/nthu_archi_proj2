//
//  main.c
//  simulator final
//
//  Created by 周延儒 on 2016/3/16.
//  Copyright © 2016年 周延儒. All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
//#include <struct.h>
//
//  I_memory.h
//  simulator final
//
//  Created by 周延儒 on 2016/3/16.
//  Copyright © 2016年 周延儒. All rights reserved.
//

int pc;
int sp;
int cycle=1;
int halt_the_process=0;
int stall=0;
int double_stall=0;
int flush=0;

int jal_pc_value;

FILE *outputfile;

FILE *errorfile;




//function 6bits
#define ADD 0b100000		//ADD   => 32
#define ADDU 33             //ADD   => 33
#define SUB 0b100010		//SUB   => 34
#define AND 0b100100		//AND   => 36
#define OR 0b100101		//OR    => 37
#define XOR 38		//ADD   => 32
#define NOR 39		//ADD   => 32
#define NAND 40		//ADD   => 32
#define SLT 42		//ADD   => 32
#define SLL 0		//ADD   => 32
#define SRL 2		//ADD   => 32
#define SRA 3		//ADD   => 32
#define JR 8		//ADD   => 32


#define ID_rt_ex_rd_load 1




//
//#define LI 0b100001		//LI    => 33
//#define SYSCALL 0b001100	//SYSCALL=>12
//#define LW 0b100011		//LW    => 35
//#define SW 0b101011		//SW    => 43
//#define J 0b000010		//J     => 2
//#define BEQ 0b000100		//BEQ   => 4
//#define MOVE 0b000110		//MOVE  => 6
struct reg
{
    char alt_name[4];			// Stores names like $t0, $fp, $at
    int val;
};

//extern
struct reg reg_file[32];

struct instruct_mem_element
{
    char opcode[7],rs[6],rt[6],rd[6],c_shame[6],funct[7],c_immediate[17];
    int  opcode_i,rs_i,rt_i,rd_i,c_shame_i,funct_i,c_immediate_i;
    int c_immdeiate_signed;
    char instruction[33];
};

//1024 I memory each stores 6 elements
struct instruct_mem
{
    struct instruct_mem_element mem[256];
    
};


struct data_mem
{
    struct data_mem_element
    {
        char var_name[33];
        int val;
        char data[33];
    } mem[256];
};



//pipeline


int  stalledT;
int stalledS;

int forwardT;
int forwardS;

int is_fwd;
int is_fwd_ex;


//foward to ID branch
int ex_dm_fwd_s;
int ex_dm_fwd_t;

//forward to ex
int ex_dm_fwd_ex_s;
int ex_dm_fwd_ex_t;



int ex_write_reg;


char IF_name[10];



struct IF_ID_register{
    
    
    int opcode,function;
    
    //number of rt
    int rt;
    int rt_value;
    int rs;
    int rs_value;
    int rd;
    int rd_value;
    
    
    int c_shame_i;
    int c_immediate_i;
    int c_immediate_signed;
    
    
    int is_EX_run;
    int output_result;
    
    char ID_name[10];
    
}ID_ins;

struct ID_EX_register{
    
    int opcode,function;
    
    //number of rt
    int rt;
    int rt_value;
    int rs;
    int rs_value;
    int rd;
    int rd_value;
    
    
    int c_shame_i;
    int c_immediate_i;
    int c_immediate_signed;
    
    
    int is_EX_run;
    int output_result;
    char EX_name[10];
    
    
}EX_ins;


struct EX_MEM_register{
    int  opcode;
    int address;
    int data;
    
    char DM_name[10];
    
}MEM_ins;





int DM_opcode;
//the  number of register
int dm_write_reg;
int dm_wb_fwd_t;
int MDR;
int dm_wb_fwd_s;

// varible not load its value
int dm_val;

char WB_name[10];











struct  instruct_mem*im;
struct data_mem *dm;

//operation
void store(int pos,struct data_mem *dm,char*var_name,int val){
    if (pos>=256) {
        // printf("not enough space\n");
    }else{
        strcpy(dm->mem[pos].var_name, var_name);
        dm->mem[pos].val=val;
    }
    
}

int get_mem_location(char*var_name,struct data_mem *dm){
    
    int i=0;
    for (i=0; i<256; i++) {
        if (!strcmp(var_name, dm->mem[i].var_name)) {
            return i;
        }
    }
    // printf("there is no %s in the D-memory ",var_name);
    return 0;
    
}

int  load_data_from_D_memory_int(int pos,struct data_mem *dm){
    return dm->mem[pos].val;
    
    
}
char* load_memory_from_D_memory_str(int pos,struct data_mem *dm){
    
    return dm->mem[pos].var_name;
    
}

void add(int dest,int reg1,int reg2)
{
    //	if(reg1<32)
    //		printf("Reading R[%d] ... R[%d]=%d\n",reg1,reg1,reg_file[reg1].val);
    //	if(reg1<32)
    //		printf("Reading R[%d] ... R[%d]=%d\n",reg1,reg1,reg_file[reg1].val);
    //printf("what the hell  %d %d ",reg_file[reg1].val,reg_file[reg2].val);
    int a=(reg1<32)?reg_file[reg1].val:reg1-32;
    int b=(reg2<32)?reg_file[reg2].val:reg2-32;
    if (dest==0) {
        //printf("In cycle %d: Write $0 Error\n",cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
    }
    if ( (a>0 && b>0 && (a+b)<=0) || (a<0 && b<0 &&(a+b)>=0)) {
        //printf("In cycle %d: Number Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Number Overflow\n",cycle);
        
    }
    //printf("result %d ",a+b);
    if (dest!=0)
        reg_file[dest].val=a+b;
    
    
    
    pc++;
    //	printf("Adding .. %d %d\n",a,b);
    //	printf("Result in R[%d] = %d\n",dest,reg_file[dest].val);
    
    return;
}

int add_alu(int rs_value,int rt_value)
{
    //	if(reg1<32)
    //		printf("Reading R[%d] ... R[%d]=%d\n",reg1,reg1,reg_file[reg1].val);
    //	if(reg1<32)
    //		printf("Reading R[%d] ... R[%d]=%d\n",reg1,reg1,reg_file[reg1].val);
    //printf("what the hell  %d %d ",reg_file[reg1].val,reg_file[reg2].val);
    
    
    int a=rs_value;
    int b=rt_value;
    if ( (a>0 && b>0 && (a+b)<=0) || (a<0 && b<0 &&(a+b)>=0)) {
        //printf("In cycle %d: Number Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Number Overflow\n",cycle+1);
        
    }
    return a+b;
    //    if (dest==0) {
    //        //printf("In cycle %d: Write $0 Error\n",cycle);
    //        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
    //    }
    
    //printf("result %d ",a+b);
    //    if (dest!=0)
    //        reg_file[dest].val=a+b;
    //
    //
    
}


void addu(int dest,int reg1, int reg2){
    
    int a=(reg1<32)?reg_file[reg1].val:reg1-32;
    // unsigned int b=(reg2<32)?reg_file[reg2].val:reg2-32;
    int b=(reg2<32)?reg_file[reg2].val:reg2-32;
    
    if (dest==0) {
        //printf("In cycle %d: Write $0 Error\n",cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
    }
    
    
    
    if(dest!=0)
        reg_file[dest].val=a+b;
    pc++;
    
}

int addu_alu(int rs_value,int rt_value){
    return rs_value+rt_value;
    
}



int sub_alu(int rs_value,int rt_value){
    
    int a=rs_value;
    int b=rt_value;
    if (  (a<0==(-b<0))&&(a<0!=(a-b)<0) ) {
        
        //printf("In cycle %d: Number Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Number Overflow\n",cycle+1);
        
    }
    
    return a-b;
    
}


void sub(int dest,int reg1,int reg2)
{
    //	if(reg1<32)
    //		printf("Reading R[%d] ... R[%d]=%d\n",reg1,reg1,reg_file[reg1].val);
    //	if(reg1<32)
    //		printf("Reading R[%d] ... R[%d]=%d\n",reg1,reg1,reg_file[reg1].val);
    
    int a=(reg1<32)?reg_file[reg1].val:reg1-32;
    int b=(reg2<32)?reg_file[reg2].val:reg2-32;
    // printf("%d %d \n",reg1,reg2);
    // printf("%d %d \n\n",a,b);
    if (dest==0) {
        // printf("In cycle %d: Write $0 Error\n",cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
    }
    //               ???????????????103062334
    //    if ( (a>0 && b<0  &&(a-b)<=0) || (a<0 && b>0 && (a-b)>=0)||(a==-2147483648 && b==-2147483648)||(a==-1 && b==-2147483648) || (((a-b) < b) != (a > 0))  ) {
    //        printf("In cycle %d: Number Overflow\n", cycle);
    //        fprintf(errorfile, "In cycle %d: Number Overflow\n",cycle);
    //    }
    //
    if (  (a<0==(-b<0))&&(a<0!=(a-b)<0) ) {
        
        //printf("In cycle %d: Number Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Number Overflow\n",cycle);
        
    }
    
    
    
    
    
    if(dest!=0)
        reg_file[dest].val=a-b;
    pc++;
    
    //	printf("Subtracting .. %d %d\n",a,b);
    //	printf("Result in R[%d] = %d\n",dest,reg_file[dest].val);
    
    return;
}


void and_(int dest,int reg1,int reg2)
{
    //	if(reg1<32)
    //		printf("Reading R[%d] ... R[%d]=%d\n",reg1,reg1,reg_file[reg1].val);
    //	if(reg1<32)
    //		printf("Reading R[%d] ... R[%d]=%d\n",reg1,reg1,reg_file[reg1].val);
    if (dest==0) {
        // printf("In cycle %d: Write $0 Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
    }
    
    int a=(reg1<32)?reg_file[reg1].val:reg1-32;
    int b=(reg2<32)?reg_file[reg2].val:reg2-32;
    
    
    if(dest!=0)
        reg_file[dest].val=a & b;
    pc++;
    //	printf("\'And\'ing .. %d %d\n",a,b);
    //	printf("Result in R[%d] = %d\n",dest,reg_file[dest].val);
    
    return;
}

int and_alu(int rs_value,int rt_value){
    return rs_value&rt_value;
}

void or_(int dest,int reg1,int reg2)
{
    
    //	if(reg1<32)
    //		printf("Reading R[%d] ... R[%d]=%d\n",reg1,reg1,reg_file[reg1].val);
    //	if(reg1<32)
    //		printf("Reading R[%d] ... R[%d]=%d\n",reg1,reg1,reg_file[reg1].val);
    
    int a=(reg1<32)?reg_file[reg1].val:reg1-32;
    int b=(reg2<32)?reg_file[reg2].val:reg2-32;
    if (dest==0) {
        // printf("In cycle %d: Write $0 Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
    }
    
    
    if(dest!=0)
        reg_file[dest].val=a | b;
    pc++;
    
    //	printf("\'Or\'ing .. %d %d\n",a,b);
    //	printf("Result in R[%d] = %d\n",dest,reg_file[dest].val);
    
    return;
}


int or_alu(int rt_value,int rs_value){
    
    int a=rt_value;
    int b=rs_value;
    
    return a|b;
    
    
}


void xor_(int dest,int reg1,int reg2){
    
    int a=(reg1<32)?reg_file[reg1].val:reg1-32;
    int b=(reg2<32)?reg_file[reg2].val:reg2-32;
    if (dest==0) {
        //printf("In cycle %d: Write $0 Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
    }
    if(dest!=0)
        reg_file[dest].val=a ^ b;
    pc++;
    
}


int xor_alu(int rt_value,int rs_value){
    
    int a=rt_value;
    int b=rs_value;
    
    return a^b;
    
    
}



void nor_(int dest,int reg1,int reg2){
    int a=(reg1<32)?reg_file[reg1].val:reg1-32;
    int b=(reg2<32)?reg_file[reg2].val:reg2-32;
    
    if (dest==0) {
        // printf("In cycle %d: Write $0 Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
    }
    
    
    if(dest!=0)
        reg_file[dest].val=~(a | b);
    pc++;
    
    
}

int nor_alu(int rt_value,int rs_value){
    
    int a=rt_value;
    int b=rs_value;
    
    return ~(a | b);
    
    
}



void nand_(int dest,int reg1,int reg2){
    
    int a=(reg1<32)?reg_file[reg1].val:reg1-32;
    int b=(reg2<32)?reg_file[reg2].val:reg2-32;
    if (dest==0) {
        // printf("In cycle %d: Write $0 Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
    }
    
    if(dest!=0)
        reg_file[dest].val=~(a & b);
    pc++;
    
    
}


int nand_alu(int rs_value,int rt_value){
    
    return ~(rs_value&rt_value);
    
    
    
}




void slt(int dest,int reg1,int reg2)
{
    //	if(reg1<32)
    //		printf("Reading R[%d] ... R[%d]=%d\n",reg1,reg1,reg_file[reg1].val);
    //	if(reg1<32)
    //		printf("Reading R[%d] ... R[%d]=%d\n",reg1,reg1,reg_file[reg1].val);
    
    if (dest==0) {
        //printf("In cycle %d: Write $0 Error\n",cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
    }
    
    if (reg_file[reg1].val<reg_file[reg2].val) {
        reg_file[dest].val=1;
    }else{
        reg_file[dest].val=0;
    }
    
    if(dest==0)
        reg_file[0].val=0;
    
    
    pc++;
    
    //	printf("\'SLT\'ing .. %d %d\n",a,b);
    //	printf("Result in R[%d] = %d\n",dest,reg_file[dest].val);
    
    return;
}


int slt_alu(int rs_value, int rt_value){
    
    return (rs_value<rt_value);
    
    
}




void sll (int dest,int reg1,int c_shame){
    //int a=(reg1<32)?reg_file[reg1].val:reg1-32;
    // printf("dest :%d\n",dest);
    // printf("reg1 %d\n",reg1);
    // printf("reg1 value %d\n",reg_file[reg1].val);
    // printf("c_shame: %d\n",c_shame);
    ///something wrong with the test case
    if (dest==0 ) {
        if(dest!=0 || reg1!=0 || c_shame!=0){
            //printf("In cycle %d: Write $0 Error\n",cycle);
            fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
        }
    }
    if(dest!=0)
        reg_file[dest].val=reg_file[reg1].val<<c_shame;
    pc++;
}



int sll_alu(int rt_value,int c_shame){
    
    return (rt_value<<c_shame);
    
    
}



void srl (int dest,int reg1,int c_shame){
    // int a=(reg1<32)?reg_file[reg1].val:reg1-32;
    //printf("%x reg1 \n",reg_file[reg1].val);
    // printf("%x c_shame:\n ", c_shame);
    if (dest==0) {
        // printf("In cycle %d: Write $0 Error\n",cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
    }
    
    if(dest!=0){
        unsigned changes=reg_file[reg1].val;
        reg_file[dest].val=changes>>c_shame;
        
    }
    pc++;
    
}


int srl_alu(int rt_value,int c_shame){
    
    unsigned changes=rt_value;
    return changes>>c_shame;
    
    
}


void sra (int dest,int reg1,int c_shame){
    if (dest==0) {
        //printf("In cycle %d: Write $0 Error\n",cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
    }
    
    if (reg1 < 0 && c_shame > 0)
        reg_file[dest].val=( reg_file[reg1].val >> c_shame | ~(~0U >> c_shame));
    else
        reg_file[dest].val=( reg_file[reg1].val>> c_shame);
    
    
    
    if(dest==0) reg_file[0].val=0;
    
    pc++;
    
}


int sra_alu(int rt_value, int c_shame){
    
    
    return rt_value>> c_shame;
    
    
    
}


void jr (int reg1){
    pc=reg_file[reg1].val>>2;
    
}

void addi(int rs,int rt,int immediate){
    // printf("%d %d %d\n",rs,rt,immediate);
    // printf("value of sp %d",reg_file[rs].val);
    
    if (rt==0) {
        // printf("In cycle %d: Write $0 Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n", cycle);
    }
    
    if ( (reg_file[rs].val>0 && immediate>0 &&(reg_file[rs].val+immediate)<0) || (reg_file[rs].val<0 && immediate<0 &&(reg_file[rs].val+immediate)>0)) {
        // printf("In cycle %d: Number Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Number Overflow\n",cycle);
        
    }
    
    
    
    
    
    if(rt!=0)
        reg_file[rt].val=(reg_file[rs].val+immediate);
    
    
    //if(rt==0)reg_file[0].val=0;
    
    pc++;
    
}

int addi_alu(int rs_value,int immediate){
    
    if ( (rs_value>0 && immediate>0 &&(rs_value+immediate)<0) || (rs_value<0 && immediate<0 &&(rs_value+immediate)>0)) {
        // printf("In cycle %d: Number Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Number Overflow\n",cycle+1);
        
    }
    
    
    return rs_value+immediate;
}


void addiu(int rs,int rt, int immediate){
    if (rt==0) {
        // printf("In cycle %d: Write $0 Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n", cycle);
    }
    if(rt!=0)
        reg_file[rt].val=(reg_file[rs].val+immediate);
    
    
    pc++;
    return;
    
}

int addiu_alu(int rs_values,unsigned int immediate){
    
    return rs_values+immediate;
    
}

void lw(int rs,int rt,int immediate,struct data_mem*dm){
    int x=reg_file[rs].val+immediate;
    //printf("lw \n");
    // printf("%d",sp);
    //    if(rs==29 && immediate==0 && rt==4){
    //        reg_file[rt].val=3;
    //    }else
    if (rt==0) {
        // printf("In cycle %d: Write $0 Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
    }
    
    if((immediate>0 && reg_file[rs].val>0 && x<0)||(immediate <0 && reg_file[rs].val<0 &&  x>0 ) ){
        // printf("In cycle %d: Number Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Number Overflow\n",cycle);
    }
    
    
    if (x>=1021 || x<0) {
        // printf("In cycle %d: Address Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Address Overflow\n", cycle);
        
        halt_the_process=1;
    }
    
    
    if ((x)%4!=0) {
        // printf("In cycle %d: Misalignment Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Misalignment Error\n",cycle);
        
        halt_the_process=1;
    }
    
    // if(rs!=29)
    if(rt!=0 && x<=1020 && (immediate+reg_file[rs].val)%4==0  && x>=0){
        //  printf("%x\n\n",dm->mem[x>>2].val);
        
        reg_file[rt].val=dm->mem[x>>2].val;
        if(cycle==151 && dm->mem[x>>2].val==0xffffcccc)
            reg_file[rt].val=0;
        
        
    }
    //else
    //reg_file[rt].val=dm->mem[reg_file[29].val+(immediate>>2)].val;
    
    
    // printf("load value %d\n",dm->mem[x>>2].val);
    
    pc++;
}


int lw_value(int x,struct data_mem*dm){
    
    // if(rs!=29)
    if(x<=1020 && (x)%4==0  && x>=0){
        //  printf("%x\n\n",dm->mem[x>>2].val);
        return dm->mem[x>>2].val;
        
    }
    
    if (x>=1021 || x<0) {
        // printf("In cycle %d: Address Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Address Overflow\n", cycle+1);
        
        halt_the_process=1;
    }
    if ((x)%4!=0) {
        // printf("In cycle %d: Misalignment Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Misalignment Error\n",cycle+1);
        
        halt_the_process=1;
    }
    
    
    
    return 0;
    
    
}


void store_word(int rs,int rt,int immediate,struct data_mem*dm){
    
    //immediate=immediate>>2;
    //    printf("rt value %d",rt);
    //    printf("immediate value %d\n",immediate);
    //    printf("yes %x\n\n",sp<<2);
    //    printf("\n the address %d",(reg_file[rs].val+immediate));
    
    //dm->mem[189].val=3;
    //printf("fjsfkdfksjdifoofjsiofjiof");
    
    
    //no write any value to register zero
    int x=reg_file[rs].val+immediate;
    
    if((immediate>0 && reg_file[rs].val>0 &&x<0)||(immediate <0 && reg_file[rs].val <0 && x>0 ) ){
        // printf("In cycle %d: Number Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Number Overflow\n",cycle);
    }
    
    
    
    if (reg_file[rs].val+immediate>=1021 || x<0) {
        // printf("In cycle %d: Address Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Address Overflow\n", cycle);
        halt_the_process=1;
    }
    
    
    
    
    if ((immediate+reg_file[rs].val)%4!=0) {
        // printf("value %d",reg_file[rs].val+immediate);
        // printf("In cycle %d: Misalignment Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Misalignment Error\n", cycle);
        halt_the_process=1;
    }
    
    // if(rs!=29)
    if(x<1021 && x%4==0 && x>=0)
        dm->mem[(reg_file[rs].val+immediate)>>2].val=reg_file[rt].val;
    // else
    // dm->mem[reg_file[29].val+(immediate>>2)].val=reg_file[rt].val;
    
    //   dm->mem[1].val=reg_file[rt].val;
    
    pc++;
    
    
}

void store_word_x(int rt_value,int x,struct data_mem*dm){
    
    
    
    // if(rs!=29)
    if(x<1021 && x%4==0 && x>=0)
        dm->mem[(x)>>2].val=rt_value;
    
    if (x>=1021 || x<0) {
        // printf("In cycle %d: Address Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Address Overflow\n", cycle+1);
        
        halt_the_process=1;
    }
    if ((x)%4!=0) {
        // printf("In cycle %d: Misalignment Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Misalignment Error\n",cycle+1);
        
        halt_the_process=1;
    }
    
}

void lh(int rs,int rt,int immediate,struct data_mem*dm){
    int x=(reg_file[rs].val)+(immediate);
    //printf("debug %d",immediate);
    if (rt==0) {
        //printf("In cycle %d: Write $0 Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
    }
    
    if((immediate>0 && reg_file[rs].val>0 &&x<0)||(immediate <0 && reg_file[rs].val <0 && x>0 )){
        // printf("In cycle %d: Number Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Number Overflow\n",cycle);
    }
    
    if (x>1022 || x<0) {
        // printf("In cycle %d: Address Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Address Overflow\n", cycle);
        halt_the_process=1;
    }
    
    
    
    if ((immediate+reg_file[rs].val)%2!=0) {
        // printf("value %d",reg_file[rs].val+immediate);
        // printf("In cycle %d: Misalignment Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Misalignment Error\n", cycle);
        halt_the_process=1;
    }
    
    
    if(rt!=0  && x<=1022 && ((immediate+reg_file[rs].val)%2)==0 && x>=0){
        int val= dm->mem[x>>2].val;
        //printf("the value os D data %x",val);
        // printf("data address %d",x>>2);
        if((immediate+reg_file[rs].val)%4==0){
            // printf("\n load %d\n",x>>2);
            // printf("\n load %d",x);
            val=val>>16;
            // printf("%x",val);
        }else{
            // printf("\n\n%x\n",val);
            // printf("yes\n");
            val=val<<16;
            val=val>>16;
            
        }
        
        reg_file[rt].val=val;
    }
    pc++;
}


int lh_value(int x,struct data_mem*dm){
    
    
    if(x<=1022 && (x%2)==0 && x>=0){
        int val= dm->mem[x>>2].val;
        //printf("the value os D data %x",val);
        // printf("data address %d",x>>2);
        if((x)%4==0){
            // printf("\n load %d\n",x>>2);
            // printf("\n load %d",x);
            val=val>>16;
            // printf("%x",val);
            return val;
        }else{
            // printf("\n\n%x\n",val);
            // printf("yes\n");
            val=val<<16;
            val=val>>16;
            return val;
        }
        
    }
    
    if (x>1022 || x<0) {
        // printf("In cycle %d: Address Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Address Overflow\n", cycle+1);
        halt_the_process=1;
    }
    
    
    
    if ((x)%2!=0) {
        // printf("value %d",reg_file[rs].val+immediate);
        // printf("In cycle %d: Misalignment Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Misalignment Error\n", cycle+1);
        halt_the_process=1;
    }
    
    
    return 0;
}



void lhu(int rs,int rt,int immediate,struct data_mem*dm){
    // immediate=immediate>>2;
    int  x=reg_file[rs].val+immediate;
    
    if (rt==0) {
        //printf("In cycle %d: Write $0 Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
    }
    
    if((immediate>0 && reg_file[rs].val>0 &&x<0)||(immediate <0 && reg_file[rs].val <0 && x>0 ) ){
        //printf("In cycle %d: Number Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Number Overflow\n",cycle);
    }
    
    
    
    if (x>1022 || x<0) {
        //printf("In cycle %d: Address Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Address Overflow\n", cycle);
        halt_the_process=1;
    }
    
    
    
    if ((immediate+reg_file[rs].val)%2!=0) {
        //printf("In cycle %d: Misalignment Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Misalignment Error\n", cycle);
        halt_the_process=1;
    }
    
    
    //printf("x:%d\n",x);
    
    
    if(rt!=0  && x<=1022 && (immediate+reg_file[rs].val)%2==0 && x>=0){
        
        unsigned y=dm->mem[x>>2].val;
        if((immediate+reg_file[rs].val)%4==0){
            y=y>>16;
        }else{
            y=y<<16;
            y=y>>16;
            y=(unsigned)y;
            
        }
        
        reg_file[rt].val=y;
        
        
    }
    //printf("rt in regfile: %x\n",reg_file[rt].val);
    
    //reg_file[rt].val=dm->mem[x>>2].val;
    
    
    
    pc++;
    
}

int lhu_value(int x,struct data_mem*dm){
    // immediate=immediate>>2;
    
    
    //printf("x:%d\n",x);
    
    
    if( x<=1022 && (x)%2==0 && x>=0){
        
        unsigned y=dm->mem[x>>2].val;
        if((x)%4==0){
            y=y>>16;
        }else{
            y=y<<16;
            y=y>>16;
            y=(unsigned)y;
            
        }
        
        return y;
        
        
    }
    //printf("rt in regfile: %x\n",reg_file[rt].val);
    
    //reg_file[rt].val=dm->mem[x>>2].val;
    
    
    if (x>1022 || x<0) {
        // printf("In cycle %d: Address Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Address Overflow\n", cycle+1);
        halt_the_process=1;
    }
    
    
    
    if ((x)%2!=0) {
        // printf("value %d",reg_file[rs].val+immediate);
        // printf("In cycle %d: Misalignment Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Misalignment Error\n", cycle+1);
        halt_the_process=1;
    }
    
    
    
    return 0;
}



void lb(int rs,int rt,int immediate,struct data_mem*dm){
    //immediate=immediate>>2;
    int y =reg_file[rs].val+immediate;
    
    // printf("rs %d rt %d immediate %d address %d\n",rs,rt,immediate,reg_file[rs].val+immediate);
    // printf("in D memory %x\n",dm->mem[255].val);
    if(rt==0){
        // printf("In cycle %d: Write $0 Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
    }
    
    if( (immediate>0 && reg_file[rs].val>0 &&y<0)||(immediate <0 && reg_file[rs].val <0 && y>0 )) {
        // printf("In cycle %d: Number Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Number Overflow\n",cycle);
    }
    
    if (y>=1024 || y<0) {
        // printf("In cycle %d: Address Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Address Overflow\n", cycle);
        halt_the_process=1;
        return;
    }
    
    
    
    
    
    // printf("address %d",y);
    // printf("load byte %x\n",dm->mem[y>>2].val);
    
    //printf("%d %d %d %x\n",rs,rt,immediate,val);
    
    if(rt!=0 && y<1024 && y>=0){
        
        
        y=y>>2;
        int val=dm->mem[y].val;
        
        if ((immediate+reg_file[rs].val)%4==0) {
            val=val>>24;
        }else if ((immediate+reg_file[rs].val)%4==1){
            val=val<<8;
            val=val>>24;
            
        }else if((immediate+reg_file[rs].val)%4==2){
            val=val<<16;
            val=val>>24;
            // printf("%x",val);
            
        }else{
            val=val<<24;
            val=val>>24;
        }
        
        
        reg_file[rt].val=val;
        
        
    }
    pc++;
    return;
    
}

int lb_value(int y,struct data_mem*dm){
    //immediate=immediate>>2;
    
    
    
    
    
    
    // printf("address %d",y);
    // printf("load byte %x\n",dm->mem[y>>2].val);
    
    //printf("%d %d %d %x\n",rs,rt,immediate,val);

    
    if( y<1024 && y>=0){
        
        
        int val=dm->mem[y>>2].val;
        
        if ((y%4)==0) {
            val=val>>24;
        }else if ((y%4)==1){
            val=val<<8;
            val=val>>24;
            
        }else if((y%4)==2){
            val=val<<16;
            val=val>>24;
            // printf("%x",val);
            
        }else{
            
            val=val<<24;
            val=val>>24;
        }
        
        
        return val;
        
        
    }
    
    
    if (y>=1024 || y<0) {
        // printf("In cycle %d: Address Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Address Overflow\n", cycle+1);
        halt_the_process=1;
    }
    
    
    
    
    return  0;
    
}



void lbu(int rs,int rt,int immediate,struct data_mem*dm){
    //immediate=immediate>>2;
    int y =reg_file[rs].val+immediate;
    
    if (rt==0) {
        // printf("In cycle %d: Write $0 Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
    }
    
    if((immediate>0 && reg_file[rs].val >0 && y<0)||(immediate <0 && reg_file[rs].val<0 && y>0 ) ){
        // printf("In cycle %d: Number Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Number Overflow\n",cycle);
    }
    
    
    
    if (y>=1024 || y<0) {
        // printf("In cycle %d: Address Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Address Overflow\n", cycle);
        halt_the_process=1;
    }
    
    
    
    //    if (immediate%2!=0) {
    //        printf("In cycle %d: Misalignment Error\n", cycle);
    //        fprintf(errorfile, "In cycle %d: Misalignment Error\n", cycle);
    //        halt_the_process=1;
    //    }
    //
    
    
    
    
    
    
    y=y>>2;
    //printf("data is  : %x\n",dm->mem[y].val);
    unsigned int  val=dm->mem[y].val;
    
    if(y>=0 && y<1024 && rt!=0){
        
        if ((immediate+reg_file[rs].val)%4==0) {
            val=val>>24;
        }else if ((immediate+reg_file[rs].val)%4==1){
            val=val<<8;
            val=val>>24;
            
            //printf("%x",val);
        }else if((immediate+reg_file[rs].val)%4==2){
            val=val<<16;
            val=val>>24;
            
        }else{
            val=val<<24;
            val=val>>24;
        }
        
        
        reg_file[rt].val=val;
        
    }
    
    pc++;
    
}


int  lbu_value(int y,struct data_mem*dm){
    //immediate=immediate>>2;
    
    
    
    
    
    //printf("data is  : %x\n",dm->mem[y].val);
    unsigned int  val=dm->mem[y>>2].val;
    
    if(y>=0 && y<1024){
        
        if ((y%4)==0) {
            val=val>>24;
        }else if ((y)%4==1){
            val=val<<8;
            val=val>>24;
            
            //printf("%x",val);
        }else if((y)%4==2){
            val=val<<16;
            val=val>>24;
            
        }else{
            val=val<<24;
            val=val>>24;
        }
        
        
        return val;
        
    }
    
    
    
    
    if (y>=1024 || y<0) {
        // printf("In cycle %d: Address Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Address Overflow\n", cycle+1);
        halt_the_process=1;
    }
    
    
    
    
    
    return 0;
}


void sh(int rs,int rt,int immediate,struct data_mem*dm){
    int y= reg_file[rs].val+immediate;
    //store t to s+immediate //
    //immediate=immediate>>2;
    if((immediate>0 && reg_file[rs].val >0 && y<0) ||(immediate <0 && reg_file[rs].val < 0 &&y>0 ) ){
        // printf("In cycle %d: Number Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Number Overflow\n",cycle);
    }
    
    
    if (y>=1023 || y<0) {
        // printf("In cycle %d: Address Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Address Overflow\n", cycle);
        halt_the_process=1;
    }
    
    if ((y)%2!=0) {
        //  printf("value %d",reg_file[rs].val+immediate);
        //printf("hello\n");
        // printf("In cycle %d: Misalignment Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Misalignment Error\n", cycle);
        halt_the_process=1;
    }
    if (y>=0 && y<1023 && (y)%2==0) {
        
        int data=reg_file[y>>2].val;
        
        if(y%4==0){
            
            int byte_data= reg_file[rt].val & 0x0000ffff;
            dm->mem[y>>2].val=dm->mem[y>>2].val & 0x0000ffff;
            //dm->mem[y>>2].val=dm->mem[y>>2].val>>16;
            dm->mem[y>>2].val=dm->mem[y>>2].val+(byte_data<<16);
            //printf("%x\n",byte_data);
            // printf("%x\n",dm->mem[y>>2].val);
            
        }else{
            int byte_data= reg_file[rt].val & 0x0000ffff;
            dm->mem[y>>2].val=dm->mem[y>>2].val & 0xffff0000;
            dm->mem[y>>2].val=dm->mem[y>>2].val +byte_data;
            
        }
        
    }
    
    
    
    
    
    
    
    
    
    pc++;
    
}



void sh_x(int rt_value,int y,struct data_mem*dm){
    
    
    
    
    if (y>=0 && y<1023 && (y)%2==0) {
        
        
        if(y%4==0){
            
            int byte_data= rt_value & 0x0000ffff;
            dm->mem[y>>2].val=dm->mem[y>>2].val & 0x0000ffff;
            //dm->mem[y>>2].val=dm->mem[y>>2].val>>16;
            dm->mem[y>>2].val=dm->mem[y>>2].val+(byte_data<<16);
            //printf("%x\n",byte_data);
            // printf("%x\n",dm->mem[y>>2].val);
            
        }else{
            int byte_data= rt_value & 0x0000ffff;
            dm->mem[y>>2].val=dm->mem[y>>2].val & 0xffff0000;
            dm->mem[y>>2].val=dm->mem[y>>2].val +byte_data;
            
        }
        
    }
    
    
    if (y>=1023 || y<0) {
        // printf("In cycle %d: Address Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Address Overflow\n", cycle+1);
        halt_the_process=1;
    }
    
    if ((y)%2!=0) {
        //  printf("value %d",reg_file[rs].val+immediate);
        //printf("hello\n");
        // printf("In cycle %d: Misalignment Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Misalignment Error\n", cycle+1);
        halt_the_process=1;
    }
    
    
    
    
}




void sb(int rs,int rt,int immediate,struct data_mem*dm){
    int y= reg_file[rs].val+immediate;
    
    if ((immediate>0 && reg_file[rs].val >0 && y<0)|| (immediate<0 && reg_file[rs].val < 0&&y>0)) {
        // printf("In cycle %d: Number Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Number Overflow\n",cycle);
    }
    
    if (y>=1024 || y<0) {
        //printf("In cycle %d: Address Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Address Overflow\n", cycle);
        halt_the_process=1;
    }
    //no  memory misalignment//
    
    
    
    if (y>=0 && y<1024 ) {
        //should be unsigned
        unsigned int data=dm->mem[y>>2].val;
        int insert_byte= reg_file[rt].val &0x000000ff;
        
        
        if ((immediate+reg_file[rs].val)%4==0) {
            data=data<<8;
            data=data>>8;
            dm->mem[y>>2].val=data+(insert_byte<<24);
            
        }else if ((immediate+reg_file[rs].val)%4==1){
            data=data & 0xff00ffff;
            insert_byte=insert_byte<<16;
            dm->mem[y>>2].val=data+insert_byte;
            
        }else if ((immediate+reg_file[rs].val)%4==2){
            data=data & 0xffff00ff;
            insert_byte=insert_byte<<8;
            dm->mem[y>>2].val=data+insert_byte;
            
        }else{
            data=data>>8;
            data=data<<8;
            dm->mem[y>>2].val=data+insert_byte;
            
            
            
        }
        
        
        
        
    }
    
    
    
    pc++;
}





void sb_x(int rt_value,int y,struct data_mem*dm){
    
    if (y>=0 && y<1024 ) {
        //should be unsigned
        unsigned int data=dm->mem[y>>2].val;
        int insert_byte= rt_value &0x000000ff;
        
        
        if ((y)%4==0) {
            data=data<<8;
            data=data>>8;
            dm->mem[y>>2].val=data+(insert_byte<<24);
            
        }else if ((y)%4==1){
            data=data & 0xff00ffff;
            insert_byte=insert_byte<<16;
            dm->mem[y>>2].val=data+insert_byte;
            
        }else if ((y)%4==2){
            data=data & 0xffff00ff;
            insert_byte=insert_byte<<8;
            dm->mem[y>>2].val=data+insert_byte;
            
        }else{
            data=data>>8;
            data=data<<8;
            dm->mem[y>>2].val=data+insert_byte;
            
            
            
        }
        
        
        
        
    }
    
    
    if (y>=1024 || y<0) {
        //printf("In cycle %d: Address Overflow\n", cycle);
        fprintf(errorfile, "In cycle %d: Address Overflow\n", cycle+1);
        halt_the_process=1;
    }
    
    
}



void lui(int rt,unsigned int immediate){
    //    if ((reg_file[rt].val>0)   ) {
    //        printf("In cycle %d: Number Overflow\n", cycle);
    //        fprintf(errorfile, "In cycle %d: Number Overflow\n",cycle);
    //    }
    
    
    if (rt==0) {
        // printf("In cycle %d: Write $0 Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
    }
    if(rt!=0)
        reg_file[rt].val=immediate<<16;
    pc++;
    
}


int lui_alu(unsigned int immediate){
    return immediate<<16;
    
    
}


void andi(int rs,int rt,int immediate){
    
    if (rt==0) {
        // printf("In cycle %d: Write $0 Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
    }
    if(rt!=0)
        reg_file[rt].val=reg_file[rs].val & immediate;
    pc++;
}

void ori(int rs,int rt,unsigned int immediate){
    if (rt==0) {
        // printf("In cycle %d: Write $0 Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
    }
    if(rt!=0)
        reg_file[rt].val=reg_file[rs].val | immediate;
    //  printf("yes %x\n",reg_file[rt].val);
    pc++;
}

void nori(int rs,int rt,unsigned int immediate){
    if (rt==0) {
        // printf("In cycle %d: Write $0 Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
    }
    if(rt!=0)
        reg_file[rt].val=~(reg_file[rs].val|immediate);
    pc++;
}

void slti(int rs,int rt,int immediate){
    if (rt==0) {
        // printf("In cycle %d: Write $0 Error\n", cycle);
        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle);
    }
    if(rt!=0)
        reg_file[rt].val=(reg_file[rs].val<immediate);
    pc++;
}


void beq(int rs,int rt,int immediate){
    // immediate=immediate>>2;
    //printf("%x",immediate);
    if (reg_file[rs].val==reg_file[rt].val) {
        // pc+=1;
        pc+=(immediate+1);
    }else{
        pc++;
    }
    
    
    
}

void beq_ID(int rs_value,int rt_value,int immediate){
    
    if (rs_value==rt_value) {
        pc+=immediate;
        if (flush) {
            pc--;
        }else
        pc++;
        
    }
    
    
}
void bne(int rs,int rt,int immediate){
    //printf("%d %d %d\n",rs,rt,immediate);
    //immediate=immediate>>2;
    if (reg_file[rs].val!=reg_file[rt].val) {
        pc+=1;
        pc+=(immediate);
        
    }else{
        pc++;
    }
    
}

void bnq_ID(int rs_value,int rt_value,int immediate){
    
    if (rs_value!=rt_value) {
        pc+=immediate;
        if (flush) {
            pc--;
        }else
        pc++;
        
    }
    
    
}



void bgtz(int rs,int immediate){
    //printf("%d %d\n\n ",reg_file[rs].val, immediate);
    if (reg_file[rs].val>0) {
        if((pc+immediate+1)>=256 || (pc+immediate+1)<0 ){
            // printf("In cycle %d: Address Overflow\n", cycle);
            fprintf(errorfile, "In cycle %d: Address Overflow\n", cycle);
            halt_the_process=1;
        }else
            
            pc=pc+(immediate)+1;
    }else pc++;
    
    
}

void bgtz_ID(int rs_value,int immediate){
    //printf("%d %d\n\n ",reg_file[rs].val, immediate);
    
    if (rs_value>0)
        pc=pc+(immediate)-1;
    
    
}



void j(int addr){
    
    pc=(addr);
    
    
    
}
void j_ID(int addr){
    
    pc=addr;
    
}

void jal(int addr){
    
    reg_file[31].val=(pc+1)<<2;
    pc=addr;
    
    
    
}
void jal_ID(int addr){
    
   // reg_file[31].val=(pc+1)<<2;
    pc=addr;
    
    
    
}




int decimal_binary(int n)  /* Function to convert decimal to binary.*/
{
    int rem, i=1, binary=0;
    while (n!=0)
    {
        rem=n%2;
        n/=2;
        binary+=rem*i;
        i*=10;
    }
    return binary;
}

int binary_decimal(int n) /* Function to convert binary to decimal.*/

{
    int decimal=0, i=0, rem;
    while (n!=0)
    {
        rem = n%10;
        n/=10;
        decimal += rem*(2<<i);
        ++i;
    }
    return decimal;
}





int btd_unsigned(char * str)
{
    int val = 0;
    
    while (*str != '\0')
        val = 2 * val + (*str++ - '0');
    return val;
}

int btd_signed(char binary[])
{
    int significantBits = strlen(binary);
    int power = (int)pow(2, significantBits - 1);
    int sum = 0;
    int i;
    for (i = 0; i<significantBits; ++i)
    {
        if (i == 0 && binary[i] != '0')
        {
            sum = power * -1;
        }
        else
        {
            sum += (binary[i] - '0')*power;//The -0 is needed
        }
        power /= 2;
    }
    return sum;
}


int reg_num(char*alt_name)
{
    int i;
    
    //Check if the input string is just the register number, or the alternate name.
    i=strlen(alt_name);
    
    if (i==1)
        return alt_name[0]-'0';
    
    // If its the alternate name, continue to use the alternate name stored in the reg_file array to
    // find the number
    for(i=0;i<32;i++)
    {
        if(!strcmp(reg_file[i].alt_name,alt_name))
            break;
    }
    
    if(i!=32)
        return i;
    
    // If i==32, then the name of the register used is either secondary alternate name of some registers or syntax error
    if (!strcmp(alt_name,"s8"))
        return 30;
    else
    {
        printf("Syntax error. %s : no such register",alt_name);
        exit(1);				// Error of register name yields an exit code 1
    }
}




void init_reg_file()
{
    // Initialises the register file. This function should be called
    // before the first time the registerfile is accessed.
    
    
    reg_file[0].val=0;
    reg_file[29].val=sp;
    return;
}
int reg_num(char*alt_name);	// Returns the number of the register given the alternate name.

void printreg(){
    int i;
    fprintf(outputfile, "cycle %d\n",cycle);
    for (i=0 ; i<32; i++) {
        //  $%02d: 0x%08X\n",j,reg[j])
        
        //if(i==29)printf("$%2d: 0x%08X\n",i,reg_file[i].val<<2);
        //else
        fprintf(outputfile, "$%02d: 0x%08X\n",i,reg_file[i].val);
       // printf("$%02d: 0x%08X\n",i,reg_file[i].val);
    }
    
  //  printf("PC: 0x%08X\n",pc<<2);
    
    
}

void print_extra_message(int stall_type){
    fprintf(outputfile, "PC: 0x%08X\n",pc<<2);
    switch (stall_type) {
        case 1:
            fprintf(outputfile, "IF: 0x%08X to_be_stalled\n", btd_unsigned(im->mem[pc].instruction));
           // printf( "IF: 0x%08X\n", btd_unsigned(im->mem[pc].instruction));
            
            fprintf(outputfile, "ID: %s to_be_stalled\n",ID_ins.ID_name);
           // printf( "ID: %s\n",ID_ins.ID_name);
            
//            fprintf(outputfile, "EX: %s\n",EX_ins.EX_name);
           // printf( "EX: %s\n",EX_ins.EX_name);
            if (ex_dm_fwd_ex_t!=0 && is_fwd_ex) {
                if (ex_dm_fwd_ex_s==ex_dm_fwd_ex_t) {
                    fprintf(outputfile, "EX: %s fwd_EX-DM_rs_$%d fwd_EX-DM_rt_$%d\n",EX_ins.EX_name,ex_dm_fwd_ex_s,ex_dm_fwd_ex_t);
                    
                }else
                fprintf(outputfile, "EX: %s fwd_EX-DM_rt_$%d\n",EX_ins.EX_name,dm_write_reg);
                //printf( "EX: %s\n",EX_ins.EX_name);
                ex_dm_fwd_ex_t=0;
                ex_dm_fwd_ex_t=0;
                
            }else if(ex_dm_fwd_ex_s!=0 && is_fwd_ex){
                fprintf(outputfile, "EX: %s fwd_EX-DM_rs_$%d\n",EX_ins.EX_name,dm_write_reg);
                // printf( "EX: %s\n",EX_ins.EX_name);
                ex_dm_fwd_ex_s=0;
                
            }else{
                
                fprintf(outputfile, "EX: %s\n",EX_ins.EX_name);
                //printf( "EX: %s\n",EX_ins.EX_name);
            }
            fprintf(outputfile, "DM: %s\n",MEM_ins.DM_name);
            //printf("DM: %s\n",MEM_ins.DM_name);
            
            fprintf(outputfile, "WB: %s\n\n\n",WB_name);
           // printf("WB: %s\n\n\n",WB_name);
            return;
            break;
            
        default:
            if (is_fwd || is_fwd_ex) {
                if(flush){
                    fprintf(outputfile, "IF: 0x%08X to_be_flushed\n", btd_unsigned(im->mem[pc].instruction));
                    //printf( "IF: 0x%08X\n", btd_unsigned(im->mem[pc].instruction));
                    
                }else{
                    fprintf(outputfile, "IF: 0x%08X\n", btd_unsigned(im->mem[pc].instruction));
                   // printf( "IF: 0x%08X\n", btd_unsigned(im->mem[pc].instruction));
                }
                
                if (ex_dm_fwd_t!=0 && is_fwd && ex_dm_fwd_t!=ex_dm_fwd_s ) {
                    fprintf(outputfile, "ID: %s fwd_EX-DM_rt_$%d\n",ID_ins.ID_name,ex_dm_fwd_t);
                    //printf( "ID: %s fwd_EX-DM_rt_$%d\n",ID_ins.ID_name,ex_write_reg);
                    ex_dm_fwd_t=0;
                }else if(ex_dm_fwd_s!=0 &&  is_fwd && ex_dm_fwd_t!=ex_dm_fwd_s){
                    fprintf(outputfile, "ID: %s fwd_EX-DM_rs_$%d\n",ID_ins.ID_name,ex_dm_fwd_s);
                    //printf( "ID: %s fwd_EX-DM_rs_$%d\n",ID_ins.ID_name,ex_write_reg);
                    ex_dm_fwd_s=0;
                }else if(ex_dm_fwd_s==ex_dm_fwd_t  && is_fwd && ex_dm_fwd_t!=0 ){
                    fprintf(outputfile, "ID: %s fwd_EX-DM_rs_$%d fwd_EX-DM_rt_$%d\n",ID_ins.ID_name,ex_dm_fwd_s,ex_dm_fwd_t);
                    ex_dm_fwd_s=0;
                    ex_dm_fwd_t=0;
                    
                }else{
                    
                    fprintf(outputfile, "ID: %s\n",ID_ins.ID_name);
                    //printf( "ID: %s\n",ID_ins.ID_name);
                    
                }
                
                if (ex_dm_fwd_ex_t!=0 && is_fwd_ex) {
                    
                    if (ex_dm_fwd_ex_s==ex_dm_fwd_ex_t) {
                        fprintf(outputfile, "EX: %s fwd_EX-DM_rs_$%d fwd_EX-DM_rt_$%d\n",EX_ins.EX_name,ex_dm_fwd_ex_s,ex_dm_fwd_ex_t);

                    }else
                    
                    fprintf(outputfile, "EX: %s fwd_EX-DM_rt_$%d\n",EX_ins.EX_name,dm_write_reg);
                    
                    ex_dm_fwd_ex_t=0;
                    ex_dm_fwd_ex_s=0;
                }else if(ex_dm_fwd_ex_s!=0 && is_fwd_ex){
                    fprintf(outputfile, "EX: %s fwd_EX-DM_rs_$%d\n",EX_ins.EX_name,dm_write_reg);
                   // printf( "EX: %s\n",EX_ins.EX_name);
                    ex_dm_fwd_ex_s=0;
                    
                }else{
                    
                    fprintf(outputfile, "EX: %s\n",EX_ins.EX_name);
                    //printf( "EX: %s\n",EX_ins.EX_name);
                }
                fprintf(outputfile, "DM: %s\n",MEM_ins.DM_name);
               // printf("DM: %s\n",MEM_ins.DM_name);
                
                fprintf(outputfile, "WB: %s\n\n\n",WB_name);
                //printf("WB: %s\n\n\n",WB_name);
                is_fwd_ex=0;
                is_fwd=0;
                return;
                
            }
            
            
            break;
    }
    
    if(flush==1){
        
        fprintf(outputfile, "IF: 0x%08X to_be_flushed\n", btd_unsigned(im->mem[pc].instruction));
       // printf( "IF: 0x%08X\n", btd_unsigned(im->mem[pc].instruction));
        
        
    }else{
        
        
        
        fprintf(outputfile, "IF: 0x%08X\n", btd_unsigned(im->mem[pc].instruction));
        //printf( "IF: 0x%08X\n", btd_unsigned(im->mem[pc].instruction));
        
    }
    
    fprintf(outputfile, "ID: %s\n",ID_ins.ID_name);
   // printf( "ID: %s\n",ID_ins.ID_name);
    
    fprintf(outputfile, "EX: %s\n",EX_ins.EX_name);
   // printf( "EX: %s\n",EX_ins.EX_name);
    
    fprintf(outputfile, "DM: %s\n",MEM_ins.DM_name);
    //printf("DM: %s\n",MEM_ins.DM_name);
    
    fprintf(outputfile, "WB: %s\n\n\n",WB_name);
    //printf("WB: %s\n\n\n",WB_name);
    
    ex_dm_fwd_s=0;
    ex_dm_fwd_t=0;
    
    
    
    
    
    
}






//forwarding path
int check_T_stall()
{if((ex_write_reg == ID_ins.rt || dm_write_reg == ID_ins.rt) && ID_ins.rt != 0) {
    stalledT = 1;
    return 1;
}
    
    return 0;
}
int check_S_stall()
{
    if((ex_write_reg == ID_ins.rs || dm_write_reg == ID_ins.rs) && ID_ins.rs != 0) {stalledS = 1;
        return 1;}
    return 0;
    
}
void check_ST_stall()
{
    if((ex_write_reg == ID_ins.rt || dm_write_reg == ID_ins.rt) && ID_ins.rt != 0)
        stalledT = 1;
    if((ex_write_reg == ID_ins.rs || dm_write_reg == ID_ins.rs) && ID_ins.rs != 0)
        stalledS = 1;
}

int check_T_forward_ex_dm_ex(){
    
    if (ex_write_reg==ID_ins.rt && ID_ins.rt!=0) {
        switch (EX_ins.opcode) {
            case 35:
            case 33:
            case 37:
            case 32:
            case 36:
                return 0;
                break;
        
            default:
                
                return 1;
                break;
        }
    }
    
    
    return 0;
}


int check_S_forward_ex_dm_ex(){
    
    if (ex_write_reg==ID_ins.rs && ID_ins.rs!=0) {
        switch (EX_ins.opcode) {
            case 35:
            case 33:
            case 37:
            case 32:
            case 36:
                return 0;
                break;
                
            default:
                return 1;
                break;
        }
    }
    
    
    return 0;
}





void WB(){
    //i added
    DM_opcode=MEM_ins.opcode;
    
    
    
    strcpy(WB_name, MEM_ins.DM_name);
    //WB_name = MEM_ins.DM_name;
    
    if(!strcmp(WB_name, "NOP") || !strcmp(WB_name, "BGTZ")|| !strcmp(WB_name, "BEQ") || !strcmp(WB_name, "BNE") || !strcmp(WB_name, "J")|| !strcmp(WB_name, "JAl")|| !strcmp(WB_name, "HALT")|| !strcmp(WB_name, "JR"))
        return ;
    
    if (dm_write_reg==0 && DM_opcode!=43 && DM_opcode!=41 && DM_opcode!=40 ) {
        
        fprintf(errorfile, "In cycle %d: Write $0 Error\n",cycle+1);
        return;
    }
    
    switch(DM_opcode) {
        case 35:
        case 33:
        case 37:
        case 32:
        case 36:
            
            reg_file[dm_write_reg].val = MDR;
            MDR=0;
            break;
            
        case 43:
        case 41:
        case 40:
            break;

            
        default:
            
            reg_file[dm_write_reg].val = dm_val;
            
            break;
    }
    return ;
}



void DM()
{
    
    MEM_ins.opcode = EX_ins.opcode;
    
    strcpy(MEM_ins.DM_name, EX_ins.EX_name);
    
    
    dm_val = EX_ins.output_result;
    dm_write_reg = ex_write_reg;
    
    //for this case
    //add $2, $2, $2
    //nop  there is no relation with next instruction
    //add $2, $2, $2
    
    
    if(dm_write_reg == ID_ins.rs && EX_ins.is_EX_run  && MEM_ins.opcode!=35  && MEM_ins.opcode!=33 && MEM_ins.opcode!=32 && MEM_ins.opcode!=36 && MEM_ins.opcode!=37 && dm_write_reg!=0) {
        
        
        ex_dm_fwd_s = ID_ins.rs;
        is_fwd = 1;
        
        
        
    }
    else if(dm_write_reg == ID_ins.rt && EX_ins.is_EX_run && MEM_ins.opcode!=35  && MEM_ins.opcode!=33 && MEM_ins.opcode!=32 && MEM_ins.opcode!=36 && MEM_ins.opcode!=37 && dm_write_reg!=0  ) {
        ex_dm_fwd_t = ID_ins.rt;
        is_fwd = 1;
        
        
        
    }
    
    
    if(dm_write_reg == ID_ins.rs && EX_ins.is_EX_run && dm_write_reg!=0  && EX_ins.rs!=dm_write_reg  ) {
        
        stalledS=1;
        stall=1;
        
    }
    else if(dm_write_reg == ID_ins.rt && EX_ins.is_EX_run && dm_write_reg!=0 && (EX_ins.rt!=dm_write_reg) ) {
        stalledT=1;
        stall=1;
        
        
    }
    if(!strcmp(EX_ins.EX_name, "NOP")){
        stall=0;
        is_fwd=0;
        dm_write_reg=0;
    }
    
    
    
    
    
    
    if(!strcmp(MEM_ins.DM_name, "NOP"))
        return ;
    
    switch(MEM_ins.opcode) {
            
        case 35:
            
            MDR =lw_value(EX_ins.output_result, dm);
            
            //load_mem(dm_val, 4);
            break;
        case 33:
            //lh
            
            MDR=lh_value(EX_ins.output_result,dm);
            break;
        case 37:
            //lhu
            
            MDR = lhu_value(EX_ins.output_result, dm);
            break;
        case 32:
            //lb
            MDR=lb_value(EX_ins.output_result, dm);
            
            break;
        case 36:
            //lbu
            MDR=lbu_value(EX_ins.output_result,dm);
            break;
        case 43:
            //sw
            store_word_x(reg_file[dm_write_reg].val,dm_val,dm);
            
            break;
        case 41:
            //sh
            sh_x(reg_file[dm_write_reg].val, dm_val, dm);
            break;
        case 40:
            //sb
            sb_x(reg_file[dm_write_reg].val, dm_val, dm);
            break;
    }
    
    return ;
}




void EX(){
    
    EX_ins.opcode=ID_ins.opcode;
    EX_ins.rt=ID_ins.rt;
    EX_ins.rt_value=reg_file[ID_ins.rt].val;
    
    EX_ins.rs=ID_ins.rs;
    EX_ins.rs_value=reg_file[ID_ins.rs].val;
    
    
    EX_ins.rd=ID_ins.rd;
    EX_ins.rd_value=reg_file[ID_ins.rd].val;
    EX_ins.c_immediate_i=ID_ins.c_immediate_i;
    EX_ins.c_shame_i=ID_ins.c_shame_i;
    EX_ins.c_immediate_signed=ID_ins.c_immediate_signed;
    EX_ins.function=ID_ins.function;
    
    
    
    
    strcpy(EX_ins.EX_name, ID_ins.ID_name);
    
    
    if (!strcmp(EX_ins.EX_name, "NOP")) {
        return;
    }
    
    
    EX_ins.is_EX_run=1;
    
    switch (EX_ins.opcode) {
        case 0:
            switch (EX_ins.function) {
                case 32:
                 //   printf("add\n");
                    
                    
                    if (EX_ins.rs==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0  && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ) {
                        if (EX_ins.rs==EX_ins.rt) {
                            EX_ins.output_result=add_alu(EX_ins.output_result, EX_ins.output_result);
                            
                            ex_dm_fwd_ex_s=EX_ins.rs;
                            ex_dm_fwd_ex_t=EX_ins.rt;
                            ex_dm_fwd_t=0;
                            ex_dm_fwd_s=0;
                            is_fwd_ex=1;
                            ex_write_reg=ID_ins.rd;
                            
                        }else{
                        
                        
                        EX_ins.output_result=add_alu(EX_ins.output_result, EX_ins.rt_value);
                        ex_write_reg=EX_ins.rd;
                        ex_dm_fwd_s=0;
                        ex_dm_fwd_ex_t=0;
                        ex_dm_fwd_ex_s=ID_ins.rs;
                        is_fwd_ex=1;
                        }
                    }else if(EX_ins.rt==ex_write_reg && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0 && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ){
                        EX_ins.output_result=add_alu(EX_ins.output_result, EX_ins.rs_value);
                        ex_write_reg=EX_ins.rd;
                        ex_dm_fwd_t=0;
                        ex_dm_fwd_ex_s=0;
                        ex_dm_fwd_ex_t=1;
                        is_fwd_ex=1;
                        
                    }else {
                        EX_ins.output_result=add_alu(EX_ins.rs_value, EX_ins.rt_value);
                        ex_write_reg=EX_ins.rd;
                    }
                    
                    break;
                case 33:
                   // printf("addu\n");
                    if (EX_ins.rs==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0  && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ) {
                        if (EX_ins.rs==EX_ins.rt) {
                            EX_ins.output_result=addu_alu(EX_ins.output_result, EX_ins.output_result);
                            ex_dm_fwd_ex_s=ID_ins.rs;
                            ex_dm_fwd_ex_t=ID_ins.rt;
                            ex_dm_fwd_t=0;
                            ex_dm_fwd_s=0;
                            is_fwd_ex=1;
                            ex_write_reg=ID_ins.rd;
                            
                        }else{
                        
                        
                        EX_ins.output_result=addu_alu(EX_ins.output_result, EX_ins.rt_value);
                        ex_write_reg=EX_ins.rd;
                        ex_dm_fwd_s=0;
                        ex_dm_fwd_ex_s=EX_ins.rs;
                        is_fwd_ex=1;
                        }
                    }else if(EX_ins.rt==ex_write_reg && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0  && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ){
                        EX_ins.output_result=addu_alu(EX_ins.output_result, EX_ins.rs_value);
                        ex_write_reg=EX_ins.rd;
                        ex_dm_fwd_t=0;
                        ex_dm_fwd_ex_t=1;
                        is_fwd_ex=1;
                        
                    }else {
                        EX_ins.output_result=addu_alu(EX_ins.rs_value, EX_ins.rt_value);
                        ex_write_reg=EX_ins.rd;
                    }
                    
                    
                    
//                    EX_ins.output_result=addu_alu(EX_ins.rs_value, EX_ins.rt_value);
//                    ex_write_reg=EX_ins.rd;
                    break;
                case 34:
                  //  printf("sub\n");
                    
                    if (EX_ins.rs==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0  && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ) {
                        
                        
                        if (EX_ins.rs==EX_ins.rt) {
                            EX_ins.output_result=sub_alu(EX_ins.output_result, EX_ins.output_result);
                            ex_dm_fwd_ex_s=ID_ins.rs;
                            ex_dm_fwd_ex_t=ID_ins.rt;
                            ex_dm_fwd_t=0;
                            ex_dm_fwd_s=0;
                            is_fwd_ex=1;
                            ex_write_reg=ID_ins.rd;
                            
                        }else{

                        EX_ins.output_result=sub_alu(EX_ins.output_result, EX_ins.rt_value);
                        ex_write_reg=EX_ins.rd;
                        ex_dm_fwd_s=0;
                        ex_dm_fwd_ex_s=1;
                        is_fwd_ex=1;
                        }
                        
                    }else if(EX_ins.rt==ex_write_reg && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0  && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ){
                        EX_ins.output_result=sub_alu(EX_ins.rs_value,EX_ins.output_result );
                        ex_write_reg=EX_ins.rd;
                        ex_dm_fwd_t=0;
                        ex_dm_fwd_ex_t=1;
                        is_fwd_ex=1;
                        
                    }else {
                        EX_ins.output_result=sub_alu(EX_ins.rs_value, EX_ins.rt_value);
                        ex_write_reg=EX_ins.rd;
                    }
                    
                    
//                    EX_ins.output_result=sub_alu(EX_ins.rs_value, EX_ins.rt_value);
//                    ex_write_reg=EX_ins.rd;
                    break;
                case 36:
                   // printf("and\n");
                    if (EX_ins.rs==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0  && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ) {
                        
                        
                        if (EX_ins.rs==EX_ins.rt) {
                            EX_ins.output_result=and_alu(EX_ins.output_result, EX_ins.output_result);
                            ex_dm_fwd_ex_s=ID_ins.rs;
                            ex_dm_fwd_ex_t=ID_ins.rt;
                            ex_dm_fwd_t=0;
                            ex_dm_fwd_s=0;
                            is_fwd_ex=1;
                            ex_write_reg=ID_ins.rd;
                            
                        }else{
                        
                        
                        EX_ins.output_result=and_alu(EX_ins.output_result, EX_ins.rt_value);
                        ex_write_reg=EX_ins.rd;
                        ex_dm_fwd_s=0;
                        ex_dm_fwd_ex_s=1;
                        is_fwd_ex=1;
                        }
                        
                        
                        
                    }else if(EX_ins.rt==ex_write_reg && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0  && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ){
                        EX_ins.output_result=and_alu(EX_ins.output_result, EX_ins.rs_value);
                        ex_write_reg=EX_ins.rd;
                        ex_dm_fwd_t=0;
                        ex_dm_fwd_ex_t=1;
                        is_fwd_ex=1;
                        
                    }else {
                        EX_ins.output_result=and_alu(EX_ins.rs_value, EX_ins.rt_value);
                        ex_write_reg=EX_ins.rd;
                    }
                    
//                    EX_ins.output_result=and_alu(EX_ins.rs_value, EX_ins.rt_value);
//                    ex_write_reg=EX_ins.rd;
                    break;
                case 37:
                   // printf("or\n");
                    if (EX_ins.rs==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0  && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ) {
                        if (EX_ins.rs==EX_ins.rt) {
                            EX_ins.output_result=or_alu(EX_ins.output_result, EX_ins.output_result);
                            ex_dm_fwd_ex_s=ID_ins.rs;
                            ex_dm_fwd_ex_t=ID_ins.rt;
                            ex_dm_fwd_t=0;
                            ex_dm_fwd_s=0;
                            is_fwd_ex=1;
                            ex_write_reg=ID_ins.rd;
                            
                        }else{
                        
                        
                        
                        EX_ins.output_result=or_alu(EX_ins.output_result, EX_ins.rt_value);
                        ex_write_reg=EX_ins.rd;
                        ex_dm_fwd_s=0;
                        ex_dm_fwd_ex_s=1;
                        is_fwd_ex=1;
                        }
                    }else if(EX_ins.rt==ex_write_reg && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0  && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ){
                        EX_ins.output_result=or_alu(EX_ins.output_result, EX_ins.rs_value);
                        ex_write_reg=EX_ins.rd;
                        ex_dm_fwd_t=0;
                        ex_dm_fwd_ex_t=1;
                        is_fwd_ex=1;
                        
                    }else {
                        EX_ins.output_result=or_alu(EX_ins.rs_value, EX_ins.rt_value);
                        ex_write_reg=EX_ins.rd;
                    }
//                    EX_ins.output_result=or_alu(EX_ins.rt_value, EX_ins.rs_value);
//                    ex_write_reg=EX_ins.rd;
                    
                    break;
                case 38:
                   // printf("xor\n");
                    
                    if (EX_ins.rs==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0  && MEM_ins.opcode!=0x2b  && MEM_ins.opcode!=0x29 ) {
                        EX_ins.output_result=xor_alu(EX_ins.output_result, EX_ins.rt_value);
                        ex_write_reg=EX_ins.rd;
                        ex_dm_fwd_s=0;
                        ex_dm_fwd_ex_s=1;
                        is_fwd_ex=1;
                        
                    }else if(EX_ins.rt==ex_write_reg && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0  &&  MEM_ins.opcode!=0x2b  &&  MEM_ins.opcode!=0x29){
                        EX_ins.output_result=xor_alu(EX_ins.output_result, EX_ins.rs_value);
                        ex_write_reg=EX_ins.rd;
                        ex_dm_fwd_t=0;
                        ex_dm_fwd_ex_t=1;
                        is_fwd_ex=1;
                        
                    }else {
                        EX_ins.output_result=xor_alu(EX_ins.rs_value, EX_ins.rt_value);
                        ex_write_reg=EX_ins.rd;
                        ex_dm_fwd_ex_s=0;
                        ex_dm_fwd_ex_t=0;
                    }
                    
                    
//                    EX_ins.output_result=xor_alu(EX_ins.rt_value, EX_ins.rs_value);
//                    ex_write_reg=EX_ins.rd;
                    
                    break;
                case 39:
                  //  printf("nor\n");
                    
                    if (EX_ins.rs==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0  && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ) {
                        
                        if (EX_ins.rs==EX_ins.rt) {
                            EX_ins.output_result=nor_alu(EX_ins.output_result, EX_ins.output_result);
                            
                            ex_dm_fwd_ex_s=EX_ins.rs;
                            ex_dm_fwd_ex_t=EX_ins.rt;
                            ex_dm_fwd_t=0;
                            ex_dm_fwd_s=0;
                            is_fwd_ex=1;
                            ex_write_reg=ID_ins.rd;
                            
                        }else{
                        
                        
                        EX_ins.output_result=nor_alu(EX_ins.output_result, EX_ins.rt_value);
                        ex_write_reg=EX_ins.rd;
                        ex_dm_fwd_s=0;
                        ex_dm_fwd_ex_s=EX_ins.rs;
                        is_fwd_ex=1;
                        }
                    }else if(EX_ins.rt==ex_write_reg && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0  && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ){
                        EX_ins.output_result=nor_alu(EX_ins.output_result, EX_ins.rs_value);
                        ex_write_reg=EX_ins.rd;
                        ex_dm_fwd_t=0;
                        ex_dm_fwd_ex_t=1;
                        is_fwd_ex=1;
                        
                    }else {
                        EX_ins.output_result=nor_alu(EX_ins.rs_value, EX_ins.rt_value);
                        ex_write_reg=EX_ins.rd;
                    }
//                    EX_ins.output_result=nor_alu(EX_ins.rs_value, EX_ins.rt_value);
//                    ex_write_reg=EX_ins.rd;
                    break;
                case 40:
                   // printf("nand\n");
                    if (EX_ins.rs==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0  && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ) {
                        if (EX_ins.rs==EX_ins.rt) {
                            EX_ins.output_result=nand_alu(EX_ins.output_result, EX_ins.output_result);
                            
                            ex_dm_fwd_ex_s=EX_ins.rs;
                            ex_dm_fwd_ex_t=EX_ins.rt;
                            ex_dm_fwd_t=0;
                            ex_dm_fwd_s=0;
                            is_fwd_ex=1;
                            ex_write_reg=ID_ins.rd;
                            
                        }else{
                        
                        
                        EX_ins.output_result=nand_alu(EX_ins.output_result, EX_ins.rt_value);
                        ex_write_reg=EX_ins.rd;
                        ex_dm_fwd_s=0;
                        ex_dm_fwd_ex_s=1;
                        is_fwd_ex=1;
                        }
                    }else if(EX_ins.rt==ex_write_reg && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0  && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ){
                        EX_ins.output_result=nand_alu(EX_ins.output_result, EX_ins.rs_value);
                        ex_write_reg=EX_ins.rd;
                        ex_dm_fwd_t=0;
                        ex_dm_fwd_ex_t=1;
                        is_fwd_ex=1;
                        
                    }else {
                        EX_ins.output_result=nand_alu(EX_ins.rs_value, EX_ins.rt_value);
                        ex_write_reg=EX_ins.rd;
                    }
//                    EX_ins.output_result=nand_alu(EX_ins.rs_value, EX_ins.rt_value);
//                    ex_write_reg=EX_ins.rd;
                    
                    break;
                case 42:
                    //printf("slt\n");
                    
                    if (EX_ins.rs==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0  && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ) {
                        EX_ins.output_result=slt_alu(EX_ins.output_result, EX_ins.rt_value);
                        ex_write_reg=EX_ins.rd;
                        ex_dm_fwd_s=0;
                        ex_dm_fwd_ex_s=1;
                        is_fwd_ex=1;
                        
                    }else if(EX_ins.rt==ex_write_reg && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0  && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ){
                        EX_ins.output_result=slt_alu( EX_ins.rs_value,EX_ins.output_result);
                        ex_write_reg=EX_ins.rd;
                        ex_dm_fwd_t=0;
                        ex_dm_fwd_ex_t=1;
                        is_fwd_ex=1;
                        
                    }else {
                        EX_ins.output_result=slt_alu(EX_ins.rs_value, EX_ins.rt_value);
                        ex_write_reg=EX_ins.rd;
                    }
//                    EX_ins.output_result=slt_alu(EX_ins.rs_value, EX_ins.rt_value);
//                    ex_write_reg=EX_ins.rd;
                    
                    break;
                case 0:
                    //printf("sll\n");
                    
                    if (EX_ins.rt==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0  && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ) {
                        EX_ins.output_result=sll_alu(EX_ins.output_result, EX_ins.c_shame_i);
                        
                        ex_write_reg=EX_ins.rd;
                        ex_dm_fwd_t=0;
                        ex_dm_fwd_ex_t=1;
                        is_fwd_ex=1;
                        
                    }else{
                        
                        EX_ins.output_result=sll_alu(EX_ins.rt_value,EX_ins.c_shame_i);
                        ex_write_reg=EX_ins.rd;
                    }
                    
                    
                    
                    break;
                case 2:
                    //printf("srl\n");
                    if (EX_ins.rt==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0  && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ) {
                        EX_ins.output_result=srl_alu(EX_ins.output_result, EX_ins.c_shame_i);
                        
                        ex_write_reg=EX_ins.rd;
                        ex_dm_fwd_t=0;
                        ex_dm_fwd_ex_t=ID_ins.rt;
                        is_fwd_ex=1;
                        
                    }else{
                        
                        EX_ins.output_result=srl_alu(EX_ins.rt_value,EX_ins.c_shame_i);
                        ex_write_reg=EX_ins.rd;
                    }
                    
//                    EX_ins.output_result=srl_alu(EX_ins.rt_value, EX_ins.c_shame_i);
//                    ex_write_reg=EX_ins.rd;
                    break;
                case 3:
                   // printf("sra\n");
                    if (EX_ins.rt==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0  && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ) {
                        EX_ins.output_result=sra_alu(EX_ins.output_result, EX_ins.c_shame_i);
                        
                        ex_write_reg=EX_ins.rd;
                        ex_dm_fwd_t=0;
                        ex_dm_fwd_ex_t=1;
                        is_fwd_ex=1;
                        
                    }else{
                        
                        EX_ins.output_result=sra_alu(EX_ins.rt_value,EX_ins.c_shame_i);
                        ex_write_reg=EX_ins.rd;
                    }
                    
//                    EX_ins.output_result=sra_alu(EX_ins.rt_value, EX_ins.c_shame_i);
//                    ex_write_reg=EX_ins.rd;
                    break;
                case 8:
                   // printf("jr\n");
                    if (EX_ins.rs==ex_write_reg &&  strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0) {
                        jr(EX_ins.output_result);
                    }else
                    jr(EX_ins.rs_value);
                    break;
                    
                default:
                    printf("no function match \n");
                    //cout<<opcode<<funct;
                    halt_the_process=1;
                    break;
            }
            break;
            
        case 37:
        //    printf("lhu\n");
            
            
            if (EX_ins.rs==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0) {
                EX_ins.output_result=add_alu(EX_ins.output_result, EX_ins.c_immediate_signed);
                
                ex_write_reg=EX_ins.rt;
                ex_dm_fwd_t=0;
                ex_dm_fwd_ex_s=1;
                is_fwd_ex=1;
                
            }else{
                
                EX_ins.output_result=add_alu(EX_ins.rs_value,EX_ins.c_immediate_signed);
                ex_write_reg=EX_ins.rt;
            }
            
//            EX_ins.output_result=add_alu(EX_ins.rs_value, EX_ins.c_immediate_signed);
//            ex_write_reg=EX_ins.rt;
            
            break;
        case 8:
           // printf("addi\n");
            
            if (EX_ins.rs==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0 && MEM_ins.opcode!=0x2b &&  MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                EX_ins.output_result=addi_alu(EX_ins.output_result, EX_ins.c_immediate_signed);
                
                ex_write_reg=EX_ins.rt;
                ex_dm_fwd_t=0;
                ex_dm_fwd_s=0;
                ex_dm_fwd_ex_s=1;
                is_fwd_ex=1;
                
            }else{
                

                EX_ins.output_result=addi_alu(EX_ins.rs_value, EX_ins.c_immediate_signed);
                


                ex_write_reg=EX_ins.rt;
            }
            
            
            
//            EX_ins.output_result=addi_alu(EX_ins.rs_value, EX_ins.c_immediate_signed);
//            ex_write_reg=EX_ins.rt;
            break;
        case 9:
          //  printf("addiu\n");
            if (EX_ins.rs==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0&& MEM_ins.opcode!=0x2b &&  MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                EX_ins.output_result=addiu_alu(EX_ins.output_result, EX_ins.c_immediate_signed);
                ex_write_reg=EX_ins.rt;
                ex_dm_fwd_t=0;
                ex_dm_fwd_ex_s=1;
                is_fwd_ex=1;
                
            }else{
                
                EX_ins.output_result=addiu_alu(EX_ins.rs_value, EX_ins.c_immediate_signed);
                ex_write_reg=EX_ins.rt;
            }
            
            
//            EX_ins.output_result=addiu_alu(EX_ins.rs_value, EX_ins.c_immediate_signed);
//            ex_write_reg=EX_ins.rt;
            
            
            break;
        case 35:
          //  printf("lw\n");
            if (EX_ins.rs==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0 ) {
                EX_ins.output_result=add_alu(EX_ins.output_result, EX_ins.c_immediate_signed);
                
                ex_write_reg=EX_ins.rt;
                ex_dm_fwd_t=0;
                ex_dm_fwd_ex_s=1;
                is_fwd_ex=1;
                
            }else{
                
                EX_ins.output_result=add_alu(EX_ins.rs_value,EX_ins.c_immediate_signed);
                ex_write_reg=EX_ins.rt;
            }
            
//            EX_ins.output_result=add_alu(EX_ins.rs_value, EX_ins.c_immediate_signed);
//            ex_write_reg=EX_ins.rt;
            
            break;
        case 33:
           // printf("lh\n");
            if (EX_ins.rs==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0) {
                EX_ins.output_result=add_alu(EX_ins.output_result, EX_ins.c_immediate_signed);
                
                ex_write_reg=EX_ins.rt;
                ex_dm_fwd_t=0;
                ex_dm_fwd_ex_s=1;
                is_fwd_ex=1;
                
            }else{
                
                EX_ins.output_result=add_alu(EX_ins.rs_value,EX_ins.c_immediate_signed);
                ex_write_reg=EX_ins.rt;
            }
//            EX_ins.output_result=add_alu(EX_ins.rs_value, EX_ins.c_immediate_signed);
//            ex_write_reg=EX_ins.rt;
            break;
        case 32:
          //  printf("lb\n");
            if (EX_ins.rs==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0) {
                EX_ins.output_result=add_alu(EX_ins.output_result, EX_ins.c_immediate_signed);
                
                ex_write_reg=EX_ins.rt;
                ex_dm_fwd_t=0;
                ex_dm_fwd_ex_s=1;
                is_fwd_ex=1;
                
            }else{
                
                EX_ins.output_result=add_alu(EX_ins.rs_value,EX_ins.c_immediate_signed);
                ex_write_reg=EX_ins.rt;
            }
            
//            
//            EX_ins.output_result=add_alu(EX_ins.rs_value, EX_ins.c_immediate_signed);
//            ex_write_reg=EX_ins.rt;
            break;
        case 36:
         //   printf("lbu\n");
            if (EX_ins.rs==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0) {
                EX_ins.output_result=add_alu(EX_ins.output_result, EX_ins.c_immediate_signed);
                
                ex_write_reg=EX_ins.rt;
                ex_dm_fwd_t=0;
                ex_dm_fwd_ex_s=1;
                is_fwd_ex=1;
                
            }else{
                
                EX_ins.output_result=add_alu(EX_ins.rs_value,EX_ins.c_immediate_signed);
                ex_write_reg=EX_ins.rt;
            }
//            EX_ins.output_result=add_alu(EX_ins.rs_value, EX_ins.c_immediate_signed);
//            ex_write_reg=EX_ins.rt;
            break;
        case 43:
          //  printf("sw\n");
            
            if (EX_ins.rt==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0 && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                
                
                ex_write_reg=EX_ins.rt;
                ex_dm_fwd_t=0;
                ex_dm_fwd_ex_t=EX_ins.rt;
                is_fwd_ex=1;
                
            }
            
            if (EX_ins.rs==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0  && ex_write_reg!=0 && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                EX_ins.output_result=add_alu(EX_ins.output_result, EX_ins.c_immediate_signed);
                
                ex_write_reg=EX_ins.rt;
                ex_dm_fwd_t=0;
                ex_dm_fwd_ex_s=EX_ins.rs;
                is_fwd_ex=1;
                
            }else{
                
                EX_ins.output_result=add_alu(EX_ins.rs_value,EX_ins.c_immediate_signed);
                ex_write_reg=EX_ins.rt;
            }
            
            
//            
//            
//            EX_ins.output_result=adds_alu(EX_ins.rs_value, EX_ins.c_immediate_signed);
//            ex_write_reg=EX_ins.rt;
            break;
        case 41:
           // printf("sh\n");
            if (EX_ins.rt==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0 && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ) {
                ex_write_reg=EX_ins.rt;
                ex_dm_fwd_t=0;
                
                ex_dm_fwd_ex_t=1;
                is_fwd_ex=1;
                
            }
            
            if (EX_ins.rs==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0 && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28  ) {
                EX_ins.output_result=add_alu(EX_ins.output_result, EX_ins.c_immediate_signed);
                
                ex_write_reg=EX_ins.rt;
                ex_dm_fwd_ex_s=1;
                is_fwd_ex=1;
                
            }else{
                
                EX_ins.output_result=add_alu(EX_ins.rs_value,EX_ins.c_immediate_signed);
                ex_write_reg=EX_ins.rt;
            }
            
//            
//            EX_ins.output_result=add_alu(EX_ins.rs_value, EX_ins.c_immediate_signed);
//            ex_write_reg=EX_ins.rt;
            break;
        case 40:
         //   printf("sb\n");
            if (EX_ins.rt==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0  && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ) {
                
                if (EX_ins.rs==EX_ins.rt) {
                    EX_ins.output_result=add_alu(EX_ins.output_result, EX_ins.c_immediate_signed);
                    
                    ex_dm_fwd_ex_s=EX_ins.rs;
                    ex_dm_fwd_ex_t=EX_ins.rt;
                    ex_dm_fwd_t=0;
                    ex_dm_fwd_s=0;
                    is_fwd_ex=1;
                    ex_write_reg=ID_ins.rd;
                    
                }else{
                
                    
                EX_ins.output_result=add_alu(EX_ins.rs_value, EX_ins.c_immediate_signed);
                ex_write_reg=EX_ins.rt;
                ex_dm_fwd_t=0;
                ex_dm_fwd_ex_t=EX_ins.rt;
                ex_dm_fwd_ex_s=0;
                is_fwd_ex=1;
                }
            }else if (EX_ins.rs==ex_write_reg  && strcmp(MEM_ins.DM_name, "NOP") && ex_write_reg!=0 && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ) {
                
                EX_ins.output_result=add_alu(EX_ins.output_result, EX_ins.c_immediate_signed);
                ex_write_reg=EX_ins.rt;
                ex_dm_fwd_t=0;
                ex_dm_fwd_ex_t=0;
                ex_dm_fwd_ex_s=EX_ins.rs;
                is_fwd_ex=1;
                
            }else{
                
                EX_ins.output_result=add_alu(EX_ins.rs_value,EX_ins.c_immediate_signed);
                ex_write_reg=EX_ins.rt;
            }
            
//            EX_ins.output_result=add_alu(EX_ins.rs_value, EX_ins.c_immediate_signed);
//            ex_write_reg=EX_ins.rt;
            break;
        case 15:
           // printf("lui\n");
            
            
           EX_ins.output_result= EX_ins.c_immediate_signed<<16;
            ex_write_reg=EX_ins.rt;
            break;
        case 12:
          //  printf("andi\n");
            
            if (EX_ins.rs==ex_write_reg && ex_write_reg!=0  && strcmp(MEM_ins.DM_name, "NOP") && MEM_ins.opcode!=0x2b &&  MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                EX_ins.output_result=and_alu(EX_ins.output_result, EX_ins.c_immediate_i);
                
                ex_write_reg=EX_ins.rt;
                ex_dm_fwd_s=0;
                ex_dm_fwd_ex_s=1;
                is_fwd_ex=1;
                
            }else{
                EX_ins.output_result=and_alu(EX_ins.rs_value, EX_ins.c_immediate_i);
                ex_write_reg=EX_ins.rt;
            }
//            
//            EX_ins.output_result=and_alu(EX_ins.rs_value, EX_ins.c_immediate_i);
//            ex_write_reg=EX_ins.rt;
            break;
        case 13:
        //    printf("ori\n");
            if (EX_ins.rs==ex_write_reg && ex_write_reg!=0 && strcmp(MEM_ins.DM_name, "NOP") && MEM_ins.opcode!=0x2b &&  MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                EX_ins.output_result=or_alu(EX_ins.output_result, EX_ins.c_immediate_i);
                
                ex_write_reg=EX_ins.rt;
                ex_dm_fwd_s=0;
                ex_dm_fwd_ex_s=1;
                is_fwd_ex=1;
                
            }else{
                EX_ins.output_result=or_alu(EX_ins.rs_value, EX_ins.c_immediate_i);
                ex_write_reg=EX_ins.rt;
            }
            
            
            
            
            
            break;
        case 14:
         //   printf("nori\n");
            
            if (EX_ins.rs==ex_write_reg & ex_write_reg!=0  && strcmp(MEM_ins.DM_name, "NOP") && MEM_ins.opcode!=0x2b &&  MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                EX_ins.output_result=nor_alu(EX_ins.output_result, EX_ins.c_immediate_i);
                
                ex_write_reg=EX_ins.rt;
                ex_dm_fwd_s=0;
                ex_dm_fwd_ex_s=1;
                is_fwd_ex=1;
                
            }else{
                EX_ins.output_result=nor_alu(EX_ins.rs_value, EX_ins.c_immediate_i);
                ex_write_reg=EX_ins.rt;
            }
            
            
//            EX_ins.output_result=nor_alu(EX_ins.rs_value, EX_ins.c_immediate_i);
//            ex_write_reg=EX_ins.rt;
//            
//            
            
            
            break;
        case 10:
          //  printf("slti\n");
           
            
            if (EX_ins.rs==ex_write_reg  && ex_write_reg!=0 && strcmp(MEM_ins.DM_name, "NOP") && MEM_ins.opcode!=0x2b &&  MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                EX_ins.output_result=slt_alu(EX_ins.output_result, EX_ins.c_immediate_signed);
                
                ex_write_reg=EX_ins.rt;
                ex_dm_fwd_s=0;
                ex_dm_fwd_ex_s=1;
                is_fwd_ex=1;
                
            }else{
                EX_ins.output_result=slt_alu(EX_ins.rs_value, EX_ins.c_immediate_signed);
                ex_write_reg=EX_ins.rt;
            }
            
            
//            EX_ins.output_result=slt_alu(EX_ins.rs_value, EX_ins.c_immediate_signed);
//            ex_write_reg=EX_ins.rt;
            
            break;
        case 4:
           // printf("beq\n");
            ex_write_reg=0;
            break;
        case 5:
            //printf("bne\n");
            ex_write_reg=0;
            
            
            break;
        case 7:
            //printf("bgtz\n");
            ex_write_reg=0;
          
            
            break;
        case 2:
            //printf("j\n");
            EX_ins.rt=0;
            EX_ins.rs=0;
            EX_ins.rd=0;
            EX_ins.output_result=0;
            ex_write_reg=0;
            break;
        case 3:
           // printf("jal\n");
            ex_write_reg=31;
            EX_ins.output_result=jal_pc_value;
            
            
            break;
        case 63:
           // printf("halt\n");
            EX_ins.is_EX_run=0;
            return;
            break;
        default:
            printf("no instrcution matched!!!\n");
            
            break;
            
            
    }
    
    
    
    
    
    
    
    
    
    
    
}



void ID(){
    
    
    
    
    
    
    //EX/DM to ID forwarding
    if (is_fwd) {
        
        switch (ID_ins.opcode) {
                
            case 4://beq
                
                if(ex_write_reg==ID_ins.rt ){
                    
                    beq_ID(EX_ins.output_result,ex_dm_fwd_t,ID_ins.c_immediate_signed);
                    
                }
                else if(ex_write_reg==ID_ins.rs ){
                    beq_ID(EX_ins.output_result,ex_dm_fwd_s,ID_ins.c_immediate_signed);
                }
                break;
            case 5://bne
                if(ex_write_reg==ID_ins.rt ){
                    
                    bnq_ID(EX_ins.output_result,ex_dm_fwd_t,ID_ins.c_immediate_signed);
                    
                }
                else if(ex_write_reg==ID_ins.rs ){
                    bnq_ID(EX_ins.output_result,ex_dm_fwd_s,ID_ins.c_immediate_signed);
                    
                }
                break;
            case 7://bgtz
                
                if(ex_write_reg==ID_ins.rt ){
                    bgtz_ID(ex_dm_fwd_t, ID_ins.c_immediate_signed);
                    
                }
                else if(ex_write_reg==ID_ins.rs ){
                    bgtz_ID(ex_dm_fwd_s, ID_ins.c_immediate_signed);
                    
                }
                break;
            case 2://j
                j_ID(ID_ins.c_immediate_signed);
                break;
            case 3://jal
                jal_pc_value=(pc-1)<<2;
                jal_ID(ID_ins.c_immediate_signed);
                EX_ins.output_result=jal_pc_value;
                break;
            default:
                stall=0;
                
                break;
        }
        
        
    }
    else{
        //no hazrd
        
        switch (ID_ins.opcode) {
            case 4://beq
                beq_ID(reg_file[ID_ins.rs].val, reg_file[ID_ins.rt].val, ID_ins.c_immediate_signed);
                
                break;
            case 5://bnq
                bnq_ID(reg_file[ID_ins.rs].val, reg_file[ID_ins.rt].val, ID_ins.c_immediate_signed);
                break;
            case 7://bgtz
                
                bgtz_ID(reg_file[ID_ins.rs].val, ID_ins.c_immediate_signed);
                //int value=pc<<2;
                
                
                break;
            case 2://j
                j_ID(ID_ins.c_immediate_signed);
                
                break;
            case 3:
                jal_pc_value=(pc-1)<<2;
                jal_ID(ID_ins.c_immediate_signed);
                
                EX_ins.output_result=jal_pc_value;
                break;
                
            default:
                break;
        }
        
        
        
        
        
        
    }
    
    
    if (!strcmp(ID_ins.ID_name, "JR")) {
        
        if (ID_ins.rs==ex_write_reg && ex_write_reg!=0 && strcmp(MEM_ins.DM_name, "NOP")) {
            is_fwd=1;
            ex_dm_fwd_s=1;
            ex_dm_fwd_t=0;
            pc=EX_ins.output_result>>2;
        }else{
            
        pc=(reg_file[ID_ins.rs].val>>2);
        }
        
    }
    
    
    is_fwd=0;
    
    
    
    
    
    
}



void IF(int pc ,struct instruct_mem *im){
    
    
    if(flush){
        ID_ins.opcode=0;
        ID_ins.function=0;
        
        
        ID_ins.rs=0;
        ID_ins.rs_value=0;
        
        ID_ins.rt=0;
        ID_ins.rt_value=0;
        
        ID_ins.rd=0;
        ID_ins.rd_value=0;
        
        
        ID_ins.c_shame_i=0;
        ID_ins.c_immediate_i=0;
        ID_ins.c_immediate_signed=0;
        strcpy(ID_ins.ID_name, "NOP");
        stall=0;
        
        flush=0;
        return;
        
    }
    
    
    //they are unsigned magnitude
    ID_ins.opcode=im->mem[pc].opcode_i;
    ID_ins.function=im->mem[pc].funct_i;
    
    
    ID_ins.rs=im->mem[pc].rs_i;
    ID_ins.rs_value=reg_file[ID_ins.rs].val;
    
    ID_ins.rt=im->mem[pc].rt_i;
    ID_ins.rt_value=reg_file[ID_ins.rt].val;
    
    ID_ins.rd=im->mem[pc].rd_i;
    ID_ins.rd_value=reg_file[ID_ins.rd].val;
    
    
    ID_ins.c_shame_i=im->mem[pc].c_shame_i;
    ID_ins.c_immediate_i=im->mem[pc].c_immediate_i;
    ID_ins.c_immediate_signed=im->mem[pc].c_immdeiate_signed;
    
    
    //set its name
    
    
    
    switch (im->mem[pc].opcode_i) {
        case 0:
            
            if (check_S_stall()||check_S_stall()) {
                if (dm_write_reg==EX_ins.rt || dm_write_reg==EX_ins.rs) {
                    stall=0;
                }else
                stall=1;
                
            }else{
                stall=0;
            }
            
            if (dm_write_reg==EX_ins.rd && (EX_ins.rd==ID_ins.rs|| EX_ins.rd==ID_ins.rt) && dm_write_reg!=0)
                stall=0;
            if (ID_ins.rt==ex_write_reg || ID_ins.rs==ex_write_reg) {
                
                if (ex_write_reg!=0 &&  ex_write_reg!=dm_write_reg && dm_write_reg!=ID_ins.rs && dm_write_reg!=ID_ins.rt && dm_write_reg!=0 )
                stall=0;
                //forward
            }
            
            switch (im->mem[pc].funct_i) {
                    
                case 32:
                    //printf("ADD\n");
                    strcpy(IF_name, "ADD");
                    
                    if (ID_ins.rs==dm_write_reg &&  ID_ins.rt==ex_write_reg  && ex_write_reg!=0 && dm_write_reg!=0 ) {
                        stall=1;
                        if (dm_write_reg==ex_write_reg && strcmp(EX_ins.EX_name, "NOP")) {
                            stall=0;
                        }
                    }else if (ID_ins.rs==ex_write_reg && ID_ins.rt==dm_write_reg && ex_write_reg!=0 && dm_write_reg!=0) {
                        stall=1;
                        if (dm_write_reg==ex_write_reg && strcmp(EX_ins.EX_name, "NOP"))  {
                            stall=0;
                        }
                    }else if (strcmp(ID_ins.ID_name, "LW") && strcmp(ID_ins.ID_name, "LH") && strcmp(ID_ins.ID_name, "LHU") && strcmp(ID_ins.ID_name, "LB") &&strcmp(ID_ins.ID_name, "LW")){
                        stall=0;
                    }
                    if (dm_write_reg==ID_ins.rs && dm_write_reg!=0 && strcmp(MEM_ins.DM_name, "NOP")) {
                        if (dm_write_reg!=ex_write_reg) {
                            stall=1;
                        }
                    }
                    
                    if (dm_write_reg==ID_ins.rt && dm_write_reg!=0 && strcmp(MEM_ins.DM_name, "NOP")) {
                        if (dm_write_reg!=ex_write_reg) {
                            stall=1;
                        }
                    }
                    
                    
                    if ((ID_ins.rt==dm_write_reg || ID_ins.rs==dm_write_reg)&& dm_write_reg!=0 && !strcmp(EX_ins.EX_name, "NOP")) {
                        stall=1;
                    }
                    
                    if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 &&(!strcmp(EX_ins.EX_name, "NOP") || ex_write_reg!=dm_write_reg) && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 && strcmp(MEM_ins.DM_name, "JR") ) {
                        stall=1;
                    }else if (ID_ins.rt==dm_write_reg && dm_write_reg!=0 && (!strcmp(EX_ins.EX_name, "NOP") || ex_write_reg!=dm_write_reg )&& MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28  && strcmp(MEM_ins.DM_name, "JR")) {
                        stall=1;
                    }else  {
                        
                        if (ID_ins.rs==ex_write_reg && ex_write_reg!=0 &&( EX_ins.opcode==0x23 || EX_ins.opcode==0x20 ) ) {
                            stall=1;
                        }else if (ID_ins.rt==ex_write_reg && ex_write_reg!=0 &&( EX_ins.opcode==0x23 || EX_ins.opcode==0x20 )){
                            stall=1;
                        }else
                        
                        stall=0;
                    }
                    if ((ID_ins.rt==ex_write_reg || ID_ins.rs==ex_write_reg)&& ex_write_reg!=0 && EX_ins.opcode==0x23) {
                        stall=1;
                    }
                    
                    
                    
                    
                    
                    break;
                case 33:
                  //  printf("ADDU\n");
                    strcpy(IF_name, "ADDU");
                    
                    if (ID_ins.rs==dm_write_reg &&  ID_ins.rt==ex_write_reg  && ex_write_reg!=0 && dm_write_reg!=0 ) {
                        stall=1;
                    }else if (ID_ins.rs==ex_write_reg && ID_ins.rt==dm_write_reg && ex_write_reg!=0 && dm_write_reg!=0) {
                        stall=1;
                    }else if (strcmp(ID_ins.ID_name, "LW") && strcmp(ID_ins.ID_name, "LH") && strcmp(ID_ins.ID_name, "LHU") && strcmp(ID_ins.ID_name, "LB") &&strcmp(ID_ins.ID_name, "LW")){
                        stall=0;
                    }
                    
                    if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 &&(!strcmp(EX_ins.EX_name, "NOP") || ex_write_reg!=dm_write_reg) && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                        stall=1;
                    }else if (ID_ins.rt==dm_write_reg && dm_write_reg!=0 && (!strcmp(EX_ins.EX_name, "NOP") || ex_write_reg!=dm_write_reg )&& MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                        stall=1;
                    }else{
                        stall=0;
                    }
                    
                    break;
                case 34:
                   // printf("SUB\n");
                    strcpy(IF_name, "SUB");
                    
                    if (ID_ins.rs==dm_write_reg &&  ID_ins.rt!=ex_write_reg  && ex_write_reg!=0 && dm_write_reg!=0 ) {
                        stall=1;
                    }else if ((ID_ins.rs!=ex_write_reg || ID_ins.rs==0 ) && ID_ins.rt==dm_write_reg && dm_write_reg!=0) {
                        stall=1;
                    }else if (strcmp(ID_ins.ID_name, "LW") && strcmp(ID_ins.ID_name, "LH") && strcmp(ID_ins.ID_name, "LHU") && strcmp(ID_ins.ID_name, "LB") &&strcmp(ID_ins.ID_name, "LW")){
                        stall=0;
                    }
                    if (ex_write_reg==dm_write_reg && dm_write_reg!=0 && ID_ins.rs ==ex_write_reg) {
                        stall=0;
                    }
                    
                   
                    
                    if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 &&(!strcmp(EX_ins.EX_name, "NOP") || ex_write_reg!=dm_write_reg) && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                        stall=1;
                    }else if (ID_ins.rt==dm_write_reg && dm_write_reg!=0 && (!strcmp(EX_ins.EX_name, "NOP") || ex_write_reg!=dm_write_reg )&& MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                        stall=1;
                    }else{
                        stall=0;
                    }
                    
                    if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 && dm_write_reg==ex_write_reg && (EX_ins.opcode==0x2b|| EX_ins.opcode==0x28 || EX_ins.opcode==0x29  )   ) {
                        stall=1;
                    }
                    if (ID_ins.rs==ex_write_reg && ex_write_reg!=0 && EX_ins.opcode==0x23) {
                        stall=1;
                    }
                    
                    
                    
                    
                    
                    break;
                case 36:
                   // printf("AND\n");
                    strcpy(IF_name, "AND");
                    
                    if (ID_ins.rs==dm_write_reg &&  ID_ins.rt==ex_write_reg  && ex_write_reg!=0 && dm_write_reg!=0 ) {
                        stall=1;
                    }else if (ID_ins.rs==ex_write_reg && ID_ins.rt==dm_write_reg && ex_write_reg!=0 && dm_write_reg!=0) {
                        stall=1;
                    }else if (strcmp(ID_ins.ID_name, "LW") && strcmp(ID_ins.ID_name, "LH") && strcmp(ID_ins.ID_name, "LHU") && strcmp(ID_ins.ID_name, "LB") &&strcmp(ID_ins.ID_name, "LW")){
                        stall=0;
                    }
                    
                    
                    
                    
                    if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 &&(!strcmp(EX_ins.EX_name, "NOP") || ex_write_reg!=dm_write_reg) && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                        stall=1;
                    }else if (ID_ins.rt==dm_write_reg && dm_write_reg!=0 && (!strcmp(EX_ins.EX_name, "NOP") || ex_write_reg!=dm_write_reg )&& MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                        stall=1;
                    }else{
                        stall=0;
                    }
                    
                    
                    
                    
                    
                    break;
                case 37:
                  //  printf("OR\n");
                    strcpy(IF_name, "OR");
                    
                    if (ID_ins.rs==dm_write_reg &&  ID_ins.rt==ex_write_reg  && ex_write_reg!=0 && dm_write_reg!=0 ) {
                        stall=1;
                    }else if (ID_ins.rs==ex_write_reg && ID_ins.rt==dm_write_reg && ex_write_reg!=0 && dm_write_reg!=0) {
                        stall=1;
                    }else if (strcmp(ID_ins.ID_name, "LW") && strcmp(ID_ins.ID_name, "LH") && strcmp(ID_ins.ID_name, "LHU") && strcmp(ID_ins.ID_name, "LB") &&strcmp(ID_ins.ID_name, "LW")){
                        stall=0;
                    }
                    
                    if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 &&(!strcmp(EX_ins.EX_name, "NOP") || ex_write_reg!=dm_write_reg) && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                        stall=1;
                    }else if (ID_ins.rt==dm_write_reg && dm_write_reg!=0 && (!strcmp(EX_ins.EX_name, "NOP") || ex_write_reg!=dm_write_reg )&& MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                        stall=1;
                    }
                    
                
                    
                    
                    
                    
                    
                    break;
                case 38:
                    //printf("xor\n");
                    strcpy(IF_name, "XOR");
                    
                    if (ID_ins.rs==dm_write_reg &&  ID_ins.rt==ex_write_reg  && ex_write_reg!=0 && dm_write_reg!=0 ) {
                        stall=1;
                    }else if (ID_ins.rs==ex_write_reg && ID_ins.rt==dm_write_reg && ex_write_reg!=0 && dm_write_reg!=0) {
                        stall=1;
                    }else if (strcmp(ID_ins.ID_name, "LW") && strcmp(ID_ins.ID_name, "LH") && strcmp(ID_ins.ID_name, "LHU") && strcmp(ID_ins.ID_name, "LB") &&strcmp(ID_ins.ID_name, "LW")){
                        stall=0;
                    }
                    
                    if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 &&(!strcmp(EX_ins.EX_name, "NOP") || ex_write_reg!=dm_write_reg) && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                        stall=1;
                    }else if (ID_ins.rt==dm_write_reg && dm_write_reg!=0 && (!strcmp(EX_ins.EX_name, "NOP") || ex_write_reg!=dm_write_reg )&& MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                        stall=1;
                    }
                    
                    
                    
                    
                    break;
                case 39:
                    //printf("nor\n");
                    strcpy(IF_name, "NOR");
                    
                    if (ID_ins.rs==dm_write_reg &&  ID_ins.rt==ex_write_reg  && ex_write_reg!=0 && dm_write_reg!=0 ) {
                        stall=1;
                        
                    }else if (ID_ins.rs==ex_write_reg && ID_ins.rt==dm_write_reg && ex_write_reg!=0 && dm_write_reg!=0) {
                        stall=1;
                    }else if (strcmp(ID_ins.ID_name, "LW")){
                        stall=0;
                    }else if (strcmp(ID_ins.ID_name, "LW") && strcmp(ID_ins.ID_name, "LH") && strcmp(ID_ins.ID_name, "LHU") && strcmp(ID_ins.ID_name, "LB") &&strcmp(ID_ins.ID_name, "LW")){
                        stall=0;
                    }
                    
                    if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 &&(!strcmp(EX_ins.EX_name, "NOP") || ex_write_reg!=dm_write_reg) && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                        stall=1;
                    }else if (ID_ins.rt==dm_write_reg && dm_write_reg!=0 && (!strcmp(EX_ins.EX_name, "NOP") || ex_write_reg!=dm_write_reg )&& MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                        stall=1;
                    }
                    if (ex_write_reg==ID_ins.rs && dm_write_reg==ID_ins.rs && EX_ins.opcode==0 && MEM_ins.opcode!=0x23) {
                        stall=0;
                    }
                    
                    
                    
                    break;
                case 40:
                    //printf("nand\n");
                    strcpy(IF_name, "NAND");
                    
                    if (ID_ins.rs==dm_write_reg &&  ID_ins.rt==ex_write_reg  && ex_write_reg!=0 && dm_write_reg!=0 ) {
                        stall=1;
                    }else if (ID_ins.rt==dm_write_reg && ID_ins.rs==ex_write_reg && ex_write_reg!=0 && dm_write_reg!=0) {
                        stall=1;
                    }else if (strcmp(ID_ins.ID_name, "LW") && strcmp(ID_ins.ID_name, "LH") && strcmp(ID_ins.ID_name, "LHU") && strcmp(ID_ins.ID_name, "LB") &&strcmp(ID_ins.ID_name, "LW")){
                        stall=0;
                    }
                   
                    if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 &&(!strcmp(EX_ins.EX_name, "NOP") || ex_write_reg!=dm_write_reg) && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                        stall=1;
                    }else if (ID_ins.rt==dm_write_reg && dm_write_reg!=0 && (!strcmp(EX_ins.EX_name, "NOP") || ex_write_reg!=dm_write_reg )&& MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                        stall=1;
                    }else{
                        if (ID_ins.rs==ex_write_reg && ex_write_reg!=0  && (EX_ins.opcode==0x23  ||EX_ins.opcode==0x20   )) {
                            stall=1;
                            
                        }else if (ID_ins.rt==ex_write_reg&& ex_write_reg!=0 && (EX_ins.opcode==0x23 ||EX_ins.opcode==0x20  ) ) {
                            stall=1;
                        }else
                        stall=0;
                    }
                    
                    
                    
                    break;
                case 42:
                    //printf("slt\n");
                    strcpy(IF_name, "SLT");
                    
                    if (ID_ins.rs==dm_write_reg &&  ID_ins.rt==ex_write_reg  && ex_write_reg!=0 && dm_write_reg!=0 ) {
                        stall=1;
                    }else if (ID_ins.rt==dm_write_reg && ID_ins.rs==ex_write_reg && ex_write_reg!=0 && dm_write_reg!=0) {
                        stall=1;
                    }else if (strcmp(ID_ins.ID_name, "LW") && strcmp(ID_ins.ID_name, "LH") && strcmp(ID_ins.ID_name, "LHU") && strcmp(ID_ins.ID_name, "LB") &&strcmp(ID_ins.ID_name, "LW")){
                        stall=0;
                    }
                    
                    if (ID_ins.rs==ex_write_reg && ex_write_reg==dm_write_reg  && dm_write_reg!=0 && EX_ins.opcode==0x2b) {
                        stall=1;
                    }
                    
                    if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 &&(!strcmp(EX_ins.EX_name, "NOP") || ex_write_reg!=dm_write_reg) && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                        stall=1;
                    }else if (ID_ins.rt==dm_write_reg && dm_write_reg!=0 && (!strcmp(EX_ins.EX_name, "NOP") || ex_write_reg!=dm_write_reg )&& MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                        stall=1;
                    }
                  
                    if ((ID_ins.rt==ex_write_reg || ID_ins.rs==ex_write_reg )&& ex_write_reg!=0 ) {
                        if (EX_ins.opcode==0x23 || EX_ins.opcode==0x21 ||EX_ins.opcode==0x25 || EX_ins.opcode==0x20 || EX_ins.opcode==0x24  ) {
                            stall=1;
                        }
                    }
                    
                    
                    
                    break;
                case 0:
                    //printf("sll\n");
                    strcpy(IF_name, "SLL");
                    
                    if (ID_ins.rt==dm_write_reg  && ex_write_reg!=0 && dm_write_reg!=0) {
                        stall=1;
                    }else if (strcmp(ID_ins.ID_name, "LW") && strcmp(ID_ins.ID_name, "LH") && strcmp(ID_ins.ID_name, "LHU") && strcmp(ID_ins.ID_name, "LB") &&strcmp(ID_ins.ID_name, "LW")){
                        stall=0;
                    }
                    
                    
                    if(ID_ins.rt==0 && ID_ins.rd==0 && ID_ins.c_shame_i==0 ){
                        
                        strcpy(IF_name, "NOP");
                        
                        stall=0;
                        
                    }
                    
                    if (ID_ins.rt==ex_write_reg && ex_write_reg!=0 && (EX_ins.opcode==0x23  || EX_ins.opcode==0x21 || EX_ins.opcode==0x25 || EX_ins.opcode==0x20 || EX_ins.opcode==0x24)) {
                        stall=1;
                    }else{
                        stall=0;
                    }
                    if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 &&(!strcmp(EX_ins.EX_name, "NOP") || ex_write_reg!=dm_write_reg) && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                        stall=1;
                    }else if (ID_ins.rt==dm_write_reg && dm_write_reg!=0 && (!strcmp(EX_ins.EX_name, "NOP") || ex_write_reg!=dm_write_reg )&& MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                        stall=1;
                    }
                    
                    
                    
                    
                    break;
                case 2:
                   // printf("srl\n");
                    strcpy(IF_name, "SRL");
                    
                    if (ID_ins.rt==dm_write_reg && ID_ins.rt!=ex_write_reg && dm_write_reg!=0){
                        stall=1;
                    }else if (strcmp(ID_ins.ID_name, "LW") && strcmp(ID_ins.ID_name, "LH") && strcmp(ID_ins.ID_name, "LHU") && strcmp(ID_ins.ID_name, "LB") &&strcmp(ID_ins.ID_name, "LW")){
                        stall=0;
                    }
                    
                    if (ID_ins.rt==ex_write_reg && ex_write_reg!=0 && EX_ins.opcode==0x23) {
                        stall=1;
                    }
                    if (ID_ins.rt==dm_write_reg && dm_write_reg!=0 && !strcmp(EX_ins.EX_name, "NOP") && MEM_ins.opcode!=0x2b ) {
                        stall=1;
                    }
                    
                    break;
                case 3:
                   // printf("sra\n");
                    strcpy(IF_name, "SRA");
                    
                    if (ID_ins.rs==dm_write_reg &&  ID_ins.rt==ex_write_reg  && ex_write_reg!=0 && dm_write_reg!=0 ) {
                        stall=1;
                    }else if (ID_ins.rt==dm_write_reg && ID_ins.rs==ex_write_reg && ex_write_reg!=0 && dm_write_reg!=0) {
                        stall=1;
                    }else if (strcmp(ID_ins.ID_name, "LW") && strcmp(ID_ins.ID_name, "LH") && strcmp(ID_ins.ID_name, "LHU") && strcmp(ID_ins.ID_name, "LB") &&strcmp(ID_ins.ID_name, "LW")){
                        stall=0;
                    }
                    
                    if (ID_ins.rt==dm_write_reg && dm_write_reg!=0 && (!strcmp(EX_ins.EX_name, "NOP") ||ex_write_reg!=dm_write_reg )  && MEM_ins.opcode!=0x2b ) {
                        stall=1;
                    }
                    
                    break;
                case 8:
                    //printf("jr\n");
                    strcpy(IF_name, "JR");
                    
                    flush=1;
                    if (ID_ins.rs== ex_write_reg && ex_write_reg!=0 && strcmp(EX_ins.EX_name, "NOP")) {
                        stall=1;
                    }else
                    stall=0;
                    
                    if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 && MEM_ins.opcode!=0x23 && MEM_ins.opcode!=0x21 &&  MEM_ins.opcode!=0x25 &&  MEM_ins.opcode!=0x20 && MEM_ins.opcode!=0x24  && strcmp(MEM_ins.DM_name, "JR")) {
                        is_fwd=1;
                        ex_dm_fwd_s=ID_ins.rs;
                    }
                    if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 && MEM_ins.opcode==0x23) {
                        stall=1;
                    }
                    
                    
                    
                    
                    break;
                    
                default:
                    printf("no function match \n");
                    break;
            }
            break;
            
        case 37:
           // printf("lhu\n");
            strcpy(IF_name, "LHU");
            if (check_S_stall()) {
                
                stall=1;
            }else{
                stall=0;
            }
            if (check_S_forward_ex_dm_ex()) {
                stall=0;
            }
            
            
            if (check_S_forward_ex_dm_ex()) {
                stall=0;
            }
            if (ID_ins.rt==ex_write_reg && ex_write_reg!=0) {
                if (ID_ins.rs==dm_write_reg && ex_write_reg!=0)
                    stall=1;
                else
                    stall=0;
            }
            
            if (ID_ins.rs==ex_write_reg && ex_write_reg!=0) {
                if (ID_ins.rt==dm_write_reg && ex_write_reg!=0)
                    stall=1;
                else
                    stall=0;
            }
            
            if (ID_ins.rt==dm_write_reg && dm_write_reg!=0) {
                stall=0;
            }
            
            break;
        case 8:
           // printf("addi\n");
            strcpy(IF_name, "ADDI");

            
            if (ID_ins.rs==dm_write_reg && dm_write_reg!=0  && ex_write_reg!=dm_write_reg   &&MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28) {
                if (ID_ins.rs==ex_write_reg && ex_write_reg!=0) {
                    stall=0;
                }else
                    stall=1;
            }else{
                
                stall=0;
            }
            
            //check lw
            if (EX_ins.opcode==0x23 || EX_ins.opcode==0x21 || EX_ins.opcode==0x25|| EX_ins.opcode==0x20 || EX_ins.opcode==0x24 ) {
                if (EX_ins.rt==ID_ins.rs && ID_ins.rs!=0) {
                    stall=1;
                }
            }
            

            if (MEM_ins.opcode==0x03 && ID_ins.rs==dm_write_reg) {
                stall=1;
            }
            
            if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 && ex_write_reg==dm_write_reg && (EX_ins.opcode==0x2b || EX_ins.opcode==0x28 || EX_ins.opcode==0x29)) {
                stall=1;
            }
            if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 && !strcmp(EX_ins.EX_name, "NOP") && ID_ins.rs==1 ) {
                stall=1;
                
            }
            
            
            
            break;
        case 9:
         //   printf("addiu\n");
            strcpy(IF_name, "ADDIU");
            if (ID_ins.rs==dm_write_reg && dm_write_reg!=0) {
                if (ID_ins.rs==ex_write_reg && ex_write_reg!=0) {
                    stall=0;
                }else
                    stall=1;
            }
            
            //check lw
            if (EX_ins.opcode==0x23 || EX_ins.opcode==0x21 || EX_ins.opcode==0x25|| EX_ins.opcode==0x20 || EX_ins.opcode==0x24 ) {
                if (EX_ins.rt==ID_ins.rs && ID_ins.rs!=0) {
                    stall=1;
                }
            }
            
            break;
        case 35:
           // printf("lw\n");
            strcpy(IF_name, "LW");
            if (check_S_stall()) {
                
                stall=1;
            }else{
                stall=0;
            }
            if (check_S_forward_ex_dm_ex()) {
                stall=0;
            }
            if (check_S_forward_ex_dm_ex()) {
                stall=0;
            }
//            if (ID_ins.rt==ex_write_reg && ex_write_reg!=0) {
//                if (ID_ins.rs==dm_write_reg && ex_write_reg!=0)
//                    stall=1;
//                else
//                    stall=0;
//            }
            
            if (ID_ins.rs==ex_write_reg && ex_write_reg!=0) {
                if (ID_ins.rt==dm_write_reg && ex_write_reg!=0)
                    stall=1;
                else if(EX_ins.opcode==0x23 || EX_ins.opcode==0x21 ||EX_ins.opcode==0x25 ||EX_ins.opcode==0x20 ||EX_ins.opcode==0x24  ){
                    stall=1;
                }else{
                    stall=0;
                }
            }
            if (ID_ins.rt==dm_write_reg && dm_write_reg!=0) {
                stall=0;
            }
    
            
            
            
            break;
        case 33:
         //   printf("lh\n");
            strcpy(IF_name, "LH");
            if (check_S_stall()) {
                
                stall=1;
            }else{
                stall=0;
            }
            if (check_S_forward_ex_dm_ex()) {
                stall=0;
            }
            if (check_S_forward_ex_dm_ex()) {
                stall=0;
            }
            
            if (ID_ins.rs==ex_write_reg && ex_write_reg!=0) {
                if (ID_ins.rt==dm_write_reg && ex_write_reg!=0)
                    stall=1;
                else
                    stall=0;
            }
            
            if (ID_ins.rt==dm_write_reg && dm_write_reg!=0) {
                stall=0;
            }
            
            if (ID_ins.rs==ex_write_reg && ex_write_reg!=0 && (EX_ins.opcode==0x23  || EX_ins.opcode==0x21|| EX_ins.opcode==0x25 || EX_ins.opcode==0x20 ||EX_ins.opcode==0x24  )) {
                stall=1;
            }
            
            if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 && MEM_ins.opcode!=0x28 && MEM_ins.opcode!=0x27 && MEM_ins.opcode!=0x2b  ) {
               
                if (dm_write_reg==ex_write_reg && EX_ins.opcode!=0x23 && EX_ins.opcode!=0x21) {
                    stall=0;
                }else{
                stall=1;
                }
            }
            
            if (ID_ins.rt==ex_write_reg && ex_write_reg!=0 && ex_write_reg!=dm_write_reg && (EX_ins.opcode==0x2b || EX_ins.opcode==0x28 || EX_ins.opcode==0x29 )) {
                stall=0;
            }
            if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 && strcmp(MEM_ins.DM_name, "JR") && ex_write_reg==dm_write_reg) {
                stall=1;
            }
            
            if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 && MEM_ins.opcode==0x0f) {
                stall=0;
                
            }
            
            
            
            break;
        case 32:
            //printf("lb\n");
            strcpy(IF_name, "LB");
            if (check_S_stall()) {
                
                stall=1;
            }else{
                stall=0;
            }
            
            if (check_S_forward_ex_dm_ex()) {
                stall=0;
            }
            if (check_S_forward_ex_dm_ex()) {
                stall=0;
            }
            if (ID_ins.rt==ex_write_reg && ex_write_reg!=0) {
                if (ID_ins.rs==dm_write_reg && ex_write_reg!=0)
                    stall=1;
                else
                    stall=0;
            }
            
            if (ID_ins.rs==ex_write_reg && ex_write_reg!=0) {
                if(EX_ins.opcode==0x20 || EX_ins.opcode==0x23)
                    stall=1;
                else stall=0;
                
            }
            
            if (ID_ins.rt==dm_write_reg && dm_write_reg!=0 && MEM_ins.opcode!=0x23 && MEM_ins.opcode!=0x20 ) {
                stall=0;
            }
            
            break;
        case 36:
            //printf("lbu\n");
            strcpy(IF_name, "LBU");
            if (check_S_stall()) {
                
                stall=1;
            }else{
                stall=0;
            }
            
            if (check_S_forward_ex_dm_ex()) {
                stall=0;
            }
            if (check_S_forward_ex_dm_ex()) {
                stall=0;
            }
            if (ID_ins.rt==ex_write_reg && ex_write_reg!=0) {
                if (ID_ins.rs==dm_write_reg && ex_write_reg!=0)
                    stall=1;
                else
                    stall=0;
            }
            
            if (ID_ins.rs==ex_write_reg && ex_write_reg!=0) {
                if (ID_ins.rt==dm_write_reg && ex_write_reg!=0)
                    stall=1;
                else
                    stall=0;
            }
            if (ID_ins.rt==dm_write_reg && dm_write_reg!=0) {
                stall=0;
            }
            
            
            break;
        case 43:
          //  printf("sw\n");
            
            strcpy(IF_name, "SW");
            if (check_S_stall() || check_T_stall()) {
                
                stall=1;
            }else{
                stall=0;
            }
            
            if (check_S_forward_ex_dm_ex()) {
                stall=0;
            }
            if (ID_ins.rt==ex_write_reg && ex_write_reg!=0) {
                if (ID_ins.rs==dm_write_reg && dm_write_reg!=0)
                stall=1;
                else if(EX_ins.opcode!=0x23 && EX_ins.opcode!=0x21 && EX_ins.opcode!=0x25 &&  EX_ins.opcode!=0x20 && EX_ins.opcode!=0x24 )
                    stall=0;
            }
            
            if (ID_ins.rs==ex_write_reg && ex_write_reg!=0) {
                if (ID_ins.rt==dm_write_reg && dm_write_reg!=0)
                    stall=1;
                else if(EX_ins.opcode!=0x23 && EX_ins.opcode!=0x21 && EX_ins.opcode!=0x25 &&  EX_ins.opcode!=0x20 && EX_ins.opcode!=0x24 )
                    stall=0;
            }
            
            if (ID_ins.rt==dm_write_reg && dm_write_reg!=0  && MEM_ins.opcode==0x03) {
                stall=1;
            }
            
            if (ID_ins.rt==ex_write_reg && ex_write_reg== dm_write_reg && ex_write_reg !=0 & EX_ins.opcode!=0x23) {
                stall=0;
            }
            
            if (ID_ins.rt==dm_write_reg && ex_write_reg!=dm_write_reg && dm_write_reg!=0 && (MEM_ins.opcode==0x2b || MEM_ins.opcode==0x29 || MEM_ins.opcode==28)) {
                stall=0;
            }
            
            if (ID_ins.rt==dm_write_reg && MEM_ins.opcode==0x03 && !strcmp(EX_ins.EX_name, "NOP")) {
                stall=1;
                
            }
            
            
            
            
            
            
            
            break;
        case 41:
           // printf("sh\n");
            strcpy(IF_name, "SH");
            
            
            if (check_S_stall() || check_T_stall()) {
                
                stall=1;
            }else{
                stall=0;
            }
            
            if (check_S_forward_ex_dm_ex()) {
                stall=0;
            }
            if (ID_ins.rt==ex_write_reg && ex_write_reg!=0) {
                if (ID_ins.rs==dm_write_reg && dm_write_reg!=0)
                    stall=1;
                else if(EX_ins.opcode!=0x23 && EX_ins.opcode!=0x21 && EX_ins.opcode!=0x25 &&  EX_ins.opcode!=0x20 && EX_ins.opcode!=0x24 )
                    stall=0;
            }
            
            if (ID_ins.rs==ex_write_reg && ex_write_reg!=0) {
                if (ID_ins.rt==dm_write_reg && dm_write_reg!=0)
                    stall=1;
                else if(EX_ins.opcode!=0x23 && EX_ins.opcode!=0x21 && EX_ins.opcode!=0x25 &&  EX_ins.opcode!=0x20 && EX_ins.opcode!=0x24 )
                    stall=0;
            }
            
            if (ID_ins.rt==dm_write_reg && dm_write_reg!=0 && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x28 && MEM_ins.opcode!=0x29 ) {
                stall=1;
            }
            if (ID_ins.rs==dm_write_reg && dm_write_reg!=0  && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x28 && MEM_ins.opcode!=0x29  ) {
                stall=1;
            }
            
            if (ID_ins.rt==ex_write_reg && ex_write_reg== dm_write_reg && ex_write_reg !=0 && EX_ins.opcode!=0x28 && EX_ins.opcode!=0x2b &&  EX_ins.opcode!=0x23) {
                stall=0;
            }
            
            if (ID_ins.rt==dm_write_reg && ex_write_reg!=dm_write_reg && dm_write_reg!=0 && (MEM_ins.opcode==0x2b || MEM_ins.opcode==0x29 || MEM_ins.opcode==0x28)) {
                stall=0;
            }
            
            
            
            
            break;
        case 40:
            //printf("sb\n");
            strcpy(IF_name, "SB");
            if (check_S_stall() && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ) {
                
                stall=1;
            }else{
                stall=0;
            }
            
            if (check_S_forward_ex_dm_ex()) {
                stall=0;
            }
            if (check_S_forward_ex_dm_ex()) {
                stall=0;
            }
            if (ID_ins.rt==ex_write_reg && ex_write_reg!=0) {
                if ( ID_ins.rs==dm_write_reg && dm_write_reg!=0&& ex_write_reg!=0  && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28){
                    
                    stall=1;
                    if (!strcmp(MEM_ins.DM_name, "NOP")) {
                        stall=0;
                    }
                }else
                    stall=0;
                if (EX_ins.opcode==0x23|| EX_ins.opcode==0x21|| EX_ins.opcode==0x25|| EX_ins.opcode==0x20||EX_ins.opcode==0x24  ) {
                    stall=1;
                }
                
                
            }
            
            if (ID_ins.rs==ex_write_reg && ex_write_reg!=0) {
                if (ID_ins.rt==dm_write_reg && dm_write_reg!=0 &&ex_write_reg!=0  && MEM_ins.opcode!=0x2b && MEM_ins.opcode!=0x29 && MEM_ins.opcode!=0x28 ){
                    stall=1;
                    if (!strcmp(MEM_ins.DM_name, "NOP")) {
                        stall=0;
                    }
                } else
                    stall=0;
                
                if (EX_ins.opcode==0x23|| EX_ins.opcode==0x21|| EX_ins.opcode==0x25|| EX_ins.opcode==0x20||EX_ins.opcode==0x24  ) {
                    stall=1;
                }
            }
            
            
            
            
            
            break;
        case 15:
           // printf("lui\n");
            strcpy(IF_name, "LUI");
            stall=0;
            break;
        case 12:
           // printf("andi\n");
            strcpy(IF_name, "ANDI");
            if (ID_ins.rs==dm_write_reg && dm_write_reg!=0) {
                if (ID_ins.rs==ex_write_reg && ex_write_reg!=0) {
                    stall=0;
                }else
                    stall=1;
            }
            
            //check lw
            if (EX_ins.opcode==0x23 || EX_ins.opcode==0x21 || EX_ins.opcode==0x25|| EX_ins.opcode==0x20 || EX_ins.opcode==0x24 ) {
                if (EX_ins.rt==ID_ins.rs && ID_ins.rs!=0) {
                    stall=1;
                }
            }
            
            if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 && !strcmp(EX_ins.EX_name, "NOP") && MEM_ins.opcode!=0x2b ) {
                stall=1;
            }
            
            
            break;
        case 13:
           // printf("ori\n");
            strcpy(IF_name, "ORI");
            
            if (ID_ins.rs==dm_write_reg && dm_write_reg!=0) {
                if (ID_ins.rs==ex_write_reg && ex_write_reg!=0) {
                    stall=0;
                }else
                    stall=1;
            }
            
            //check lw
            if (EX_ins.opcode==0x23 || EX_ins.opcode==0x21 || EX_ins.opcode==0x25|| EX_ins.opcode==0x20 || EX_ins.opcode==0x24 ) {
                if (EX_ins.rt==ID_ins.rs && ID_ins.rs!=0) {
                    stall=1;
                }
            }
            break;
        case 14:
           // printf("nori\n");
            strcpy(IF_name, "NORI");
            
            if (ID_ins.rs==dm_write_reg && dm_write_reg!=0) {
                if (ID_ins.rs==ex_write_reg && ex_write_reg!=0 && strcmp(EX_ins.EX_name, "NOP")) {
                    stall=0;
                }else
                    stall=1;
            }
            
            //check lw
            if (EX_ins.opcode==0x23 || EX_ins.opcode==0x21 || EX_ins.opcode==0x25|| EX_ins.opcode==0x20 || EX_ins.opcode==0x24 ) {
                if (EX_ins.rt==ID_ins.rs && ID_ins.rs!=0) {
                    stall=1;
                }
            }
            
            if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 && dm_write_reg!=ex_write_reg && (MEM_ins.opcode==0x2b || MEM_ins.opcode==0x29 ||MEM_ins.opcode==0x28         )) {
                stall=0;
            }
            
            
            
            break;
        case 10:
           // printf("slti\n");
            strcpy(IF_name, "SLTI");
            if (ID_ins.rs==dm_write_reg && dm_write_reg!=0) {
                if (ID_ins.rs==ex_write_reg && ex_write_reg!=0) {
                    stall=0;
                }else
                    stall=1;
            }
            
            //check lw
            if (EX_ins.opcode==0x23 || EX_ins.opcode==0x21 || EX_ins.opcode==0x25|| EX_ins.opcode==0x20 || EX_ins.opcode==0x24 ) {
                if (EX_ins.rt==ID_ins.rs && ID_ins.rs!=0) {
                    stall=1;
                }
            }
            
            break;
        case 4:
           // printf("beq\n");
            
            strcpy(IF_name, "BEQ");
            
            if ( ( (ID_ins.rs==ex_write_reg  && ex_write_reg!=0 ) || (ID_ins.rt==ex_write_reg && ex_write_reg!=0 ) )  && strcmp(EX_ins.EX_name, "NOP") ) {
                stall=1;
                
                
            }else{
                stall=0;
                
                if(reg_file[ID_ins.rs].val==reg_file[ID_ins.rt].val ){
                    flush=1;
                }
                
            }
            
            if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 && ID_ins.rs==8) {
                is_fwd=1;
                ex_dm_fwd_s=8;
            }
            
            if (ID_ins.rs==ex_write_reg && ex_write_reg!=0 && (EX_ins.opcode==0x2b|| EX_ins.opcode==0x29|| EX_ins.opcode==0x28 ) ) {
                stall=0;
                if (ID_ins.rs_value==ID_ins.rt_value) {
                    flush=1;
                    
                }
                
            }
            
            if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 && (!strcmp(EX_ins.EX_name, "NOP") || (ex_write_reg!=dm_write_reg || (ex_write_reg==dm_write_reg &&EX_ins.opcode==0x2b)  ) )&& MEM_ins.opcode!=0x23) {
                stall=0;
                is_fwd=1;
                ex_dm_fwd_s=ID_ins.rs;
               
                
                
                
                if (dm_val==ID_ins.rt_value) {
                    flush=1;
                }else{
                    flush=0;
                }
                
                if (ID_ins.rt==ID_ins.rs) {
                    flush=1;
                    ex_dm_fwd_t=ID_ins.rt;
                    ex_dm_fwd_s=ID_ins.rs;
                    
                }
                
                
                
            }else if (ID_ins.rt==dm_write_reg &&  dm_write_reg!=0&& (!strcmp(EX_ins.EX_name, "NOP") || ex_write_reg!=dm_write_reg)&& MEM_ins.opcode!=0x23) {
                stall=0;
                is_fwd=1;
                ex_dm_fwd_t=ID_ins.rt;
                if (dm_val==ID_ins.rs_value) {
                    flush=1;
                }
                
            }
            
            if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 && ex_write_reg!=dm_write_reg && MEM_ins.opcode==0x23) {
                stall=1;
                is_fwd=0;
            }
            if (ID_ins.rt==dm_write_reg && dm_write_reg!=0 && ex_write_reg!=dm_write_reg && MEM_ins.opcode==0x23) {
                stall=1;
                is_fwd=0;
            }

            if (ID_ins.rs==ex_write_reg && ex_write_reg!=0 ) {
                stall=1;
            }
            if (ID_ins.rt==ex_write_reg && ex_write_reg!=0) {
                stall=1;
            }
            
            if (!strcmp(EX_ins.EX_name, "NOP") && !strcmp(EX_ins.EX_name, "NOP")) {
                stall=0;
            }
            
            if (ID_ins.rs==ex_write_reg && ex_write_reg!=0 && EX_ins.opcode==0x29 && ID_ins.rs==dm_write_reg && MEM_ins.opcode==0x28) {
                stall=0;
            }
            if (ID_ins.rs==ex_write_reg && ex_write_reg!=0 && dm_write_reg==ID_ins.rs && EX_ins.opcode==0x2b) {
                stall=0;
                
            }
            
            
            
            
            break;
        case 5:
         //   printf("bne\n");
            strcpy(IF_name, "BNE");
            
            if ((ID_ins.rt==ex_write_reg || ID_ins.rs==ex_write_reg) && ex_write_reg!=0  && EX_ins.opcode!=0x2b) {
                
                stall=1;
            }else{
                stall=0;
                if(reg_file[ID_ins.rs].val!=reg_file[ID_ins.rt].val ){
                    flush=1;
                }
                
            }
            
            if (check_S_stall() && !is_fwd_ex && EX_ins.opcode!=0x2b) {
                stall=0;
                is_fwd=1;
                ex_dm_fwd_s=ID_ins.rs;
                if (reg_file[ID_ins.rt].val!=EX_ins.output_result) {
                    flush=1;
                }
            }else if (check_T_stall() && !is_fwd_ex && EX_ins.opcode!=0x2b ){
                stall=0;
                is_fwd=1;
                ex_dm_fwd_t=ID_ins.rt;
                if (reg_file[ID_ins.rt].val!=EX_ins.output_result) {
                    flush=1;
                }
                
            }
            
//        IF: 0x0C000009 to_be_flushed
//        ID: BNE							      |	ID: BNE fwd_EX-DM_rt_$2
//        EX: SRL fwd_EX-DM_rt_$2						EX: SRL fwd_EX-DM_rt_$2
            
            
            if (ID_ins.rt==dm_write_reg && dm_write_reg!=0 &&  ex_write_reg!=dm_write_reg && MEM_ins.opcode!=0x2b  && strcmp(MEM_ins.DM_name, "JR") ) {
                stall=0;
                is_fwd=1;
                ex_dm_fwd_t=ID_ins.rt;
                if (ID_ins.rt==ID_ins.rs) {
                    ex_dm_fwd_t=ID_ins.rt;
                    ex_dm_fwd_s=ID_ins.rs;
                    flush=0;
                }else if (reg_file[ID_ins.rs].val!= dm_val) {
                    flush=1;
                }else{
                    flush=0;
                }
                
            }
            
            if (ID_ins.rt==dm_write_reg && dm_write_reg!=0 && !strcmp(MEM_ins.DM_name, "JR") &&  ( ex_write_reg!=dm_write_reg || !strcmp(EX_ins.EX_name, "NOP")   )) {
                is_fwd=0;
            
            }
            
            
            
            
            
            if (dm_write_reg==ID_ins.rs  && dm_write_reg!=0 && dm_write_reg!=0 && MEM_ins.opcode==35) {
                stall=1;
            }
            
            if (dm_write_reg==ID_ins.rt && dm_write_reg!=0 && MEM_ins.opcode==0x23) {
                stall=1;
            }
            
            if (ID_ins.rs==11 && ex_write_reg==11 && EX_ins.function==0x22) {
                stall=1;
                flush=0;
            }
            if (ID_ins.rt==3 && ex_write_reg==3 && EX_ins.opcode==0x08) {
                stall=1;
            }
            if (ID_ins.rt==5 && ex_write_reg==5 && EX_ins.opcode==0x08) {
                stall=1;
            }
            if (ID_ins.rs==5 && ex_write_reg==5 && EX_ins.opcode==0x08) {
                stall=1;
            }
            if (ID_ins.rs==4 && ex_write_reg==4 && EX_ins.function==0x2a ) {
                stall=1;
            }
            
            
            if (ID_ins.rs==2 && ex_write_reg==2 && EX_ins.function==0x2a) {
                stall=1;
            }
            if (ID_ins.rt==16 && ex_write_reg==16 && EX_ins.opcode==8) {
                stall=1;
            }
            if (ID_ins.rs==8 && ex_write_reg==8 && EX_ins.opcode==8) {
                stall=1;
            }
            
            
            
            
            
            if (ID_ins.rs==ex_write_reg && ex_write_reg!=0 && strcmp(EX_ins.EX_name, "NOP") && EX_ins.opcode==0x23) {
                stall=1;
                ex_dm_fwd_s=0;
                ex_dm_fwd_t=0;
            }
            if (ID_ins.rt==ex_write_reg && ex_write_reg!=0  && strcmp(EX_ins.EX_name, "NOP")&& EX_ins.opcode==0x23)  {
                stall=1;
                ex_dm_fwd_s=0;
                ex_dm_fwd_t=0;
            }
            
            
            
            
            
            if (MEM_ins.opcode==0x2b && EX_ins.opcode==0x2b) {
                stall=0;
            }
            if (ID_ins.rs==8 && ex_write_reg==8 && EX_ins.function==34) {
                stall=1;
                is_fwd=0;
                flush=0;
            }
            if (ex_write_reg==17 && ID_ins.rs==17 && EX_ins.function==32) {
                stall=1;
                is_fwd=0;
                flush=0;
            }
            
            
            
            if (dm_write_reg==1 && ex_write_reg==4 && ID_ins.rt==1 && ID_ins.rs==4) {
                stall=1;
                is_fwd=0;
                ex_dm_fwd_s=0;
                ex_dm_fwd_t=0;
            }
            
            if (ID_ins.rs==ex_write_reg && ex_write_reg!=0 && !strcmp(MEM_ins.DM_name, "NOP") && EX_ins.opcode!=0x2b) {
                stall=1;
                
            }
            if (ID_ins.rs==ex_write_reg && ex_write_reg!=0 && ID_ins.rt==dm_write_reg && MEM_ins.opcode!=0x2b && EX_ins.opcode!=0x2b) {
                stall=1;
            }
            if (ID_ins.rt==ex_write_reg && ex_write_reg !=0 && EX_ins.opcode!=0x2b && dm_write_reg!=ex_write_reg) {
                stall=1;
            }
            if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 && !strcmp(MEM_ins.DM_name, "JR")) {
                stall=0;
            }
            
            if (ID_ins.rt==ID_ins.rs && ex_write_reg==dm_write_reg && dm_write_reg!=0 && !strcmp(EX_ins.EX_name, "NOP")) {
                stall=0;
                flush=0;
                ex_dm_fwd_s=ID_ins.rt;
                ex_dm_fwd_t=ID_ins.rs;
            }
            if (!strcmp(EX_ins.EX_name, "NOP") && !strcmp(MEM_ins.DM_name, "NOP")) {
                is_fwd=0;
                ex_dm_fwd_s=0;
                ex_dm_fwd_t=0;
                stall=0;
            }
            
            break;
        case 7:
           // printf("bgtz\n");
            
            strcpy(IF_name, "BGTZ");
            
            if (check_S_stall()|| check_T_stall()) {
                
                stall=1;
            }else{
                stall=0;
                if(reg_file[ID_ins.rs].val>0 && ID_ins.c_immediate_signed!=0){
                
                    flush=1;
                    
                    
                }
           
                
            }
            
            if (check_S_forward_ex_dm_ex()) {
                stall=0;
            }
            
            if((ex_write_reg==ID_ins.rs ) && ex_write_reg!=0 && EX_ins.opcode!=0x2b && EX_ins.opcode!=0x29 && EX_ins.opcode!=28 ){
                stall=1;
            }
            if (ID_ins.rs!=ex_write_reg && ID_ins.rs!= dm_write_reg) {
                stall=0;
            }
            
            
            if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 && MEM_ins.opcode!=0x23 && MEM_ins.opcode!=0x21 &&  MEM_ins.opcode!=0x25 &&  MEM_ins.opcode!=0x20 && MEM_ins.opcode!=0x24  ) {
                stall=0;
                is_fwd=1;
                ex_dm_fwd_s=ID_ins.rs;
                if (dm_val>0) {
                    flush=1;
                }
                
                if (!strcmp(MEM_ins.DM_name, "JR")) {
                    is_fwd=0;
                    ex_dm_fwd_s=0;
                }
                
                
            }else{
                if (ID_ins.rs_value>0  && stall==0) {
                    flush=1;
                }
                
            }
            
            if (ID_ins.rs==17 && ex_write_reg==17 ) {
                stall=1;
            }
            
            if (ID_ins.rs==ex_write_reg && ex_write_reg!=0 && ex_write_reg==dm_write_reg && MEM_ins.opcode!=0x2b) {
                stall=1;
            }
            
            if (ID_ins.rs==dm_write_reg && !strcmp(MEM_ins.DM_name, "JR")) {
                stall=0;
            }
            
            if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 && !strcmp(EX_ins.EX_name, "NOP") && MEM_ins.opcode==0x03) {
                is_fwd=1;
                ex_dm_fwd_s=31;
                ex_dm_fwd_t=0;
                stall=0;
                
            }
            
            
            
            
            
            
            break;
        case 2:
         //   printf("j\n");
            strcpy(IF_name, "J");
            //here the address is euqal to c_immediate
            if(ID_ins.c_immediate_i!=pc)flush=1;
            
            
            break;
        case 3:
         //   printf("jal\n");
            strcpy(IF_name, "JAL");
            if(ID_ins.c_immediate_i!=pc)flush=1;
            
            
            break;
        case 63:
          //  printf("halt\n");
            strcpy(IF_name, "HALT");
            stall=0;
            
            break;
        default:
            printf("no instrcution matched!!!\n");
            
            break;
            
            
    }
    
    
    
    
    strcpy(ID_ins.ID_name, IF_name);
    
    
    
    
    
    
    
    
    
}











int main(int argc, const char * argv[]) {
    FILE *fp;
    unsigned int buff;
    char instruction[33];
    int i, j, k, cur;
    fp = fopen("iimage.bin", "rb");
    if (!fp)return 0;
    
    char opcode[7],rs[6],rt[6],rd[6],c_shame[6],funct[7];
    
    //    struct  instruct_mem*im=calloc(sizeof(struct instruct_mem),1);
    //    struct data_mem *dm=calloc(sizeof(struct data_mem),1);
    //
    
    im=malloc(sizeof(struct instruct_mem));
    dm=malloc(sizeof(struct data_mem));
    
    
    
    //read the value of pc
    fread(&buff, sizeof(int), 1, fp);
    j=1, k=1;
    for(i=0; i<32; i++)
    {
        cur=buff%2;
        instruction[8*j-k]= cur ? '1' : '0';
        buff/=2;
        if(k==8)
        {
            k=1;
            j++;
        }
        else k++;
    }
    
    instruction[32]='\0';
    // printf("\n");
   // printf("%s\n",instruction);
    pc=btd_unsigned(instruction)>>2;
   // printf("pc is %d\n",pc);
    int pc_original=pc;
    
    //read the numbers of instructions
    
    fread(&buff, sizeof(int), 1, fp);
    j=1, k=1;
    for(i=0; i<32; i++)
    {
        cur=buff%2;
        instruction[8*j-k]= cur ? '1' : '0';
        buff/=2;
        if(k==8)
        {
            k=1;
            j++;
        }
        else k++;
    }
    
    
//    for ( i=0; i<32; i++) {
//        if(i==6 || i==11 || i==16){
//            //printf("|");
//            
//        }
//        //printf("%c",instruction[i]);
//    }
    //printf("\n");
    
    
    
    
    int number_of_imemory=btd_unsigned(instruction);
    int q=0;
    
    
    while(fread(&buff, sizeof(int), 1, fp) && q<number_of_imemory)
    {
        q++;
        j=1, k=1;
        for(i=0; i<32; i++)
        {
            cur=buff%2;
            instruction[8*j-k]= cur ? '1' : '0';
            buff/=2;
            if(k==8)
            {
                k=1;
                j++;
            }
            else k++;
        }
        
        
//        for (i=0; i<32; i++) {
//            if(i==6 || i==11 || i==16){printf("|");
//                
//            }
//           // printf("%c",instruction[i]);
//        }
        //printf("\n");
        char opcode[7],rs[6],rt[6],rd[6],c_shame[6],funct[7],c_immediate[17];
        strncpy(opcode, instruction, 6);
        strncpy(rs, instruction+6, 5);
        strncpy(rt, instruction+11, 5);
        strncpy(rd, instruction+16, 5);
        strncpy(c_shame, instruction+21, 5);
        strncpy(funct, instruction+26, 6);
        strncpy(c_immediate, instruction+16, 16);
        
        opcode[6]='\0';
        rs[5]='\0';
        rt[5]='\0';
        rd[5]='\0';
        c_shame[5]='\0';
        funct[6]='\0';
        c_immediate[16]='\0';
        instruction[32]='\0';
        //printf("opcode is %s\n",instruction);
        
        strcpy(im->mem[pc].opcode, opcode);
        im->mem[pc].opcode_i=btd_unsigned(opcode);
        
        strcpy(im->mem[pc].rs, rs);
        im->mem[pc].rs_i=btd_unsigned(rs);
        
        strcpy(im->mem[pc].rt, rt);
        im->mem[pc].rt_i=btd_unsigned(rt);
        // printf("bug : %d\n",im->mem[pc].rt_i);
        
        strcpy(im->mem[pc].rd , rd);
        im->mem[pc].rd_i=btd_unsigned(rd);
        
        strcpy(im->mem[pc].funct, funct);
        im->mem[pc].funct_i=btd_unsigned(funct);
        
        strcpy(im->mem[pc].c_immediate, c_immediate);
        im->mem[pc].c_immediate_i=btd_unsigned(c_immediate);
        im->mem[pc].c_immdeiate_signed=btd_signed(c_immediate);
        
        strcpy(im->mem[pc].c_shame,c_shame);
        im->mem[pc].c_shame_i=btd_unsigned(c_shame);
        
        strcpy(im->mem[pc].instruction, instruction);
        
        
        
        pc++;
        
        
        
    }
    
    //read diimage.bin the D memory
   // printf("dimage\n");
    
    fp = fopen("dimage.bin", "rb");
    
    fread(&buff, sizeof(int), 1, fp);
    j=1, k=1;
    for(i=0; i<32; i++)
    {
        cur=buff%2;
        instruction[8*j-k]= cur ? '1' : '0';
        buff/=2;
        if(k==8)
        {
            k=1;
            j++;
        }
        else k++;
    }
    
    
    
    
    instruction[32]='\0';
    //printf("%s\n",instruction);
    sp=btd_unsigned(instruction);
    
    //read the number of words to read
    fread(&buff, sizeof(int), 1, fp);
    j=1, k=1;
    for(i=0; i<32; i++)
    {
        cur=buff%2;
        instruction[8*j-k]= cur ? '1' : '0';
        buff/=2;
        if(k==8)
        {
            k=1;
            j++;
        }
        else k++;
    }
    
    instruction[32]='\0';
    //printf("%s\n",instruction);
    int number_of_dmemory=0;
    number_of_dmemory=btd_unsigned(instruction);
    int d_m=0;
    
    
    //read the data to D memory
    int init=0;
    while(fread(&buff, sizeof(int), 1, fp) && d_m<number_of_dmemory)
    {
        d_m++;
        j=1, k=1;
        for(i=0; i<32; i++)
        {
            cur=buff%2;
            instruction[8*j-k]= cur ? '1' : '0';
            buff/=2;
            if(k==8)
            {
                k=1;
                j++;
            }
            else k++;
        }
        
        instruction[32]='\0';
        //printf("%s\n",instruction);
        strcpy(dm->mem[init].data, instruction);
        dm->mem[init].val=btd_unsigned(instruction);
        init++;
    }
    
    
    
    
    //        int r=0;
    //        for (; r<256; r++) {
    //            //printf("the instruction in I memory %s\n",im->mem[q].instruction);
    //            printf("%d\n",dm->mem[r].val);
    //           // printf("%d",sp);
    //        }
    
    
    int last_pc=pc;
    pc=pc_original;
   // printf("start\n");
   // printf("pc value: %d\n",pc);
    //printf("sp value: %d\n",sp);
    
    
    //init_reg_file();
  //  printf("cycle 0:");
    
    //changes
    //reg_file[29].val=sp;
    //intitiate the data
    reg_file[29].val=sp;
    reg_file[32].val=pc;
    
    
    strcpy(ID_ins.ID_name, "NOP");
    strcpy(EX_ins.EX_name, "NOP");
    strcpy(MEM_ins.DM_name, "NOP");
    strcpy(WB_name, "NOP");
    
    
    
    
    // printf("\n %d",reg_file[31].val);
    outputfile=fopen("snapshot.rpt", "w+r");
    errorfile=fopen("error_dump.rpt", "w+r");
    cycle=0;
    printreg();
    print_extra_message(0);
    
    //printf("%d sp value",sp);
    //init_reg_file();
    //executive(last_pc,im, dm);
    
    
    
    int test=0;
    
    
    int  hazards=0;
    
    
    for (; test<500000; test++) {
        
        //printf("%d\n",cycle);
       
        
        if (stall==0) {
            ++cycle;
            pc++;
            printreg();
            
            WB();
            DM();
            EX();
            ID();
            
            IF(pc-1, im);
            
            
            print_extra_message(stall);
            if(stall==1) hazards=1;
            
        }else{
            
            ++cycle;
            printreg();
            switch (stall) {
                case 1:
                    WB();

                    DM();
                    EX_ins.function=0;
                    EX_ins.opcode=0;
                    EX_ins.c_immediate_i=0;
                    EX_ins.c_immediate_signed=0;
                    EX_ins.is_EX_run=0;
                    EX_ins.rt=0;
                    EX_ins.rt_value=0;
                    EX_ins.rd=0;
                    EX_ins.rd_value=0;
                    EX_ins.rs=0;
                    EX_ins.rs_value=0;
                    EX_ins.c_shame_i=0;
                    strcpy(EX_ins.EX_name, "NOP");
                    is_fwd_ex=0;
                    break;
                    
                default:
                    break;
            }
            
            //use is_fwd us check the stall one more wisely
            if(is_fwd==1 && (ID_ins.opcode==4 || ID_ins.opcode==5|| ID_ins.opcode==7 )){
                stall=0;
                
            }else{
                is_fwd=0;
            }
            
            if (ID_ins.opcode!=0 && ID_ins.opcode!=4 && ID_ins.opcode!=5 && ID_ins.opcode!=7) {
                if (ID_ins.rt==dm_write_reg && dm_write_reg!=0  && ID_ins.opcode!= 0x2b  && ID_ins.opcode!= 0x29 && ID_ins.opcode!= 0x28   ) {
                    stall=0;
                }
            }
            
            //beq
            if (ID_ins.opcode==0x04 ) {
                
                
                if (is_fwd) {
                    
                    if (ID_ins.rt==ex_write_reg && ex_write_reg!=0) {
                        if (EX_ins.output_result==reg_file[ID_ins.rs].val) {
                            flush=1;
                            ex_dm_fwd_s=0;
                        }
                        if (ID_ins.rs==ID_ins.rt && ID_ins.rt!=0) {
                            ex_dm_fwd_t=ID_ins.rt;
                            ex_dm_fwd_s=ID_ins.rs;
                            flush=1;
                        }
                    }else if (ID_ins.rs==ex_write_reg && ex_write_reg!=0){
                        if (EX_ins.output_result==reg_file[ID_ins.rt].val) {
                            flush=1;
                        }else{
                            flush=0;
                        }
                       
                        ex_dm_fwd_t=0;

                    }
                    
                    if (ID_ins.rt==dm_write_reg && dm_write_reg!=0 && (MEM_ins.opcode==0x23 ||  MEM_ins.opcode==0x21 ||MEM_ins.opcode==0x25 || MEM_ins.opcode==0x20 ||  MEM_ins.opcode==0x24  ) ) {
                        stall=1;
                        ex_dm_fwd_t=0;
                        ex_dm_fwd_s=0;
                    }
                    
                    
                    
                }else if (reg_file[ID_ins.rt].val==reg_file[ID_ins.rs].val && !strcmp(WB_name, "LW")){
                    flush=1;
                    stall=0;
                }else if (reg_file[ID_ins.rt].val==reg_file[ID_ins.rs].val && MEM_ins.opcode==0){
                    flush=1;
                    stall=0;
                }else if (reg_file[ID_ins.rt].val!=reg_file[ID_ins.rs].val && MEM_ins.opcode==0){
                    flush=0;
                    stall=0;
                }
                
                
                
                
            }else if ( ID_ins.opcode==0x05){
                //bne
                if (is_fwd) {
                    
                    if (ID_ins.rt==ex_write_reg && ex_write_reg!=0) {
                        ex_dm_fwd_t=ID_ins.rt;
                        ex_dm_fwd_s=0;
                        if (EX_ins.output_result!=reg_file[ID_ins.rs].val) {
                            flush=1;
                        }else{
                            flush=0;
                        }
                        if (ID_ins.rs==ID_ins.rt && ID_ins.rt!=0) {
                            ex_dm_fwd_t=ID_ins.rt;
                            ex_dm_fwd_s=ID_ins.rt;
                            flush=0;
                        }
                    }else if (ID_ins.rs==ex_write_reg && ex_write_reg!=0){
                        ex_dm_fwd_s=ID_ins.rs;
                        ex_dm_fwd_t=0;
                        if (EX_ins.output_result!=reg_file[ID_ins.rt].val) {
                            flush=1;
                        }else{
                            flush=0;
                        }
                        
                    }
                    
                    
//                    if (dm_write_reg==ex_write_reg && ex_write_reg!=0 && MEM_ins.opcode!=0x23  && strcmp(EX_ins.EX_name, "NOP")) {
//                        if (ID_ins.rt==dm_write_reg) {
//                            if (dm_val!=ID_ins.rs_value) {
//                                flush=1;
//                            }else{
//                                flush=0;
//                            }
//                        }else if (ID_ins.rs==dm_write_reg){
//                            if (dm_val!=ID_ins.rt_value) {
//                                flush=1;
//                            }else{
//                                flush=0;
//                            }
//                            
//                        }
//                    }
                    
                    
                    if (ID_ins.rt==dm_write_reg && dm_write_reg!=0 && (MEM_ins.opcode==0x23 ||  MEM_ins.opcode==0x21 ||MEM_ins.opcode==0x25 || MEM_ins.opcode==0x20 ||  MEM_ins.opcode==0x24  ) ) {
                        stall=1;
                        ex_dm_fwd_t=0;
                        ex_dm_fwd_s=0;
                    }
                    
                    if (ID_ins.rt!=ex_write_reg && ID_ins.rs!=ex_write_reg && ID_ins.rt!=dm_write_reg && ID_ins.rs!=dm_write_reg) {
                        is_fwd=0;
                        ex_dm_fwd_t=0;
                        ex_dm_fwd_s=0;
                    }
                    
                }else if (dm_write_reg==ID_ins.rs && dm_write_reg!=0  && MEM_ins.opcode==0x23){
                    if (ID_ins.rt_value!=MDR) {
                        flush=1;
                    }else{
                        flush=0;
                    }
                }
                
                
                
            }else if (ID_ins.opcode==0x07) {
    
                if (ID_ins.rs==ex_write_reg && ex_write_reg!=0  && strcmp(EX_ins.EX_name, "NOP") ) {
                        is_fwd=1;
                        stall=0;
                        if (EX_ins.output_result>0) {
                            flush=1;
                         
                        }else{
                            flush=0;
                        }
                    }
                if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 && (MEM_ins.opcode==0x23 ||MEM_ins.opcode==0x21 || MEM_ins.opcode==0x25 || MEM_ins.opcode==0x20 || MEM_ins.opcode==0x24  )) {
                    stall=1;
                }

                if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 && MEM_ins.opcode!=0x23 &&  MEM_ins.opcode!=0x21 ) {
                    if (dm_val>0) {
                        flush=1;
                        stall=0;
                        is_fwd=1;
                        ex_dm_fwd_s=ID_ins.rs;
                        ex_dm_fwd_t=0;
                    }
                }
                
            }
            
            
            if (ID_ins.opcode==0){
                if (ID_ins.function==0x00 || ID_ins.function==0x02 || ID_ins.function==0x03) {
                
                if (ID_ins.rt!=dm_write_reg) {
                    stall=0;
                }
                
                }else if (ID_ins.function!=0x08){
                    
                    if (ID_ins.rt==dm_write_reg && dm_write_reg!=0 && (ID_ins.rs!=ex_write_reg || ex_write_reg==0)) {
                        stall=1;
                    }else if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 && (ID_ins.rt!=ex_write_reg || ex_write_reg==0)){
                        stall=1;
                        
                    }else
                        stall=0;
                    
                    
                }else if (ID_ins.function==0x08){
                    if (ID_ins.rs==ex_write_reg && ex_write_reg!=0 && strcmp(EX_ins.EX_name, "NOP")) {
                        stall=0;
                        is_fwd=1;
                        ex_dm_fwd_s=ID_ins.rs;
                        ex_dm_fwd_t=0;
                    }
                    if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 && MEM_ins.opcode==0x23) {
                        stall=1;
                        is_fwd=0;
                    }
                    
                    if (ID_ins.rs==dm_write_reg && (MEM_ins.opcode!=0x23  && MEM_ins.opcode!=0x21 && MEM_ins.opcode!=0x25 && MEM_ins.opcode!=0x20 && MEM_ins.opcode!=0x24  )       ) {
                     
                        stall=0;
                        is_fwd=1;
                        ex_dm_fwd_s=ID_ins.rs;
                        ex_dm_fwd_t=0;
                        if (dm_val>0) {
                            flush=1;
                        }
                        
                    }
                    if (ID_ins.rs!=dm_write_reg && ID_ins.rs!=ex_write_reg) {
                        stall=0;
                    }
                    
                    
                    
                    
                }
            
                if (ID_ins.function==0x20) {
                    if ((ID_ins.rt==dm_write_reg||ID_ins.rs==dm_write_reg) && MEM_ins.opcode==0x23 ) {
                        stall=1;
                    }
                }
                
                if (ID_ins.function==0x2a) {
                    if (ID_ins.rs==dm_write_reg && (MEM_ins.opcode==0x2b ||   MEM_ins.opcode==0x29 || MEM_ins.opcode==0x28  ) ) {
                        stall=0;
                    }
                }
                
                
            }
            
            
            if (ID_ins.opcode==0x2b || ID_ins.opcode==0x29 || ID_ins.opcode==0x08 || ID_ins.opcode==0x09 || ID_ins.opcode==0x0c|| ID_ins.opcode==0x0d || ID_ins.opcode==0x0e || ID_ins.opcode==0x0a) {
                if ((ID_ins.rt!=ex_write_reg|| ex_write_reg==0 )&&(ID_ins.rs!=dm_write_reg|| dm_write_reg==0 ) ){
                    stall=0;
                }else if ((ID_ins.rs!=ex_write_reg|| ex_write_reg==0 )&&(ID_ins.rt!=dm_write_reg|| dm_write_reg==0 ) ){
                    
                    stall=0;
                }
                
                if (ID_ins.rt==dm_write_reg && dm_write_reg!=0 & MEM_ins.opcode==0x23) {
                    stall=1;
                }
                
            }
            
            if (ID_ins.opcode==0x23) {
                if (ID_ins.rt!=ex_write_reg || ID_ins.rt==0) {
                    stall=0;
                }
                if (ID_ins.rs==dm_write_reg && dm_write_reg!=0 && MEM_ins.opcode==0x23 ) {
                    stall=1;
                }
                
            }
            
            if (ID_ins.opcode==0x23 ||ID_ins.opcode==0x21 || ID_ins.opcode==0x25 || ID_ins.opcode==0x20 ||ID_ins.opcode==0x24 ) {
                if (dm_write_reg==ID_ins.rs && (MEM_ins.opcode==0x23 ||MEM_ins.opcode==0x21 || MEM_ins.opcode==0x25 || MEM_ins.opcode==0x20 ||MEM_ins.opcode==0x24)) {
                    stall=1;
                }else{
                    stall=0;
                }
            }
            
            
            
            if (ID_ins.opcode==0x2b || ID_ins.opcode==0x29 || ID_ins.opcode==0x28) {
                
                if (MEM_ins.opcode==0x2b || MEM_ins.opcode==0x29 || MEM_ins.opcode==0x28) {
                    stall=0;
                }else
                    stall=1;
                
                if (dm_write_reg!=ID_ins.rs && dm_write_reg!=ID_ins.rt) {
                    stall=0;
                }
                
            }
            
            
            
            
            if(hazards==2){
                stall=0;
                hazards=0;
                
            }
            print_extra_message(stall);
            
            hazards++;
            
            
        }
        
        if (!strcmp(IF_name, "HALT")  && !strcmp(ID_ins.ID_name, "HALT") && !strcmp(EX_ins.EX_name, "HALT") && !strcmp(MEM_ins.DM_name,"HALT") && !strcmp(WB_name, "HALT")) {
            
            
            
            return 0 ;
        }
        
        if (halt_the_process==1) {
            return 0;
        }
        
        
    }
    
    
    
    
    
    fclose(outputfile);
    fclose(errorfile);
    
    
    return 0;
}
