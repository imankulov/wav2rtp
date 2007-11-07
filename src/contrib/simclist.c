#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <limits.h>
#include <stdint.h>

/* disable asserts */
#ifndef SIMCLIST_DEBUG
#define NDEBUG
#endif

#include <assert.h>

#ifdef SIMCLIST_WITH_THREADS
/* limit (approx) to the number of threads running
 * for threaded operations. Only meant when
 * SIMCLIST_WITH_THREADS is defined */
#define SIMCLIST_MAXTHREADS   2
#endif

/*
 * how many elems to keep as spare. During a deletion, an element
 * can be saved in a "free-list", not free()d immediately. When
 * latter insertions are performed, spare elems can be used instead
 * of malloc()ing new elems.
 *
 * about this param, some values for appending
 * 10 million elems into an empty list:
 * (#, time[sec], gain[%], gain/no[%])
 * 0    2,164   0,00    0,00    <-- feature disabled
 * 1    1,815   34,9    34,9
 * 2    1,446   71,8    35,9    <-- MAX gain/no
 * 3    1,347   81,7    27,23
 * 5    1,213   95,1    19,02
 * 8    1,064   110,0   13,75
 * 10   1,015   114,9   11,49   <-- MAX gain w/ likely sol
 * 15   1,019   114,5   7,63
 * 25   0,985   117,9   4,72
 * 50   1,088   107,6   2,15
 * 75   1,016   114,8   1,53
 * 100  0,988   117,6   1,18
 * 150  1,022   114,2   0,76
 * 200  0,939   122,5   0,61    <-- MIN time
 */
#ifndef SIMCLIST_MAX_SPARE_ELEMS
#define SIMCLIST_MAX_SPARE_ELEMS        5
#endif


#ifdef SIMCLIST_WITH_THREADS
#include <pthread.h>
#endif

#include "simclist.h"


/* minumum number of elements for sorting with quicksort instead of insertion */
#define SIMCLIST_MINQUICKSORTELS   24


/* deletes tmp from list, with care wrt its position (head, tail, middle) */
static int list_drop_elem(list_t *l, struct list_entry_s *tmp, unsigned int pos);

/* set default values for initialized lists */
static int list_attributes_setdefaults(list_t *l);

#ifndef NDEBUG
/* check whether the list internal REPresentation is valid -- Costs O(n) */
static int list_repOk(const list_t *l);

/* check whether the list attribute set is valid -- Costs O(1) */
static int list_attrOk(const list_t *l);
#endif

/* do not inline, this is recursive */
static void list_sort_quicksort(list_t *l, int versus, 
        unsigned int first, struct list_entry_s *fel,
        unsigned int last, struct list_entry_s *lel);

static inline void list_sort_selectionsort(list_t *l, int versus, 
        unsigned int first, struct list_entry_s *fel,
        unsigned int last, struct list_entry_s *lel);

static void *list_get_minmax(const list_t *l, int versus);

static inline struct list_entry_s *list_findpos(const list_t *l, int posstart);

/* list initialization */
int list_init(list_t *l) {
    if (l == NULL) return -1;

    l->numels = 0;

    /* head/tail sentinels and mid pointer */
    l->head_sentinel = (struct list_entry_s *)malloc(sizeof(struct list_entry_s));
    l->tail_sentinel = (struct list_entry_s *)malloc(sizeof(struct list_entry_s));
    l->head_sentinel->next = l->tail_sentinel;
    l->tail_sentinel->prev = l->head_sentinel;
    l->head_sentinel->prev = l->tail_sentinel->next = l->mid = NULL;
    l->head_sentinel->data = l->tail_sentinel->data = NULL;

    /* iteration attributes */
    l->iter_active = 0;
    l->iter_pos = 0;
    l->iter_curentry = NULL;

    /* free-list attributes */
    l->spareels = (struct list_entry_s **)malloc(SIMCLIST_MAX_SPARE_ELEMS * sizeof(struct list_entry_s *));
    l->spareelsnum = 0;

#ifdef SIMCLIST_WITH_THREADS
    l->threadcount = 0;
#endif

    list_attributes_setdefaults(l);

    assert(list_repOk(l));
    assert(list_attrOk(l));

    return 0;
}

void list_destroy(list_t *l) {
    unsigned int i;

    list_clear(l);
    for (i = 0; i < l->spareelsnum; i++) {
        free(l->spareels[i]);
    }
    free(l->spareels);
    free(l->head_sentinel);
    free(l->tail_sentinel);
}

int list_attributes_setdefaults(list_t *l) {
    l->attrs.comparator = NULL;

    /* also free() element data when removing and element from the list */
    l->attrs.meter = NULL;
    l->attrs.copy_data = 0;

    l->attrs.hasher = NULL;

    assert(list_attrOk(l));
    
    return 0;
}

/* setting list properties */
int list_attributes_comparator(list_t *l, element_comparator comparator_fun) {
    if (l == NULL || comparator_fun == NULL)
        return -1;
    l->attrs.comparator = comparator_fun;

    assert(list_attrOk(l));
    
    return 0;
}

int list_attributes_copy(list_t *l, element_meter metric_fun, int copy_data) {
    if (metric_fun == NULL && copy_data != 0) return -1;
    l->attrs.meter = metric_fun;
    l->attrs.copy_data = copy_data;
    assert(list_attrOk(l));
    return 0;
}

int list_attributes_hash_computer(list_t *l, element_hash_computer hash_computer_fun) {
    if (l == NULL || hash_computer_fun == NULL)
        return -1;

    l->attrs.hasher = hash_computer_fun;
    assert(list_attrOk(l));
    return 0;
}

int list_append(list_t *l, const void *data) {
    return list_insert_at(l, data, l->numels);
}

void *list_fetch(list_t *l) {
    return list_extract_at(l, 0);
}

void *list_get_at(const list_t *l, unsigned int pos) {
    struct list_entry_s *tmp;

    tmp = list_findpos(l, pos);

    return (tmp != NULL ? tmp->data : NULL);
}

void *list_get_max(const list_t *l) {
    return list_get_minmax(l, +1);
}

void *list_get_min(const list_t *l) {
    return list_get_minmax(l, -1);
}

/* REQUIRES {list->numels >= 1}
 * return the min (versus < 0) or max value (v > 0) in l */
static void *list_get_minmax(const list_t *l, int versus) {
    void *curminmax;
    struct list_entry_s *s;

    if (l->attrs.comparator == NULL || l->numels == 0)
        return NULL;
    
    curminmax = l->head_sentinel->next->data;
    for (s = l->head_sentinel->next->next; s != l->tail_sentinel; s = s->next) {
        if (l->attrs.comparator(curminmax, s->data) * versus > 0)
            curminmax = s->data;
    }

    return curminmax;
}

/* set tmp to point to element at index posstart in l */
static inline struct list_entry_s *list_findpos(const list_t *l, int posstart) {
    struct list_entry_s *ptr;
    float x;
    int i;

    /* accept 1 slot overflow for fetching head and tail sentinels */
    if (posstart < -1 || posstart > (int)l->numels) return NULL;

    x = (float)(posstart+1) / l->numels;
    if (x <= 0.25) {
        /* first quarter: get to posstart from head */
        for (i = -1, ptr = l->head_sentinel; i < posstart; ptr = ptr->next, i++);
    } else if (x < 0.5) {
        /* second quarter: get to posstart from mid */
        for (i = (l->numels-1)/2, ptr = l->mid; i > posstart; ptr = ptr->prev, i--);
    } else if (x <= 0.75) {
        /* third quarter: get to posstart from mid */
        for (i = (l->numels-1)/2, ptr = l->mid; i < posstart; ptr = ptr->next, i++);
    } else {
        /* fourth quarter: get to posstart from tail */
        for (i = l->numels, ptr = l->tail_sentinel; i > posstart; ptr = ptr->prev, i--);
    }

    return ptr;
}

void *list_extract_at(list_t *l, unsigned int pos) {
    struct list_entry_s *tmp;
    void *data;
    
    if (l->iter_active || pos >= l->numels) return NULL;

    tmp = list_findpos(l, pos);
    data = tmp->data;

    tmp->data = NULL;   /* save data from list_drop_elem() free() */
    list_drop_elem(l, tmp, pos);
    l->numels--;
    
    assert(list_repOk(l));

    return data;
}

int list_insert_at(list_t *l, const void *data, unsigned int pos) {
    struct list_entry_s *lent, *succ, *prec;
    
    if (l->iter_active || pos > l->numels) return -1;
    
    /* this code optimizes malloc() with a free-list */
    if (l->spareelsnum > 0) {
        lent = l->spareels[l->spareelsnum-1];
        l->spareelsnum--;
    } else {
        lent = (struct list_entry_s *)malloc(sizeof(struct list_entry_s));
        if (lent == NULL)
            return -1;
    }

    if (l->attrs.copy_data) {
        /* make room for user' data (has to be copied) */
        size_t datalen = l->attrs.meter(data);
        lent->data = (struct list_entry_s *)malloc(datalen);
        memcpy(lent->data, data, datalen);
    } else {
        lent->data = (void*)data;
    }

    /* actually append element */
    prec = list_findpos(l, pos-1);
    succ = prec->next;
    
    prec->next = lent;
    lent->prev = prec;
    lent->next = succ;
    succ->prev = lent;

    l->numels++;

    /* fix mid pointer */
    if (l->numels == 1) { /* first element, set pointer */
        l->mid = lent;
    } else if (l->numels % 2) {    /* now odd */
        if (pos >= (l->numels-1)/2) l->mid = l->mid->next;
    } else {                /* now even */
        if (pos <= (l->numels-1)/2) l->mid = l->mid->prev;
    }

    assert(list_repOk(l));

    return 1;
}

int list_delete_at(list_t *l, unsigned int pos) {
    struct list_entry_s *delendo;


    if (l->iter_active || pos >= l->numels) return -1;

    delendo = list_findpos(l, pos);

    list_drop_elem(l, delendo, pos);

    l->numels--;


    assert(list_repOk(l));

    return  0;
}

int list_delete_range(list_t *l, unsigned int posstart, unsigned int posend) {
    struct list_entry_s *lastvalid, *tmp, *tmp2;
    unsigned int i;
    int movedx;
    unsigned int numdel, midposafter;

    if (l->iter_active || posend < posstart || posend >= l->numels) return -1;

    tmp = list_findpos(l, posstart);    /* first el to be deleted */
    lastvalid = tmp->prev;              /* last valid element */

    numdel = posend - posstart + 1;
    midposafter = (l->numels-1-numdel)/2;

    midposafter = midposafter < posstart ? midposafter : midposafter+numdel;
    movedx = midposafter - (l->numels-1)/2;

    if (movedx > 0) { /* move right */
        for (i = 0; i < (unsigned int)movedx; l->mid = l->mid->next, i++);
    } else {    /* move left */
        movedx = -movedx;
        for (i = 0; i < (unsigned int)movedx; l->mid = l->mid->prev, i++);
    }

    assert(posstart == 0 || lastvalid != l->head_sentinel);
    i = posstart;
    if (l->attrs.copy_data) {
        /* also free element data */
        for (; i <= posend; i++) {
            tmp2 = tmp;
            tmp = tmp->next;
            if (tmp2->data != NULL) free(tmp2->data);
            if (l->spareelsnum < SIMCLIST_MAX_SPARE_ELEMS) {
                l->spareels[l->spareelsnum++] = tmp2;
            } else {
                free(tmp2);
            }
        }
    } else {
        /* only free containers */
        for (; i <= posend; i++) {
            tmp2 = tmp;
            tmp = tmp->next;
            if (l->spareelsnum < SIMCLIST_MAX_SPARE_ELEMS) {
                l->spareels[l->spareelsnum++] = tmp2;
            } else {
                free(tmp2);
            }
        }
    }
    assert(i == posend+1 && (posend != l->numels || tmp == l->tail_sentinel));

    lastvalid->next = tmp;
    tmp->prev = lastvalid;

    l->numels -= posend - posstart + 1;

    assert(list_repOk(l));

    return 0;
}

int list_clear(list_t *l) {
    struct list_entry_s *s;

    if (l->iter_active) return -1;
    
    if (l->attrs.copy_data) {        /* also free user data */
        /* spare a loop conditional with two loops: spareing elems and freeing elems */
        for (s = l->head_sentinel->next; l->spareelsnum < SIMCLIST_MAX_SPARE_ELEMS && s != l->tail_sentinel; s = s->next) {
            /* move elements as spares as long as there is room */
            if (s->data != NULL) free(s->data);
            l->spareels[l->spareelsnum++] = s;
        }
        while (s != l->tail_sentinel) {
            /* free the remaining elems */
            if (s->data != NULL) free(s->data);
            s = s->next;
            free(s->prev);
        }
        l->head_sentinel->next = l->tail_sentinel;
        l->tail_sentinel->prev = l->head_sentinel;
    } else { /* only free element containers */
        /* spare a loop conditional with two loops: spareing elems and freeing elems */
        for (s = l->head_sentinel->next; l->spareelsnum < SIMCLIST_MAX_SPARE_ELEMS && s != l->tail_sentinel; s = s->next) {
            /* move elements as spares as long as there is room */
            l->spareels[l->spareelsnum++] = s;
        }
        while (s != l->tail_sentinel) {
            /* free the remaining elems */
            s = s->next;
            free(s->prev);
        }
        l->head_sentinel->next = l->tail_sentinel;
        l->tail_sentinel->prev = l->head_sentinel;
    }
    l->numels = 0;
    l->mid = NULL;

    assert(list_repOk(l));

    return 0;
}

unsigned int list_size(const list_t *l) {
    return l->numels;
}

int list_empty(const list_t *l) {
    return (l->numels > 0);
}

int list_find(const list_t *l, const void *data) {
    struct list_entry_s *el;
    int pos = 0;
    
    if (l->attrs.comparator != NULL) {
        /* use comparator */
        for (el = l->head_sentinel->next; el != l->tail_sentinel; el = el->next, pos++) {
            if (l->attrs.comparator(data, el->data) == 0) break;
        }
    } else {
        /* compare references */
        for (el = l->head_sentinel->next; el != l->tail_sentinel; el = el->next, pos++) {
            if (el->data == data) break;
        }
    }
    if (el == l->tail_sentinel) return -1;

    return pos;
}

int list_contains(const list_t *l, const void *data) {
    return (list_find(l, data) >= 0);
}

int list_concat(const list_t *l1, const list_t *l2, list_t *dest) {
    struct list_entry_s *el, *srcel;
    unsigned int cnt;
    int err;


    if (l1 == NULL || l2 == NULL || dest == NULL || l1 == dest || l2 == dest)
        return -1;

    list_init(dest);

    dest->numels = l1->numels + l2->numels;
    if (dest->numels == 0)
        return 0;

    /* copy list1 */
    srcel = l1->head_sentinel->next;
    el = dest->head_sentinel;
    while (srcel != l1->tail_sentinel) {
        el->next = (struct list_entry_s *)malloc(sizeof(struct list_entry_s));
        el->next->prev = el;
        el = el->next;
        el->data = srcel->data;
        srcel = srcel->next;
    }
    dest->mid = el;     /* approximate position (adjust later) */
    /* copy list 2 */
    srcel = l2->head_sentinel->next;
    while (srcel != l2->tail_sentinel) {
        el->next = (struct list_entry_s *)malloc(sizeof(struct list_entry_s));
        el->next->prev = el;
        el = el->next;
        el->data = srcel->data;
        srcel = srcel->next;
    }
    el->next = dest->tail_sentinel;
    dest->tail_sentinel->prev = el;
    
    /* fix mid pointer */
    err = l2->numels - l1->numels;
    if ((err+1)/2 > 0) {        /* correct pos RIGHT (err-1)/2 moves */
        err = (err+1)/2;
        for (cnt = 0; cnt < (unsigned int)err; cnt++) dest->mid = dest->mid->next;
    } else if (err/2 < 0) { /* correct pos LEFT (err/2)-1 moves */
        err = -err/2;
        for (cnt = 0; cnt < (unsigned int)err; cnt++) dest->mid = dest->mid->prev;
    }
 
    assert(!(list_repOk(l1) && list_repOk(l2)) || list_repOk(dest));

    return 0;
}

int list_sort(list_t *l, int versus) {
    if (l->iter_active || l->attrs.comparator == NULL) /* cannot modify list in the middle of an iteration */
        return -1;

    if (l->numels <= 1)
        return 0;
    srandom(time(NULL)*time(NULL));
    list_sort_quicksort(l, versus, 0, l->head_sentinel->next, l->numels-1, l->tail_sentinel->prev);
    assert(list_repOk(l));
    return 0;
}

#ifdef SIMCLIST_WITH_THREADS
struct list_sort_wrappedparams {
    list_t *l;
    int versus;
    unsigned int first, last;
    struct list_entry_s *fel, *lel;
};

static void *list_sort_quicksort_threadwrapper(void *wrapped_params) {
    struct list_sort_wrappedparams *wp = (struct list_sort_wrappedparams *)wrapped_params;
    list_sort_quicksort(wp->l, wp->versus, wp->first, wp->fel, wp->last, wp->lel);
    free(wp);
    pthread_exit(NULL);
    return NULL;
}
#endif

static inline void list_sort_selectionsort(list_t *l, int versus, 
        unsigned int first, struct list_entry_s *fel,
        unsigned int last, struct list_entry_s *lel) {
    struct list_entry_s *cursor, *toswap, *firstunsorted;
    void *tmpdata;

    if (last <= first) /* <= 1-element lists are always sorted */
        return;
    
    for (firstunsorted = fel; firstunsorted != lel; firstunsorted = firstunsorted->next) {
        /* find min or max in the remainder of the list */
        for (toswap = firstunsorted, cursor = firstunsorted->next; cursor != lel->next; cursor = cursor->next)
            if (l->attrs.comparator(toswap->data, cursor->data) * -versus > 0) toswap = cursor;
        if (toswap != firstunsorted) { /* swap firstunsorted with toswap */
            tmpdata = firstunsorted->data;
            firstunsorted->data = toswap->data;
            toswap->data = tmpdata;
        }
    }
}

static void list_sort_quicksort(list_t *l, int versus, 
        unsigned int first, struct list_entry_s *fel,
        unsigned int last, struct list_entry_s *lel) {
    unsigned int pivotid;
    unsigned int i;
    register struct list_entry_s *pivot;
    struct list_entry_s *left, *right;
    void *tmpdata;
#ifdef SIMCLIST_WITH_THREADS
    pthread_t tid;
    int traised;
#endif


    if (last <= first)      /* <= 1-element lists are always sorted */
        return;
    
    if (last - first+1 <= SIMCLIST_MINQUICKSORTELS) {
        list_sort_selectionsort(l, versus, first, fel, last, lel);
        return;
    }
    
    /* base of iteration: one element list */
    if (! (last > first)) return;

    pivotid = (random() % (last - first + 1));
    /* pivotid = (last - first + 1) / 2; */

    /* find pivot */
    if (pivotid < (last - first + 1)/2) {
        for (i = 0, pivot = fel; i < pivotid; pivot = pivot->next, i++);
    } else {
        for (i = last - first, pivot = lel; i > pivotid; pivot = pivot->prev, i--);
    }

    /* smaller PIVOT bigger */
    left = fel;
    right = lel;
    /* iterate     --- left ---> PIV <--- right --- */
    while (left != pivot && right != pivot) {
        for (; left != pivot && (l->attrs.comparator(left->data, pivot->data) * -versus <= 0); left = left->next);
        /* left points to a smaller element, or to pivot */
        for (; right != pivot && (l->attrs.comparator(right->data, pivot->data) * -versus >= 0); right = right->prev);
        /* right points to a bigger element, or to pivot */
        if (left != pivot && right != pivot) {
            /* swap, then move iterators */
            tmpdata = left->data;
            left->data = right->data;
            right->data = tmpdata;

            left = left->next;
            right = right->prev;
        }
    }

    /* now either left points to pivot (end run), or right */
    if (right == pivot) {    /* left part longer */
        while (left != pivot) {
            if (l->attrs.comparator(left->data, pivot->data) * -versus > 0) {
                tmpdata = left->data;
                left->data = pivot->prev->data;
                pivot->prev->data = pivot->data;
                pivot->data = tmpdata;
                pivot = pivot->prev;
                pivotid--;
                if (pivot == left) break;
            } else {
                left = left->next;
            }
        }
    } else {                /* right part longer */
        while (right != pivot) {
            if (l->attrs.comparator(right->data, pivot->data) * -versus < 0) {
                /* move current right before pivot */
                tmpdata = right->data;
                right->data = pivot->next->data;
                pivot->next->data = pivot->data;
                pivot->data = tmpdata;
                pivot = pivot->next;
                pivotid++;
                if (pivot == right) break;
            } else {
                right = right->prev;
            }
        }
    }

    /* sort sublists A and B :       |---A---| pivot |---B---| */

#ifdef SIMCLIST_WITH_THREADS
    traised = 0;
    if (pivotid > 0) {
        /* prepare wrapped args, then start thread */
        if (l->threadcount < SIMCLIST_MAXTHREADS-1) {
            struct list_sort_wrappedparams *wp = (struct list_sort_wrappedparams *)malloc(sizeof(struct list_sort_wrappedparams));
            l->threadcount++;
            traised = 1;
            wp->l = l;
            wp->versus = versus;
            wp->first = first;
            wp->fel = fel;
            wp->last = first+pivotid-1;
            wp->lel = pivot->prev;
            if (pthread_create(&tid, NULL, list_sort_quicksort_threadwrapper, wp) != 0) {
                free(wp);
                traised = 0;
                list_sort_quicksort(l, versus, first, fel, first+pivotid-1, pivot->prev);
            }
        } else {
            list_sort_quicksort(l, versus, first, fel, first+pivotid-1, pivot->prev);
        }
    }
    if (first + pivotid < last) list_sort_quicksort(l, versus, first+pivotid+1, pivot->next, last, lel);
    if (traised) {
        pthread_join(tid, (void **)NULL);
        l->threadcount--;
    }
#else
    if (pivotid > 0) list_sort_quicksort(l, versus, first, fel, first+pivotid-1, pivot->prev);
    if (first + pivotid < last) list_sort_quicksort(l, versus, first+pivotid+1, pivot->next, last, lel);
#endif
}

int list_iterator_start(list_t *l) {
    if (l->iter_active) return 0;
    l->iter_pos = 0;
    l->iter_active = 1;
    l->iter_curentry = l->head_sentinel->next;
    return 1;
}

void *list_iterator_next(list_t *l) {
    void *toret;

    if (! l->iter_active) return NULL;

    toret = l->iter_curentry->data;
    l->iter_curentry = l->iter_curentry->next;
    l->iter_pos++;

    return toret;
}

int list_iterator_hasnext(const list_t *l) {
    if (! l->iter_active) return 0;
    return (l->iter_pos < l->numels);
}

int list_iterator_stop(list_t *l) {
    if (! l->iter_active) return 0;
    l->iter_pos = 0;
    l->iter_active = 0;
    return 1;
}

list_hash_t list_hash(const list_t *l) {
    struct list_entry_s *x;
    list_hash_t hash;
    
    hash = l->numels * 2 + 100;
    if (l->attrs.hasher == NULL) {
        /* only use element references */
        for (x = l->head_sentinel->next; x != l->tail_sentinel; x = x->next) {
            hash += (hash ^ (list_hash_t)x->data);
            hash += hash % l->numels;
        }
    } else {
        /* hash each element with the user-given function */
        for (x = l->head_sentinel->next; x != l->tail_sentinel; x = x->next) {
            hash += hash ^ l->attrs.hasher(x->data);
            hash += hash % l->numels;
        }
    }
    return hash;
}

int list_drop_elem(list_t *l, struct list_entry_s *tmp, unsigned int pos) {
    if (tmp == NULL) return -1;

    /* fix mid pointer */
    if (l->numels % 2) {    /* now odd */
        if (pos >= l->numels/2) l->mid = l->mid->prev;
    } else {                /* now even */
        if (pos < l->numels/2) l->mid = l->mid->next;
    }
    
    tmp->prev->next = tmp->next;
    tmp->next->prev = tmp->prev;

    /* free what's to be freed */
    if (l->attrs.copy_data && tmp->data != NULL)
        free(tmp->data);

    if (l->spareelsnum < SIMCLIST_MAX_SPARE_ELEMS) {
        l->spareels[l->spareelsnum++] = tmp;
    } else {
        free(tmp);
    }

    return 0;
}


/* ready-made comparators and meters */
#define SIMCLIST_NUMBER_COMPARATOR(type)     int list_comparator_##type(const void *a, const void *b) { return( *(type *)a < *(type *)b) - (*(type *)a > *(type *)b); } 

SIMCLIST_NUMBER_COMPARATOR(int8_t);
SIMCLIST_NUMBER_COMPARATOR(int16_t);
SIMCLIST_NUMBER_COMPARATOR(int32_t);
SIMCLIST_NUMBER_COMPARATOR(int64_t);

SIMCLIST_NUMBER_COMPARATOR(uint8_t);
SIMCLIST_NUMBER_COMPARATOR(uint16_t);
SIMCLIST_NUMBER_COMPARATOR(uint32_t);
SIMCLIST_NUMBER_COMPARATOR(uint64_t);

SIMCLIST_NUMBER_COMPARATOR(float);
SIMCLIST_NUMBER_COMPARATOR(double);

int list_comparator_string(const void *a, const void *b) { return strcmp((const char *)b, (const char *)a); }

/* ready-made metric functions */
#define SIMCLIST_METER(type)        size_t list_meter_##type(const void *el) { return sizeof(type); }

SIMCLIST_METER(int8_t);
SIMCLIST_METER(int16_t);
SIMCLIST_METER(int32_t);
SIMCLIST_METER(int64_t);

SIMCLIST_METER(uint8_t);
SIMCLIST_METER(uint16_t);
SIMCLIST_METER(uint32_t);
SIMCLIST_METER(uint64_t);

SIMCLIST_METER(float);
SIMCLIST_METER(double);

size_t list_meter_string(const void *el) { return strlen((const char *)el) + 1; }

/* ready-made hashing functions */
#define SIMCLIST_HASHCOMPUTER(type)    list_hash_t list_hashcomputer_##type(const void *el) { return (list_hash_t)(*(type *)el); }

SIMCLIST_HASHCOMPUTER(int8_t);
SIMCLIST_HASHCOMPUTER(int16_t);
SIMCLIST_HASHCOMPUTER(int32_t);
SIMCLIST_HASHCOMPUTER(int64_t);

SIMCLIST_HASHCOMPUTER(uint8_t);
SIMCLIST_HASHCOMPUTER(uint16_t);
SIMCLIST_HASHCOMPUTER(uint32_t);
SIMCLIST_HASHCOMPUTER(uint64_t);

SIMCLIST_HASHCOMPUTER(float);
SIMCLIST_HASHCOMPUTER(double);

list_hash_t list_hashcomputer_string(const void *el) {
    size_t l;
    list_hash_t hash = 123;
    const char *str = (const char *)str;
    char plus;

    for (l = 0; str[l] != '\0'; l++) {
        if (l) plus = hash ^ str[l];
        else plus = hash ^ (str[l] - str[0]);
        hash += (plus << (CHAR_BIT * (l % sizeof(list_hash_t))));
    }

    return hash;
}


#ifndef NDEBUG
static int list_repOk(const list_t *l) {
    int ok, i;
    struct list_entry_s *s;

    ok = (l != NULL) && (
            /* head/tail checks */
            (l->head_sentinel != NULL && l->tail_sentinel != NULL) &&
                (l->head_sentinel != l->tail_sentinel) && (l->head_sentinel->prev == NULL && l->tail_sentinel->next == NULL) &&
            /* empty list */
            (l->numels > 0 || (l->mid == NULL && l->head_sentinel->next == l->tail_sentinel && l->tail_sentinel->prev == l->head_sentinel)) &&
            /* spare elements checks */
            l->spareelsnum <= SIMCLIST_MAX_SPARE_ELEMS
         );
    
    if (!ok) return 0;

    if (l->numels >= 1) {
        /* correct referencing */
        for (i = -1, s = l->head_sentinel; i < (int)(l->numels-1)/2 && s->next != NULL; i++, s = s->next) {
            if (s->next->prev != s) break;
        }
        ok = (i == (int)(l->numels-1)/2 && l->mid == s);
        if (!ok) return 0;
        for (; s->next != NULL; i++, s = s->next) {
            if (s->next->prev != s) break;
        }
        ok = (i == (int)l->numels && s == l->tail_sentinel);
    }

    return ok;
}

static int list_attrOk(const list_t *l) {
    int ok;

    ok = (l->attrs.copy_data == 0 || l->attrs.meter != NULL);
    return ok;
}

#endif

