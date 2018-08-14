//
// Created by lepet on 7/1/2018.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>

#include "hotel-ll.h"


int print(char *str, void *arg) {
    printf("%s ", str);
    return 0;
}

int to_list(char *str, list_t *list) {
    list_insert_end(list, str);
    return 0;
}

void print_set(ordered_set_t *set, const char *str) {
    char c = *str;
    if (c == '\0') {
        set_iterate(set, (iter_func_t) print, 0);
        printf("\n");
    } else if (isdigit(c)) {
        int n = atoi(str);
        list_t *list = list_init();

        set_iterate(set, (iter_func_t) to_list, list);
        char *result = list_get(list, n);
        if (result != NULL)
            printf("%s\n", result);

        list_free(&list);
    }
}

int main() {
    printf("Enter '...' to insert a string into the set.\n"
           "Enter '-...' to remove a string from the set.\n"
           "Enter '=' to print the set in order.\n"
           "Enter '=...' to print the n-th element in the set.\n"
           "Enter '?' to print the number of elements\n");

    ordered_set_t *set = set_init((cmp_func_t) strcmp);

    while (1) {
        char *str = malloc(16);
        printf(">> ");
        scanf("%s", str);
        if (strcmp(str, "") == 0)
            break;

        switch (*str) {
            case '-':
                set_delete(set, str + 1);
                free(str);
                break;
            case '=':
                print_set(set, str + 1);
                break;
            case '?':
                printf("%d\n", set_size(set));
                break;
            default:
                set_insert(set, str);
                break;
        }
        printf("\n");
    }

    return 0;
}
