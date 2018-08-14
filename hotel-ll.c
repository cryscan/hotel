//
// Created by lepet on 6/30/2018.
// The implementation of link list, set and map.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <assert.h>
#include <signal.h>

#include "hotel-ll.h"

/* Implementation of link list */

typedef struct list_node node_t;

int is_head(node_t *head) {
    if (head == NULL)
        return 0;
    return head->content == NULL;
}


node_t *list_init() {
    node_t *head = malloc(sizeof(node_t));
    assert(head != NULL);

    memset(head, 0, sizeof(node_t));
    return head;
}

void list_free(node_t **pNode) {
    node_t *ptr = *pNode;
    node_t *temp = NULL;

    while (ptr->next != NULL) {
        temp = ptr;
        ptr = ptr->next;
        free(temp);
    }
    free(ptr);
    *pNode = NULL;
}

void list_clear(node_t *head) {
    assert(is_head(head));
    if (head->next != NULL)
        list_free(&head->next);
}

int list_empty(node_t *head) {
    assert(is_head(head));
    return head->next == NULL;
}

int list_length(node_t *head) {
    assert(is_head(head));
    node_t *ptr = head;

    int i = 0;
    while (ptr->next != NULL) {
        i = i + 1;
        ptr = ptr->next;
    }
    return i;
}


int list_search(node_t *head, void *content) {
    assert(is_head(head));
    node_t *ptr = head;

    int i = 0;
    while (ptr != NULL) {
        if (ptr->content == content)
            return i;
        i = i + 1;
        ptr = ptr->next;
    }
    return -1;
}

void list_insert_begin(node_t *head, void *content) {
    list_insert_after(head, content, 0);
}

void list_insert_end(node_t *head, void *content) {
    assert(is_head(head));
    node_t *ptr = head;

    node_t *temp = list_init();
    temp->content = content;

    while (ptr->next != NULL)
        ptr = ptr->next;
    ptr->next = temp;
}

// Insert a node after i-th node.
void list_insert_after(node_t *head, void *content, int pos) {
    assert(is_head(head));
    node_t *ptr = head;

    if (pos > list_length(head))
        return;

    for (int i = 0; i < pos; ++i)
        ptr = ptr->next;
    // ptr is now the i-th node, assuming that head is the 0-th node.

    node_t *temp = list_init();
    temp->content = content;
    temp->next = ptr->next;
    ptr->next = temp;
}


void list_remove_begin(node_t *head) {
    list_remove_after(head, 0);
}

void list_remove_end(node_t *head) {
    assert(is_head(head));
    if (list_empty(head))
        return;

    node_t *ptr = head;
    node_t *temp = head;
    while (ptr->next != NULL) {
        temp = ptr;
        ptr = ptr->next;
    }
    temp->next = NULL;
    free(ptr);
}

void list_remove_after(node_t *head, int pos) {
    assert(is_head(head));
    if (list_empty(head))
        return;

    node_t *ptr = head;
    node_t *temp = NULL;

    if (pos > list_length(head) - 1)
        return;

    for (int i = 0; i < pos; ++i)
        ptr = ptr->next;

    temp = ptr->next;
    ptr->next = temp->next;
    free(temp);
}

void list_merge(node_t *head1, node_t *head2) {
    assert(is_head(head1));
    assert(is_head(head2));

    node_t *ptr = head1;
    while (ptr->next != NULL)
        ptr = ptr->next;
    ptr->next = head2->next;
    head2->next = NULL;
}

list_t *list_split(node_t *head, int pos) {
    list_t *list = list_init();

    assert(is_head(head));
    if (pos > list_length(head))
        return list;

    node_t *temp = head;
    node_t *ptr = head;
    for (int i = 0; i < pos; ++i) {
        temp = ptr;
        ptr = ptr->next;
    }
    temp->next = NULL;
    list->next = ptr;
    return list;
}


// Return the pointer to the content of the i-th node.
void *list_get(node_t *head, int pos) {
    assert(is_head(head));
    if (pos > list_length(head))
        return NULL;

    node_t *ptr = head;
    for (int i = 0; i < pos; ++i)
        ptr = ptr->next;
    return ptr->content;
}

// Apply fp for each node in the list.
void list_iterate(node_t *head, iter_func_t iter_func, void *arg) {
    assert(is_head(head));
    if (list_empty(head))
        return;

    node_t *ptr = head;
    while (ptr->next != NULL) {
        ptr = ptr->next;
        if (iter_func(ptr->content, arg) != 0)
            break;
    }
}


/* Set and Map are based on AVL Trees */

typedef struct tree_node {
    struct tree_node *parent;
    struct tree_node *left;
    struct tree_node *right;
    int height;

    void *content;
} tree_node_t;

typedef struct ordered_set set_t;

typedef ordered_map_t map_t;

/* Methods of Nodes */

tree_node_t *node_init(void *content) {
    tree_node_t *node = malloc(sizeof(tree_node_t));
    assert(node != NULL);

    memset(node, 0, sizeof(tree_node_t));
    node->content = content;
    return node;
}

void node_free(tree_node_t **pNode) {
    tree_node_t *temp = *pNode;

    if (temp->left != NULL)
        node_free(&temp->left);
    if (temp->right != NULL)
        node_free(&temp->right);

    free(temp);
    *pNode = NULL;
}

void node_height(tree_node_t *node) {
    assert(node != NULL);

    tree_node_t *left = node->left;
    tree_node_t *right = node->right;
    int left_height = left == NULL ? 0 : left->height;
    int right_height = right == NULL ? 0 : right->height;

    node->height = left_height > right_height ? left_height : right_height;
    node->height++;
}

int node_delta(tree_node_t *node) {
    assert(node != NULL);

    tree_node_t *left = node->left;
    tree_node_t *right = node->right;
    int left_height = left == NULL ? 0 : left->height;
    int right_height = right == NULL ? 0 : right->height;
    return left_height - right_height;
}

// Set the right child of node to right.
void node_right(tree_node_t *node, tree_node_t *right) {
    assert(node != NULL);
    node->right = right;

    if (right != NULL)
        right->parent = node;
}

// Set the left child of node to left.
void node_left(tree_node_t *node, tree_node_t *left) {
    assert(node != NULL);
    node->left = left;

    if (left != NULL)
        left->parent = node;
}

// Substitute a child of node.
void node_subs(tree_node_t *node, tree_node_t *old, tree_node_t *new) {
    if (new != NULL)
        new->parent = node;

    if (node == NULL)
        return;

    if (node->left == old)
        node->left = new;
    else if (node->right == old)
        node->right = new;
}

void node_rotate_right(tree_node_t *node) {
    assert(node != NULL);
    tree_node_t *pivot = node->left;
    tree_node_t *parent = node->parent;
    if (pivot == NULL)
        return;

    node_left(node, pivot->right);
    node_right(pivot, node);
    node_subs(parent, node, pivot);

    node_height(node);  // Update height after rotation.
}

void node_rotate_left(tree_node_t *node) {
    assert(node != NULL);
    tree_node_t *pivot = node->right;
    tree_node_t *parent = node->parent;
    if (pivot == NULL)
        return;

    node_right(node, pivot->left);
    node_left(pivot, node);
    node_subs(parent, node, pivot);

    node_height(node);  // Update height after rotation.
}

void node_insert(tree_node_t *root, tree_node_t *node, cmp_func_t cmp_func, deduced_cmp_func_t deduced_cmp_func) {
    assert(root != NULL);
    assert(node != NULL);

    tree_node_t *ptr = root;
    tree_node_t *temp = root;
    int cmp = 0;
    while (ptr != NULL) {
        temp = ptr;
        cmp = cmp_func(ptr->content, node->content, deduced_cmp_func); // new < ptr.
        ptr = cmp > 0 ? ptr->left : ptr->right;
    }

    if (cmp > 0)
        temp->left = node;
    else
        temp->right = node;
    node->parent = temp;

    ptr = node;
    while (ptr != NULL) {
        node_height(ptr);
        int balance_factor = node_delta(ptr);
        if (balance_factor > 1) {
            // The tree is left heavy and unbalanced.
            if (node_delta(ptr->left) < 0) {
                // The left subtree is right heavy.
                // Do double right rotation.
                node_rotate_left(ptr->left);
                node_rotate_right(ptr);
            } else
                node_rotate_right(ptr);
        }
        if (balance_factor < -1) {
            if (node_delta(ptr->right) > 0) {
                node_rotate_right(ptr->right);
                node_rotate_left(ptr);
            } else
                node_rotate_left(ptr);
        }

        ptr = ptr->parent;
    }
}

void node_delete(tree_node_t *node) {
    assert(node != NULL);

    tree_node_t *temp = node;
    tree_node_t *ptr = node;
    if (ptr->left == NULL)
        node_subs(ptr->parent, ptr, ptr->right);
    else {
        ptr = ptr->left;
        while (ptr->right != NULL)
            ptr = ptr->right;
        temp->content = ptr->content;
        node_subs(ptr->parent, ptr, NULL);
    }
    temp = ptr;
    ptr = ptr->parent;
    free(temp);

    while (ptr != NULL) {
        node_height(ptr);
        int balance_factor = node_delta(ptr);
        if (balance_factor > 1) {
            if (node_delta(ptr->left) < 0) {
                node_rotate_left(ptr->left);
                node_rotate_right(ptr);
            } else
                node_rotate_right(ptr);
        }
        if (balance_factor < -1) {
            if (node_delta(ptr->right) > 0) {
                node_rotate_right(ptr->right);
                node_rotate_left(ptr);
            } else
                node_rotate_left(ptr);
        }

        ptr = ptr->parent;
    }
}

tree_node_t *node_get(tree_node_t *root,
                      void *content,
                      cmp_func_t cmp_func,
                      deduced_cmp_func_t deduced_cmp_func) {
    assert(root != NULL);
    tree_node_t *ptr = root;

    while (ptr != NULL) {
        int cmp = cmp_func(ptr->content, content, deduced_cmp_func);
        if (cmp == 0)
            break;
        else if (cmp > 0)    // content is in left of ptr.
            ptr = ptr->left;
        else
            ptr = ptr->right;
    }

    return ptr;    // Not found.
}

tree_node_t *node_get_minimum(tree_node_t *root) {
    assert(root != NULL);
    tree_node_t *ptr = root;

    while (ptr->left != NULL)
        ptr = ptr->left;

    return ptr;
}

// Iterate every element in in-order (non-recursive version).
void node_iterate(tree_node_t *root, iter_func_t iter_func, void *arg) {
    list_t *stack = list_init();
    tree_node_t *ptr = root;

    while (!list_empty(stack) || ptr != NULL) {
        while (ptr != NULL) {
            list_insert_begin(stack, ptr);
            ptr = ptr->left;
        }
        ptr = list_get(stack, 1);
        list_remove_begin(stack);   // Pop ptr.
        if (iter_func(ptr->content, arg) != 0)
            break;    // Do sth with ptr... then forget it.
        ptr = ptr->right;
    }
    free(stack);
}


/* Methods of Set */

set_t *set_init(cmp_func_t cmp_func) {
    set_t *set = malloc(sizeof(set_t));
    assert(set != NULL);

    memset(set, 0, sizeof(set_t));
    set->cmp_func = cmp_func;
    return set;
}

void set_free(set_t **pSet) {
    set_t *temp = *pSet;

    if (temp->root != NULL)
        node_free(&temp->root);

    free(temp);
    *pSet = NULL;
}

int set_empty(set_t *set) {
    assert(set != NULL);
    return set->root == NULL;
}

int set_size(set_t *set) {
    assert(set != NULL);
    return set->size;
}

void *set_check(set_t *set, void *content) {
    assert(set != NULL);
    if (set_empty(set))
        return 0;

    tree_node_t *ptr = node_get(set->root, content, set->cmp_func, 0);
    if (ptr == NULL)
        return NULL;
    return ptr->content;
}

void set_insert(set_t *set, void *content) {
    assert(set != NULL);
    tree_node_t *temp = node_init(content);

    if (!set_empty(set)) {
        node_insert(set->root, temp, set->cmp_func, 0);
    } else
        set->root = temp;
    set->size++;

    // Redirect the root.
    tree_node_t *ptr = set->root;
    while (ptr->parent != NULL)
        ptr = ptr->parent;
    set->root = ptr;
}

void set_delete(set_t *set, void *content) {
    assert(set != NULL);
    if (set_empty(set))
        return;

    tree_node_t *ptr = node_get(set->root, content, set->cmp_func, 0);
    if (ptr == NULL)
        return;
    if (ptr->left == NULL && ptr->parent == NULL)
        set->root = ptr->right;
    node_delete(ptr);
    set->size--;

    // Redirect the root.
    ptr = set->root;
    if (ptr == NULL)
        return;

    while (ptr->parent != NULL)
        ptr = ptr->parent;
    set->root = ptr;
}

void *set_get_minimum(set_t *set) {
    assert(set != NULL);
    if (set_empty(set))
        return NULL;

    tree_node_t *ptr = node_get_minimum(set->root);
    assert(ptr != NULL);

    return ptr->content;
}

void set_iterate(set_t *set, iter_func_t iter_func, void *arg) {
    assert(set != NULL);
    if (set_empty(set))
        return;

    node_iterate(set->root, iter_func, arg);
}


/* Methods of Map */

pair_t *pair_init(void *key, void *value) {
    pair_t *temp = malloc(sizeof(pair_t));
    assert(temp != NULL);

    temp->key = key;
    temp->value = value;
    return temp;
}

map_t *map_init(cmp_func_t cmp_func) {
    map_t *temp = malloc(sizeof(map_t));
    assert(temp != NULL);

    memset(temp, 0, sizeof(map_t));
    temp->cmp_func = cmp_func;
    return temp;
}

void map_free(map_t **pMap) {
    map_t *temp = *pMap;

    if (temp->root != NULL)
        node_free(&temp->root);

    free(temp);
    *pMap = NULL;
}

int pair_cmp_func(pair_t *pair1, pair_t *pair2, deduced_cmp_func_t cmp_func) {
    return cmp_func(pair1->key, pair2->key);
}

void map_insert(map_t *map, void *key, void *value) {
    assert(map != NULL);
    tree_node_t *temp = node_init(pair_init(key, value));

    if (!set_empty(map)) {
        node_insert(map->root, temp, (cmp_func_t) pair_cmp_func, (deduced_cmp_func_t) map->cmp_func);
    } else
        map->root = temp;
    map->size++;

    // Redirect the root.
    tree_node_t *ptr = map->root;
    while (ptr->parent != NULL)
        ptr = ptr->parent;
    map->root = ptr;
}

void map_delete(map_t *map, void *key) {
    assert(map != NULL);
    if (set_empty(map))
        return;

    pair_t *pair = pair_init(key, 0);
    tree_node_t *ptr = node_get(map->root, pair, (cmp_func_t) pair_cmp_func, (deduced_cmp_func_t) map->cmp_func);
    free(pair);

    if (ptr == NULL)
        return;
    if (ptr->left == NULL && ptr->parent == NULL)
        map->root = ptr->right;
    node_delete(ptr);
    map->size--;

    // Redirect the root.
    ptr = map->root;
    if (ptr == NULL)
        return;

    while (ptr->parent != NULL)
        ptr = ptr->parent;
    map->root = ptr;
}

void *map_get(map_t *map, void *key) {
    assert(map != NULL);
    if (set_empty(map))
        return NULL;

    pair_t *pair = pair_init(key, 0);
    tree_node_t *ptr = node_get(map->root, pair, (cmp_func_t) pair_cmp_func, (deduced_cmp_func_t) map->cmp_func);
    free(pair);

    if (ptr == NULL)
        return NULL;
    else {
        pair = ptr->content;
        return pair->value;
    }
}

/* Useful in priority queue */
void *map_get_minimum(map_t *map) {
    assert(map != NULL);
    if (set_empty(map))
        return NULL;

    tree_node_t *ptr = node_get_minimum(map->root);
    assert(ptr != NULL);

    pair_t *pair = ptr->content;
    return pair->value;
}


void string_pair_insert(list_t *list, const char *str1, const char *str2) {
    char *key = malloc(strlen(str1) + 1);
    char *value = malloc(strlen(str2) + 1);
    strcpy(key, str1);
    strcpy(value, str2);

    pair_t *pair = pair_init(key, value);
    list_insert_end(list, pair);
}

void string_pair_free(pair_t **pPair) {
    pair_t *pair = *pPair;
    free(pair->key);
    free(pair->value);
    free(pair);
    *pPair = NULL;
}