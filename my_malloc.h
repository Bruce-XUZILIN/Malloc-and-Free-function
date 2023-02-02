/**
 * @date 2023-01-27
 * @name my_malloc.h
 * @author Zilin Xu
 * This is the header file of my_malloc.c. It has fields and functions prototype and the datastructure I use.
*/

#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

//the datastructure: doubly LinkedList to represent the memory blocks
typedef struct datastructure{
  size_t size;//size of block
  int isFREE;//the free mode of one block
  struct datastructure* next;//next pointer
  struct datastructure* prev;//previous pointer
}block_t;

//some global variables
block_t* head = NULL;
block_t* tail = NULL;

//by request
void* ff_malloc(size_t size);
void ff_free(void *ptr);
void* bf_malloc(size_t size);
void bf_free(void *ptr);
unsigned long get_largest_free_data_segment_size();
unsigned long get_total_free_size();

//helper functions 
block_t* find_ff(size_t size);
block_t* find_bf(size_t size);
block_t* allocate_block(size_t size);
block_t* split_block(block_t* p, size_t size);
void merge_block(block_t* target);
void free_blocks(void* p);

//function for testing
void printList();


#endif
