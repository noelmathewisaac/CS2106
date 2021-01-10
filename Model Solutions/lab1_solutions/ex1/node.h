/*
	You can modify this file for your testing purpose 
	but note that we will use our own copies of this file for testing during grading so any changes in this file will be overwritten
*/

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
