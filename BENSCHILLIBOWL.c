#include "BENSCHILLIBOWL.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>

bool IsEmpty(BENSCHILLIBOWL* bcb);
bool IsFull(BENSCHILLIBOWL* bcb);
void AddOrderToBack(Order **orders, Order *order);

MenuItem BENSCHILLIBOWLMenu[] = { 
    "BensChilli", 
    "BensHalfSmoke", 
    "BensHotDog", 
    "BensChilliCheeseFries", 
    "BensShake",
    "BensHotCakes",
    "BensCake",
    "BensHamburger",
    "BensVeggieBurger",
    "BensOnionRings",
};
int BENSCHILLIBOWLMenuLength = 10;

/* Select a random item from the Menu and return it */
MenuItem PickRandomMenuItem() {
    return BENSCHILLIBOWLMenu[rand() % BENSCHILLIBOWLMenuLength];
}

/* Allocate memory for the Restaurant, then create the mutex and condition variables needed to instantiate the Restaurant */

BENSCHILLIBOWL* OpenRestaurant(int max_size, int expected_num_orders) {
  // First allocate memory for the restaurant.
  BENSCHILLIBOWL *bcb = (BENSCHILLIBOWL*) malloc(sizeof(BENSCHILLIBOWL));
  bcb->current_size = 0;
  bcb->max_size = max_size;
  bcb->orders = NULL;
  bcb->next_order_number = 1;
  bcb->orders_handled = 0;
  bcb->expected_num_orders = expected_num_orders;
  
  // Now, create the mutex, and condition variables.
  pthread_mutex_init(&(bcb->mutex), NULL);
  pthread_cond_init(&(bcb->can_add_orders), NULL);
  pthread_cond_init(&(bcb->can_get_orders), NULL);
  
  printf("Restaurant is open!\n");
  return bcb;
}


/* check that the number of orders received is equal to the number handled (ie.fullfilled). Remember to deallocate your resources */

void CloseRestaurant(BENSCHILLIBOWL* bcb) {
  // check that the number of orders received is equal to the number handled (ie.fullfilled)
  if (bcb->expected_num_orders != bcb->orders_handled) {
    // deallocate resources first.
    pthread_mutex_destroy(&(bcb->mutex));
    free(bcb);
    // Throw error if not all orders were handled.    
    fprintf(stderr, "Number of orders handled is not as expected.\n");
    exit(0);
  }

  // Deallocate resources now.
  pthread_mutex_destroy(&(bcb->mutex));
  free(bcb);
  
  printf("Restaurant is closed!\n");
}

/* add an order to the back of queue */
int AddOrder(BENSCHILLIBOWL* bcb, Order* order) {
  // lock mutex before changes.
  pthread_mutex_lock(&(bcb->mutex));
  
  // Wait until order can be added if queue is full.
  while (IsFull(bcb)) {
    pthread_cond_wait(&(bcb->can_add_orders), &(bcb->mutex));
  }
  
  // Now, add order to the back of the queue. 
  order->order_number = bcb->next_order_number;
  AddOrderToBack(&(bcb->orders), order);
  
  // update the size and next order number of the restaurant. 
  bcb->next_order_number += 1;
  bcb->current_size += 1;
  
  // Broadcast to the cond that orders are available.
  pthread_cond_broadcast(&(bcb->can_get_orders));
  
  // unlock mutex.
  pthread_mutex_unlock(&(bcb->mutex));
  
  return order->order_number;
}

/* remove an order from the queue */
Order *GetOrder(BENSCHILLIBOWL* bcb) {
  // lock mutex before changes.
  pthread_mutex_lock(&(bcb->mutex));
  
  // Wait until orders are available if queue is empty.
  while (IsEmpty(bcb)) {
    // Check for this condition here to prevent cooks getting stuck at empty queue.
    if (bcb->orders_handled >= bcb->expected_num_orders) {
      pthread_mutex_unlock(&(bcb->mutex));
      return NULL;
    }
    pthread_cond_wait(&(bcb->can_get_orders), &(bcb->mutex));
  }
  
  // Fetch order from queue and update necessary info. 
  Order* order = bcb->orders;
  bcb->orders = bcb->orders->next;
  
  bcb->current_size -= 1;
  bcb->orders_handled += 1;
  
  // Broadcast to the cond that orders can be added. 
  pthread_cond_broadcast(&(bcb->can_add_orders));
  
  // unlock mutex. 
  pthread_mutex_unlock(&(bcb->mutex));
  return order;
}

// Optional helper functions (you can implement if you think they would be useful)
bool IsEmpty(BENSCHILLIBOWL* bcb) {
  if (bcb->current_size > 0) {
    return false;
  } else {
    return true;
  }
}

bool IsFull(BENSCHILLIBOWL* bcb) {
  if (bcb->current_size == bcb->max_size) {
    return true;
  } else {
    return false;
  }
}

/* this methods adds order to rear of queue */
void AddOrderToBack(Order **orders, Order *order) {
  if (*orders != NULL){
    Order* curr_order = *orders;
    while (curr_order->next){
      curr_order = curr_order->next;
    }
    curr_order->next = order;
  } else {
    *orders = order;
  }
}

