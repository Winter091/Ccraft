/*

    My simple linked list implementation using 
    a lot of preprocessor shit

*/

#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include "stdlib.h"

#define LINKEDLIST_DEFINITION(TYPE, TYPENAME)\
\
typedef struct LinkedListNode_##TYPENAME\
{\
    TYPE data;\
    struct LinkedListNode_##TYPENAME* ptr_next;\
}\
LinkedListNode_##TYPENAME;\
\
typedef struct\
{\
    LinkedListNode_##TYPENAME *head, *tail;\
    size_t size;\
}\
LinkedList_##TYPENAME;\
\
LinkedList_##TYPENAME* list_##TYPENAME ##_create();\
void list_##TYPENAME ##_push_back(LinkedList_##TYPENAME* list, TYPE elem);\
void list_##TYPENAME ##_push_front(LinkedList_##TYPENAME* list, TYPE elem);\
TYPE list_##TYPENAME ##_pop_front(LinkedList_##TYPENAME* list);\
int  list_##TYPENAME ##_remove(LinkedList_##TYPENAME* list, TYPE elem);\
int  list_##TYPENAME ##_contains(LinkedList_##TYPENAME* list, TYPE elem);\
TYPE list_##TYPENAME ##_get(LinkedList_##TYPENAME* list, size_t index);\
TYPE list_##TYPENAME ##_front(LinkedList_##TYPENAME* list);\
TYPE list_##TYPENAME ##_back(LinkedList_##TYPENAME* list);\
void list_##TYPENAME ##clear(LinkedList_##TYPENAME* list);\
void list_##TYPENAME ##_delete(LinkedList_##TYPENAME* list);\

#define LINKEDLIST_IMPLEMENTATION(TYPE, TYPENAME)\
\
LinkedList_##TYPENAME* list_##TYPENAME ##_create()\
{\
    LinkedList_##TYPENAME* list = malloc(sizeof(LinkedList_##TYPENAME));\
    list->head = NULL;\
    list->tail = NULL;\
    list->size = 0;\
    return list;\
}\
\
void list_##TYPENAME ##_push_back(LinkedList_##TYPENAME* list, TYPE elem)\
{\
    LinkedListNode_##TYPENAME* node = malloc(sizeof(LinkedListNode_##TYPENAME));\
    node->data = elem;\
    node->ptr_next = NULL;\
\
    if (!list->head)\
    {\
        list->head = node;\
        list->tail = node;\
    }\
\
    else\
    {\
        LinkedListNode_##TYPENAME* prev_tail = list->tail;\
        list->tail = node;\
        prev_tail->ptr_next = node;\
    }\
\
    list->size++;\
}\
\
void list_##TYPENAME ##_push_front(LinkedList_##TYPENAME* list, TYPE elem)\
{\
    LinkedListNode_##TYPENAME* node = malloc(sizeof(LinkedListNode_##TYPENAME));\
    node->data = elem;\
    node->ptr_next = NULL;\
\
    if (!list->size)\
    {\
        list->head = node;\
        list->tail = node;\
    }\
\
    else\
    {\
        LinkedListNode_##TYPENAME* prev_head = list->head;\
        list->head = node;\
        list->head->ptr_next = prev_head;\
    }\
\
    list->size++;\
}\
\
TYPE list_##TYPENAME ##_pop_front(LinkedList_##TYPENAME* list)\
{\
    if (list->size == 0) return (TYPE)0;\
\
    if (list->size == 1)\
    {\
        TYPE elem = list->head->data;\
        free(list->head);\
        list->head = NULL;\
        list->tail = NULL;\
        list->size = 0;\
        return elem;\
    }\
\
    TYPE elem = list->head->data;\
    LinkedListNode_##TYPENAME* prev_head = list->head;\
    list->head = list->head->ptr_next;\
    list->size--;\
    free(prev_head);\
    return elem;\
}\
\
int list_##TYPENAME ##_remove(LinkedList_##TYPENAME* list, TYPE elem)\
{\
    if (!list->size) return 0;\
\
    if (list->size == 1)\
    {\
        if (list->head->data != elem)\
            return 0;\
\
        free(list->head);\
        list->head = NULL;\
        list->tail = NULL;\
        list->size = 0;\
        return 1;\
    }\
\
    else\
    {\
        if (list->head->data == elem)\
        {\
            LinkedListNode_##TYPENAME* prev_head = list->head;\
            list->head = list->head->ptr_next;\
            list->size--;\
            free(prev_head);\
            return 1;\
        }\
\
        LinkedListNode_##TYPENAME* curr_node = list->head;\
        while (curr_node->ptr_next && curr_node->ptr_next->data != elem)\
            curr_node = curr_node->ptr_next;\
\
        if (!curr_node->ptr_next) return 0;\
\
        if (curr_node->ptr_next == list->tail)\
        {\
            LinkedListNode_##TYPENAME* prev_tail = list->tail;\
            list->tail = curr_node;\
            list->tail->ptr_next = NULL;\
            free(prev_tail);\
        }\
\
        else\
        {\
            LinkedListNode_##TYPENAME* node_to_del = curr_node->ptr_next;\
            curr_node->ptr_next = curr_node->ptr_next->ptr_next;\
            free(node_to_del);\
        }\
\
        list->size--;\
        return 1;\
    }\
}\
\
int list_##TYPENAME ##_contains(LinkedList_##TYPENAME* list, TYPE elem)\
{\
    if (list->size == 0) return 0;\
\
    if (list->head->data == elem || list->tail->data == elem)\
        return 1;\
\
    LinkedListNode_##TYPENAME* curr_node = list->head;\
    while (curr_node && curr_node->data != elem)\
        curr_node = curr_node->ptr_next;\
\
    return curr_node ? 1 : 0;\
}\
\
TYPE list_##TYPENAME ##_get(LinkedList_##TYPENAME* list, size_t index)\
{\
    if (index + 1 >= list->size)\
        return (TYPE)0;\
\
    if (index == 0)\
        return list->head->data;\
\
    else if (index == list->size - 1)\
        return list->tail->data;\
\
    else\
    {\
        LinkedListNode_##TYPENAME* curr_node = list->head;\
        while (index--)\
            curr_node = curr_node->ptr_next;\
\
        return curr_node->data;\
    }\
}\
\
TYPE list_##TYPENAME ##_front(LinkedList_##TYPENAME* list)\
{\
    if (!list->size) return (TYPE)0;\
    return list->head->data;\
}\
\
TYPE list_##TYPENAME ##_back(LinkedList_##TYPENAME* list)\
{\
    if (!list->size) return (TYPE)0;\
    return list->tail->data;\
}\
\
void list_##TYPENAME ##_clear(LinkedList_##TYPENAME* list)\
{\
    LinkedListNode_##TYPENAME* curr_node = list->head;\
\
    while (curr_node)\
    {\
        LinkedListNode_##TYPENAME* next_node = curr_node->ptr_next;\
        free(curr_node);\
        curr_node = next_node;\
    }\
    list->size = 0;\
    list->head = NULL;\
    list->tail = NULL;\
}\
\
void list_##TYPENAME ##_delete(LinkedList_##TYPENAME* list)\
{\
    list_##TYPENAME ##_clear(list);\
    free(list);\
}\

#endif
