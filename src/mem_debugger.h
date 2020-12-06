#ifndef MEM_DEBUGGER_H_
#define MEM_DEBUGGER_H_

#include "stdlib.h"

void* debug_malloc (size_t size,                const char* file_name, int src_line);
void* debug_calloc (size_t num, size_t size,    const char* file_name, int src_line);
void* debug_realloc(void* ptr, size_t new_size, const char* file_name, int src_line);
void  debug_free   (void* ptr,                  const char* file_name, int src_line);

void mem_debugger_dump_info(int use_file, const char* file_name);

#ifdef MEM_DEBUGGER_IMPLEMENTATION

// to use unsafe c functions without warning
#define _CRT_SECURE_NO_WARNINGS

#include "stdio.h"
#include "string.h"
#include "stdint.h"

// parameters of bytes written afrer any allocated memory block
#define BOUND_CHECK_BYTES_COUNT 4
#define BOUND_CHECK_BYTE_VALUE 218

// i'm not sure which value is optimal
#define HASHMAP_LIST_LEN 1024

typedef unsigned long long ull;

// ======== HashMap for for allocations implementation =========

typedef struct
{
    void* block;
    size_t size;
    const char* file_name;
    int src_line;
}
AllocData;

typedef struct ListAllocDataNode
{
    AllocData* data;
    struct ListAllocDataNode* ptr_next;
}
ListAllocDataNode;

typedef struct
{
    ListAllocDataNode *head, *tail;
    size_t size;
}
ListAllocs;

static ListAllocs* list_allocs_create()
{
    ListAllocs* llist = malloc(sizeof(ListAllocs));
    llist->head = NULL;
    llist->tail = NULL;
    llist->size = 0;
    return llist;
}

static void list_allocs_push_front(ListAllocs* llist, AllocData* elem)
{
    ListAllocDataNode* node = malloc(sizeof(ListAllocDataNode));
    node->data = elem;
    node->ptr_next = NULL;

    if (!llist->size)
    {
        llist->head = node;
        llist->tail = node;
    }

    else
    {
        ListAllocDataNode* prev_head = llist->head;
        llist->head = node;
        llist->head->ptr_next = prev_head;
    }

    llist->size++;
}

static int list_allocs_remove(ListAllocs* llist, void* block)
{
    if (!llist->size) return 0;

    if (llist->size == 1)
    {
        if (llist->head->data->block != block)
            return 0;

        free(llist->head->data);
        free(llist->head);
        llist->head = NULL;
        llist->tail = NULL;
        llist->size = 0;
        return 1;
    }

    // size > 1

    // if data is in head
    if (llist->head->data->block == block)
    {
        ListAllocDataNode* prev_head = llist->head;
        llist->head = llist->head->ptr_next;
        llist->size--;
        free(prev_head->data);
        free(prev_head);
        return 1;
    }

    // elem should be in curr_node->ptr_next
    ListAllocDataNode* curr_node = llist->head;
    while (curr_node->ptr_next && curr_node->ptr_next->data->block != block)
        curr_node = curr_node->ptr_next;

    // no such element is list?
    if (!curr_node->ptr_next) return 0;

    // if data is in tail
    if (curr_node->ptr_next == llist->tail)
    {
        ListAllocDataNode* prev_tail = llist->tail;
        llist->tail = curr_node;
        llist->tail->ptr_next = NULL;
        free(prev_tail->data);
        free(prev_tail);
    }

    // data is not in tail
    else
    {
        ListAllocDataNode* node_to_del = curr_node->ptr_next;
        curr_node->ptr_next = curr_node->ptr_next->ptr_next;
        free(node_to_del->data);
        free(node_to_del);
    }

    llist->size--;
    return 1;
}

static AllocData* list_allocs_get(ListAllocs* llist, void* block)
{
    if (llist->size == 0) return NULL;

    if (llist->head->data->block == block)
        return llist->head->data;

    if (llist->tail->data->block == block)
        return llist->tail->data;

    ListAllocDataNode* curr_node = llist->head;
    while (curr_node && curr_node->data->block != block)
        curr_node = curr_node->ptr_next;

    return curr_node ? curr_node->data : NULL;
}

typedef struct
{
    ListAllocs** array;
    
    size_t array_size;
    size_t size;
}
HashmapAllocs;

static HashmapAllocs* map_allocs_create(size_t array_size)
{
    HashmapAllocs* map = malloc(array_size * sizeof(HashmapAllocs));

    map->array = malloc(array_size * sizeof(ListAllocs*));
    for (int i = 0; i < array_size; i++)
        map->array[i] = list_allocs_create();
    
    map->array_size = array_size;
    map->size = 0;
    
    return map;
}

static uint32_t map_allocs_hash_func(void* ptr)
{
    // use pointer as key
    // (little detour to remove warning)
    uint32_t key = (uint32_t)(uintptr_t)ptr;
    
    key = ~key + (key << 15); 
    key = key ^ (key >> 12);
    key = key + (key << 2);
    key = key ^ (key >> 4);
    key = key * 2057; 
    key = key ^ (key >> 16);

    return key;
}

static void map_allocs_insert(HashmapAllocs* map, AllocData* elem)
{
    uint32_t index = map_allocs_hash_func(elem->block) % map->array_size;
    list_allocs_push_front(map->array[index], elem);
    map->size++;
}

static int map_allocs_remove(HashmapAllocs* map, void* block)
{
    uint32_t index = map_allocs_hash_func(block) % map->array_size;

    if (list_allocs_remove(map->array[index], block))
    {
        map->size--;
        return 1;
    }
    return 0;
}

// search node with node->data->block == data->block
// and set new data for this node
static void  map_allocs_update(HashmapAllocs* map, AllocData* data)
{
    uint32_t index = map_allocs_hash_func(data->block) % map->array_size;
    
    ListAllocDataNode* curr_node = map->array[index]->head;

    while (curr_node->data->block != data->block)
        curr_node = curr_node->ptr_next;

    free(curr_node->data);
    curr_node->data = data;
}

// return NULL if block is not found, otherwise return
// AllocData* element where data->block == block
static AllocData* map_allocs_get(HashmapAllocs* map, void* block)
{
    uint32_t index = map_allocs_hash_func(block) % map->array_size;
    return list_allocs_get(map->array[index], block);
}

// ====== HashMap for for allocations implementation end =======

// ======== HashMap for for alloc sites implementation =========

typedef struct
{
    size_t bytes;
    const char* file_name;
    int src_line;
}
AllocSiteData;

typedef struct ListAllocSiteNode
{
    AllocSiteData* data;
    struct ListAllocSiteNode* ptr_next;
}
ListAllocSiteNode;

typedef struct
{
    ListAllocSiteNode *head, *tail;
    size_t size;
}
ListAllocSites;

static ListAllocSites* list_allocsites_create()
{
    ListAllocSites* llist = malloc(sizeof(ListAllocSites));
    llist->head = NULL;
    llist->tail = NULL;
    llist->size = 0;
    return llist;
}

static void list_allocsites_push_front(ListAllocSites* llist, AllocSiteData* elem)
{
    ListAllocSiteNode* node = malloc(sizeof(ListAllocSiteNode));
    node->data = elem;
    node->ptr_next = NULL;

    if (!llist->size)
    {
        llist->head = node;
        llist->tail = node;
    }

    else
    {
        ListAllocSiteNode* prev_head = llist->head;
        llist->head = node;
        llist->head->ptr_next = prev_head;
    }

    llist->size++;
}

static AllocSiteData* list_allocsites_get(ListAllocSites* llist, AllocData* elem)
{
    if (llist->size == 0) return NULL;

    if (llist->head->data->src_line == elem->src_line
        && !strcmp(llist->head->data->file_name, elem->file_name))
        return llist->head->data;

    if (llist->tail->data->src_line == elem->src_line
        && !strcmp(llist->tail->data->file_name, elem->file_name))
        return llist->tail->data;

    ListAllocSiteNode* curr_node = llist->head;
    while (curr_node && (curr_node->data->src_line != elem->src_line || strcmp(curr_node->data->file_name, elem->file_name)))
        curr_node = curr_node->ptr_next;

    return curr_node ? curr_node->data : NULL;
}

static void list_allocsites_update(ListAllocSites* llist, AllocData* elem)
{
    // find particular allocation site to add size 
    if (llist->head->data->src_line == elem->src_line
        && !strcmp(llist->head->data->file_name, elem->file_name))
        llist->head->data->bytes += elem->size;

    else if (llist->tail->data->src_line == elem->src_line
        && !strcmp(llist->tail->data->file_name, elem->file_name))
        llist->tail->data->bytes += elem->size;

    else
    {
        ListAllocSiteNode* curr_node = llist->head;

        while (curr_node->data->src_line != elem->src_line || strcmp(curr_node->data->file_name, elem->file_name))
            curr_node = curr_node->ptr_next;

        curr_node->data->bytes += elem->size;
    }
}

typedef struct
{
    ListAllocSites** array;
    
    size_t bytes_alloc_at_all;
    size_t array_size;
    size_t size;
}
HashmapAllocSites;

static HashmapAllocSites* map_allocsites_create(size_t array_size)
{
    HashmapAllocSites* map = malloc(array_size * sizeof(HashmapAllocSites));

    map->array = malloc(array_size * sizeof(ListAllocSites*));
    for (int i = 0; i < array_size; i++)
        map->array[i] = list_allocsites_create();
    
    map->bytes_alloc_at_all = 0;
    map->array_size = array_size;
    map->size = 0;
    
    return map;
}

static uint32_t map_allocsites_hash_func(const char* file, int line)
{
    // firstly hash using file name
    uint32_t key = 5381;

    int c;
    while (c = *file++)
        key = ((key << 5) + key) + c;
    
    // then hash using line
    key = ~line + (key << 15); 
    key = key ^ (key >> 12);
    key = key + (key << 2);
    key = key ^ (key >> 4);
    key = key * 2057; 
    key = key ^ (key >> 16);

    return key;
}

static void map_allocsites_insert(HashmapAllocSites* map, AllocSiteData* data)
{
    uint32_t index = map_allocsites_hash_func(data->file_name, data->src_line) % map->array_size;
    list_allocsites_push_front(map->array[index], data);
    map->size++;
}

// return NULL if data is not found, otherwise return
// AllocSiteData* with file name and src_line identical to given AllocData
static AllocSiteData* map_allocsites_get(HashmapAllocSites* map, AllocData* data)
{
    uint32_t index = map_allocsites_hash_func(data->file_name, data->src_line) % map->array_size;
    return list_allocsites_get(map->array[index], data);
}

// Updates existing allocation site with new bytes
static void map_allocsites_update(HashmapAllocSites* map, AllocData* data)
{
    map->bytes_alloc_at_all += data->size;
    
    uint32_t index = map_allocsites_hash_func(data->file_name, data->src_line) % map->array_size;
    list_allocsites_update(map->array[index], data);
}

static int compar_func(const void* p1, const void* p2)
{
    AllocSiteData* d1 = *(AllocSiteData**)p1;
    AllocSiteData* d2 = *(AllocSiteData**)p2;

    if (d1->bytes > d2->bytes) return -1;
    else if (d1->bytes == d2->bytes) return 0;
    else return 1;
}

// returns descending-sorted array with allocation sites. It's 
// descending-sorted for easy printing, just print first n elems
// and you'll get n heaviest allocation sites
static AllocSiteData** map_allocsites_get_sorted_array(HashmapAllocSites* map)
{
    //AllocSiteData** arr = realloc(llist->size * sizeof(AllocSiteData*));
    AllocSiteData** arr = malloc(map->size * sizeof(AllocSiteData*));
    int curr_ptr = 0;

    for (int i = 0; i < map->array_size; i++)
    {
        ListAllocSiteNode* curr_node = map->array[i]->head;
        while (curr_node)
        {
            arr[curr_ptr++] = curr_node->data;
            curr_node = curr_node->ptr_next;
        }
    }

    qsort(arr, map->size, sizeof(AllocSiteData*), compar_func);

    return arr;
}

// ====== HashMap for for alloc sites implementation end =======

// ==================== Helper functions =======================

// singleton maps are controlled by these 2 functions
static HashmapAllocs* instance_map_allocs()
{
    static HashmapAllocs* map = NULL;
    if (!map) map = map_allocs_create(HASHMAP_LIST_LEN);
    return map;
}

static HashmapAllocSites* instance_map_allocsites()
{
    static HashmapAllocSites* map = NULL;
    if (!map) map = map_allocsites_create(HASHMAP_LIST_LEN);
    return map;
}

static void warn_null_return(const char* from_func, const char* file_name, int src_line, size_t block_size)
{
    fprintf(stderr, "Warning!\n");
    fprintf(stderr, "%s at %s:%d returned NULL! (size was %llu)\n\n", 
            from_func, file_name, src_line, (ull)block_size);
    abort();
}

static void warn_wrong_ptr(const char* from_func, const char* file_name, int src_line)
{
    fprintf(stderr, "Warning!\n");
    fprintf(stderr, "%s at %s:%d is used with pointer that does not point to active heap memory!\n\n",
            from_func, file_name, src_line);
    abort();
}

static void warn_bound_violation(const char* file_name, int src_line, size_t block_size)
{
    fprintf(stderr, "Warning!\n");
    fprintf(stderr, "Out-of-bounds writing occured after memory block, allocated at:\n");
    fprintf(stderr, "%s:%d (%llu bytes)\n\n", file_name, src_line, (ull)block_size);
    abort();
}

static AllocData* blockdata_create(void* block, size_t size, const char* file_name, int src_line)
{
    AllocData* block_data = malloc(sizeof(AllocData));
    block_data->block = block;
    block_data->size = size;
    block_data->file_name = file_name;
    block_data->src_line = src_line;
    return block_data;
}

static void update_alloc_data(AllocData* block_data)
{
    // try to find that particular allocation site
    AllocSiteData* alloc_data = map_allocsites_get(instance_map_allocsites(), block_data);

    // if there were no allocations on that line previously, 
    // create new AllocSiteData structure and add it to map
    if (!alloc_data)
    {
        alloc_data = malloc(sizeof(AllocSiteData));
        alloc_data->bytes = 0;
        alloc_data->src_line = block_data->src_line;
        alloc_data->file_name = block_data->file_name;
        map_allocsites_insert(instance_map_allocsites(), alloc_data);
    }

    // update total bytes allocated at that line in that file,
    // the allocation site is guranteed to be in map
    map_allocsites_update(instance_map_allocsites(), block_data);
}

// =================== Helper functions end ====================

// ========== Debug versions of allocation functions ===========

void* debug_malloc(size_t size, const char* file_name, int src_line)
{
    // additional space for bound-checking bytes
    void* block = malloc(size + BOUND_CHECK_BYTES_COUNT);

    if (!block)
        warn_null_return("malloc()", file_name, src_line, size);

    // set bytes for bound checking
    memset((unsigned char*)block + size, BOUND_CHECK_BYTE_VALUE, BOUND_CHECK_BYTES_COUNT);

    AllocData* block_data = blockdata_create(block, size, file_name, src_line);
    map_allocs_insert(instance_map_allocs(), block_data);
    update_alloc_data(block_data);

    return block;
}

void* debug_calloc(size_t num, size_t size, const char* file_name, int src_line)
{
    // additional space for bound-checking bytes
    void* block = calloc((num * size) + BOUND_CHECK_BYTES_COUNT, 1);

    if (!block)
        warn_null_return("calloc()", file_name, src_line, num * size);

    // set bytes for bound checking
    memset((unsigned char*)block + (num * size), BOUND_CHECK_BYTE_VALUE, BOUND_CHECK_BYTES_COUNT);

    AllocData* block_data = blockdata_create(block, num * size, file_name, src_line);
    map_allocs_insert(instance_map_allocs(), block_data);
    update_alloc_data(block_data);

    return block;
}

void* debug_realloc(void* ptr, size_t new_size, const char* file_name, int src_line)
{
    // this ptr should be in active allocations map
    AllocData* found_block = map_allocs_get(instance_map_allocs(), ptr);

    // if it's not correct pointer to heap memory
    // (but nullptr is allowed), crash
    if (!found_block && ptr)
        warn_wrong_ptr("realloc()", file_name, src_line);

    void* block = realloc(ptr, new_size + BOUND_CHECK_BYTES_COUNT);

    if (!block)
        warn_null_return("realloc()", file_name, src_line, new_size);

    // set bytes for bound checking
    memset((unsigned char*)block + new_size, BOUND_CHECK_BYTE_VALUE, BOUND_CHECK_BYTES_COUNT);

    AllocData* block_data = blockdata_create(block, new_size, file_name, src_line);

   // ptr = NULL, realloc acts as malloc
    if (!ptr)
        map_allocs_insert(instance_map_allocs(), block_data);

    // realloc expanded existing block
    else if (ptr == block)
        map_allocs_update(instance_map_allocs(), block_data);

    // realloc decided to allocate new block
    else
    {
        map_allocs_remove(instance_map_allocs(), ptr);
        map_allocs_insert(instance_map_allocs(), block_data);
    }

    update_alloc_data(block_data);

    return block;
}

void debug_free(void* ptr, const char* file_name, int src_line)
{
    // nullptrs do nothing
    if (ptr == NULL) return;

    // this ptr should be in active allocs map
    AllocData* found_block = map_allocs_get(instance_map_allocs(), ptr);

    // if it's not correct pointer to heap memory, crash
    if (!found_block)
        warn_wrong_ptr("free()", file_name, src_line);

    // check bytes after block, they shouldn't be changed
    unsigned char required_bytes[BOUND_CHECK_BYTES_COUNT];
    memset(required_bytes, BOUND_CHECK_BYTE_VALUE, BOUND_CHECK_BYTES_COUNT);

    //if (memcmp((unsigned char*)ptr + found_block->size,
    //           required_bytes, BOUND_CHECK_BYTES_COUNT))
    //{
    //    warn_bound_violation(found_block->file_name,
    //                         found_block->src_line, found_block->size);
    //}

    // if all good, remove ptr form map of active allocs
    map_allocs_remove(instance_map_allocs(), ptr);

    free(ptr);
}

// ======= Debug versions of allocation functions end ==========

// if strlen > n, prints last n characters of the string
// otherwise, fill remaining space with spaces
static void print_str_with_width(FILE* stream, const char* str, int n)
{
    int str_len = strlen(str);

    if (str_len <= n)
    {
        fprintf(stream, "%s", str);
        for (int i = str_len; i < n; i++)
            putc(' ', stream);
    }

    else
    {
        for (int i = 0; i < 3; i++)
            putc('.', stream);
        
        if (n > 3)
            fprintf(stream, "%s", str + str_len - n + 3);
    }
}

static void print_info(FILE* stream)
{
    // print info about memory leaks
    fprintf(stream, "%s", "Current unfreed allocations:\n");

    fprintf(stream, "%-42.42s %-5.5s %-16.16s\n",
            "Source file", "Line", "Bytes");

    size_t total_size = 0;
    size_t blocks_count = 0;

    HashmapAllocs* map = instance_map_allocs();

    for (int i = 0; i < map->array_size; i++)
    {
        ListAllocDataNode* curr_node = map->array[i]->head;

        while (curr_node)
        {
            print_str_with_width(stream, curr_node->data->file_name, 41);
            fprintf(stream, "  %-5d %-16llu\n",
                    curr_node->data->src_line,
                    (ull)curr_node->data->size);

            total_size += curr_node->data->size;
            blocks_count++;

            curr_node = curr_node->ptr_next;
        }
    }

    fprintf(stream, "\nUnfreed total: %llu bytes from %llu allocation(s)\n\n",
            (ull)total_size, (ull)blocks_count);

    // print info about heavy-hitter allocations
    fprintf(stream, "Top-5 heaviest allocations so far:\n");

    fprintf(stream, "%-42.42s %-5.5s %-10.10s %-10.10s\n",
            "Source file", "Line", "Bytes", "Percentage");

    AllocSiteData** arr = map_allocsites_get_sorted_array(instance_map_allocsites());
    size_t arr_size = instance_map_allocsites()->size;
    size_t total_bytes = instance_map_allocsites()->bytes_alloc_at_all;

    for (int i = 0; i < 5 && i < arr_size; i++)
    {
        print_str_with_width(stream, arr[i]->file_name, 41);
        fprintf(stream, "  %-5d %-10llu %.2lf%%\n",
                arr[i]->src_line,
                (ull)arr[i]->bytes,
                100.0 * (double)arr[i]->bytes / total_bytes);
    }

    free(arr);

    fprintf(stream, "\nAllocated total: %llu bytes\n\n", (ull)total_bytes);
}

// user should only use this function
void mem_debugger_dump_info(int use_file, const char* file_name)
{
    if (use_file && !file_name)
    {
        fprintf(stderr, "Warning!\n");
        fprintf(stderr, "%s: use_file=1, but no file_name provided!\n", __func__);
        return;
    }

    FILE* stream = use_file ? fopen(file_name, "a") : stderr;

    if (stream)
        print_info(stream);
    else
    {
        fprintf(stderr, "Warning!\n");
        fprintf(stderr, "%s: unable to open file for writing: %s\n", __func__, file_name);
        return;
    }

    if (use_file)
        fclose(stream);
}

#endif //MEM_DEBUGGER_IMPLEMENTATION

// Actual defines which will replace memory functions
#ifndef MEM_DEBUGGER_DISABLE
#define malloc(size)           debug_malloc (size,          __FILE__, __LINE__)
#define calloc(num, size)      debug_calloc (num, size,     __FILE__, __LINE__)
#define realloc(ptr, new_size) debug_realloc(ptr, new_size, __FILE__, __LINE__)
#define free(ptr)              debug_free   (ptr,           __FILE__, __LINE__)
#endif

#endif
