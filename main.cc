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
    //for (Index_t i = 0; i < 2; ++i)
    {
        //cilk_migrate_hint(M0[nl2]); // DO NOT USE THIS!, adds migrations
        cilk_migrate_hint(M0+nl2);
        cilk_spawn store_val(M0[nl1], M0[nl2], i);
    }
    cilk_sync;
    
    return 0;
}

/*
one iterate:

- 1 migration from 0 to 5 // TID 0 follows migration hint to 5, spawns TID 1
- 1 migration from 5 to 2 // TID 1 migrates from 5 to 2 for data acccess
- 1 migration from 5 to 0 // TID 0 migrates back to zero and issues sync
- 1 migration from 2 to 5 // TID 1 migrates from 2 to 5 to do kernel
- 1 migration from 5 to 0 // TID 1 migrates from 5 to 0 to finish (ADDM?)

TID 0 NLET 0 MIGRATE DEST 5 FUNC main @cycle 1530 TPC 0x80002173 Inst LDE
TID 0 NLET 5 SPAWN CHILDTID 1 FUNC main @cycle 5070
TID 1 NLET 5 MIGRATE DEST 2 FUNC cilkhelper0 @cycle 5245 TPC 0x80002331 Inst LDE
TID 0 NLET 5 MIGRATE DEST 0 FUNC __cilk_sync @cycle 5306 TPC 0x80002418 Inst ADDM
TID 1 NLET 2 MIGRATE DEST 5 FUNC cilkhelper0 @cycle 5598 TPC 0x8000234f Inst ADD
TID 0 NLET 0 DIED NLET 0 FUNC __cilk_sync @cycle 5669
TID 1 NLET 5 MIGRATE DEST 0 FUNC main @cycle 30585 TPC 0x8000225d Inst ADDM
TID 1 NLET 0 DIED NLET 0 FUNC _Exit @cycle 41535

REMOTES MAP shows 64 remotes from 5 to 2, which is the number of iterations
in the loop.

Multiplying these results by 2 should give:
2 migrations from 0 to 5
2 migrations from 5 to 2
2 migrations from 2 to 5
4 migrations from 5 to 0

REMOTES 5 to 2: 64*2 = 128

However, one gets
4 migrations from 0 to 5
2 migrations from 5 to 2
2 migrations from 2 to 5
6 migrations from 5 to 0

REMOTES 5 to 2: 128

CHANGING the migrate hint to not dereference the pointer, viz.,
cilk_migrate_hint(M0 + nl2)
gives
2 migrations from 0 to 5
2 migrations from 5 to 2
2 migrations from 2 to 5
4 migrations from 5 to 0

 */
