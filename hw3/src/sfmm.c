/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sfmm.h"

#include <errno.h>

void placeFreeBlock(sf_size_t size, sf_block *block)
{
    int range[NUM_FREE_LISTS];
    for (int i = 0; i < NUM_FREE_LISTS; i++)
    {
        if (i == 0)
        {
            range[i] = 1;
            if (size == range[i] * 32)
            {
                sf_free_list_heads[i].body.links.prev->body.links.next = block;
                block->body.links.prev = sf_free_list_heads[i].body.links.prev;
                sf_free_list_heads[i].body.links.prev = block;
                block->body.links.next = &sf_free_list_heads[i];
            }
        }
        else if (i == NUM_FREE_LISTS - 1)
        {
            range[i] = range[i - 1];
            if (size >= (range[i] * 32))
            {
                sf_free_list_heads[i].body.links.prev->body.links.next = block;
                block->body.links.prev = sf_free_list_heads[i].body.links.prev;
                sf_free_list_heads[i].body.links.prev = block;
                block->body.links.next = &sf_free_list_heads[i];
            }
        }
        else
        {
            range[i] = 2 * range[i - 1];
            if (size > (range[i - 1] * 32) && size <= (range[i] * 32))
            {
                sf_free_list_heads[i].body.links.prev->body.links.next = block;
                block->body.links.prev = sf_free_list_heads[i].body.links.prev;
                sf_free_list_heads[i].body.links.prev = block;
                block->body.links.next = &sf_free_list_heads[i];
            }
        }
    }
}

int initializeHeap(sf_size_t initSize)
{
    initSize += 48;
    for (sf_size_t i = 0; i < initSize; i += PAGE_SZ)
    {
        if (sf_mem_grow() == NULL)
        {
            return -1;
        }
    }
    // Prologue
    sf_header *hdr = sf_mem_start();
    hdr++;
    *hdr = (0x20 | THIS_BLOCK_ALLOCATED) ^ MAGIC;
    // Epilogue
    hdr = sf_mem_end();
    hdr--;
    *hdr = THIS_BLOCK_ALLOCATED ^ MAGIC;
    // Initialize free lists and sentinels
    for (int i = 0; i < NUM_FREE_LISTS; i++)
    {
        sf_free_list_heads[i].body.links.next = &sf_free_list_heads[i];
        sf_free_list_heads[i].body.links.prev = &sf_free_list_heads[i];
    }
    // Determine size of free block
    sf_size_t size = (sf_mem_end() - sf_mem_start()) - 48;
    // Initialize header and footer
    sf_block *block = sf_mem_start();
    block++;
    block->header = (size | PREV_BLOCK_ALLOCATED) ^ MAGIC;
    hdr--;
    *hdr = (size | PREV_BLOCK_ALLOCATED) ^ MAGIC;
    // Add free block to largest free list and link to sentinel
    placeFreeBlock(size, block);
    return 0;
}

void insertQuick(sf_size_t size, sf_block *block)
{
    sf_size_t minSize = 32;
    for (int i = 0; i < NUM_QUICK_LISTS; i++)
    {
        if (i > 0)
            minSize += 16;
        if (size == minSize)
        {
            if (sf_quick_lists[i].length == 0)
            {   
                block->body.links.next = NULL;
                block->body.links.prev = NULL;
                sf_quick_lists[i].first = block;
                sf_quick_lists[i].length++;
                block->header = (size | THIS_BLOCK_ALLOCATED | IN_QUICK_LIST) ^ MAGIC;
                return;
            }
            else if (sf_quick_lists[i].length == (QUICK_LIST_MAX - 1))
            {
                for (int j = 0; j < QUICK_LIST_MAX; j++)
                {
                    sf_block *block = sf_quick_lists[i].first;
                    sf_size_t size = (block->header ^ MAGIC) & 0xFFFFFFF0;
                    sf_quick_lists[i].first = sf_quick_lists[i].first->body.links.next;
                    placeFreeBlock(size, block);
                }
                block->body.links.next = NULL;
                block->body.links.prev = NULL;
                sf_quick_lists[i].length = 0;
                sf_quick_lists[i].first = block;
                sf_quick_lists[i].length++;
                block->header = (size | THIS_BLOCK_ALLOCATED | IN_QUICK_LIST) ^ MAGIC;
                return;
            }
            else
            {   
                block->body.links.next = sf_quick_lists[i].first;
                sf_quick_lists[i].first = block;
                sf_quick_lists[i].length++;
                block->header = (size | THIS_BLOCK_ALLOCATED | IN_QUICK_LIST) ^ MAGIC;
                return;
            }
        }
    }
}

void *findBlock(sf_size_t size)
{
    // Search quick lists
    for (int i = 0; i < NUM_QUICK_LISTS; i++)
    {
        if (sf_quick_lists[i].length == 0)
            continue;
        else
        {
            sf_block *block = sf_quick_lists[i].first;
            if ((block->header & 0xFFFFFFF0) >= size)
            {
                sf_quick_lists[i].first = sf_quick_lists[i].first->body.links.next;
                sf_quick_lists[i].length--;
                return block;
            }
            for (int j = 0; j < QUICK_LIST_MAX; j++)
            {
                block = block->body.links.next;
                if ((block->header & 0xFFFFFFF0) >= size)
                {
                    sf_quick_lists[i].first = sf_quick_lists[i].first->body.links.next;
                    sf_quick_lists[i].length--;
                    return block;
                }
            }
        }
    }
    // Search free lists
    for (int i = 0; i < NUM_FREE_LISTS; i++)
    {
        if (sf_free_list_heads[i].body.links.next == &sf_free_list_heads[i])
            continue;
        sf_block *block = sf_free_list_heads[i].body.links.next;
        while (block != &sf_free_list_heads[i])
        {
            if (((block->header ^ MAGIC) & 0xFFFFFFF0) >= size)
            {
                block->body.links.prev->body.links.next = block->body.links.next;
                block->body.links.next->body.links.prev = block->body.links.prev;

                sf_size_t blockSize = ((block->header ^ MAGIC) & 0xFFFFFFF0) - size;
                if (blockSize >= 32)
                {
                    sf_header *start = (sf_header *)(block);
                    start = start + (size / 8);
                    sf_block *freeBlock = (sf_block *)(start);
                    freeBlock->header = (blockSize | PREV_BLOCK_ALLOCATED) ^ MAGIC;
                    sf_footer *ftr = &freeBlock->header;
                    ftr = ftr + (blockSize / 8 - 1);
                    *ftr = freeBlock->header;
                    placeFreeBlock(blockSize, freeBlock);
                }
                return block;
            }
            block = block->body.links.next;
        }
    }

    return NULL;
}

void *sf_malloc(sf_size_t size)
{
    if (size == 0)
        return NULL;

    // Initialize the heap
    if (sf_mem_start() == sf_mem_end())
    {
        if (initializeHeap(size) == -1)
        {
            sf_errno = ENOMEM;
            return NULL;
        }
    }

    sf_size_t asize;
    sf_header payload;
    sf_block *ptr;

    // Set minimum block size and alignment
    payload = size;
    payload = payload << 32;
    asize = size + 8;
    if (asize <= 32)
        asize = 32;
    if (asize % 16 != 0)
        asize = asize + (16 - (asize % 16));

    // Find free block
    if ((ptr = findBlock(asize)) == NULL)
    {
        sf_errno = ENOMEM;
        return NULL;
    }
    ptr->header = (payload | asize | THIS_BLOCK_ALLOCATED | PREV_BLOCK_ALLOCATED) ^ MAGIC;
    return ptr;
}

void sf_free(void *pp)
{
    if (pp == NULL || ((unsigned long)pp % 16))
        abort();
    sf_block *block = (sf_block *)pp;

    if (&block->header < (sf_header *)sf_mem_start() + 5 || &block->header > (sf_header *)sf_mem_end() - 1)
        abort();
    sf_size_t size = (block->header ^ MAGIC) & 0xFFFFFFF0;
    if (size < 32 || size % 16)
        abort();
    if (!(((block->header ^ MAGIC) & 0xF) & THIS_BLOCK_ALLOCATED))
    {
        abort();
    }
    else if (!(((block->header ^ MAGIC) & 0xF) & PREV_BLOCK_ALLOCATED))
    {
        if (!block->prev_footer)
            abort();
    }
    block->header = size ^ MAGIC;
    if (size <= (32 + ((NUM_QUICK_LISTS - 2) * 16)))
        insertQuick(size, block);
    else
    {
        sf_footer *ftr = &block->header;
        ftr = ftr + (size / 8 - 1);
        *ftr = block->header;
        placeFreeBlock(size, block);
    }
}

void *sf_realloc(void *pp, sf_size_t rsize)
{
    // TO BE IMPLEMENTED
    abort();
}

double sf_internal_fragmentation()
{
    // TO BE IMPLEMENTED
    abort();
}

double sf_peak_utilization()
{
    // TO BE IMPLEMENTED
    abort();
}
