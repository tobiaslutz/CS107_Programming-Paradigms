#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
    assert(elemSize > 0);
    assert(initialAllocation >= 0);
    v->elemSize = elemSize;
    if(initialAllocation == 0)
        v->allocLen = 4;
    else
        v->allocLen = initialAllocation;
    v->logicalLen = 0;
    v->elems = malloc(elemSize * initialAllocation);
    v->freefn = freeFn;
    assert(v->elems != NULL);
}

void VectorDispose(vector *v)
{
    if(v->freefn != NULL)
        for(int i = 0; i < v->logicalLen; i++)
            v->freefn((char *)v->elems + i * v->elemSize);
    
    free(v->elems);
}

int VectorLength(const vector *v)
{
    return v->logicalLen;
}

void *VectorNth(const vector *v, int position)
{
    assert(position >= 0 && position < v->logicalLen);
    
    return (char *)v->elems + position * v->elemSize;
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{
    assert(position >= 0 && position < v->logicalLen);
    void *dest = (char *)v->elems + position * v->elemSize;
    if(v->freefn != NULL)
        v->freefn(dest);
    memcpy(dest, elemAddr, v->elemSize);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
     assert(position >= 0 && position <= v->logicalLen);
     if(v->logicalLen == v->allocLen)
        vectorGrow(v);
    
    void *oldPos = (char *)v->elems + position * v->elemSize;
    void *newPos = (char *)v->elems + (position + 1) * v->elemSize;
    memmove(newPos, oldPos, (v->logicalLen - position) * v->elemSize);
    memcpy(oldPos, elemAddr, v->elemSize);
    v->logicalLen++;
}

void VectorAppend(vector *v, const void *elemAddr)
{
    VectorInsert(v, elemAddr, v->logicalLen);
}

void VectorDelete(vector *v, int position)
{
    assert(position >= 0 && position < v->logicalLen);
    void *toDel = (char *)v->elems + position * v->elemSize;
    if(v->freefn != NULL)
        v->freefn(toDel);
    if(position < v->logicalLen - 1)    {
        void *toShift = (char *)v->elems + (position + 1) * v->elemSize;
        memcpy(toDel, toShift, (v->logicalLen - position - 1) * v->elemSize);
    }
    v->logicalLen--;
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
    qsort(v->elems, v->logicalLen, v->elemSize, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
    assert(mapFn != NULL);
    for(int i = 0; i < v->logicalLen; i++)
        mapFn((char *)v->elems + i * v->elemSize, auxData);
}

static void vectorGrow(vector *v)   {
    v->allocLen *= 2;
    v->elems = realloc(v->elems, v->allocLen * v->elemSize);
    assert(v->elems != NULL);
}

static const int kNotFound = -1;

int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{
    void *start = (char *)v->elems + startIndex * v->elemSize;
    void *found;
    if(isSorted)    {
        found = bsearch(key, start, v->logicalLen - startIndex, v->elemSize, searchFn);
        if(found == NULL)   return kNotFound;
        return (int)(((char *)found - (char *)v->elems) / v->elemSize);
    }
    else
        for(int i = startIndex; i < v->logicalLen; i++)
            if(searchFn(key,(char *)v->elems + i * v->elemSize) == 0)
                return i;
    
    return kNotFound;
}
