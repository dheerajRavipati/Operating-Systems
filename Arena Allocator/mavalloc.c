/*

  Details of group:

    1. Naga Dheeraj Ravipati ( 1002032126 ) : CSE 3320 - 003
    2. Prudhvi Kumar Medicharla ( 1002027482 ) : CSE 3320 - 001

*/


// The MIT License (MIT)
// 
// Copyright (c) 2022 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "mavalloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

//to define predefined constants if node is being used or if it is free
enum TYPE
{
  FREE = 0,
  USED
};

//Linked list structure for node properties 
struct Node {
  size_t size;
  enum TYPE type;
  void * arena;
  struct Node * next;
  struct Node * prev;
};

//globals for allocation list and last accessed node
struct Node *alloc_list;
struct Node *previous_node;


//Arena pointer to track memory
void * arena;

//global variable, allocation_algorithm, for determining the algorithm in use
enum ALGORITHM allocation_algorithm = FIRST_FIT;

//Mavalloc_init function to use malloc to allocate a pool of memory that is size bytes long.

int mavalloc_init( size_t size, enum ALGORITHM algorithm )
{   
  //If the size parameter is less than zero, it will return -1.
  if( size < 0 ) 
  {
    return -1;
  }

  //Memory allocation using ALIGN4 macro, provided by professor in mavalloc.h
  //This will ensure size is 4 byte long.
  arena = malloc( ALIGN4( size ) );

  // If allocation fails return -1
  if( arena == NULL )
  {
    return -1;
  }
    
  allocation_algorithm = algorithm;

  alloc_list = ( struct Node * )malloc( sizeof( struct Node ));

  alloc_list -> arena = arena;
  alloc_list -> size  = ALIGN4(size);
  alloc_list -> type  = FREE;
  alloc_list -> next  = NULL;
  alloc_list -> prev  = NULL;

  previous_node  = alloc_list;

  return 0;
}

//Function to assign leftover space in memory allocation algorithms
void assign_leftover( struct Node * node, size_t size, size_t aligned_size ){
  int leftover_size = 0; // leftover_size to determine the size of a node that is leftover,
                         // after a process is allocated to a node which is less than node size.
  
  node -> type  = USED; // FREE node is marked as USED after a process is allocated to it.
  leftover_size = node -> size - aligned_size;
  node -> size =  aligned_size;

  //If there is any leftover space after a process is allocated to the node, the arena
  //pointer will be pointing to start of leftover space and a new node with leftover space
  //will be created.
  if( leftover_size > 0 )
  {
    struct Node * previous_next = node -> next;
    struct Node * leftover_node = ( struct Node * ) malloc ( sizeof( struct Node ));    

    leftover_node -> arena = node -> arena + size;
    leftover_node -> type  = FREE;
    leftover_node -> size  = leftover_size;
    leftover_node -> next  = previous_next;

    node -> next = leftover_node;
  }
  previous_node = node;
}

// Function to find node with maximum size to implement in worst fit
void find_maxNode( struct Node * maxNode, struct Node * node )
{
  // This will iterate through all available/free nodes and check for any node
  // that is greater than maximum node and assign that value to 'maxNode' variable.
  while ( node -> next != NULL )
  {
    node = node -> next;
    if ( node -> size > maxNode -> size && node -> type == FREE )
    {
      maxNode = node;
    }
  }
}


//mavalloc_destroy() function to free the allocated arena and empty the linked list
void mavalloc_destroy( )
{
  //To free the allocated arena
  free( arena );
  
  // Iterate over the linked list and free the nodes
  struct Node *list;

  while( alloc_list != NULL ) 
  {
    list = alloc_list;
    alloc_list = alloc_list -> next;
    free( list );
  } 

  return;
}

// mavalloc_alloc function will allocate size bytes from preallocated memory arena using the
// heap allocation algorithm that was specified during mavalloc_init. 
// This function returns a pointer to the memory on success and NULL on failure. 
void * mavalloc_alloc( size_t size )
{
  struct Node * node;

  // If allocation algorithm is not Next fit, then check entire list of nodes
  // else start from previous node where process is allocated before. 
  if( allocation_algorithm != NEXT_FIT )
  { 
    node = alloc_list;
  }

  else if ( allocation_algorithm == NEXT_FIT )
  {
    node = previous_node;
  }

  // Print error if algorithm is other than first fit, next fit, worst fit and best fit
  else
  {
    printf("ERROR: Unknown allocation algorithm!\n");
    exit(0);
  }

  size_t aligned_size = ALIGN4( size );

  // If allocation algorithm is first fit.
  if( allocation_algorithm == FIRST_FIT )
  {
    // Assign process to first available free node, whose size is greater than the process size
    while( node )
    {

      if( node -> size >= aligned_size  && node -> type == FREE )
      {
        assign_leftover(node, size, aligned_size);
        previous_node = node;
        return ( void * ) node -> arena;
      }
      node = node -> next;
    }
  }

  // If allocation algorithm is next fit  
  else if (allocation_algorithm == NEXT_FIT)
  {
    // Same as first fit algorithm, but we check nodes starting from previously assigned node
    // and return arena
    while ( node )
    {
      if ( node -> size >= aligned_size && node -> type == FREE )
      {
        assign_leftover( node, size, aligned_size );
        previous_node = node;
        return node -> arena;
      }
      node = node -> next;

      if ( node == previous_node ) 
        break;

      if ( node == NULL )
        node = alloc_list;
    }

  }

  // If allocation algorithm is Worst Fit
  if ( allocation_algorithm == WORST_FIT )
  {
    // In this algorithm, we check for maximum available node among the list 
    // and assign process to it.
    struct Node *maxNode;

    while(node)
    {
      if ( node -> size >= aligned_size && node -> type == FREE )
      {
        maxNode = node;
        find_maxNode( maxNode, node );
        node = maxNode;

        assign_leftover( node, size, aligned_size );
        previous_node = node;
        return node -> arena;
      }
      node = node -> next;
    }
  }

  // If allocation algorithm is Best Fit
  if ( allocation_algorithm == BEST_FIT )
  {
    // we check for the node that the process can be best fit in
    struct Node *best_node = NULL;
    int best_size = INT_MAX;

    // To find best node 
    while ( node )
    {
      if (node -> size >= aligned_size && node -> type == FREE && 
          best_size > (node -> size - size)) 
      {
        best_size = node -> size - size;
        best_node = node;
      }
      node = node -> next;
    }

    if ( best_node != NULL )
    {
      assign_leftover( best_node, size, aligned_size );
      previous_node = best_node;
      return best_node -> arena;
    }
  }

  return NULL;
}

// This function will free the block pointed by the pointer back to preallocated memory arena.
// This function returns no value.  
void mavalloc_free( void * ptr )
{
  
  struct Node *node = alloc_list;

  // check and update the node to free if pointer is found
  while ( node )
  {
    if ( node -> arena == ptr ) {
      node -> type = FREE;
    }

    node = node -> next;
  }

  node = alloc_list;
  
  // check all the nodes and combine the consecutive free nodes
  while ( node )
  {
    if ( node -> type == FREE && node -> next && node -> next -> type == FREE )
    {
      node -> size += node -> next -> size;
      node -> next = node -> next -> next;
      free( node -> next );
      continue;
    }

    node = node -> next;
  }

  return;
}


// mavalloc_size() to return the number of nodes in the memory area
int mavalloc_size( )
{
  int number_of_nodes = 0;
  struct Node * ptr = alloc_list;

  while( ptr )
  {
    number_of_nodes ++;
    ptr = ptr -> next; 
  }

  return number_of_nodes;
}
