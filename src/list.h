/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#ifndef __RS_LIST_H_INCLUDED__
#define __RS_LIST_H_INCLUDED__

/**
 * \defgroup list List Manipulation
 *
 * A generic singly-linked list type.
 *
 * This is a data type used internally by libredstone, but it is
 * exposed here as a convenience to C developers who do not (by
 * default) have such a data type.
 *
 * List cells are meant to be used directly, as well as with the
 * functions outlined here; iteration is usually done by accessing the
 * data and next members directly.
 *
 * Lists of zero length are everywhere represented by the NULL pointer.
 *
 * @{
 */

/**
 * A generic function used in rs_list_foreach().
 */
typedef void (*RSListFunction)(void* data);

/**
 * A generic singly-linked list type.
 *
 * This structure represents a single cell of a singly-linked
 * list. Lists of zero length are represented everywhere by the NULL
 * pointer.
 */
typedef struct _RSList RSList;
struct _RSList
{
    /** the list item stored in this cell */
	void* data;
    /** the next cell in this list, or NULL if there is no more */
	RSList* next;
};

/**
 * Create a new list cell.
 *
 * Usually, this function is only used internally. If you want to add
 * an item to this list, use rs_list_push().
 * 
 * \return the new cell
 * \sa rs_list_push
 */
RSList* rs_list_cell_new(void);

/**
 * Get the length of a list.
 *
 * This function must iterate through the entire list to calculate
 * it's length, so you should avoid using this if possible on large
 * lists. If you simply want to iterate through a list, it is better
 * to use the data and next elements of each cell directly.
 *
 * \param first the first cell in a list
 * \return the length of the list
 */
unsigned int rs_list_size(RSList* first);

/**
 * Get the nth piece of data stored in a list.
 *
 * Due to the nature of singly-linked lists, this function is
 * expensive for large lists. If you simply want to iterate through a
 * list, it is better to use the data and next elements of each cell
 * directly.
 *
 * If the given index is larger than the length of the list, NULL is
 * returned and warning is printed. If you wish to descriminate
 * between a NULL data and an out-of-bound index, use
 * rs_list_nth_cell().
 *
 * \param first the first cell in a list
 * \param i the (zero-based) index of the item to retrieve
 * \return the piece of data, or NULL
 * \sa rs_list_nth_cell
 */
void* rs_list_nth(RSList* first, unsigned int i);

/**
 * Get the nth cell of a list.
 *
 * Due to the nature of singly-linked lists, this function is
 * expensive for large lists. If you simply want to iterate through a
 * list, it is better to use the data and next elements of each cell
 * directly.
 *
 * If the given index is larger than the length of the list, NULL is
 * returned.
 *
 * \param first the first cell in a list
 * \param i the (zero-based) index of the item to retrieve
 * \return the cell, or NULL
 * \sa rs_list_nth, rs_list_remove
 */
RSList* rs_list_nth_cell(RSList* first, unsigned int i);

/**
 * Find the cell containing a given piece of data.
 *
 * This function iterates through the list, looking for the given
 * piece of data. On success, the cell containing it is
 * returned. Otherwise, it returns NULL.
 *
 * The returned cell can be used as a truth value for a membership
 * test, or as an argument to rs_list_remove().
 *
 * \param first the first cell in a list
 * \param data the data to search for
 * \return the cell contaning that data, or NULL
 * \sa rs_list_remove
 */
RSList* rs_list_find(RSList* first, void* data);

/**
 * Remove the given cell from a list.
 *
 * Make sure to set whatever variable you use for first to the return
 * value, as the first cell of a list may change if you remove it.
 *
 * \param first the first cell in a list
 * \param cell the cell to remove
 * \return the new first cell of the list
 * \sa rs_list_find, rs_list_nth_cell
 */
RSList* rs_list_remove(RSList* first, RSList* cell);

/**
 * Push a piece of data into a list.
 *
 * This function adds a cell containing the given data to the
 * beginning of a list. If you want to add elements to the end, it is
 * recommended to push them all on in reverse order, then use
 * rs_list_reverse().
 *
 * Make sure to use the return value, as this function is guaranteed
 * to change the first cell.
 *
 * \param first the first cell in a list
 * \param data the piece of data to add to the list
 * \return the new first cell of the list
 * \sa rs_list_pop, rs_list_reverse
 */
RSList* rs_list_push(RSList* first, void* data);

/**
 * Remove an element from a list.
 *
 * This function removes the first cell of a list, and returns the
 * second (the new first cell).
 *
 * /param first the first cell in a list
 * /return the new first cell in the list
 * /sa rs_list_push
 */
RSList* rs_list_pop(RSList* first);

/**
 * Reverse a list in-place.
 *
 * This function efficiently reverses a list of elements, and returns the new first element.
 *
 * \param first the first cell in a list
 * \return the new first cell in the list
 */
RSList* rs_list_reverse(RSList* first);

/**
 * Perform a function on each element in a list.
 *
 * This function is a convenience for situations where you want to run
 * one function on all elements of a list, for example, if you want to
 * rs_free() some list of structures you previously allocated.
 *
 * \param first the first cell in a list
 * \param func the function to use
 */
void rs_list_foreach(RSList* first, RSListFunction func);

/**
 * Free an entire list.
 *
 * This function will free the memory allocated for all the cells in
 * the given list. It will not free the data contained in these cells;
 * if you wish to do that, use rs_list_foreach() in combination with
 * rs_free() (or some other deallocator) before calling this function.
 *
 * \param first the first cell in a list
 * \sa rs_list_foreach()
 */
void rs_list_free(RSList* first);

/** @} */

#endif /* __RS_LIST_H_INCLUDED__ */
