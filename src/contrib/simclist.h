
#ifndef SIMCLIST_H
#define SIMCLIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

/**
 * Type representing list hashes.
 *
 * This is a signed integer value.
 */
typedef long list_hash_t;

/**
 * a comparator of elements.
 *
 * A comparator of elements is a function that:
 * @ receives two references to elements a and b
 * @ returns {-1, 0, 1} if (a > b), (a == b), (a < b) respectively
 *
 * It is responsability of the function to handle possible NULL values.
 */
typedef int (*element_comparator)(const void *a, const void *b);

/**
 * an element lenght meter.
 *
 * An element meter is a function that:
 * @ receives the reference to an element el
 * @ returns its size in bytes
 *
 * It is responsability of the function to handle possible NULL values.
 */
typedef size_t (*element_meter)(const void *el);

/**
 * a function computing the hash of elements.
 *
 * An hash computing function is a function that:
 * @ receives the reference to an element el
 * @ returns a hash value for el
 *
 * It is responsability of the function to handle possible NULL values.
 */
typedef list_hash_t (*element_hash_computer)(const void *el);

/* [private-use] list entry -- olds actual user datum */
struct list_entry_s {
    void *data;

    /* doubly-linked list service references */
    struct list_entry_s *next;
    struct list_entry_s *prev;
};

/* [private-use] list attributes */
struct list_attributes_s {
    /* user-set routine for comparing list elements */
    element_comparator comparator;
    /* user-set routine for determining the length of an element */
    element_meter meter;
    int copy_data;
    /* user-set routine for computing the hash of an element */
    element_hash_computer hasher;
};

/** list object */
typedef struct {
    struct list_entry_s *head_sentinel;
    struct list_entry_s *tail_sentinel;
    struct list_entry_s *mid;

    unsigned int numels;

    /* array of spare elements */
    struct list_entry_s **spareels;
    unsigned int spareelsnum;

#ifdef SIMCLIST_WITH_THREADS
    /* how many threads are currently running */
    unsigned int threadcount;
#endif

    /* service variables for list iteration */
    int iter_active;
    unsigned int iter_pos;
    struct list_entry_s *iter_curentry;

    /* list attributes */
    struct list_attributes_s attrs;
} list_t;

/**
 * initialize a list object for use.
 *
 * @param l     must point to a user-provided memory location
 * @return      0 for success. -1 for failure
 */
int list_init(list_t *l);

/**
 * completely remove the list from memory.
 *
 * This function is the inverse of list_init(). It is meant to be called when
 * the list is no longer going to be used. Elements and possible memory taken
 * for internal use are freed.
 *
 * @param l     list to destroy
 */
void list_destroy(list_t *l);

/**
 * set the comparator function for list elements.
 *
 * Comparator functions are used for searching and sorting.
 * A comparator function is one which is given two user elements by reference,
 * and returns values in {-1, 0, 1}:
 * - -1 is returned if element a is 'greater' than b
 * - 0 is returned if element a and b are equivalent (or "equals")
 * - 1 is returned if element a is 'smaller' than b
 *
 * It is responsability of this function to correctly handle NULL values, if
 * NULL elements are inserted into the list.
 *
 * @param l     list to operate
 * @param comparator_fun    pointer to the actual comparator function
 * @return      0 if the attribute was successfully set; -1 otherwise
 */
int list_attributes_comparator(list_t *l, element_comparator comparator_fun);

/**
 * require to free element data when list entry is removed (default: don't free).
 *
 * [ advanced preference ]
 *
 * By default, when an element is removed from the list, it disappears from
 * the list by its actual data is not free()d. With this option, every
 * deletion causes element data to be freed.
 *
 * It is responsability of this function to correctly handle NULL values, if
 * NULL elements are inserted into the list.
 *
 * @param l             list to operate
 * @param metric_fun    pointer to the actual metric function
 * @param copy_data     0: do not free element data (default); non-0: do free
 * @return          0 if the attribute was successfully set; -1 otherwise
 */
int list_attributes_copy(list_t *l, element_meter metric_fun, int copy_data);

/**
 * set the element hash computing function for the list elements.
 *
 * [ advanced preference ]
 *
 * An hash can be requested depicting the list status at a given time. An hash
 * only depends on the elements and their order. By default, the hash of an
 * element is only computed on its reference. With this function, the user can
 * set a custom function computing the hash of an element. If such function is
 * provided, the list_hash() function automatically computes the list hash using
 * the custom function instead of simply referring to element references.
 *
 * @param l             list to operate
 * @param hash_computer_fun pointer to the actual hash computing function
 * @return              0 if the attribute was successfully set; -1 otherwise
 */
int list_attributes_hash_computer(list_t *l, element_hash_computer hash_computer_fun);

/**
 * [list-Queue] append data at the end of the list.
 *
 * This function is for using a list with a FIFO/queue policy.
 *
 * @param l     list to operate
 * @param data  pointer to user data to append
 *
 * @return      1 for success. < 0 for failure
 */
int list_append(list_t *l, const void * data);

/**
 * [list-Queue] extract the element in the top of the list.
 *
 * This function is for using a list with a FIFO/queue policy.
 *
 * @param l     list to operate
 * @return      reference to user datum, or NULL on errors
 */
void *list_fetch(list_t *l);

/**
 * retrieve an element at a given position.
 *
 * @param l     list to operate
 * @param pos   [0,size-1] position index of the element wanted
 * @return      reference to user datum, or NULL on errors
 */
void *list_get_at(const list_t *l, unsigned int pos);

/**
 * return the maximum element of the list.
 *
 * This function requires a comparator to be set for the list. See
 * list_attributes_comparator().
 *
 * @param l     list to operate
 * @return      the reference to the element, or NULL
 */
void *list_get_max(const list_t *l);

/**
 * return the minimum element of the list.
 *
 * This function requires a comparator to be set for the list. See
 * list_attributes_comparator().
 *
 * @param l     list to operate
 * @return      the reference to the element, or NULL
 */
void *list_get_min(const list_t *l);

/**
 * retrieve and removes from list an element at a given position.
 *
 * @param l     list to operate
 * @param pos   [0,size-1] position index of the element wanted
 * @return      reference to user datum, or NULL on errors
 */
void *list_extract_at(list_t *l, unsigned int pos);

/**
 * insert an element at a given position.
 *
 * @param l     list to operate
 * @param data  reference to data to be inserted
 * @param pos   [0,size-1] position index to insert the element at
 * @return      positive value on success. Negative on failure
 */
int list_insert_at(list_t *l, const void *data, unsigned int pos);

/**
 * expunge an element at a given position from the list.
 *
 * @param l     list to operate
 * @param pos   [0,size-1] position index of the element to be deleted
 * @return      0 on success. Negative value on failure
 */
int list_delete_at(list_t *l, unsigned int pos);

/**
 * expunge an array of elements from the list, given their position range.
 *
 * @param l     list to operate
 * @param posstart  [0,size-1] position index of the first element to be deleted
 * @param posend    [posstart,size-1] position of the last element to be deleted
 * @return      the number of elements successfully removed
 */
int list_delete_range(list_t *l, unsigned int posstart, unsigned int posend);

/**
 * clear all the elements off of the list.
 *
 * The element datums will not be freed.
 *
 * @param l     list to operate
 * @return      the number of elements in the list before cleaning
 *
 * @see list_delete_range()
 * @see list_size()
 */
int list_clear(list_t *l);

/**
 * inspect the number of elements in the list.
 *
 * @param l     list to operate
 * @return      number of elements currently held by the list
 */
unsigned int list_size(const list_t *l);

/**
 * inspect whether the list is empty.
 *
 * @param l     list to operate
 * @return      0 iff the list is not empty
 * 
 * @see list_size()
 */
int list_empty(const list_t *l);

/**
 * find an element in a list.
 *
 * Inspects the given list looking for the given element; if the element
 * is found, its position into the list is returned.
 * Elements are inspected comparing references if a comparator has not been
 * set. Otherwise, the comparator is used to find element.
 *
 * @param l     list to operate
 * @param data  reference of the element to search for
 * @return      position of element in the list, or <0 if not found
 * 
 * @see list_attributes_comparator()
 * @see list_get_at()
 */
int list_find(const list_t *l, const void *data);

/**
 * inspect whether some data is member of the list.
 *
 * By default, a per-reference comparison is accomplished. That is,
 * the data is in list if any element of the list points to the same
 * location of data.
 * A "semantic" comparison is accomplished, otherwise, if a comparator
 * function has been set previously, with list_attributes_comparator();
 * in which case, the given data reference is believed to be in list iff
 * comparator_fun(elementdata, userdata) == 0 for any element in the list.
 * 
 * @param l     list to operate
 * @param data  reference to the data to search
 * @return      0 iff the list does not contain data as an element
 *
 * @see list_attributes_comparator()
 */
int list_contains(const list_t *l, const void *data);

/**
 * concatenate two lists
 *
 * Concatenates one list with another, and stores the result into a
 * user-provided list object, which must be different from both the
 * lists to concatenate. Attributes from the original lists are not
 * cloned.
 * The destination list referred is threated as virgin room: if it
 * is an existing list containing elements, memory leaks will happen.
 *
 * @param l1    base list
 * @param l2    list to append to the base
 * @param dest  reference to the destination list
 * @return      0 for success, -1 for errors
 */
int list_concat(const list_t *l1, const list_t *l2, list_t *dest);

/**
 * sort list elements in ascending or descending order.
 *
 * This function requires a comparator to be set for the list. See
 * list_attributes_comparator().
 *
 * @param l     list to operate
 * @param versus positive: order small to big; negative: order big to small
 * @return      0: sorting went OK      non-0: errors happened
 */
int list_sort(list_t *l, int versus);

/**
 * start an iteration session.
 *
 * This function prepares the list to be iterated.
 *
 * @param l     list to operate
 * @return 		0 if the list cannot be currently iterated. >0 otherwise
 * 
 * @see list_iterator_stop()
 */
int list_iterator_start(list_t *l);

/**
 * return the next element in the iteration session.
 *
 * @param l     list to operate
 * @return		element datum, or NULL on errors
 */
void *list_iterator_next(list_t *l);

/**
 * inspect whether more elements are available in the iteration session.
 *
 * @param l     list to operate
 * @return      0 iff no more elements are available.
 */
int list_iterator_hasnext(const list_t *l);

/**
 * end an iteration session.
 *
 * @param l     list to operate
 * @return      0 iff the iteration session cannot be stopped
 */
int list_iterator_stop(list_t *l);

/**
 * return the hash of the current status of the list.
 *
 * @param l     list to operate
 * @return      an hash of the list
 */
list_hash_t list_hash(const list_t *l);

/* ready-made comparators, meters and hash computers */
                                /* comparator functions */
/**
 * ready-made comparator for int8_t elements.
 * @see list_attributes_comparator()
 */
int list_comparator_int8_t(const void *a, const void *b);

/**
 * ready-made comparator for int16_t elements.
 * @see list_attributes_comparator()
 */
int list_comparator_int16_t(const void *a, const void *b);

/**
 * ready-made comparator for int32_t elements.
 * @see list_attributes_comparator()
 */
int list_comparator_int32_t(const void *a, const void *b);

/**
 * ready-made comparator for int64_t elements.
 * @see list_attributes_comparator()
 */
int list_comparator_int64_t(const void *a, const void *b);

/**
 * ready-made comparator for uint8_t elements.
 * @see list_attributes_comparator()
 */
int list_comparator_uint8_t(const void *a, const void *b);

/**
 * ready-made comparator for uint16_t elements.
 * @see list_attributes_comparator()
 */
int list_comparator_uint16_t(const void *a, const void *b);

/**
 * ready-made comparator for uint32_t elements.
 * @see list_attributes_comparator()
 */
int list_comparator_uint32_t(const void *a, const void *b);

/**
 * ready-made comparator for uint64_t elements.
 * @see list_attributes_comparator()
 */
int list_comparator_uint64_t(const void *a, const void *b);

/**
 * ready-made comparator for float elements.
 * @see list_attributes_comparator()
 */
int list_comparator_float(const void *a, const void *b);

/**
 * ready-made comparator for double elements.
 * @see list_attributes_comparator()
 */
int list_comparator_double(const void *a, const void *b);

/**
 * ready-made comparator for string elements.
 * @see list_attributes_comparator()
 */
int list_comparator_string(const void *a, const void *b);

                                /*          metric functions        */
/**
 * ready-made metric function for int8_t elements.
 * @see list_attributes_copy()
 */
size_t list_meter_int8_t(const void *el);

/**
 * ready-made metric function for int16_t elements.
 * @see list_attributes_copy()
 */
size_t list_meter_int16_t(const void *el);

/**
 * ready-made metric function for int32_t elements.
 * @see list_attributes_copy()
 */
size_t list_meter_int32_t(const void *el);

/**
 * ready-made metric function for int64_t elements.
 * @see list_attributes_copy()
 */
size_t list_meter_int64_t(const void *el);

/**
 * ready-made metric function for uint8_t elements.
 * @see list_attributes_copy()
 */
size_t list_meter_uint8_t(const void *el);

/**
 * ready-made metric function for uint16_t elements.
 * @see list_attributes_copy()
 */
size_t list_meter_uint16_t(const void *el);

/**
 * ready-made metric function for uint32_t elements.
 * @see list_attributes_copy()
 */
size_t list_meter_uint32_t(const void *el);

/**
 * ready-made metric function for uint64_t elements.
 * @see list_attributes_copy()
 */
size_t list_meter_uint64_t(const void *el);

/**
 * ready-made metric function for float elements.
 * @see list_attributes_copy()
 */
size_t list_meter_float(const void *el);

/**
 * ready-made metric function for double elements.
 * @see list_attributes_copy()
 */
size_t list_meter_double(const void *el);

/**
 * ready-made metric function for string elements.
 * @see list_attributes_copy()
 */
size_t list_meter_string(const void *el);

                                /*          hash functions          */
/**
 * ready-made hash function for int8_t elements.
 * @see list_attributes_hash_computer()
 */
list_hash_t list_hashcomputer_int8_t(const void *el);

/**
 * ready-made hash function for int16_t elements.
 * @see list_attributes_hash_computer()
 */
list_hash_t list_hashcomputer_int16_t(const void *el);

/**
 * ready-made hash function for int32_t elements.
 * @see list_attributes_hash_computer()
 */
list_hash_t list_hashcomputer_int32_t(const void *el);

/**
 * ready-made hash function for int64_t elements.
 * @see list_attributes_hash_computer()
 */
list_hash_t list_hashcomputer_int64_t(const void *el);

/**
 * ready-made hash function for uint8_t elements.
 * @see list_attributes_hash_computer()
 */
list_hash_t list_hashcomputer_uint8_t(const void *el);

/**
 * ready-made hash function for uint16_t elements.
 * @see list_attributes_hash_computer()
 */
list_hash_t list_hashcomputer_uint16_t(const void *el);

/**
 * ready-made hash function for uint32_t elements.
 * @see list_attributes_hash_computer()
 */
list_hash_t list_hashcomputer_uint32_t(const void *el);

/**
 * ready-made hash function for uint64_t elements.
 * @see list_attributes_hash_computer()
 */
list_hash_t list_hashcomputer_uint64_t(const void *el);

/**
 * ready-made hash function for float elements.
 * @see list_attributes_hash_computer()
 */
list_hash_t list_hashcomputer_float(const void *el);

/**
 * ready-made hash function for double elements.
 * @see list_attributes_hash_computer()
 */
list_hash_t list_hashcomputer_double(const void *el);

/**
 * ready-made hash function for string elements.
 * @see list_attributes_hash_computer()
 */
list_hash_t list_hashcomputer_string(const void *el);

#ifdef __cplusplus
}
#endif

#endif

