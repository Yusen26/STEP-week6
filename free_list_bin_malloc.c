//
// >>>> malloc challenge! <<<<
//
// Your task is to improve utilization and speed of the following malloc
// implementation.
// Initial implementation is the same as the one implemented in simple_malloc.c.
// For the detailed explanation, please refer to simple_malloc.c.

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Interfaces to get memory pages from OS
//

void *mmap_from_system(size_t size);
void munmap_to_system(void *ptr, size_t size);

//
// Struct definitions
//

typedef struct my_metadata_t {
  size_t size;
  struct my_metadata_t *next;
} my_metadata_t;

typedef struct my_heap_t {
  my_metadata_t *free_head[4];
  my_metadata_t dummy;
} my_heap_t;

//
// Static variables (DO NOT ADD ANOTHER STATIC VARIABLES!)
//
my_heap_t my_heap;

//
// Helper functions (feel free to add/remove/edit!)
//
int classify_list(size_t size){
  size_t cls_size[5] = {0, 128, 1024, 2048, 4096};
  int i;
  int class;
  for(i=0;i<4;i++){
    if (size>cls_size[i] && size<=cls_size[i+1]){
      class = i;
    }
   }
    return class;
}

void my_add_to_free_list(my_metadata_t *metadata) {
  assert(!metadata->next);
  int class = classify_list(metadata->size);
  metadata->next = my_heap.free_head[class];
  my_heap.free_head[class] = metadata;
}

void my_remove_from_free_list(my_metadata_t *metadata, my_metadata_t *prev) {
  int class = classify_list(metadata->size);
  if (prev) {
    prev->next = metadata->next;
  } else {
    my_heap.free_head[class] = metadata->next;
  }
  metadata->next = NULL;
}

//
// Interfaces of malloc (DO NOT RENAME FOLLOWING FUNCTIONS!)
//

// This is called at the beginning of each challenge.
void my_initialize() {
  int i;
  for(i=0;i<4;i++){
  my_heap.free_head[i] = &my_heap.dummy;
  }
  my_heap.dummy.size = 0;
  my_heap.dummy.next = NULL;
}

// my_malloc() is called every time an object is allocated.
// |size| is guaranteed to be a multiple of 8 bytes and meets 8 <= |size| <=
// 4000. You are not allowed to use any library functions other than
// mmap_from_system() / munmap_to_system().
void *my_malloc(size_t size) {
  my_metadata_t *metadata;
  int class = classify_list(size);
  metadata = my_heap.free_head[class];
  my_metadata_t *prev = NULL;
  size_t min_size = 5000;
  my_metadata_t *final_meta = NULL;
  my_metadata_t *final_prev = NULL;
  //printf("%ld\n", size);
  while (1){
    // free_head[class]で見つからなかったら、classを1増やして探す
    // class=まで増やしても見つからなったら、breakする
    if (!metadata){
      class += 1;
      if (class<4){
      metadata = my_heap.free_head[class];
      }
      else break;
    }
    else if (metadata->size >= size && metadata->size < min_size){
      min_size = metadata->size;
      final_meta = metadata;
      final_prev = prev;
      if (metadata->size == size){
        break;
      }
    }
    prev = metadata;
    metadata = metadata->next;
  }
  metadata = final_meta;
  prev = final_prev;
  
  // now, metadata points to the best fit free slot
  // and prev is the previous entry.

  if (!metadata) {
    //printf("bbb");
    // There was no free slot available. We need to request a new memory region
    // from the system by calling mmap_from_system().
    //
    //     | metadata | free slot |
    //     ^
    //     metadata
    //     <---------------------->
    //            buffer_size
    size_t buffer_size = 4096;
    my_metadata_t *metadata = (my_metadata_t *)mmap_from_system(buffer_size);
    metadata->size = buffer_size - sizeof(my_metadata_t);
    metadata->next = NULL;
    // Add the memory region to the free list.
    my_add_to_free_list(metadata);
    // Now, try my_malloc() again. This should succeed.
    return my_malloc(size);
  }

  // |ptr| is the beginning of the allocated object.
  //
  // ... | metadata | object | ...
  //     ^          ^
  //     metadata   ptr
  void *ptr = metadata + 1;
  size_t remaining_size = metadata->size - size;
  metadata->size = size;
  // Remove the free slot from the free list.
  my_remove_from_free_list(metadata, prev);

  if (remaining_size > sizeof(my_metadata_t)) {
    // Create a new metadata for the remaining free slot.
    //
    // ... | metadata | object | metadata | free slot | ...
    //     ^          ^        ^
    //     metadata   ptr      new_metadata
    //                 <------><---------------------->
    //                   size       remaining size
    my_metadata_t *new_metadata = (my_metadata_t *)((char *)ptr + size);
    new_metadata->size = remaining_size - sizeof(my_metadata_t);
    new_metadata->next = NULL;
    // Add the remaining free slot to the free list.
    my_add_to_free_list(new_metadata);
  }
  return ptr;
}

// This is called every time an object is freed.  You are not allowed to
// use any library functions other than mmap_from_system / munmap_to_system.
void my_free(void *ptr) {
  // Look up the metadata. The metadata is placed just prior to the object.
  //
  // ... | metadata | object | ...
  //     ^          ^
  //     metadata   ptr
  my_metadata_t *metadata = (my_metadata_t *)ptr - 1;
  // Add the free slot to the free list.
  my_add_to_free_list(metadata);
}

// This is called at the end of each challenge.
void my_finalize() {
  // Nothing is here for now.
  // feel free to add something if you want!
}

void test() {
  // Implement here!
  assert(1 == 1); /* 1 is 1. That's always true! (You can remove this.) */
}
