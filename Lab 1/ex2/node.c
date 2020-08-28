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

// inserts a new node with data value at index (counting from the front
// starting at 0)
void insert_at_beginning(list *lst, int data)
{
    node *new_node =
        (node *)malloc(sizeof(node));
    new_node->data = data;
    new_node->next = lst->head;
    new_node->prev = NULL;
    lst->head->prev = new_node;
    lst->head = new_node;
}

void insert_at_end(list *lst, int data)
{
    node *new_node =
        (node *)malloc(sizeof(node));
    new_node->data = data;
    new_node->next = NULL;
    new_node->prev = lst->tail;
    lst->tail->next = new_node;
    lst->tail = new_node;
}
void insert_node_from_head_at(list *lst, int index, int data)
{
    node *new_node =
        (node *)malloc(sizeof(node));
    new_node->data = data;
    if (lst->head == NULL)
    {
        lst->head = new_node;
        lst->tail = new_node;
        new_node->next = NULL;
        new_node->prev = NULL;
    }
    else
    {
        node *head = lst->head;
        for (int i = 0; i < index - 1; i++)
        {
            if (head->next != NULL)
            {
                head = head->next;
            }
            else
            {
                break;
            }
        }

        if (head->prev == NULL && index == 0)
        {
            // printf("beg");
            insert_at_beginning(lst, data);
        }
        else if (head->next == NULL)
        {
            // printf("end");
            insert_at_end(lst, data);
        }
        else
        {
            // printf("middle");

            new_node->prev = head;
            new_node->next = head->next;
            head->next->prev = new_node;
            head->next = new_node;
        }
    }
}

// inserts a new node with data value at index (counting from the back
// starting at 0)
void insert_node_from_tail_at(list *lst, int index, int data)
{
    node *new_node =
        (node *)malloc(sizeof(node));
    new_node->data = data;
    if (lst->tail == NULL)
    {
        lst->tail = new_node;
        lst->head = new_node;
        new_node->next = NULL;
        new_node->prev = NULL;
    }
    else
    {
        node *tail = lst->tail;
        for (int i = 0; i < index - 1; i++)
        {
            if (tail->prev != NULL)
            {
                tail = tail->prev;
            }
            else
            {
                break;
            }
        }

        if (tail->next == NULL && index == 0)
        {
            // printf("end");
            insert_at_end(lst, data);
        }
        else if (tail->prev == NULL)
        {
            // printf("beg");
            insert_at_beginning(lst, data);
        }
        else
        {
            // printf("middle");

            new_node->next = tail;
            new_node->prev = tail->prev;
            tail->prev->next = new_node;
            tail->prev = new_node;
        }
    }
}

// deletes node at index counting from the front (starting from 0)
// note: index is guaranteed to be valid
void delete_node_from_head_at(list *lst, int index)
{
    node *head = lst->head;
    for (int i = 0; i < index; i++)
    {
        head = head->next;
    }

    if (head->next == NULL && head->prev == NULL)
    {
        reset_list(lst);
    }
    else if (head->prev == NULL)
    {
        lst->head = head->next;
        head->next->prev = NULL;
    }
    else if (head->next == NULL)
    {
        lst->tail = head->prev;
        head->prev->next = NULL;
    }
    else
    {
        head->next->prev = head->prev;
        head->prev->next = head->next;
    }
    free(head);
}

// deletes node at index counting from the back (starting from 0)
// note: index is guaranteed to be valid
void delete_node_from_tail_at(list *lst, int index)
{
    node *tail = lst->tail;
    for (int i = 0; i < index; i++)
    {
        tail = tail->prev;
    }

    if (tail->next == NULL && tail->prev == NULL)
    {
        reset_list(lst);
    }
    else if (tail->prev == NULL)
    {
        lst->head = tail->next;
        tail->next->prev = NULL;
    }
    else if (tail->next == NULL)
    {
        lst->tail = tail->prev;
        tail->prev->next = NULL;
    }
    else
    {
        tail->next->prev = tail->prev;
        tail->prev->next = tail->next;
    }
    free(tail);
}

// resets list to an empty state (no nodes) and frees any allocated memory in
// the process
void reset_list(list *lst)
{
    lst->head = NULL;
    lst->tail = NULL;
    free(lst);
}

// traverses list and applies func on data values of all elements in the list
void map(list *lst, int (*func)(int))
{
    node *head = lst->head;
    while (head != NULL)
    {
        head->data = func(head->data);
        head = head->next;
    }
}

// traverses list and returns the sum of the data values of every node
// in the list
long sum_list(list *lst)
{
    long sum = 0;
    node *head = lst->head;
    while (head != NULL)
    {
        sum = sum + head->data;
        head = head->next;
    }
    printf("%ld\n", sum);

    return sum;
}
