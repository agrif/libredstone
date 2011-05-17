/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#include "list.h"

#include "memory.h"
#include "error.h"

RSList* rs_list_cell_new(void)
{
    RSList* cell = rs_new0(RSList, 1);
    return cell;
}

unsigned int rs_list_size(RSList* first)
{
    unsigned int size = 0;
    while (first)
    {
        size++;
        first = first->next;
    }
    
    return size;
}

void* rs_list_nth(RSList* first, unsigned int i)
{
    RSList* cell = rs_list_nth_cell(first, i);
    if (!cell)
    {
        rs_critical("list index out of range: %i", i);
        return NULL;
    }
    return cell->data;
}

RSList* rs_list_nth_cell(RSList* first, unsigned int i)
{
    if (!first)
        return NULL;
    
    unsigned int current = 0;
    while (current < i && first->next)
        first = first->next;
    
    if (current == i)
        return first;
    
    return NULL;
}

RSList* rs_list_find(RSList* first, void* data)
{
    while (first)
    {
        if (first->data == data)
            return first;
        
        first = first->next;
    }
    
    return NULL;
}

RSList* rs_list_remove(RSList* first, RSList* cell)
{
    if (!first)
        return NULL;
    
    if (cell == first)
    {
        RSList* next = first->next;
        rs_free(first);
        return next;
    }
    
    RSList* last = first;
    RSList* current = first->next;
    while (current)
    {
        if (current == cell)
        {
            last->next = current->next;
            rs_free(current);
            return first;
        }
        
        last = current;
        current = current->next;
    }
    
    rs_critical("rs_list_remove called on cell not in list");
    return first;
}

RSList* rs_list_push(RSList* first, void* data)
{
    RSList* cell = rs_list_cell_new();
    cell->data = data;
    cell->next = first;
    return cell;
}

RSList* rs_list_pop(RSList* first)
{
    if (!first)
    {
        rs_critical("rs_list_pop called on empty list");
        return NULL;
    }
    
    RSList* next = first->next;
    rs_free(first);
    return next;
}

RSList* rs_list_reverse(RSList* first)
{
    RSList* last = NULL;
    RSList* cell = first;
    while (cell)
    {
        RSList* next = cell->next;
        cell->next = last;
        last = cell;
        cell = next;
    }
    
    return last;
}

void rs_list_foreach(RSList* first, RSListFunction func)
{
    while (first)
    {
        func(first->data);
        first = first->next;
    }
}

void rs_list_free(RSList* first)
{
    while (first)
        first = rs_list_pop(first);
}
