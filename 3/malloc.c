#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define FIRST_FIT 1
#define BEST_FIT  2
#define WORST_FIT 3
#define QUICK_FIT 4

#ifndef STRATEGY
int STRATEGY = 1;
#endif




typedef long Align;

union header {

    struct {
        union header *ptr; /* pointer to next block */
        unsigned size; /* blocksize */
    } s;
    Align x;
};

typedef union header Header;



static Header base; /* empty list to get started */
static Header *freep = NULL; /* start of free list */
static Header *morecore(unsigned);

void *malloc(unsigned nbytes) {	
 
    Header *p, *prevp, *test_p, *test_prevp;
    test_p = NULL;
    test_prevp = NULL;

    unsigned nunits;

    if (nbytes <= 0)
        return NULL;

    nunits = (nbytes + sizeof (Header) - 1) / sizeof (Header) + 1;
    if ((prevp = freep) == NULL) {
        base.s.ptr = freep = prevp = &base;
        base.s.size = 0;
    }

    /* Try to find free block */
    for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {

        if (STRATEGY == FIRST_FIT) {
            if (p->s.size >= nunits) { /* big enough */
                break;
            }
        }

        if (STRATEGY == BEST_FIT) {
            if (p->s.size >= nunits) {
                if (test_p == NULL) {
                    test_p = p;
                    test_prevp = prevp;
                } else if (p->s.size < test_p->s.size) {
                    test_p = p;
                    test_prevp = prevp;
                }
            }
        }
        if (p == freep && test_p != NULL) {
            p = test_p;
            prevp = test_prevp;
            break;
        }


        if (STRATEGY == WORST_FIT) {
            if (p->s.size >= nunits) {
                if (test_p == NULL) {
                    test_p = p;
                    test_prevp = prevp;
                } else {
                    if (p->s.size > test_p->s.size) {
                        test_p = p;
                        test_prevp = prevp;
                    }
                }
            }
            if (p == freep && test_p != NULL) {
                p = test_p;
                prevp = test_prevp;
                break;
            }
        }/* end worst_fit*/

        if (STRATEGY == QUICK_FIT) {
            if (p->s.size >= nunits) {
                if (test_p == NULL) {
                    test_p = p;
                    test_prevp = prevp;
                } else {
                    if (p->s.size > test_p->s.size) {
                        test_p = p;
                        test_prevp = prevp;
                    }
                }
            }
            if (p == freep && test_p != NULL) {
                p = test_p;
                prevp = test_prevp;
                break;
            }
        }/* end quick_fit*/

        if (p == freep) /* wrapped around free list */
            if ((p = morecore(nunits)) == NULL)
                return NULL; /* none left */
    }

    if (p->s.size == nunits) { /* exactly */
        prevp->s.ptr = p->s.ptr;
    } else {
        p->s.size -= nunits;
        p += p->s.size;
        p->s.size = nunits;
    }
    freep = prevp;
    return (void *) (p + 1);
}

#define NALLOC  1024   /* minimum #units to request */

/* morecore:  ask system for more memory */
static Header *morecore(unsigned nu) {
    char *cp;
    Header *up;

    if (nu < NALLOC)
        nu = NALLOC;
    cp = sbrk(nu * sizeof (Header));
    if (cp == (char *) - 1) /* no space at all */
        return NULL;
    up = (Header *) cp;
    up->s.size = nu;
    free((void *) (up + 1));
    return freep;
}

/* free:  put block ap in free list */
void free(void *ap) {

    if (ap == NULL)
        return;

    Header *bp, *p;

    bp = (Header *) ap - 1; /* point to  block header */
    for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
        if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
            break; /* freed block at start or end of arena */

    if (bp + bp->s.size == p->s.ptr) { /* join to upper nbr */
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    } else
        bp->s.ptr = p->s.ptr;
    if (p + p->s.size == bp) { /* join to lower nbr */
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    } else
        p->s.ptr = bp;
    freep = p;
}

void *realloc(void *ptr, size_t new_size) {
    Header *h_ptr;
    int copy_size;
    void *new_ptr;

    h_ptr = (Header *) ptr - 1;

    if (ptr == NULL)
        return malloc(new_size);

    if (new_size == 0) {
        free(ptr);
        return NULL;
    }
    copy_size = (h_ptr->s.size - 1) * sizeof (Header);

    if (new_size < copy_size)
        copy_size = new_size;

    new_ptr = malloc(new_size);
    memcpy(new_ptr, ptr, copy_size);
    free(ptr);

    return new_ptr;

}
