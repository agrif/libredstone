#ifndef __RS_LIST_H_INCLUDED__
#define __RS_LIST_H_INCLUDED__

/* singly-linked list */

typedef struct _RSList RSList;
typedef void (*RSListFunction)(void* data);

struct _RSList
{
	void* data;
	RSList* next;
};

RSList* rs_list_cell_new(void);

unsigned int rs_list_size(RSList* first);
void* rs_list_nth(RSList* first, unsigned int i);
RSList* rs_list_nth_cell(RSList* first, unsigned int i);

RSList* rs_list_find(RSList* first, void* data);
RSList* rs_list_remove(RSList* first, RSList* cell);

RSList* rs_list_push(RSList* first, void* data);
RSList* rs_list_pop(RSList* first);

void rs_list_foreach(RSList* first, RSListFunction func);
void rs_list_free(RSList* first);

#endif /* __RS_LIST_H_INCLUDED__ */
