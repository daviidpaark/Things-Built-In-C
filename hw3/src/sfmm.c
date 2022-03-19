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
    size -= 8;
    int range[NUM_FREE_LISTS];
    for (int i = 0; i < NUM_FREE_LISTS; i++)
    {
        if (i == 0)
        {
            range[i] = 1;
            if (size == range[i] * 32)
            {
                sf_free_list_heads[i].body.links.next->body.links.prev = block;
                block->body.links.next = sf_free_list_heads[i].body.links.next;
                sf_free_list_heads[i].body.links.next = block;
                block->body.links.prev = &sf_free_list_heads[i];
                return;
            }
        }
        else if (i == NUM_FREE_LISTS - 1)
        {
            range[i] = range[i - 1];
            if (size > (range[i] * 32))
            {
                sf_free_list_heads[i].body.links.next->body.links.prev = block;
                block->body.links.next = sf_free_list_heads[i].body.links.next;
                sf_free_list_heads[i].body.links.next = block;
                block->body.links.prev = &sf_free_list_heads[i];
                return;
            }
        }
        else
        {
            range[i] = 2 * range[i - 1];
            if (size > (range[i - 1] * 32) && size <= (range[i] * 32))
            {
                sf_free_list_heads[i].body.links.next->body.links.prev = block;
                block->body.links.next = sf_free_list_heads[i].body.links.next;
                sf_free_list_heads[i].body.links.next = block;
                block->body.links.prev = &sf_free_list_heads[i];
                return;
            }
        }
    }
}

void removeFreeBlock(sf_size_t size, sf_block *block)
{
    size -= 8;
    int range[NUM_FREE_LISTS];
    for (int i = 0; i < NUM_FREE_LISTS; i++)
    {
        if (i == 0)
        {
            range[i] = 1;
            if (size == range[i] * 32)
            {
                sf_block *current = sf_free_list_heads[i].body.links.next;
                while (current != &sf_free_list_heads[i])
                {
                    if (block == current)
                    {
                        current->body.links.prev->body.links.next = current->body.links.next;
                        current->body.links.next->body.links.prev = current->body.links.prev;
                        return;
                    }
                    current = current->body.links.next;
                }
            }
        }
        else if (i == NUM_FREE_LISTS - 1)
        {
            range[i] = range[i - 1];
            if (size >= (range[i] * 32))
            {
                sf_block *current = sf_free_list_heads[i].body.links.next;
                while (current != &sf_free_list_heads[i])
                {
                    if (block == current)
                    {
                        current->body.links.prev->body.links.next = current->body.links.next;
                        current->body.links.next->body.links.prev = current->body.links.prev;
                        return;
                    }
                    current = current->body.links.next;
                }
            }
        }
        else
        {
            range[i] = 2 * range[i - 1];
            if (size > (range[i - 1] * 32) && size <= (range[i] * 32))
            {
                sf_block *current = sf_free_list_heads[i].body.links.next;
                while (current != &sf_free_list_heads[i])
                {
                    if (block == current)
                    {
                        current->body.links.prev->body.links.next = current->body.links.next;
                        current->body.links.next->body.links.prev = current->body.links.prev;
                        return;
                    }
                    current = current->body.links.next;
                }
            }
        }
    }
}

sf_block *coalesce(sf_size_t size, sf_block *block)
{
    sf_header *nextHeader = &block->header;
    nextHeader = nextHeader + (size / 8);
    if (block->prev_footer && *nextHeader)
    {
        if (((block->prev_footer ^ MAGIC) & 0xF) == 0 /* || ((block->prev_footer ^ MAGIC) & 0xF) == PREV_BLOCK_ALLOCATED */)
        {
            sf_size_t prevSize = (block->prev_footer ^ MAGIC) & 0xFFFFFFF0;
            sf_header *prevHeader = &block->header;
            prevHeader = (prevHeader - (prevSize / 8));
            sf_block *prevBlock = (sf_block *)(prevHeader - 1);
            removeFreeBlock(prevSize, prevBlock);
            sf_footer *footer = &block->header;
            footer = footer + ((size / 8) - 1);
            *prevHeader = ((prevSize + size) | ((block->prev_footer ^ MAGIC) & 0xF)) ^ MAGIC;
            *footer = *prevHeader;
            sf_block *newBlock = (sf_block *)(prevHeader - 1);
            if (((*nextHeader ^ MAGIC) & 0xF) != 0)
                return newBlock;
        }
        if (((*nextHeader ^ MAGIC) & 0xF) == 0 /*  || ((*nextHeader ^ MAGIC) & 0xF) == PREV_BLOCK_ALLOCATED */)
        {
            sf_size_t nextSize = (*nextHeader ^ MAGIC) & 0xFFFFFFF0;
            sf_block *nextBlock = (sf_block *)(nextHeader - 1);
            removeFreeBlock(nextSize, nextBlock);
            sf_footer *footer = nextHeader;
            footer = footer + ((nextSize / 8) - 1);
            block->header = ((size + nextSize) | ((*nextHeader ^ MAGIC) & 0xF)) ^ MAGIC;
            *footer = block->header;
            return block;
        }
    }
    else if (block->prev_footer)
    {
        if (((block->prev_footer ^ MAGIC) & 0xF) == 0 /* || ((block->prev_footer ^ MAGIC) & 0xF) == PREV_BLOCK_ALLOCATED */)
        {
            sf_size_t prevSize = (block->prev_footer ^ MAGIC) & 0xFFFFFFF0;
            sf_header *prevHeader = &block->header;
            prevHeader = prevHeader - (prevSize / 8);
            sf_block *prevBlock = (sf_block *)(prevHeader - 1);
            removeFreeBlock(prevSize, prevBlock);
            sf_footer *footer = &block->header;
            footer = footer + ((size / 8) - 1);
            *prevHeader = ((prevSize + size) | ((block->prev_footer ^ MAGIC) & 0xF)) ^ MAGIC;
            *footer = *prevHeader;
            sf_block *newBlock = (sf_block *)(prevHeader - 1);
            return newBlock;
        }
    }
    else if (*nextHeader)
    {
        if (((*nextHeader ^ MAGIC) & 0xF) == 0 /* || ((*nextHeader ^ MAGIC) & 0xF) == PREV_BLOCK_ALLOCATED */)
        {
            sf_size_t nextSize = (*nextHeader ^ MAGIC) & 0xFFFFFFF0;
            sf_block *nextBlock = (sf_block *)(nextHeader - 1);
            removeFreeBlock(nextSize, nextBlock);
            sf_footer *footer = nextHeader;
            footer = footer + ((nextSize / 8) - 1);
            block->header = ((size + nextSize) | ((*nextHeader ^ MAGIC) & 0xF)) ^ MAGIC;
            *footer = block->header;
            return block;
        }
    }
    return NULL;
}

sf_block *collapse(sf_size_t size, sf_block *block)
{
    sf_size_t prevSize = (block->prev_footer ^ MAGIC) & 0xFFFFFFF0;
    sf_header *prevHeader = &block->header;
    prevHeader = prevHeader - (prevSize / 8);
    sf_block *prevBlock = (sf_block *)(prevHeader - 1);
    removeFreeBlock(prevSize, prevBlock);
    sf_footer *footer = &block->header;
    footer = footer + ((size / 8) - 2);
    *prevHeader = ((prevSize + size) | ((block->prev_footer ^ MAGIC) & 0xF)) ^ MAGIC;
    *footer = *prevHeader;
    sf_block *newBlock = (sf_block *)(prevHeader - 1);
    return newBlock;
}

int initializeHeap(sf_size_t initSize)
{
    initSize += 48;
    for (sf_size_t i = 0; i < initSize; i += PAGE_SZ)
    {
        sf_mem_grow();
    }
    // Prologue
    sf_header *hdr = sf_mem_start();
    hdr++;
    *hdr = (32 | THIS_BLOCK_ALLOCATED) ^ MAGIC;
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
    *hdr = block->header;
    placeFreeBlock(size, block);
    return 0;
}

int extendHeap(sf_size_t size)
{
    sf_header *oldFtr = sf_mem_end();
    oldFtr -= 2;
    for (sf_size_t i = 0; i < size; i += PAGE_SZ)
    {
        if (sf_mem_grow() == NULL)
            return -1;
    }
    // New epilogue
    sf_header *ftr = sf_mem_end();
    ftr--;
    *ftr = THIS_BLOCK_ALLOCATED ^ MAGIC;

    sf_size_t newSize = (sf_mem_end() - (void *)oldFtr) - 8;
    sf_block *newBlock = (sf_block *)oldFtr;
    newBlock->header = newSize ^ MAGIC;
    ftr--;
    *ftr = newBlock->header;
    sf_block *tempBlock;
    tempBlock = collapse(newSize, newBlock);
    placeFreeBlock(((tempBlock->header ^ MAGIC) & 0xFFFFFFF0), tempBlock);
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
            else if (sf_quick_lists[i].length == QUICK_LIST_MAX)
            {
                sf_block *newBlock;
                block->header = (size | THIS_BLOCK_ALLOCATED | IN_QUICK_LIST) ^ MAGIC;
                for (int j = 0; j < QUICK_LIST_MAX; j++)
                {
                    sf_block *tempBlock = sf_quick_lists[i].first;
                    sf_size_t tempSize = (tempBlock->header ^ MAGIC) & 0xFFFFFFF0;
                    sf_quick_lists[i].first = sf_quick_lists[i].first->body.links.next;
                    tempBlock->header = tempSize ^ MAGIC;
                    sf_footer *ftr = &tempBlock->header;
                    ftr = ftr + (tempSize / 8 - 1);
                    *ftr = tempBlock->header;
                    newBlock = coalesce(tempSize, tempBlock);
                }
                placeFreeBlock(((newBlock->header ^ MAGIC) & 0xFFFFFFF0), newBlock);
                block->body.links.next = NULL;
                block->body.links.prev = NULL;
                sf_quick_lists[i].length = 0;
                sf_quick_lists[i].first = block;
                sf_quick_lists[i].length++;
                return;
            }
            else
            {
                block->body.links.prev = NULL;
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
    if (size <= (32 + ((NUM_QUICK_LISTS - 2) * 16)))
    {
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
                    return block;
                }
                else
                {
                    return block;
                }
            }
            block = block->body.links.next;
        }
        if (extendHeap(size) == -1)
            return NULL;
        else
            return findBlock(size);
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
    return &ptr->body.payload;
}

void sf_free(void *pp)
{
    pp = pp - 16;
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
        sf_block *newBlock = coalesce(size, block);
        if (newBlock != NULL)
            placeFreeBlock(size, newBlock);
        else
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
