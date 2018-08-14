//
// Created by lepet on 6/30/2018.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <memory.h>
#include <limits.h>
#include <signal.h>

#include "hotel-mt.h"

#define MAX_STR_LEN 1024


list_t *card_checkin(person_t *person, hotel_t *hotel);

list_t *card_checkout(person_t *person, hotel_t *hotel);


event_t *event_init() {
    event_t *event = malloc(sizeof(event_t));
    assert(event != NULL);

    memset(event, 0, sizeof(event_t));
    return event;
}

// Please free args first...
void event_free(event_t **pEvent) {
    event_t *event = *pEvent;
    free(event);

    *pEvent = NULL;
}


/* Return a list of available rooms with the same type */
int room_available_iter(room_t *room, pair_t *args) {
    room_type *type = args->key;
    list_t *list = args->value;

    if (room->vacant > 0 && room->type == *type)
        list_insert_end(list, room);
    return 0;
}

/* Return a list of available rooms with a given type */
list_t *find_available_rooms(room_type type, hotel_t *hotel) {
    list_t *list = list_init();
    pair_t args = {&type, list};
    list_iterate(hotel->rooms, (iter_func_t) room_available_iter, &args);
    return list;
}

int room_non_vacant_iter(room_t *room, pair_t *args) {
    room_type *type = args->key;
    list_t *list = args->value;

    if (room->vacant < room->capacity && room->type == *type)
        list_insert_end(list, room);
    return 0;
}

list_t *find_non_vacant_rooms(room_type type, hotel_t *hotel) {
    list_t *list = list_init();
    pair_t args = {&type, list};
    list_iterate(hotel->rooms, (iter_func_t) room_non_vacant_iter, &args);
    return list;
}

int double_cmp_func(const double *d1, const double *d2) {
    return (int) (*d2 - *d1); // positive if d1 < d2.
}

int time_cmp_func(const time_t *t1, const time_t *t2) {
    return (int) difftime(*t1, *t2);
}

room_t *find_best_dorm(hotel_t *hotel, person_t *person) {
    list_t *dorms = find_available_rooms(DORM, hotel);
    if (list_empty(dorms))
        return NULL;

    // The strategy to find the best room is to define a gain function for each room depending on the customer.
    // The goal is to maximize the average long-term gain of people in the rooms.
    // More specifically, consider a dorm with 'i(t)' visitors at day 't' from today.
    // So that each person should pay 'P / i' at the day, where 'P' is the price (constant) of the dorm,
    // 'T_i' the nights the i-th person will stay.
    // The gain is defined as 'G_i(t) = (P / i - P / (i + 1))'.
    // This represents the benefit he who is already in the room will receive.
    // The gain of the new comer is defined as 'G_a(t) = P_s - P / (i + 1)', where P_s is the price of single room.
    // This means how much benefit he could get w.r.t. booking a single room.
    // The average gain of the day is thus 'G(t) = gamma^t * (G_i(t) * i + G_a(t)) / (i + 1)'.
    // Here, we take 'gamma = 0.9'.
    // The long-term gain is thus sum of 'G(t)' from 't = 0' to 't = due of the room'.

    int n = list_length(dorms);
    ordered_map_t *queue = map_init((cmp_func_t) double_cmp_func);  // map as a priority queue.

    double gamma = 0.9;
    double *total_gain = malloc(n * sizeof(double));
    memset(total_gain, 0, n * sizeof(double));

    double price_dorm = hotel->price[DORM];
    double price_single = hotel->price[SINGLE];

    // For each room.
    for (int i = 0; i < n; ++i) {
        room_t *room = list_get(dorms, i + 1);

        int room_span = (int) difftime((time_t) fmax(room->due, person->out_time), hotel->today);
        room_span = room_span / 86400;
        time_t time = hotel->today;

        ordered_set_t *dues = set_init((cmp_func_t) time_cmp_func);
        for (int j = 0; j < list_length(room->people); ++j) {
            person_t *roommate = list_get(room->people, j + 1);
            set_insert(dues, &roommate->out_time);
        }
        set_insert(dues, &person->out_time);

        // For each day when there is person in the room.
        for (int j = 0; j < room_span; ++j) {
            double daily_gain = 0;

            for (time_t *due = set_get_minimum(dues);
                 due != NULL && *due <= time;
                 due = set_get_minimum(dues))
                set_delete(dues, due);  // Remove all out-dated dues.

            int m = set_size(dues); // Total number of people with the new comer.
            if (m > 1) {
                double roommate_gain = price_dorm / (m - 1) - price_dorm / m;
                double person_gain = price_single - price_dorm / m;
                daily_gain = pow(gamma, j) * (roommate_gain * (m - 1) + person_gain) / m;
            } else
                daily_gain = pow(gamma, j) * (price_single - price_dorm) * m;

            total_gain[i] += daily_gain;
            time += 86400;
        }
        map_insert(queue, &total_gain[i], room);
    }

    room_t *room = map_get_minimum(queue);
    map_free(&queue);
    free(total_gain);

    assert(room != NULL);
    return room;
}

void event_handle_checkin(event_t *event, hotel_t *hotel) {
    person_t *person = event->person;
    list_t *people = event->args;
    room_t *room = NULL;

    if (person->type != DORM) {
        // args: person2, ...
        list_insert_begin(people, person);
        person->type = (room_type) list_length(people); // Not necessary but to ensure he has the correct type.

        list_t *rooms = find_available_rooms(person->type, hotel);
        if (list_empty(rooms)) {
            push_message("No Available Room!", hotel);
            list_merge(hotel->garbage, people);
            return;
        }
        room = list_get(rooms, 1);
        list_free(&room->people);
        room->people = people;
        room->begin = hotel->today;
        room->due = person->out_time;
    } else {
        // args: empty.
        list_t *rooms = find_available_rooms(person->type, hotel);
        if (list_empty(rooms)) {
            push_message("No Available Room!", hotel);
            list_insert_end(hotel->garbage, person);
            return;
        }

        room = find_best_dorm(hotel, person);
        assert(room != NULL);
        list_insert_end(room->people, person);
    }

    update_room(room, hotel);
    save_db(hotel);

    list_t *card = card_checkin(person, hotel);
    if (hotel->card_handle_func != NULL)
        hotel->card_handle_func("Check-in Card", card);
    card_free(&card);
}

void event_handle_checkout(event_t *event, hotel_t *hotel) {
    person_t *person = event->person;
    room_t *room = person->room;
    int i = list_search(room->people, person);

    list_remove_after(room->people, i - 1);
    set_delete(hotel->people_name, person);
    set_delete(hotel->people_id, person);
    // map_delete(hotel->people_id, person->id);

    if (person->payment > 0 && hotel->card_handle_func != NULL) {
        char filename[MAX_STR_LEN] = "./receipts/";
        strcat(filename, person->name);
        strcat(filename, ".txt");

        list_t *card = card_checkout(person, hotel);
        hotel->card_handle_func("Check-out Card", card);
        card_save(filename, card);
        card_free(&card);
    }

    //free(person->id);
    //free(person->name);
    //free(person);
    list_insert_end(hotel->garbage, person);

    update_room(room, hotel);
    save_db(hotel);
}

void event_handle_increase(event_t *event, hotel_t *hotel) {
    person_t *person = event->person;
    room_t *room = person->room;
    if (person->type != DORM)
        person = list_get(room->people, 1);

    // args: nights and breakfast increased.
    pair_t *args = event->args;
    int nights = *(int *) args->key;
    int breakfast = *(int *) args->value;

    person->out_time += nights * 86400;
    person->breakfast += breakfast;
    hotel->income += hotel->breakfast * breakfast;

    char str[MAX_STR_LEN];
    sprintf(str, "%s (%s) of Room %d has asked to increase %d nights and %d breakfast!",
            person->name, person->id, room->number, nights, breakfast);
    push_message(str, hotel);

    free(args->key);
    free(args->value);
    free(args);
    update_room(room, hotel);
    save_db(hotel);
}


void event_handle(event_t *event, hotel_t *hotel) {
    if (event == NULL)
        return;
    switch (event->type) {
        case CHECKIN:
            event_handle_checkin(event, hotel);
            break;
        case CHECKOUT:
            event_handle_checkout(event, hotel);
            break;
        case INCREASE:
            event_handle_increase(event, hotel);
            break;
    }

    if (hotel->message_handle_func != NULL) {
        while (!list_empty(hotel->messages)) {
            char temp[MAX_STR_LEN];
            pop_message(temp, hotel);
            hotel->message_handle_func(temp);
        }
    }
}

void event_list_handle(hotel_t *hotel) {
    list_t *events = hotel->events;
    while (!list_empty(events)) {
        event_t *event = list_get(events, 1);
        list_remove_begin(events);
        event_handle(event, hotel);
        event_free(&event);
    }
}


int person_auto_checkout_iter(person_t *person, hotel_t *hotel) {
    if (person->out_time <= hotel->today) {
        event_t *event = event_init();
        event->type = CHECKOUT;
        event->person = person;
        list_insert_end(hotel->events, event);
    }

    return 0;
}

/* Checkout all customers out of due */
void auto_checkout(hotel_t *hotel) {
    set_iterate(hotel->people_id, (iter_func_t) person_auto_checkout_iter, hotel);
}


list_t *card_checkin(person_t *person, hotel_t *hotel) {
    list_t *pairs = list_init();
    room_t *room = person->room;
    char temp[MAX_STR_LEN];

    struct tm time;
    memcpy(&time, localtime(&person->in_time), sizeof(struct tm));
    strftime(temp, sizeof(temp), "%d/%m/%Y", &time);
    string_pair_insert(pairs, "Arrival", temp);

    sprintf(temp, "%d", room->number);
    string_pair_insert(pairs, "Room Number", temp);

    sprintf(temp, "%d", person->keys);
    string_pair_insert(pairs, "Keys", temp);

    if (room->type == DORM) {
        string_pair_insert(pairs, "Your Name", person->name);
        string_pair_insert(pairs, "Your ID", person->id);

        for (int i = 0; i < list_length(room->people) - 1; ++i) {
            person = list_get(room->people, i + 1);
            sprintf(temp, "Name of Roommate %d", i + 1);
            string_pair_insert(pairs, temp, person->name);
        }
    } else {
        sprintf(temp, "%d", room->type);
        string_pair_insert(pairs, "Number of Visitors", temp);

        for (int i = 0; i < room->type; ++i) {
            person = list_get(room->people, i + 1);
            sprintf(temp, "Name of Visitor %d", i + 1);
            string_pair_insert(pairs, temp, person->name);

            sprintf(temp, "ID of Visitor %d", i + 1);
            string_pair_insert(pairs, temp, person->id);
        }
    }

    return pairs;
}

list_t *card_checkout(person_t *person, hotel_t *hotel) {
    list_t *pairs = list_init();
    room_t *room = person->room;
    char temp[MAX_STR_LEN];

    string_pair_insert(pairs, "Name", person->name);
    string_pair_insert(pairs, "ID", person->id);

    sprintf(temp, "%d", room->number);
    string_pair_insert(pairs, "Room Number", temp);

    struct tm time;
    memcpy(&time, localtime(&person->out_time), sizeof(struct tm));
    strftime(temp, sizeof(temp), "%d/%m/%Y", &time);
    string_pair_insert(pairs, "Departure", temp);

    int nights = (int) (difftime(hotel->today, person->in_time) / 86400);
    sprintf(temp, "%d", nights);
    string_pair_insert(pairs, "Night(s) Spent", temp);

    sprintf(temp, "%d", person->breakfast);
    string_pair_insert(pairs, "Breakfast Served", temp);

    sprintf(temp, "%.2lf", person->payment);
    string_pair_insert(pairs, "Total Price", temp);

    return pairs;
}


int card_free_iter(pair_t *pair) {
    string_pair_free(&pair);
    return 0;
}

void card_free(list_t **pList) {
    list_t *card = *pList;
    list_iterate(card, (iter_func_t) card_free_iter, 0);
    list_free(pList);
}

int card_write_iter(pair_t *pair, FILE *fp) {
    char *key = pair->key;
    char *value = pair->value;
    fprintf(fp, "- %s: %s\n", key, value);

    return 0;
}

void card_save(const char *filename, list_t *card) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
        return;

    list_iterate(card, (iter_func_t) card_write_iter, fp);
    fclose(fp);
}


int room_income_update_iter(room_t *room, hotel_t *hotel) {
    if (room->type != DORM) {
        double payment = 0;
        person_t *person = list_get(room->people, 1);
        if (person != NULL) {
            payment += hotel->price[room->type];
            if (person->in_time == hotel->today)
                payment += hotel->breakfast * person->breakfast;

            person->payment += payment;
            hotel->income += payment;
        }
    } else {
        int n = list_length(room->people);
        for (int i = 0; i < n; ++i) {
            double payment = 0;
            person_t *person = list_get(room->people, i + 1);
            payment += hotel->price[DORM] / n;
            if (person->in_time == hotel->today)
                payment += hotel->breakfast * person->breakfast;

            person->payment += payment;
            hotel->income += payment;
        }
    }
    return 0;
}

/* Update hotel income and enter a new day */
void clearing(hotel_t *hotel) {
    list_iterate(hotel->rooms, (iter_func_t) room_income_update_iter, hotel);
    save_db(hotel);
}

void next_day(hotel_t *hotel) {
    hotel->today += 86400;
    hotel->income = 0;
    save_db(hotel);
}