//
// Created by lepet on 7/1/2018.
//
#include <stdio.h>

#include "hotel-ll.h"


int print(char *str) {
    printf("%s\n", str);
    return 0;
}

int main() {
    list_t *a = list_init();
    list_insert_begin(a, "I am the first node.");
    list_insert_end(a, "I am the end.");
    list_insert_after(a, "I am in the middle.", 1);
    list_insert_after(a, "I am before the end.", 2);
    list_iterate(a, (iter_func_t) print, 0);
    printf("\n");

    list_t *b = list_split(a, 4);
    list_iterate(a, (iter_func_t) print, 0);
    printf("\n");
    list_iterate(b, (iter_func_t) print, 0);
    printf("\n");

    list_remove_begin(a);
    list_remove_end(a);
    list_iterate(a, (iter_func_t) print, 0);


    list_free(&a);
}