/**
 * @date 2023-01-27
 * @author Zilin Xu
 * This is the main file to implement the malloc and free in C. All the steps are commented in details
*/

#include "my_malloc.h"
#include "assert.h"

/**
 * For testing purpose, print the list.
*/
void printList() {
  block_t * p = head;
  //use a pointer to traverse the list
  while (p != NULL) {
    printf("p: %p, p->size: %lu\n", p, p->size);
    p = p->next;
  }
}
/**
 * This function finds the first_fit block for ff_malloc by traversing the list.
 * @param size: size of the block needed
 * @return return the first fit block we find, otherwise return NULL
*/
block_t* find_ff(size_t size){
    //corner case: head is NULL
    if(head == NULL){
      return NULL;
    }
    //a pointer to traverse 
    block_t* p = head;
    do{
        // Find a valid block we need two conditions for first fit strategy
        if(p->isFREE == 1 && p->size >= size){
            return p;
    }
        //move the pointer
        p = p->next;
    } while(p);
    // means we don't find
    return NULL;
}
/**
 * When we are not able to find a suitable block for malloc from find_ff or find_bf, we use this function to create a new block.
 * Use the sbrk() from Linux system call.
 * @param size size of the block request
 * @return the block we initialize
*/
block_t* allocate_block(size_t size){
    //initilize the new block, named new
    block_t* new = sbrk(size + sizeof(block_t));
    //the case the sbrk fail
    if(sbrk(size+sizeof(block_t))==(void*)-1){
        return NULL;
  }
    //initilize these variables for new block
        new->next = NULL;
        new->prev = NULL;
        new->size = size;
        new->isFREE = 0;

    return new;
}
/**
 * This function splits the block* p to the given size. In the case of p is too large and no waste we want, we split it as p and new.
 * @param p: the block we have to split
 * @param size: the size we need for p
 * @return the original block but after split
*/
block_t* split_block(block_t* p, size_t size){
    //initilize the new block
    block_t* new = (block_t*)((char*)p + size + sizeof(block_t));
    //assign new values
    new->size = p->size - size - sizeof(block_t);
    new->isFREE = 1;//new block is free
    new->next = p->next;
    new->prev = p;
    //reassign values for p
    p->next = new;
    p->size = size;
    p->isFREE = 0;//original block is not free
    //maintain the tail variable if new is the tail
    if(!(new->next)){
        tail = new;
  }
    //else, new's next's prev should be new
    else{
        new->next->prev = new;
  }

    return p;
}

/**
 * This function implement first fit malloc. There are two corner cases: 1, block size is 0 2. head and tail
 * are NULL. After dealing with corner cases, we assign find_ff function and if we find, we return it, if not
 * we use allocate_block to create a new block.
 * @param size: size of the block we need.
*/
void* ff_malloc(size_t size){
    //corner case
    if (size == 0) return NULL;
    //the block we find
    block_t *block = NULL;
    //corner case 2: consider about head and tail
    if (!head && !tail) {
      //if the list is empty we assign head to new block
      head = allocate_block(size);
      if (!head) return NULL;
      tail = head;
      block = head;
    } else {
      //pointer to traverse the list
      block_t *curr = find_ff(size);
      //if we find by using find_ff
      if (curr) {
      //we are not done yet, if the block size is too large, we have to split it
        if (curr->size >= size + sizeof(block_t)) {
        curr = split_block(curr, size);
        //cannot split
      } else {
        curr->isFREE = 0;
      }
      block = curr; //assign current to block we return
    } else {
      //if we didn't find, we call allocate_block with sbrk()
      block = allocate_block(size);
      if (!block) return NULL;
      tail->next = block;
      block->prev = tail;
      tail = block;
    }
  }
  return (char *)block + sizeof(block_t);

}

/**
 * This function merges adjacent two free blocks together. It deals with one block's next and previous
 * @param target: the block we want to merge.
*/
void merge_block(block_t* target){
  //corner case 
  if(target==NULL){
        return;
    }else{
      //initilize two adjacent block
        block_t *previous = target->prev;
        block_t *nextOne = target->next;
        //their merge algorithm are similar
        if(nextOne!=NULL && nextOne->isFREE == 1){
            target->size = target->size + nextOne->size + sizeof(block_t);
            target->next = nextOne->next;
            if(target->next!=NULL){
                target->next->prev = target;
            }else{
                tail = target;
            }
        }
        if(previous!=NULL && previous->isFREE == 1){
            previous->size = previous->size + target->size + sizeof(block_t);
            previous->next = target->next;
            if(previous->next!=NULL){
                previous->next->prev = previous;
            }else{
                tail = previous;
            }
        }
    }
}
/**
 * Implement free functions. Set the target free by change the free variable, then mergeall the free blocks.
 * @param p: the target we want to free
*/
void free_blocks(void* p){
  //corner case 
    if(p == NULL){
        return;
    }
    //initilize the target we want to free
    block_t* target = (block_t*)((char*)p - sizeof(block_t));
    target->isFREE = 1;
    merge_block(target);

}
/**
 * Implement ff_free. Merge the target block with other free region, its next and its prev together
 * @param p: the pointer of the block we have to free
*/
void ff_free(void* p){
    free_blocks(p);
}
/**
 * Find the best fit block by dealing with three conditions: 1, the size is smaller or not free, continue
 * 2, size is suitable, return it, 3, no suitable, we manually find a suitable one with least difference
 * @param size: the size we want 
 * @return the block we find
*/
block_t* find_bf(size_t size){
      //corner case 
      if(head == NULL){
      return NULL;
    }
    //the pointer to traverse the list and the target we find, initially set to null
    block_t* p = head, *target = NULL;
    // the variable to record the difference between the block size and size we want
    unsigned int difference = UINT_MAX;
    //traverse the list
    do{
        //the invalid block
        if(p->isFREE == 0 ){
            p = p->next;
            continue;
    }   //also the invalid block
        if(p->size<size){
          p = p->next;
          continue;

        }
    //means we find the best fit
    if(p->size == size){
      return p;
    }
    //else we have to find the best fit ourselves by using the diff
    unsigned int diff= p->size - size;
    if(diff < difference){
      difference = diff;
      target = p;
    }
    //move the pointer
    p = p->next;

    }while(p);
  return target;

}
/**
 * Implement bf_malloc, similar with ff_malloc only difference is call different function
 * @param size: size we want
*/

void* bf_malloc(size_t size){
  if (size == 0) return NULL;

  block_t *block = NULL;
  if (!head && !tail) {
    head = allocate_block(size);
    if (!head) return NULL;
    tail = head;
    block = head;
  } else {
    block_t *curr = find_bf(size);
    if (curr) {
      if (curr->size >= size + sizeof(block_t)) {
        curr = split_block(curr, size);
      } else {
        curr->isFREE = 0;
      }
      block = curr;
    } else {
      block = allocate_block(size);
      if (!block) return NULL;
      tail->next = block;
      block->prev = tail;
      tail = block;
    }
  }
  return (char *)block + sizeof(block_t);
}

/**
 * Same as ff_free
 * @param p: the block we want to free
*/
void bf_free(void* p){
  free_blocks(p);
}
/**
 * traverse the list to get largest free size
 * @return get the output
*/
unsigned long get_largest_free_data_segment_size(){
    //the pointer 
  block_t* p = head;
  unsigned long size = 0;
  //traverse the list
  while(p != NULL){
    size = p->size > size ? p->size : size;
    p = p->next;

  }
  return size;
}

/**
 * returns the total free data size
 * @return the output
*/
unsigned long get_total_free_size(){
    //the pointer 
  block_t* curr = head;
  unsigned long res = 0;
  //traverse the loop
  while(curr != NULL){
    if(curr->isFREE == 1){
      res += sizeof(block_t) + curr->size;
    }
    curr = curr -> next;
  }
  return res;
}


