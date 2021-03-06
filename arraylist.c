/*
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 * Copyright (c) 2016 Rainer Gerhards
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#include "config.h"

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <string.h>
# include <sys/param.h>
#endif /* STDC_HEADERS */

#if defined(HAVE_STRINGS_H) && !defined(_STRING_H) && !defined(__USE_BSD)
# include <strings.h>
#endif /* HAVE_STRINGS_H */

#include "arraylist.h"

struct array_list*
array_list_new(array_list_free_fn *free_fn)
{
    struct array_list *arr;

    arr = (struct array_list*)calloc(1, sizeof(struct array_list));
    if(!arr) return NULL;
    arr->size = ARRAY_LIST_DEFAULT_SIZE;
    arr->length = 0;
    arr->free_fn = free_fn;
    if(!(arr->array = (void**)calloc(sizeof(void*), arr->size))) {
        free(arr);
        return NULL;
    }
    return arr;
}

extern void
array_list_free(struct array_list *arr)
{
    int i;
    for(i = 0; i < arr->length; i++)
    if(arr->array[i]) arr->free_fn(arr->array[i]);
    free(arr->array);
    free(arr);
}

void*
array_list_get_idx(struct array_list *arr, int i)
{
    if(i >= arr->length) return NULL;
    return arr->array[i];
}

static int array_list_expand_internal(struct array_list *arr, int max)
{
    void *t;
    int new_size;

    if(max < arr->size) return 0;
    new_size = arr->size << 1;
    if (new_size < max)
        new_size = max;
    if(!(t = realloc(arr->array, new_size*sizeof(void*)))) return -1;
    arr->array = (void**)t;
    (void)memset(arr->array + arr->size, 0, (new_size-arr->size)*sizeof(void*));
    arr->size = new_size;
    return 0;
}

int
array_list_put_idx(struct array_list *arr, int idx, void *data)
{
    if(array_list_expand_internal(arr, idx+1)) return -1;
    if(arr->array[idx]) arr->free_fn(arr->array[idx]);
    arr->array[idx] = data;
    if(arr->length <= idx) arr->length = idx + 1;
    return 0;
}

int
array_list_add(struct array_list *arr, void *data)
{
    return array_list_put_idx(arr, arr->length, data);
}

int
array_list_add_idx(struct array_list *arr, int idx, void *data)
{
    if(idx < 0) return -1;
    if(array_list_expand_internal(arr, MAX(arr->length+1, idx+1))) return -1;
    // shift all elements right, if needed
    if(arr->array[idx]) memmove(arr->array + idx + 1, arr->array + idx, (arr->length - idx)*sizeof(void*));
    arr->length = MAX(arr->length+1, idx+1);
    arr->array[idx] = data;
    return 0;
}

/*
 * Deleting the idx-th element in the array_list.
 */
void
array_list_del_idx(struct array_list *const arr, const int idx)
{
    if (idx < 0 || idx >= arr->length) {
        return;
    }
    if(arr->array[idx]) arr->free_fn(arr->array[idx]);
    if (--arr->length > idx) {
        memmove(arr->array + idx, arr->array + idx + 1, (arr->length - idx) * sizeof(void *));
    }
    arr->array[arr->length] = NULL;
    return;
}

/* work around wrong compiler message: GCC and clang do
 * not handle sort_fn correctly if -Werror is given.
 */
#ifndef _AIX
#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#else
#if __GNUC__ < 6
#pragma GCC diagnostic ignored "-Werror"
#endif
#endif
#endif
void
array_list_sort(struct array_list *arr, int(*sort_fn)(const void *, const void *))
{
    qsort(arr->array, arr->length, sizeof(arr->array[0]), sort_fn);
}
#ifndef _AIX
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#else
#if __GNUC__ < 6
#pragma GCC diagnostic ignored "-Werror"
#endif
#endif
#endif
void* array_list_bsearch(const void **key, struct array_list *arr,
        int (*sort_fn)(const void *, const void *))
{
    return bsearch(key, arr->array, arr->length, sizeof(arr->array[0]),
            sort_fn);
}
#ifndef  _AIX
#pragma GCC diagnostic pop
#endif

int
array_list_length(struct array_list *arr)
{
    return arr->length;
}
