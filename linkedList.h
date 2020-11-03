/* 
 * Name: Aaron Ennis
 * Email: ennisa@oregonstate.edu
 * Last modified: 31 October 2020
 * Description: This is the declaration/interface file for a linked list that
 * uses front and back sentinels. This file provides implementation as a deque
 * abstract data type.
* NOTE: This is a slightly modified version of a linked list solution I built
 * for an assignment in CS261 - Data Structures.
 */
#include "command.h"
#ifndef LINKED_LIST_H
#define LINKED_LIST_H

/* When implementing, this macro can be modified to use whatever 
 * type is required for the implementation.
 */
#ifndef TYPE
#define TYPE struct Command*
#endif

struct LinkedList;

struct LinkedList* linkedListCreate();
void linkedListDestroy(struct LinkedList* list);
int linkedListSize(struct LinkedList* list);

// Deque interface

int linkedListIsEmpty(struct LinkedList* list);
void linkedListAddFront(struct LinkedList* list, TYPE value);
void linkedListAddBack(struct LinkedList* list, TYPE value);
TYPE linkedListFront(struct LinkedList* list);
TYPE linkedListBack(struct LinkedList* list);
void linkedListRemoveFront(struct LinkedList* list);
void linkedListRemoveBack(struct LinkedList* list);

// List iterator

struct Iterator;

struct Iterator* createIterator(struct LinkedList* list);
void iteratorInit(struct LinkedList* list, struct Iterator* iterator);
int iteratorHasNext(struct Iterator* iterator);
TYPE iteratorNext(struct Iterator* iterator);
void iteratorRemove(struct Iterator* iterator);
void iteratorDestroy(struct Iterator* iterator);

#endif