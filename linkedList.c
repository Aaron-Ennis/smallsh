/* 
 * Name: Aaron Ennis
 * Email: ennisa@oregonstate.edu
 * Last modified: 08 October 2020
 * Description: This is the implementation file for a linked list that uses
 * front and back sentinels. This file provides implementation as a deque
 * ADT.
 * NOTE: This is a slightly modified version of a linked list solution I built
 * for an assignment in CS261 - Data Structures.
 */


#include "linkedList.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

// Double link
struct Link
{
    TYPE value;
    struct Link* next;
    struct Link* prev;
};

// Double linked list with front and back sentinels
struct LinkedList
{
	int size;
	struct Link* frontSentinel;
	struct Link* backSentinel;
};

// Struct to support an iterator
struct Iterator {
   struct LinkedList* list;
   struct Link* currentLink;
};

/**
 * Allocates the list's sentinel and sets the size to 0.
 * The sentinels' next and prev should point to eachother or NULL
 * as appropriate.
 */
static void init(struct LinkedList* list)
{
    // allocate memory for front and back sentinels
    list->frontSentinel = (struct Link *) malloc(sizeof(struct Link));
    assert(list->frontSentinel != 0);
    list->backSentinel = (struct Link *) malloc(sizeof(struct Link));
    assert(list->backSentinel != 0);
    
    // initialize next and prev values of sentinels
    list->frontSentinel->next = list->backSentinel;
    list->frontSentinel->prev = 0;
    list->backSentinel->prev = list->frontSentinel;
    list->backSentinel->next = 0;
    
    // set the size of the initial linked list to zero
    list->size = 0;
}

/**
 * Adds a new link with the given value before the given link and
 * increments the list's size.
 */
static void addLinkBefore(struct LinkedList* list, struct Link* link, TYPE value)
{
    // allocate memory for the new Link
    struct Link * newLink = (struct Link *) malloc(sizeof(struct Link));
    assert(newLink != 0);
    
    /* Put the given value into the new link */
    newLink->value = value;
    
    /* Now insert the new link before the given link by setting the new link's
     * next pointer to the given link and the new link's prev pointer to
     * whatever the given link was pointing back at. */
    newLink->next = link;
    newLink->prev = link->prev;
    
    /* The link behind the given link now needs to point forward at the new
     * link. We access the link behind with link->prev, then set its next value
     * to the new link */
    link->prev->next = newLink;
    
    /* The given link now needs to point back at the new link instead of what
     * it was pointing back at. */
    link->prev = newLink;
    
    /* Lastly, increase the size of the linked list */
    list->size++;
}

/**
 * Removes the given link from the list and
 * decrements the list's size.
 */
static void removeLink(struct LinkedList* list, struct Link* link)
{
    /* Create a temporary "copy" of the given link to hold the link while we
     * detach it */
    struct Link * tempLink = link;
    
    /* The link behind the given link needs to point foward to the link in
     * front of the given link. */
    link->prev->next = link->next;
    
    /* The link in front of the given link needs to point back to the link
     * behind the given link. */
    link->next->prev = link->prev;
    
    /* The link is now detached, but we still know where it is in memory via
     * tempLink, so we can use tempLink to free up the memory used for the
     * given link. */
    free(tempLink);
    
    /* Lastly, decrease the size of the linked list */
    list->size--;
}

/**
 * Allocates and initializes a list.
 */
struct LinkedList* linkedListCreate()
{
	struct LinkedList* newDeque = malloc(sizeof(struct LinkedList));
	init(newDeque);
	return newDeque;
}

int linkedListSize(struct LinkedList* list) {
    return list->size;
}

/**
 * Deallocates every link in the list including the sentinels,
 * and frees the list itself.
 */
void linkedListDestroy(struct LinkedList* list)
{
	while (!linkedListIsEmpty(list))
	{
		linkedListRemoveFront(list);
	}
	free(list->frontSentinel);
	free(list->backSentinel);
	free(list);
}

/**
 * Adds a new link with the given value to the front of the deque.
 */
void linkedListAddFront(struct LinkedList* list, TYPE value)
{
    /* To add to the front of the linked list, we are going to insert a new
     * link before the link currently in front of the front sentinel. */
    addLinkBefore(list, list->frontSentinel->next, value);
}

/**
 * Adds a new link with the given value to the back of the deque.
 */
void linkedListAddBack(struct LinkedList* list, TYPE value)
{
    assert(!linkedListIsEmpty(list));
    
    /* To add to the back of the linked list, we are going to insert a new
     * link before the back sentinel. */
    addLinkBefore(list, list->backSentinel, value);
}

/**
 * Returns the value of the link at the front of the deque.
 */
TYPE linkedListFront(struct LinkedList* list)
{
    assert(!linkedListIsEmpty(list));
    return list->frontSentinel->next->value;
}

/**
 * Returns the value of the link at the back of the deque.
 */
TYPE linkedListBack(struct LinkedList* list)
{
    assert(!linkedListIsEmpty(list));
    return list->backSentinel->prev->value;
}

/**
 * Removes the link at the front of the deque.
 */
void linkedListRemoveFront(struct LinkedList* list)
{
    assert(!linkedListIsEmpty(list));
    removeLink(list, list->frontSentinel->next);
}

/**
 * Removes the link at the back of the deque.
 */
void linkedListRemoveBack(struct LinkedList* list)
{
    assert(!linkedListIsEmpty(list));
    removeLink(list, list->backSentinel->prev);
}

/**
 * Returns 1 if the deque is empty and 0 otherwise.
 */
int linkedListIsEmpty(struct LinkedList* list)
{
    if (list->size == 0) {
        return 1;
    } else {
        return 0;
    }
}


/* Intialize the iterator */
void iteratorInit (struct LinkedList* list, struct Iterator* iterator) 
{
   iterator->list = list;
   iterator->currentLink = list->frontSentinel;
}

/* Create a new iterator and return it */
struct Iterator* createIterator(struct LinkedList* list) {
    struct Iterator *newIterator = malloc(sizeof (struct Iterator));
    iteratorInit(list, newIterator);
    return(newIterator);
}

/* Return 0 if we have reached the end of the list. */
int iteratorHasNext (struct Iterator* iterator) 
{
    if (iterator->currentLink->next == iterator->list->backSentinel) {
        return 0;
    } else {
        return 1;
    }
}

/* Return the value of the next link in the list. */
TYPE iteratorNext (struct Iterator* iterator)
{
    iterator->currentLink = iterator->currentLink->next;
    return iterator->currentLink->value;
}

/* This is a separate function to remove links using the iterator. */
void iteratorRemove (struct Iterator* iterator)
{
    struct Link* tempLink = iterator->currentLink;
    iterator->currentLink = iterator->currentLink->prev;
    removeLink(iterator->list, tempLink);
}

void iteratorDestroy(struct Iterator* iterator) {
    free(iterator);
}