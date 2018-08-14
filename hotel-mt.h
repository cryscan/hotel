//
// Created by lepet on 6/30/2018.
//

#ifndef HOTEL_HOTEL_MT_H
#define HOTEL_HOTEL_MT_H

#include "hotel-ll.h"
#include "hotel-db.h"

typedef enum {
    CHECKIN, CHECKOUT, INCREASE
} event_type;

typedef struct event {
    event_type type;
    person_t *person;
    void *args;
} event_t;


event_t *event_init();

void event_free(event_t **);

void event_list_handle(hotel_t *);

list_t *find_available_rooms(room_type, hotel_t *);

list_t *find_non_vacant_rooms(room_type, hotel_t *);

void auto_checkout(hotel_t *);

void clearing(hotel_t *);

void next_day(hotel_t *);

void card_free(list_t **);

void card_save(const char *, list_t *);

#endif //HOTEL_HOTEL_MT_H
