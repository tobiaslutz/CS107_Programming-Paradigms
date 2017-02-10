#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn)
{
    assert(elemSize > 0);
    assert(numBuckets > 0);
    assert(hashfn != NULL);
    assert(comparefn != NULL);
    
    h->elemSize = elemSize;
    h->numBuckets = numBuckets;
    h->hashfn = hashfn;
    h->cmpfn = comparefn;
    h->freefn = freefn;
    h->buckets = malloc(sizeof(vector) * numBuckets);
    for(int i = 0; i < numBuckets; i++)
        VectorNew(&h->buckets[i], elemSize, freefn, 4);
}

void HashSetDispose(hashset *h)
{
    for(int i = 0; i < h->numBuckets; i++)
        VectorDispose(&h->buckets[i]);
    
    free(h->buckets);
}

int HashSetCount(const hashset *h)
{
    int c = 0;
    for(int i = 0; i < h->numBuckets; i++)
        c += VectorLength(&h->buckets[i]);
    
    return c;
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
    for(int i = 0; i < h->numBuckets; i++)
        for(int j = 0; j < VectorLength(&h->buckets[i]); j++)
            mapfn(VectorNth(&h->buckets[i], j), auxData);
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
    assert(elemAddr != NULL);
    int buck = h->hashfn(elemAddr,h->numBuckets);
    assert(buck >= 0 && buck < h->numBuckets);
    
    int fPos = VectorSearch(&h->buckets[buck], elemAddr, h->cmpfn, 0, false);
    if(fPos == -1)  {
        VectorAppend(&h->buckets[buck], elemAddr);
        return;
    }
    
    VectorReplace(&h->buckets[buck], elemAddr, fPos);
    
}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{
    assert(elemAddr != NULL);
    int buck = h->hashfn(elemAddr,h->numBuckets);
    assert(buck >= 0 && buck < h->numBuckets);
    
    int fPos = VectorSearch(&h->buckets[buck], elemAddr, h->cmpfn, 0, false);
    if(fPos == -1) return NULL;
    
    return VectorNth(&h->buckets[buck], fPos);
}
