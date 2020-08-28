/*
	You can modify this file for your testing purpose 
	but note that we will use our own copies of this file for testing during grading so any changes in this file will be overwritten
*/

// general purpose standard C lib
#include <stdio.h>
#include <stdlib.h> // stdlib includes malloc() and free()

// user-defined header files
#include "node.h"

// macros
#define PRINT_LIST 0
#define INSERT_FROM_HEAD_AT 1
#define INSERT_FROM_TAIL_AT 2
#define DELETE_FROM_HEAD_AT 3
#define DELETE_FROM_TAIL_AT 4
#define RESET_LIST 5

void run_instruction(list *lst, int instr);
void print_list(list *lst);

int main()
{
    list *lst = (list *)malloc(sizeof(list));
    lst->head = NULL;
    lst->tail = NULL;

    int instr;
    while (scanf("%d", &instr) == 1)
    {
        run_instruction(lst, instr);
    }

    reset_list(lst);
    free(lst);
}

// takes an instruction enum and runs the corresponding function
// we assume input always has the right format (no input validation on runner)
void run_instruction(list *lst, int instr)
{
    int index, data;
    switch (instr)
    {
    case PRINT_LIST:
        print_list(lst);
        break;
    case INSERT_FROM_HEAD_AT:
        scanf("%d %d", &index, &data);
        insert_node_from_head_at(lst, index, data);
        break;
    case INSERT_FROM_TAIL_AT:
        scanf("%d %d", &index, &data);
        insert_node_from_tail_at(lst, index, data);
        break;
    case DELETE_FROM_HEAD_AT:
        scanf("%d", &index);
        delete_node_from_head_at(lst, index);
        break;
     case DELETE_FROM_TAIL_AT:
        scanf("%d", &index);
        delete_node_from_tail_at(lst, index);
        break;   
    case RESET_LIST:
        reset_list(lst);
    }
}

// print out the whole list in a straight line
// prints both from left to right and right to left
void print_list(list *lst)
{
    printf("Forward: [ ");
    node *curr = lst->head;
    while (curr != NULL)
    {
        printf("%d ", curr->data);
        curr = curr->next;
    }

    printf("], Backwards: [ ");
    curr = lst->tail;
    while (curr != NULL)
    {
        printf("%d ", curr->data);
        curr = curr->prev;
    }
    printf("]\n");
}
