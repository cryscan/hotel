//
// Created by lepet on 6/30/2018.
//

#ifndef HOTEL_HOTEL_DB_H
#define HOTEL_HOTEL_DB_H

#include <stdio.h>
#include <time.h>

#include "hotel-ll.h"

typedef int (*card_handle_func_t)(const char *, list_t *);

typedef int (*message_handle_func_t)(const char *);

typedef enum {
    DORM, SINGLE, DOUBLE, FAMILY
} room_type;

typedef struct person {
    char *name;
    char *id;

    room_type type;
    struct room *room;
    time_t in_time;
    time_t out_time;
    int keys;
    int breakfast;
    double payment;
} person_t;

typedef struct room {
    room_type type;
    int number;
    int capacity;
    int vacant;     // Not loaded from database.

    time_t begin;   // Not loaded...
    time_t due;     // Not loaded...
    list_t *people;
} room_t;

typedef struct hotel {
    const char *database;
    list_t *events;
    list_t *messages;
    card_handle_func_t card_handle_func;
    message_handle_func_t message_handle_func;

    list_t *rooms;
    ordered_set_t *people_id;
    ordered_set_t *people_name;
    list_t *garbage;

    time_t today;
    double price[4];
    double breakfast;
    double income;
} hotel_t;


hotel_t *hotel_init(const char *);

void hotel_free(hotel_t **);

person_t *person_init(const char *, const char *);

room_t *room_init(int, hotel_t *);

char *read_file(const char *);

void load_db(hotel_t *);

void save_db(hotel_t *);

void update_room(room_t *, hotel_t *);

void update_hotel(hotel_t *);

void push_message(const char *, hotel_t *);

void pop_message(char *, hotel_t *hotel);

room_t *find_room_by_number(int, hotel_t *);

person_t *find_person_by_id(const char *, hotel_t *);

#endif //HOTEL_HOTEL_DB_H
