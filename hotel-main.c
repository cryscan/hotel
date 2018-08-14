/* The main source file */


#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <memory.h>
#include <ctype.h>
#include <signal.h>

#include "hotel.h"
#include "hotel-main.h"

#define MAX_STR_LEN     1024
#define HOTEL_CLI       1
// Uncomment the next line to enable the demo mode.
// #define HOTEL_DEMO      1

#define MAX_EVENTS      64
#define MAX_NIGHTS      32
#define MAX_KEYS        2


int main(int argc, char *argv[]) {
#ifdef HOTEL_DEMO
    srand(0);

    int total_people = 0;
    namelist_t namelist = {0};
    read_namelist(&namelist, "./data/FirstNames.CSV", 0);
    read_namelist(&namelist, "./data/LastNames.CSV", 1);

    char temp[MAX_STR_LEN];
    prompt("Enter time span of demonstration:", temp);
    int span = strtol(temp, NULL, 10);

    for (int i = 0; i < span; ++i) {
        hotel_t *hotel;
        hotel = generate_hotel("./data/hotel.db");
        report_day(hotel);
        auto_checkout(hotel);
        event_list_handle(hotel);

        // Generate events of a day.
        generate_events(&namelist, &total_people, hotel);
        event_list_handle(hotel);

        clearing(hotel);
        report_income(hotel);
        next_day(hotel);

        print_people(hotel);

        hotel_free(&hotel);
        getchar();
    }
#else
    printf("Welcome to Hotel Management System!\n"
           "Available Commands:\n"
           "checkin - check in one or several person/people\n"
           "checkout - check out by id\n"
           "increase - increase nights and breakfast\n"
           "clearing - report income and enter a new day\n"
           "print - print all people in alphabet order\n"
           "exit - terminate the program\n");

    while (1) {
        int exit = 0;
        hotel_t *hotel = generate_hotel("./data/hotel.db");
        user_command(hotel, &exit);
        event_list_handle(hotel);
        if (exit != 0)
            break;
        hotel_free(&hotel);
    }
#endif

    return 0;
}


void read_namelist(namelist_t *namelist, const char *filename, int type) {
    char *buf = read_file(filename);
    char *ptr = buf;
    char *next = strstr(ptr, "\r\n");

    int n = 0;
    while (next != NULL) {
        ptr = next + strlen("\r\n");
        memset(next, '\0', strlen("\r\n"));
        next = strstr(ptr, "\r\n");
        n++;
    }

    char **names;
    if (type == 0) {
        namelist->first_names = calloc((size_t) n, sizeof(char *));
        names = namelist->first_names;
        namelist->first_length = n;
    } else {
        namelist->last_names = calloc((size_t) n, sizeof(char *));
        names = namelist->last_names;
        namelist->last_length = n;
    }

    ptr = buf;
    for (int i = 0; i < n; ++i) {
        names[i] = ptr;
        ptr += strlen(ptr) + strlen("\r\n");
    }
}


hotel_t *generate_hotel(const char *database) {
    hotel_t *hotel = hotel_init(database);
    hotel->card_handle_func = card_handler;
    hotel->message_handle_func = message_handler;

    if (list_empty(hotel->rooms)) {
        double price[] = {180, 100, 180, 240};
        memcpy(hotel->price, price, sizeof(price));
        hotel->breakfast = 40;

        char temp[MAX_STR_LEN];
        prompt("Enter current day (dd/mm/yyyy):", temp);
        struct tm date = {0};
        sscanf(temp, "%d/%d/%d", &date.tm_mday, &date.tm_mon, &date.tm_year);
        date.tm_mon--;
        date.tm_year -= 1900;
        hotel->today = mktime(&date);

        prompt("Enter dorm number:", temp);
        int n = strtol(temp, NULL, 10);
        for (int i = 101; i < n + 101; ++i) {
            room_t *room = room_init(i, hotel);
            room->type = DORM;
            room->capacity = 4;
        }

        prompt("Enter single room number:", temp);
        n = strtol(temp, NULL, 10);
        for (int i = 201; i < n + 201; ++i) {
            room_t *room = room_init(i, hotel);
            room->type = SINGLE;
        }

        prompt("Enter double room number:", temp);
        n = strtol(temp, NULL, 10);
        for (int i = 301; i < n + 301; ++i) {
            room_t *room = room_init(i, hotel);
            room->type = DOUBLE;
        }

        prompt("Enter family room number:", temp);
        n = strtol(temp, NULL, 10);
        for (int i = 401; i < n + 401; ++i) {
            room_t *room = room_init(i, hotel);
            room->type = FAMILY;
        }
    }

    update_hotel(hotel);
    save_db(hotel);
    return hotel;
}


void generate_checkin(namelist_t *namelist, int *total_people, hotel_t *hotel) {
    char **first_names = namelist->first_names;
    char **last_names = namelist->last_names;
    int first_length = namelist->first_length;
    int last_length = namelist->last_length;

    char name[MAX_STR_LEN];
    char id[MAX_STR_LEN];

    int random = rand(); // NOLINT
    int type = random % 4;
    int nights = random % MAX_NIGHTS + 1;
    int breakfast = type == DORM ? random % nights : random % (type * nights) + 1;
    int keys = random % MAX_KEYS + 1;

    char *first = first_names[random % first_length];
    char *last = last_names[random % last_length];
    sprintf(name, "%s %s", first, last);
    sprintf(id, "%08d", *total_people);
    *total_people += 1;

    person_t *person = person_init(name, id);
    person->type = (room_type) type;
    person->in_time = hotel->today;
    person->out_time = person->in_time + nights * 86400;
    person->breakfast = breakfast;
    person->keys = keys;

    event_t *event = event_init();
    event->type = CHECKIN;
    event->person = person;

    if (type != DORM) {
        list_t *people = list_init();
        for (int j = 1; j < type; ++j) {
            random = rand(); // NOLINT
            first = first_names[random % first_length];
            last = last_names[random % last_length];
            sprintf(name, "%s %s", first, last);
            sprintf(id, "%08d", *total_people);
            *total_people += 1;

            person = person_init(name, id);
            list_insert_end(people, person);
        }
        event->args = people;
    }
    list_insert_end(hotel->events, event);
}

void generate_increase(hotel_t *hotel) {
    int random = rand(); // NOLINT
    room_type type = (room_type) (rand() % 4); // NOLINT

    list_t *rooms = find_non_vacant_rooms(type, hotel);
    if (list_empty(rooms))
        return;
    room_t *room = list_get(rooms, random % list_length(rooms) + 1);

    random = rand(); // NOLINT
    list_t *people = room->people;
    person_t *person = list_get(people, random % list_length(people) + 1);
    int *nights = malloc(sizeof(int));
    int *breakfast = malloc(sizeof(int));
    *nights = random % MAX_NIGHTS + 1;
    *breakfast = type == DORM ? random % *nights : random % (type * *nights);

    event_t *event = event_init();
    event->person = person;
    event->type = INCREASE;
    pair_t *args = pair_init(nights, breakfast);
    event->args = args;
    list_insert_end(hotel->events, event);
}

void generate_events(namelist_t *namelist, int *total_people, hotel_t *hotel) {
    int random = rand(); // NOLINT
    int events = random % MAX_EVENTS;

    for (int i = 0; i < events; ++i) {
        double uniform = ((double) rand()) / RAND_MAX; // NOLINT
        if (uniform < 0.8)
            generate_checkin(namelist, total_people, hotel);
        else
            generate_increase(hotel);
    }
}


/* IO Methods */

#ifdef HOTEL_CLI

int message_handler(const char *str) {
    printf("... %s ...\n", str);
    return 0;
}

void prompt(const char *str, char *dest) {
    printf("%s\n", str);
    printf(">> ");
    scanf("%s", dest);
}

int pair_print_iter(pair_t *pair) {
    char *key = pair->key;
    char *value = pair->value;
    printf("- %s: %s\n", key, value);

    return 0;
}

int card_handler(const char *title, list_t *pairs) {
    printf("\n... %s ...\n", title);
    list_iterate(pairs, (iter_func_t) pair_print_iter, 0);
    printf("......\n\n");

    return 0;
}


void report_income(hotel_t *hotel) {
    char date[12];
    struct tm time = {0};
    memcpy(&time, localtime(&hotel->today), sizeof(struct tm));
    strftime(date, sizeof(date), "%d/%m/%Y", &time);

    char str[MAX_STR_LEN];
    sprintf(str, "Income of the day %s: %.2lf", date, hotel->income);
    message_handler(str);
}

void report_day(hotel_t *hotel) {
    struct tm time = {0};
    memcpy(&time, localtime(&hotel->today), sizeof(struct tm));

    char *str[64];
    strftime((char *) str, sizeof(str), "Current day: %d/%m/%Y", &time);
    message_handler((const char *) str);
}

void prompt_checkin(hotel_t *hotel) {
    char temp[MAX_STR_LEN];
    room_type type;

    prompt("Please enter room type:", temp);
    if (strcmp(temp, "dorm") == 0)
        type = DORM;
    else if (strcmp(temp, "single") == 0)
        type = SINGLE;
    else if (strcmp(temp, "double") == 0)
        type = DOUBLE;
    else if (strcmp(temp, "family") == 0)
        type = FAMILY;
    else {
        message_handler("No Such Room Type!");
        return;
    }

    list_t *available = find_available_rooms(type, hotel);
    if (list_empty(available)) {
        message_handler("No Available Rooms!");
        return;
    }

    prompt("Please enter your first name:", temp);
    char first_name[MAX_STR_LEN / 2];
    strcpy(first_name, temp);

    prompt("And your last name:", temp);
    char last_name[MAX_STR_LEN / 2];
    strcpy(last_name, temp);

    char name[MAX_STR_LEN];
    sprintf(name, "%s %s", first_name, last_name);

    prompt("Please enter your ID:", temp);
    char id[MAX_STR_LEN];
    strcpy(id, temp);

    person_t *person = person_init(name, id);
    person->in_time = hotel->today;
    person->type = type;

    prompt("How many nights would you like to book:", temp);
    int nights = strtol(temp, NULL, 10);
    person->out_time = person->in_time + nights * 86400;

    prompt("How many breakfast tickets would you like to get:", temp);
    person->breakfast = strtol(temp, NULL, 10);

    prompt("How many keys would you like to have:", temp);
    person->keys = strtol(temp, NULL, 10);


    event_t *event = event_init();
    event->type = CHECKIN;
    event->person = person;

    list_t *people = list_init();
    if (person->type != DORM) {
        for (int i = 1; i < type; ++i) {
            char str[MAX_STR_LEN];

            sprintf(str, "Please Enter the first name of member %d", i + 1);
            prompt(str, temp);
            strcpy(first_name, temp);

            sprintf(str, "And the last name of member %d", i + 1);
            prompt(str, temp);
            strcpy(last_name, temp);

            sprintf(name, "%s %s", first_name, last_name);

            sprintf(str, "Please Enter the ID of member %d", i + 1);
            prompt(str, temp);
            strcpy(id, temp);

            person = person_init(name, id);
            list_insert_end(people, person);
        }
    }

    event->args = people;
    list_insert_end(hotel->events, event);
}


int person_print_iter(person_t *person) {
    printf("Name: %24s, ID: %8s, Room: %d\n", person->name, person->id, person->room->number);
    return 0;
}

int person_checkout_iter(person_t *person, hotel_t *hotel) {
    event_t *event = event_init();
    event->type = CHECKOUT;
    event->person = person;
    list_insert_end(hotel->events, event);

    return 0;
}

void prompt_checkout(hotel_t *hotel) {
    char temp[MAX_STR_LEN];
    prompt("Please enter your ID:", temp);
    person_t *person = find_person_by_id(temp, hotel);
    if (person == NULL) {
        message_handler("Cannot find person with such ID");
        printf("Hint: you can look up all people by the command print.\n");
        return;
    }

    room_t *room = person->room;
    if (room->type != DORM)
        list_iterate(room->people, (iter_func_t) person_checkout_iter, hotel);
    else
        person_checkout_iter(person, hotel);
}

void prompt_increase(hotel_t *hotel) {
    char temp[MAX_STR_LEN];
    prompt("Please enter your ID:", temp);
    person_t *person = find_person_by_id(temp, hotel);
    if (person == NULL) {
        message_handler("Cannot find person with such ID");
        printf("Hint: you can look up all people by the command print.\n");
        return;
    }

    room_t *room = person->room;
    if (room->type != DORM)
        person = list_get(room->people, 1);

    int *nights, *breakfast;
    nights = malloc(sizeof(int));
    breakfast = malloc(sizeof(int));

    prompt("Enter night(s) you want to increase:", temp);
    *nights = strtol(temp, NULL, 10);
    prompt("Enter breakfast tickets you want to increase:", temp);
    *breakfast = strtol(temp, NULL, 10);

    pair_t *args = pair_init(nights, breakfast);
    event_t *event = event_init();
    event->type = INCREASE;
    event->person = person;
    event->args = args;
    list_insert_end(hotel->events, event);
}

// Return those who have not checked out but should have.
int person_check_iter(person_t *person, pair_t *args) {
    list_t *list = args->key;
    hotel_t *hotel = args->value;

    if (person->out_time <= hotel->today)
        list_insert_end(list, person);

    return 0;
}


void prompt_clearing(hotel_t *hotel) {
    list_t *unchecked = list_init();
    pair_t args = {unchecked, hotel};
    set_iterate(hotel->people_id, (iter_func_t) person_check_iter, &args);

    if (!list_empty(unchecked)) {
        char str[MAX_STR_LEN];
        sprintf(str, "There are %d people unchecked-out yet.\n", list_length(unchecked));
        message_handler(str);

        // Print those people.
        list_iterate(unchecked, (iter_func_t) person_print_iter, 0);

        prompt("Perform Auto-checkout? Answer [Y]es or press any key to abort.", str);
        if (strcmp(str, "Y") == 0) {
            auto_checkout(hotel);
            event_list_handle(hotel);
        } else
            return;
    }

    clearing(hotel);
    report_income(hotel);
    next_day(hotel);
    report_day(hotel);
}

void print_people(hotel_t *hotel) {
    printf("All people:\n");
    set_iterate(hotel->people_name, (iter_func_t) person_print_iter, 0);
    printf("Total number: %d", set_size(hotel->people_id));
}


void user_command(hotel_t *hotel, int *exit) {
    char temp[MAX_STR_LEN];
    prompt("", temp);
    if (strcmp(temp, "checkin") == 0)
        prompt_checkin(hotel);
    else if (strcmp(temp, "checkout") == 0)
        prompt_checkout(hotel);
    else if (strcmp(temp, "increase") == 0)
        prompt_increase(hotel);
    else if (strcmp(temp, "clearing") == 0)
        prompt_clearing(hotel);
    else if (strcmp(temp, "print") == 0)
        print_people(hotel);
    else if (strcmp(temp, "exit") == 0)
        *exit = -1;
    else
        message_handler("Unknown prompt!");
}

#endif