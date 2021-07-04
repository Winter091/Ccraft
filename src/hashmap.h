/*

    Very simple generic hashmap

*/

#ifndef HASH_MAP_H_
#define HASH_MAP_H_

#include "stdlib.h"
#include "stdint.h"

#define HASHMAP_DECLARATION(TYPE, TYPENAME)                                  \
                                                                            \
typedef struct LinkedListNodeMap_##TYPENAME                                 \
{                                                                           \
    TYPE data;                                                              \
    struct LinkedListNodeMap_##TYPENAME* ptr_next;                          \
}                                                                           \
LinkedListNodeMap_##TYPENAME;                                               \
                                                                            \
typedef struct                                                              \
{                                                                           \
    LinkedListNodeMap_##TYPENAME *head, *tail;                              \
    size_t size;                                                            \
}                                                                           \
LinkedListMap_##TYPENAME;                                                   \
                                                                            \
typedef struct                                                              \
{                                                                           \
    LinkedListMap_##TYPENAME** array;                                       \
                                                                            \
    size_t array_size;                                                      \
    size_t size;                                                            \
}                                                                           \
HashMap_##TYPENAME;                                                         \
                                                                            \
void hashmap_##TYPENAME ##_insert(HashMap_##TYPENAME* map, TYPE elem);      \
int  hashmap_##TYPENAME ##_remove(HashMap_##TYPENAME* map, TYPE elem);      \
int  hashmap_##TYPENAME ##_contains(HashMap_##TYPENAME* map, TYPE elem);    \
void hashmap_##TYPENAME ##_delete(HashMap_##TYPENAME* map);


#define HASHMAP_IMPLEMENTATION(TYPE, TYPENAME, HASH_FUNC)                               \
                                                                                        \
HashMap_##TYPENAME* hashmap_##TYPENAME ##_create(size_t array_size)                     \
{                                                                                       \
    HashMap_##TYPENAME* map = malloc(array_size * sizeof(HashMap_##TYPENAME));          \
                                                                                        \
    map->array = malloc(array_size * sizeof(LinkedListMap_##TYPENAME*));                \
    for (int i = 0; i < array_size; i++)                                                \
    {                                                                                   \
        LinkedListMap_##TYPENAME* list = malloc(sizeof(LinkedListMap_##TYPENAME));      \
        list->head = NULL;                                                              \
        list->tail = NULL;                                                              \
        list->size = 0;                                                                 \
        map->array[i] = list;                                                           \
    }                                                                                   \
                                                                                        \
    map->array_size = array_size;                                                       \
    map->size = 0;                                                                      \
                                                                                        \
    return map;                                                                         \
}                                                                                       \
                                                                                        \
void hashmap_##TYPENAME ##_insert(HashMap_##TYPENAME* map, TYPE elem)                   \
{                                                                                       \
    uint32_t index = HASH_FUNC(elem) % map->array_size;                                 \
    LinkedListMap_##TYPENAME* list = map->array[index];                                 \
                                                                                        \
    LinkedListNodeMap_##TYPENAME* node = malloc(sizeof(LinkedListNodeMap_##TYPENAME));  \
    node->data = elem;                                                                  \
    node->ptr_next = NULL;                                                              \
                                                                                        \
    if (!list->size)                                                                    \
    {                                                                                   \
        list->head = node;                                                              \
        list->tail = node;                                                              \
    }                                                                                   \
                                                                                        \
    else                                                                                \
    {                                                                                   \
        LinkedListNodeMap_##TYPENAME* prev_head = list->head;                           \
        list->head = node;                                                              \
        list->head->ptr_next = prev_head;                                               \
    }                                                                                   \
                                                                                        \
    list->size++;                                                                       \
    map->size++;                                                                        \
}                                                                                       \
                                                                                        \
int hashmap_##TYPENAME ##_remove(HashMap_##TYPENAME* map, TYPE elem)                    \
{                                                                                       \
    uint32_t index = HASH_FUNC(elem) % map->array_size;                                 \
    LinkedListMap_##TYPENAME* list = map->array[index];                                 \
                                                                                        \
    if (!list->size) return 0;                                                          \
                                                                                        \
    if (list->size == 1)                                                                \
    {                                                                                   \
        if (list->head->data != elem)                                                   \
            return 0;                                                                   \
                                                                                        \
        free(list->head);                                                               \
        list->head = NULL;                                                              \
        list->tail = NULL;                                                              \
        list->size = 0;                                                                 \
        map->size--;                                                                    \
        return 1;                                                                       \
    }                                                                                   \
                                                                                        \
    else                                                                                \
    {                                                                                   \
        if (list->head->data == elem)                                                   \
        {                                                                               \
            LinkedListNodeMap_##TYPENAME* prev_head = list->head;                       \
            list->head = list->head->ptr_next;                                          \
            list->size--;                                                               \
            map->size--;                                                                \
            free(prev_head);                                                            \
            return 1;                                                                   \
        }                                                                               \
                                                                                        \
        LinkedListNodeMap_##TYPENAME* curr_node = list->head;                           \
        while (curr_node->ptr_next && curr_node->ptr_next->data != elem)                \
            curr_node = curr_node->ptr_next;                                            \
                                                                                        \
        if (!curr_node->ptr_next) return 0;                                             \
                                                                                        \
        if (curr_node->ptr_next == list->tail)                                          \
        {                                                                               \
            LinkedListNodeMap_##TYPENAME* prev_tail = list->tail;                       \
            list->tail = curr_node;                                                     \
            list->tail->ptr_next = NULL;                                                \
            free(prev_tail);                                                            \
        }                                                                               \
                                                                                        \
        else                                                                            \
        {                                                                               \
            LinkedListNodeMap_##TYPENAME* node_to_del = curr_node->ptr_next;            \
            curr_node->ptr_next = curr_node->ptr_next->ptr_next;                        \
            free(node_to_del);                                                          \
        }                                                                               \
                                                                                        \
        list->size--;                                                                   \
        map->size--;                                                                    \
        return 1;                                                                       \
    }                                                                                   \
}                                                                                       \
                                                                                        \
int hashmap_##TYPENAME ##_contains(HashMap_##TYPENAME* map, TYPE elem)                  \
{                                                                                       \
    uint32_t index = HASH_FUNC(elem) % map->array_size;                                 \
    LinkedListMap_##TYPENAME* list = map->array[index];                                 \
                                                                                        \
    if (list->size == 0) return 0;                                                      \
                                                                                        \
    if (list->head->data == elem || list->tail->data == elem)                           \
        return 1;                                                                       \
                                                                                        \
    LinkedListNodeMap_##TYPENAME* curr_node = list->head;                               \
    while (curr_node && curr_node->data != elem)                                        \
        curr_node = curr_node->ptr_next;                                                \
                                                                                        \
    return curr_node ? 1 : 0;                                                           \
}                                                                                       \
                                                                                        \
void hashmap_##TYPENAME ##_delete(HashMap_##TYPENAME* map)                              \
{                                                                                       \
    for (int i = 0; i < map->array_size; i++)                                           \
    {                                                                                   \
        LinkedListMap_##TYPENAME* list = map->array[i];                                 \
        LinkedListNodeMap_##TYPENAME* curr_node = list->head;                           \
                                                                                        \
        while (curr_node)                                                               \
        {                                                                               \
            LinkedListNodeMap_##TYPENAME* next_node = curr_node->ptr_next;              \
            free(curr_node);                                                            \
            curr_node = next_node;                                                      \
        }                                                                               \
                                                                                        \
        free(list);                                                                     \
    }                                                                                   \
                                                                                        \
    free(map->array);                                                                   \
}

#endif

