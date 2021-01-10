/*************************************
* Lab 1 Exercise 2
* Name:
* Student No:
* Lab Group:
*************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "node.h"

// copy in your implementation of the functions from ex1
// there is on extra function called map which you have to fill up too
// feel free to add any new functions as you deem fit

// gets node at index by following head
node *get_node(node *head, int index)
{
    int curr = 0;
    node *temp = head;
    while (curr != index)
    {
        curr++;
        temp = temp->next;
    }
    return temp;
}

// creates a new node with data
node *new_node(int data)
{
    node *nx = (node *)malloc(sizeof(node));
    nx->data = data;
    nx->prev = NULL;
    nx->next = NULL;
    return nx;
}

// returns true if list is empty
int is_empty_list(list *lst)
{
    return lst->head == NULL;
}

//frees current node and updates pointers
void remove_node(node *curr)
{
    if (curr->prev)
    {
        curr->prev->next = curr->next;
    }

    if (curr->next)
    {
        curr->next->prev = curr->prev;
    }

    free(curr);
}

// inserts a new node with data value at index (counting from the front
// starting at 0)
void insert_node_from_head_at(list *lst, int index, int data)
{
    node *nx = new_node(data);
    if (is_empty_list(lst))
    {
        lst->head = nx;
        lst->tail = nx;
        return;
    }

    //node is new head
    if (!index)
    {
        nx->next = lst->head;
        nx->next->prev = nx;
        lst->head = nx;
        return;
    }

    //find node before index
    int i = 0;
    node *curr = lst->head;
    while (i != index - 1)
    {
        curr = curr->next;
        i++;
    }

    //update pointers accordingly
    nx->prev = curr;
    if (curr->next)
    {
        nx->next = curr->next;
        nx->next->prev = nx;
    }
    else
    {
        lst->tail = nx;
    }
    curr->next = nx;
}

// inserts a new node with data value at index (counting from the back
// starting at 0)
void insert_node_from_tail_at(list *lst, int index, int data)
{
    node *nx = new_node(data);
    if (is_empty_list(lst))
    {
        lst->head = nx;
        lst->tail = nx;
        return;
    }

    //node is new tail
    if (!index)
    {
        nx->prev = lst->tail;
        nx->prev->next = nx;
        lst->tail = nx;
        return;
    }

    //find node before index
    int i = 0;
    node *curr = lst->tail;
    while (i != index - 1)
    {
        curr = curr->prev;
        i++;
    }

    //update pointers accordingly
    nx->next = curr;
    if (curr->prev)
    {
        nx->prev = curr->prev;
        curr->prev->next = nx;
    }
    else
    {
        lst->head = nx;
    }
    curr->prev = nx;
}

// deletes node at index counting from the front (starting from 0)
// note: index is guarenteed to be valid
void delete_node_from_head_at(list *lst, int index)
{
    //find node at index
    int i = 0;
    node *curr = lst->head;
    while (i != index)
    {
        curr = curr->next;
        i++;
    }

    if (!curr->next)
    {
        lst->tail = curr->prev;
    }

    if (!curr->prev)
    {
        lst->head = curr->next;
    }

    remove_node(curr);
}

// deletes node at index counting from the back (starting from 0)
// note: index is guarenteed to be valid
void delete_node_from_tail_at(list *lst, int index)
{
    //find node at index
    int i = 0;
    node *curr = lst->tail;
    while (i != index)
    {
        curr = curr->prev;
        i++;
    }

    if (!curr->next)
    {
        lst->tail = curr->prev;
    }

    if (!curr->prev)
    {
        lst->head = curr->next;
    }

    remove_node(curr);
}

// resets list to an empty state (no nodes) and frees any allocated memory in
// the process
void reset_list(list *lst)
{
    node *curr = lst->head;
    while (curr != NULL)
    {
        node *temp = curr;
        curr = temp->next;
        free(temp);
    }

    lst->head = NULL;
    lst->tail = NULL;
}

// traverses list and applies func on data values of all elements in the list
void map(list *lst, int (*func)(int)) 
{
    node* curr = lst->head;
    while (curr != NULL) {
        curr->data = func(curr->data);
        curr = curr->next;
    }
}

// traverses list and returns the sum of the data values of every node 
// in the list
long sum_list(list *lst) 
{
    node* curr = lst->head;
    long res = 0;
    while (curr != NULL) {
        res += curr->data;
        curr = curr->next;
    }
    return res;
}
