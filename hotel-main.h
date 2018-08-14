//
// Created by lepet on 7/9/2018.
//

#ifndef HOTEL_HOTEL_IO_H
#define HOTEL_HOTEL_IO_H

typedef struct namelist {
    char **first_names;
    char **last_names;
    int first_length;
    int last_length;
} namelist_t;


hotel_t *generate_hotel(const char *);

void generate_events(namelist_t *, int *, hotel_t *);

void read_namelist(namelist_t *, const char *, int);

int message_handler(const char *);

void prompt(const char *, char *);

int card_handler(const char *, list_t *);

void report_income(hotel_t *);

void report_day(hotel_t *);

void prompt_checkin(hotel_t *);

void prompt_checkout(hotel_t *);

void prompt_clearing(hotel_t *);

void print_people(hotel_t *);

void user_command(hotel_t *, int *);

#endif //HOTEL_HOTEL_IO_H
