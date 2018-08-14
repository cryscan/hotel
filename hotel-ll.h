//
// Created by lepet on 6/30/2018.
//

#ifndef HOTEL_HOTEL_LL_H
#define HOTEL_HOTEL_LL_H

typedef int (*iter_func_t)(void *, void *);

// The compare function that defines the order of nodes.
typedef int (*deduced_cmp_func_t)(const void *, const void *);

typedef int (*cmp_func_t)(const void *, const void *, deduced_cmp_func_t);

typedef struct list_node {
    struct list_node *next;
    void *content;
} list_t;

typedef struct ordered_set {
    unsigned size;
    struct tree_node *root;
    cmp_func_t cmp_func;
} ordered_set_t;

typedef struct ordered_set ordered_map_t;

typedef struct pair {
    void *key;
    void *value;
} pair_t;


list_t *list_init();

void list_free(list_t **);

void list_clear(list_t *);

int list_empty(list_t *);

int list_length(list_t *);

int list_search(list_t *, void *);

void list_insert_begin(list_t *, void *);

void list_insert_end(list_t *, void *);

void list_insert_after(list_t *, void *, int);

void list_remove_begin(list_t *);

void list_remove_end(list_t *);

void list_remove_after(list_t *, int);

void list_merge(list_t *, list_t *);

list_t *list_split(list_t *, int);

void *list_get(list_t *, int);

void list_iterate(list_t *, iter_func_t, void *);

ordered_set_t *set_init(cmp_func_t);

void set_free(ordered_set_t **);

int set_empty(ordered_set_t *);

int set_size(ordered_set_t *);

void *set_check(ordered_set_t *, void *);

void set_insert(ordered_set_t *, void *);

void set_delete(ordered_set_t *, void *);

void *set_get_minimum(ordered_set_t *);

void set_iterate(ordered_set_t *, iter_func_t, void *);

pair_t *pair_init(void *, void *);

ordered_map_t *map_init(cmp_func_t);

void map_free(ordered_map_t **);

void map_insert(ordered_map_t *, void *, void *);

void map_delete(ordered_map_t *, void *);

void *map_get(ordered_map_t *, void *);

void *map_get_minimum(ordered_map_t *);

void string_pair_insert(list_t *, const char *, const char *);

void string_pair_free(pair_t **);

#endif //HOTEL_HOTEL_LL_H
