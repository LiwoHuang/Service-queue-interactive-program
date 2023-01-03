

/*
* file:  sq.c
* description:  specifies the interface for
*  	the "service queue" ADT.
*/
#include <stdio.h>
#include <stdlib.h>

#include "sq.h"

typedef int ElemType;
#define FORMAT " %i "
#define DEFAULT 0

// hidden implementation of list_struct
typedef struct list_struct LIST;

typedef struct node {
    ElemType val;
    struct node *next;
    struct node *prev;

} NODE;

struct list_struct {
    NODE *front;
    NODE *back;
};

/**
* SQ is an "incompletely specified type."  
*
* The definition of struct service_queue must
* be (hidden) in an implementation .c file.
*
* This has two consequences:
*
*	- Different implementations are free to
*	 	specify the structure as suits
*		their approach.
*
*	- "client" code can only have pointers
*		to service_queue structures (type
*		SQ *).  More importantly, it cannot
*		de-reference the pointers.  This
*		lets us enforce one of the principles
*		of ADTs:  the only operations that
*		can be performed on a service queue
*		are those specified in the interface
*		-- i.e., the functions specified below.
*/
struct service_queue {
  LIST * the_queue;
  LIST * buzzer_bucket;
  NODE* arr[1000000];
  int on_queue[1000000];
  int length;
  int buzzer_num;
};

LIST *lst_create() {
LIST *l = malloc(sizeof(LIST));

  l->front = NULL;
  l->back = NULL;
  return l;
}

void lst_free(LIST *l) {
NODE *p = l->front;
NODE *pnext;

  while(p != NULL) {
    pnext = p->next;   // keeps us from de-referencing a freed ptr
    free(p);
    p = pnext;
  }
  // now free the LIST 
  free(l);
}

void lst_print(LIST *l) {
NODE *p = l->front;

  printf("[");
  while(p != NULL) {
    printf(FORMAT, p->val);
    p = p->next;
  }
  printf("]\n");
}

void lst_push_front(LIST *l, ElemType val) {
NODE *p = malloc(sizeof(NODE));

  p->val = val;
  p->next = l->front;
  p->prev = NULL;

  l->front = p;
  if (l->back == NULL) {// was empty, now one elem
    l->back = p;
  }
   // was empty, now one elem
     
}

void lst_push_back(LIST *l, ElemType val) {
  NODE *p;

  if (l->back == NULL) // list empty - same as push_front
    lst_push_front(l, val);
  else { // at least one element before push
    p = malloc(sizeof(NODE));
    p->val = val;
    p->next = NULL;
    p->prev = l->back;
    l->back->next = p;

    l->back = p;
  }

}

int lst_length(LIST *l) {
  
  NODE *p = l->front;
  if(p==NULL){
    return 0;
  } else {
    LIST *temp = malloc(sizeof(LIST));
    temp->front = p->next;
    temp->back = l->back;
    return 1 + lst_length(temp);
  }
}

int lst_is_empty(LIST *l) {
  return l->front == NULL;
}

ElemType lst_pop_front(LIST *l) {
ElemType ret;
NODE *p;
 

  if(lst_is_empty(l))
	return DEFAULT;   // no-op

  ret = l->front->val;
  
  if(l->front == l->back) {  // one element
	free(l->front);
	l->front = NULL;
	l->back = NULL;
  }
  else {
	p = l->front;  // don't lose node being deleted
	l->front = l->front->next;  // hop over
  l->front->prev = NULL;
	free(p);
  }
  return ret;
}

/**
* Function: sq_create()
* Description: creates and intializes an empty service queue.
* 	It is returned as an SQ pointer.
* 
* RUNTIME REQUIREMENT: O(1)
*/
SQ * sq_create() {
  SQ *q = malloc(sizeof(SQ));
  q->the_queue = lst_create();
  q->buzzer_bucket = lst_create();
  q->length = 0;
  q->buzzer_num = 0;

  for (int i = 0; i < 1000000; i++){
    q->on_queue[i] = 0;
  }
  
  return q;
}

/**
* Function: sq_free()
* Description:  deallocates all memory assciated
*   with service queue given by q.
*
* RUNTIME REQUIREMENT:  O(N_b) where N_b is the number of buzzer 
*	IDs that have been used during the lifetime of the
*	service queue; in general, at any particular instant
*	the actual queue length may be less than N_b.
*
*	[See discussion of "re-using buzzers" below]
*
*/
void sq_free(SQ *q) {
  lst_free(q->the_queue);
  lst_free(q->buzzer_bucket);
  free(q);
}

/**
* Function: sq_display()
* Description:  prints the buzzer IDs currently
*    in the queue from front to back.
*
* RUNTIME REQUIREMENT:  O(N)  (where N is the current queue
*		length).
*/
void sq_display(SQ *q) {
  printf("current-queue contents:\n    ");
  lst_print(q->the_queue);
  printf("\n");
}

/**
* Function: sq_length()
* Description:  returns the current number of
*    entries in the queue.
*
* RUNTIME REQUIREMENT:  O(1)
*/
int sq_length(SQ *q) {
  return q->length;
}

/**
* Function: sq_give_buzzer()
* Description:  This is the "enqueue" operation.  For us
*    a "buzzer" is represented by an integer (starting
*    from zero).  The function selects an available buzzer 
*    and places a new entry at the end of the service queue 
*    with the selected buzer-ID. 
*    This buzzer ID is returned.
*    The assigned buzzer-ID is a non-negative integer 
*    with the following properties:
*
*       (1) the buzzer (really it's ID) is not currently 
*         taken -- i.e., not in the queue.  (It
*         may have been in the queue at some previous
*         time -- i.e., buzzer can be re-used).
*	  This makes sense:  you can't give the same
*	  buzzer to two people!
*
*       (2) If there are buzzers that can be re-used, one
*         of those buzzers is used.  A re-usable buzzer is 
*	  a buzzer that _was_ in the queue at some previous
*	  time, but currently is not.
*
*       (3) if there are no previously-used buzzers, the smallest 
*         possible buzzer-ID is used (retrieved from inventory).  
*	  Properties in this situation (where N is the current
*	  queue length):
*
*		- The largest buzzer-ID used so far is N-1
*
*		- All buzzer-IDs in {0..N-1} are in the queue
*			(in some order).
*
*		- The next buzzer-ID (from the basement) is N.
*
*    In other words, you can always get more buzzers (from
*    the basement or something), but you don't fetch an
*    additional buzzer unless you have to.
*    you don't order new buzzers 
*
* Comments/Reminders:
*
*	Rule (3) implies that when we start from an empty queue,
*	the first buzzer-ID will be 0 (zero).
*
*	Rule (2) does NOT require that the _minimum_ reuseable 
*	buzzer-ID be used.  If there are multiple reuseable buzzers, 
*	any one of them will do.
*	
*	Note the following property:  if there are no re-useable 
*	buzzers, the queue contains all buzzers in {0..N-1} where
*       N is the current queue length (of course, the buzzer IDs 
*	may be in any order.)
*
* RUNTIME REQUIREMENT:  O(1)  ON AVERAGE or "AMORTIZED"  
		In other words, if there have been M calls to 
*		sq_give_buzzer, the total time taken for those 
*		M calls is O(M).
*
*		An individual call may therefore not be O(1) so long
*		as when taken as a whole they average constant time.
*
*		(Hopefully this reminds you of an idea we employed in
*		the array-based implementation of the stack ADT).
*/
int  sq_give_buzzer(SQ *q) {
  int buzzer;

  if(!lst_is_empty(q->buzzer_bucket)) {
    buzzer = lst_pop_front(q->buzzer_bucket);   
    
    lst_push_back(q->the_queue, buzzer);
    
    
    q->arr[buzzer] = q->the_queue->back;
    
    q->length++;
    q->on_queue[buzzer] = 1;
    return buzzer;
  }
  else {
    /*  invariant:  
        if no re-useable buzzers, the buzzers 
        in the queue are {0,1,2,...,N-1} where
        N is the queue length.

        Thus, the smallest available new buzzer 
        is N
        */
    buzzer = sq_length(q);
    
    lst_push_back(q->the_queue, buzzer);
    
    
    q->arr[buzzer] = q->the_queue->back;
    
    q->length++;
    q->on_queue[buzzer] = 1;
    q->buzzer_num++;
    return buzzer;
  }
}

/**
* function: sq_seat()
* description:  if the queue is non-empty, it removes the first 
*	 entry from (front of queue) and returns the 
*	 buzzer ID.
*	 Note that the returned buzzer can now be re-used.
*
*	 If the queue is empty (nobody to seat), -1 is returned to
*	 indicate this fact.
*
* RUNTIME REQUIREMENT:  O(1)
*/
int sq_seat(SQ *q) {
int buzzer;

	if(lst_is_empty(q->the_queue))
	  return -1;
	else{
	  buzzer = lst_pop_front(q->the_queue);
	  
    lst_push_back(q->buzzer_bucket, buzzer);
    
    
    q->arr[buzzer] = q->buzzer_bucket->back;
    
    q->length--;
    q->on_queue[buzzer] = 0;
	  return buzzer;
	}
} 


/**
* function: sq_kick_out()
*
* description:  Some times buzzer holders cause trouble and
*		a bouncer needs to take back their buzzer and
*		tell them to get lost.
*
*		Specifially:
*
*		If the buzzer given by the 2nd parameter is 
*		in the queue, the buzzer is removed (and the
*		buzzer can now be re-used) and 1 (one) is
*		returned (indicating success).
*
*		If the buzzer isn't actually currently in the
*		queue, the queue is unchanged and 0 is returned
*		(indicating failure).
*
* RUNTIME REQUIREMENT:  O(1)
*/
int sq_kick_out(SQ *q, int buzzer) {
  NODE *temp;
  if (q->on_queue[buzzer] == 1) {

    if (q->the_queue->front == q->arr[buzzer]) {
      if (q->the_queue->back == q->arr[buzzer]) {
        
        temp = q->arr[buzzer];
        
        lst_push_back(q->buzzer_bucket, buzzer);
        
        
        q->arr[buzzer] = q->buzzer_bucket->back;
        
        q->length--;
        q->on_queue[buzzer] = 0;
        q->the_queue->front = NULL;
        q->the_queue->back = NULL;
        free(temp);
        return 1;
        
      } else {

        temp = q->arr[buzzer];
        q->the_queue->front->next->prev = NULL;
        q->the_queue->front = q->the_queue->front->next;
        
        lst_push_back(q->buzzer_bucket, buzzer);
        
        
        q->arr[buzzer] = q->buzzer_bucket->back;
        
        q->length--;
        q->on_queue[buzzer] = 0;
        free(temp);
        return 1;
      }
      
    } else if (q->the_queue->back == q->arr[buzzer]) {

      temp = q->arr[buzzer];
      q->the_queue->back->prev->next = NULL;
      q->the_queue->back = q->the_queue->back->prev;
      
      lst_push_back(q->buzzer_bucket, buzzer);
      
      
      q->arr[buzzer] = q->buzzer_bucket->back;
      
      q->length--;
      q->on_queue[buzzer] = 0;
      free(temp);
      return 1;
    } else {

      temp = q->arr[buzzer];
      q->arr[buzzer]->prev->next = q->arr[buzzer]->next;
      q->arr[buzzer]->next->prev = q->arr[buzzer]->prev;
      
      lst_push_back(q->buzzer_bucket, buzzer);
      
      
      q->arr[buzzer] = q->buzzer_bucket->back;
      
      q->length--;
      q->on_queue[buzzer] = 0;
      free(temp);
      return 1;
    }
    
  } else {
    return 0;
  }
  
}

/**
* function:  sq_take_bribe()
* description:  some people just don't think the rules of everyday
*		life apply to them!  They always want to be at
*		the front of the line and don't mind bribing
*		a bouncer to get there.
*
*	        In terms of the function:
*
*		  - if the given buzzer is in the queue, it is 
*		    moved from its current position to the front
*		    of the queue.  1 is returned indicating success
*		    of the operation.
*		  - if the buzzer is not in the queue, the queue 
*		    is unchanged and 0 is returned (operation failed).
*
* RUNTIME REQUIREMENT:  O(1)
*/
int sq_take_bribe(SQ *q, int buzzer) {
  NODE *temp;
  /* remove buzzer then push it on front */
  if(q->on_queue[buzzer] == 1) {
    if (q->the_queue->front == q->arr[buzzer]){
      return 1;
    } else if (q->the_queue->back == q->arr[buzzer]) { 
      temp = q->arr[buzzer];
      q->the_queue->back->prev->next = NULL;
      q->the_queue->back = q->the_queue->back->prev;
      
      lst_push_front(q->the_queue, buzzer);
      
      
      q->arr[buzzer] = q->the_queue->front;
      
      q->arr[buzzer]->next->prev = q->arr[buzzer];
      q->on_queue[buzzer] = 1;
      free(temp);
      return 1;
      
    } else {
      temp = q->arr[buzzer];
      q->the_queue->front->prev = q->arr[buzzer];
      q->arr[buzzer]->prev->next = q->arr[buzzer]->next;
      q->arr[buzzer]->next->prev = q->arr[buzzer]->prev;
      
      lst_push_front(q->the_queue, buzzer);
      
      
      q->arr[buzzer] = q->the_queue->front;
      
      q->arr[buzzer]->next->prev = q->arr[buzzer];
      q->on_queue[buzzer] = 1;
      free(temp);
      return 1;
    }
    
  } else {
    /* person has to be in line to offer a bribe */
    return 0;
  }
}




