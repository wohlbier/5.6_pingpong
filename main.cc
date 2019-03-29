#include <iostream>
#include <cilk.h>
#include <memoryweb.h>

#define NPN 10

typedef long Index_t;

static inline
void store_val(Index_t * a1, Index_t * a2, Index_t i)
{
    a1[i%NPN] = i;
    a2[i%NPN] = i + a1[i%NPN];
}

int main(int argc, char* argv[])
{
    Index_t n_per_nodelet = NPN;
    Index_t ** M = (Index_t **)mw_malloc2d(NODELETS(),
                                           n_per_nodelet * sizeof(Index_t));

    Index_t nl1 = 2;
    Index_t nl2 = 5;

    for (Index_t i = 0; i < 2048; ++i)
    {
        //cilk_migrate_hint();
        cilk_spawn store_val(M[nl1], M[nl2], i);
    }
    cilk_sync;
    
    return 0;
}
