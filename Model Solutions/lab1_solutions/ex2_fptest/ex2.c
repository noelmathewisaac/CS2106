/*************************************
* Lab 1 Exercise 2
* Name:
* Student No:
* Lab Group:
*************************************/

#include <stdio.h>
#include <stdlib.h> 
#include "node.h"
#include "function_pointers.h"

// the runner is empty now! Modify it to fulfill the requirements of the 
// exercise 

// macros
#define SUM_LIST 0
#define INSERT_FROM_HEAD_AT 1
#define INSERT_FROM_TAIL_AT 2
#define DELETE_FROM_HEAD_AT 3
#define DELETE_FROM_TAIL_AT 4
#define RESET_LIST 5
#define MAP 6

void run_instruction(list *lst, int instr, FILE* fp);

// since functions are static, we just globally keep them
//int (*func_list[5]) (int x);

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Error: expecting 1 argument, %d found\n", argc - 1);
        exit(1);
    }

    update_functions();

    char *fname = argv[1]; 
    FILE* fp = fopen(fname, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: file %s not found\n", fname);
        exit(1);
    } 
    
    list *lst = (list *)malloc(sizeof(list));
    lst->head = NULL;
    lst->tail = NULL;

    int instr;
    while (fscanf(fp, "%d", &instr) == 1)
    {
        run_instruction(lst, instr, fp);
    }

    reset_list(lst);
    free(lst);
    fclose(fp);
}

// takes an instruction enum and runs the corresponding function
// we assume input always has the right format (no input validation on runner)
void run_instruction(list *lst, int instr, FILE* fp)
{
    int index, data;
    switch (instr)
    {
    case SUM_LIST:
        printf("%ld\n", sum_list(lst));
        break;
    case INSERT_FROM_HEAD_AT:
        fscanf(fp, "%d %d", &index, &data);
        insert_node_from_head_at(lst, index, data);
        break;
    case INSERT_FROM_TAIL_AT:
        fscanf(fp, "%d %d", &index, &data);
        insert_node_from_tail_at(lst, index, data);
        break;
    case DELETE_FROM_HEAD_AT:
        fscanf(fp, "%d", &index);
        delete_node_from_head_at(lst, index);
        break;
     case DELETE_FROM_TAIL_AT:
        fscanf(fp, "%d", &index);
        delete_node_from_tail_at(lst, index);
        break;   
    case RESET_LIST:
        reset_list(lst);
        break;
    case MAP:
        fscanf(fp, "%d", &index);
        map(lst, func_list[index]);
    }
}
