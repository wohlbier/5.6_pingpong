#include <iostream>
#include <cilk.h>
#include <memoryweb.h>

#define NPN 64

typedef long Index_t;

static inline
Index_t r_map(Index_t i) { return i/NODELETS(); }

static inline
void store_val(Index_t * a1, Index_t * a2, Index_t i)
{
    Index_t ri = r_map(i);
    a2[ri] = 0;
    for (Index_t j = 0; j < NPN; ++j)
    {
        a1[ri] = i + a2[ri];
        a2[ri] = i + a1[ri];
    }
}

int main(int argc, char* argv[])
{
    Index_t n_per_nodelet = NPN;
    Index_t ** M0 = (Index_t **)mw_malloc2d(NODELETS(),
                                            n_per_nodelet * sizeof(Index_t));

    //Index_t ** M = (Index_t **)mw_mallocrepl(sizeof(Index_t **));
    //mw_replicated_init((long *)&M, (long)M0);
    
    Index_t nl1 = 2;
    Index_t nl2 = 5;

    starttiming();
    for (Index_t i = 0; i < NODELETS() * NPN; ++i)
    {
        cilk_migrate_hint(M0[nl2]);
        cilk_spawn store_val(M0[nl1], M0[nl2], i);

        // same difference with replicated or not.
        //cilk_migrate_hint(M[nl2]);
        //cilk_spawn store_val(M[nl1], M[nl2], i);
    }
    cilk_sync;
    
    return 0;
}
