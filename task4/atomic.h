//CAS C INTERFACE {
typedef void * atomic_counter;
#include <tbb/tbb.h>
#include <cassert>  // Added to make assert work

using namespace tbb;
/**
* Return the new atomic counter on success or NULL on memory allocation error.
* Also the counter is initialized to 0.
*
Threads
* @return The atomic counter
*/
static void *ose_four_allocate_atomic_counter(void)
{
    atomic<int> *ret = new atomic<int>();
    return ret;
}
/**
* Free the atomic counter.
*
* @param atomic_ctr The counter to free.
*/
static void ose_four_free_atomic_counter(atomic_counter atomic_ctr)
{
    atomic<int> *ctr = (atomic<int> *)atomic_ctr;
    assert(ctr != NULL);
    delete ctr;
}
/**
* Denote val as the value of the atomic_ctr.
* If val==compare then val = swap is done.
* That is if val equals compare then it is set to swap.
* In any case the old val is returned.
*
* THIS ACTION IS ATOMIC!
* That means that the compare and the swap are done atomically (i.e. no other change
* to the value of the counter can occur between them - only before or after both
* actions have been done).
*
* @param atomic_ctr The counter to (possibly) change and return the value of.
* @param compare The value to base the change decision on.
* @param swap The value to swap in case the compare equals the contents of the counter.
*
* @return The old value of atomic_ctr (before this function changed it, if it did).
*/
static int ose_four_compare_and_set_atomic_counter(
        atomic_counter atomic_ctr,
        int compare,
        int swap)
{
    atomic<int> *ctr = (atomic<int> *)atomic_ctr;
    assert(ctr != NULL);
    return ctr->compare_and_swap(compare, swap);
}
//} CAS C INTERFACE
