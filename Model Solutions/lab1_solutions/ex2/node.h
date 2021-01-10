/*************************************
* Lab 1 Exercise 2
* Name:
* Student No:
* Lab Group:
*************************************/

// feel free to modify this file and add more functions as you deem fit

typedef struct NODE
{
	struct NODE *prev;
	struct NODE *next;
	int data;
} node;

typedef struct
{
	node *head;
	node *tail;
} list;

void insert_node_from_head_at(list *lst, int index, int data);
void insert_node_from_tail_at(list *lst, int index, int data);
void delete_node_from_head_at(list *lst, int index);
void delete_node_from_tail_at(list *lst, int index);
void reset_list(list *lst);
void map(list *lst, int (*func)(int));
long sum_list(list *list);
