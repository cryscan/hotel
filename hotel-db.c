//
// Created by lepet on 6/30/2018.
// Read the database.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <sys/stat.h>
#include <ctype.h>
#include <limits.h>

#include "hotel-ll.h"
#include "hotel-db.h"

#define MAX_STR_LEN 1024


person_t *person_init(const char *name, const char *id) {
    person_t *person = malloc(sizeof(person_t));
    assert(person != NULL);
    memset(person, 0, sizeof(person_t));

    char *str = malloc(strlen(name) + 1);
    strcpy(str, name);
    person->name = str;

    str = malloc(strlen(id) + 1);
    strcpy(str, id);
    person->id = str;
    return person;
}

room_t *room_init(const int number, hotel_t *hotel) {
    // Start a new room.
    room_t *room = malloc(sizeof(room_t));
    assert(room != NULL);
    memset(room, 0, sizeof(room_t));
    room->number = number;
    room->people = list_init();
    room->begin = UINT_MAX;
    room->due = 0;

    // Insert the room into hotel's room list.
    list_insert_end(hotel->rooms, room);
    return room;
}


// These are necessary for passing the compile...
int fileno(FILE *); // NOLINT

// Read the whole file into memory.
char *read_file(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
        return NULL;

    struct stat s;
    fstat(fileno(fp), &s);

    char *buf = malloc((size_t) s.st_size + 1);
    fread(buf, (size_t) s.st_size, 1, fp);
    buf[s.st_size] = '\0';

    fclose(fp);
    return buf;
}


// Remove white spaces at the beginning and the end of a string.
char *trim(char *str, char *end) {
    while (isspace(*str))
        str++;
    while (isspace(*(--end)));
    end[1] = '\0';
    return str;
}

// Parse a field into a pair.
pair_t *parse_field(char *str, char *end) {
    char *eq = strchr(str, '=');
    if (eq == NULL)
        return NULL; // Not a field.

    char *key = trim(str, eq);
    char *value = trim(eq + 1, end);
    return pair_init(key, value);
}

void hotel_add_attribute(pair_t *pair, hotel_t *hotel) {
    char *key = pair->key;
    char *value = pair->value;
    if (strcmp(key, "today") == 0) {
        struct tm date = {0};
        sscanf(value, "%d/%d/%d", &date.tm_mday, &date.tm_mon, &date.tm_year);
        date.tm_mon--;
        date.tm_year -= 1900;
        hotel->today = mktime(&date);
    } else if (strcmp(key, "single") == 0)
        hotel->price[SINGLE] = strtod(value, NULL);
    else if (strcmp(key, "double") == 0)
        hotel->price[DOUBLE] = strtod(value, NULL);
    else if (strcmp(key, "family") == 0)
        hotel->price[FAMILY] = strtod(value, NULL);
    else if (strcmp(key, "dorm") == 0)
        hotel->price[DORM] = strtod(value, NULL);
    else if (strcmp(key, "breakfast") == 0)
        hotel->breakfast = strtod(value, NULL);
}

// Insert people to the room so that the number of people_id in the room is at least n.
void insert_null_people(room_t *room, int n) {
    for (int i = 0; i < n; ++i) {
        if (list_get(room->people, i + 1) != NULL)
            continue;
        else {
            person_t *person = malloc(sizeof(person_t));
            memset(person, 0, sizeof(person_t));
            list_insert_end(room->people, person);
        }
    }
}

void room_add_attribute_type(const char *type, room_t *room) {
    if (strcmp(type, "single") == 0)
        // If it's a single room, initialize one person.
        room->type = SINGLE;
    else if (strcmp(type, "double") == 0)
        room->type = DOUBLE;
    else if (strcmp(type, "family") == 0)
        room->type = FAMILY;
    else if (strcmp(type, "dorm") == 0)
        room->type = DORM;
    else
        return; // Unknown type.
}

void room_add_attribute_personal(pair_t *pair, room_t *room) {
    char *key = pair->key;
    char *value = pair->value;

    if (strncmp(key, "name", strlen("name")) == 0) {
        int n = strtol(key + strlen("name"), NULL, 10);
        insert_null_people(room, n);
        person_t *person = list_get(room->people, n);

        char *name = malloc(strlen(value) + 1);
        memcpy(name, value, strlen(value) + 1);
        person->name = name;
    } else if (strncmp(key, "id", strlen("id")) == 0) {
        int n = strtol(key + strlen("id"), NULL, 10);
        insert_null_people(room, n);
        person_t *person = list_get(room->people, n);

        char *id = malloc(strlen(value) + 1);
        memcpy(id, value, strlen(value) + 1);
        person->id = id;
    } else if (strncmp(key, "arrival", strlen("arrival")) == 0) {
        // If is not dorm, all attributes is added to member 1.
        int n = room->type == DORM ? strtol(key + strlen("arrival"), NULL, 10) : 1;
        insert_null_people(room, n);
        person_t *person = list_get(room->people, n);

        struct tm date = {0};
        sscanf(value, "%d/%d/%d", &date.tm_mday, &date.tm_mon, &date.tm_year);
        date.tm_mon--;
        date.tm_year -= 1900;
        person->in_time = mktime(&date);
        room->begin = room->begin > person->in_time ? person->in_time : room->begin;
    } else if (strncmp(key, "departure", strlen("departure")) == 0) {
        int n = room->type == DORM ? strtol(key + strlen("departure"), NULL, 10) : 1;
        insert_null_people(room, n);
        person_t *person = list_get(room->people, n);

        struct tm date = {0};
        sscanf(value, "%d/%d/%d", &date.tm_mday, &date.tm_mon, &date.tm_year);
        date.tm_mon--;
        date.tm_year -= 1900;
        person->out_time = mktime(&date);
        room->due < person->out_time ? room->due = person->out_time : room->due;
    } else if (strncmp(key, "breakfast", strlen("breakfast")) == 0) {
        int n = room->type == DORM ? strtol(key + strlen("breakfast"), NULL, 10) : 1;
        insert_null_people(room, n);
        person_t *person = list_get(room->people, n);

        person->breakfast = strtol(value, NULL, 10);
    } else if (strncmp(key, "keys", strlen("keys")) == 0) {
        int n = room->type == DORM ? strtol(key + strlen("keys"), NULL, 10) : 1;
        insert_null_people(room, n);
        person_t *person = list_get(room->people, n);

        person->keys = strtol(value, NULL, 10);
    } else if (strncmp(key, "payment", strlen("payment")) == 0) {
        int n = room->type == DORM ? strtol(key + strlen("payment"), NULL, 10) : 1;
        insert_null_people(room, n);
        person_t *person = list_get(room->people, n);

        person->payment = strtod(value, NULL);
    }
}

// Add an attribute to the room.
void room_add_attribute(pair_t *pair, room_t *room) {
    if (strcmp(pair->key, "type") == 0)
        room_add_attribute_type(pair->value, room);
    else if (strcmp(pair->key, "capacity") == 0)
        room->capacity = strtol(pair->value, NULL, 10);
    else if (strcmp(pair->key, "visitors") == 0) {
        assert(room->type == DORM);
        int visitors = strtol(pair->value, NULL, 10);
        insert_null_people(room, visitors);
    } else
        room_add_attribute_personal(pair, room);
}

void parse_file(char *buf, hotel_t *hotel) {
    room_t *room = NULL;
    char *str = buf, *end;
    while ((end = strchr(str, '\n')) != NULL) {
        char *next = end + 1;
        char *comment = strchr(str, '#');
        if (comment != NULL && comment < end)
            end = comment;
        *end = '\0';

        if (*str == '[') {
            end = strchr(str, ']');
            assert(end != NULL);    // Syntax error.
            if (strncmp(str + 1, "room ", strlen("room ")) == 0) {
                int number = strtol(str + 1 + strlen("room "), NULL, 10);
                room = room_init(number, hotel);
            }
        } else if (isalpha(*str)) {
            pair_t *field = parse_field(str, end);
            if (field != NULL) {
                if (list_empty(hotel->rooms))
                    hotel_add_attribute(field, hotel);
                else
                    room_add_attribute(field, room);
                free(field);
            }
        }
        str = next;
    }
}

void load_db(hotel_t *hotel) {
    char *buf = read_file(hotel->database);
    if (buf == NULL)
        return;

    parse_file(buf, hotel);
    update_hotel(hotel);
    free(buf);
}


void hotel_write(list_t *pairs, hotel_t *hotel) {
    char temp[MAX_STR_LEN];

    struct tm time;
    memcpy(&time, localtime(&hotel->today), sizeof(struct tm));
    strftime(temp, sizeof(temp), "%d/%m/%Y", &time);
    string_pair_insert(pairs, "today", temp);

    // Print price of all types.
    char *room_types[] = {"dorm", "single", "double", "family"};
    for (int i = 0; i < 4; ++i) {
        sprintf(temp, "%.2lf", hotel->price[i]);
        string_pair_insert(pairs, room_types[i], temp);
    }

    sprintf(temp, "%.2lf", hotel->breakfast);
    string_pair_insert(pairs, "breakfast", temp);
}

int room_write_iter(room_t *room, list_t *pairs) {
    char temp[MAX_STR_LEN];

    // Print room number.
    sprintf(temp, "%d", room->number);
    string_pair_insert(pairs, "room", temp);

    // Print room type.
    char *room_types[] = {"dorm", "single", "double", "family"};
    string_pair_insert(pairs, "type", room_types[room->type]);

    // Print room capacity.
    sprintf(temp, "%d", room->capacity);
    string_pair_insert(pairs, "capacity", temp);

    // For all people in the room.
    for (int i = 0; i < list_length(room->people); ++i) {
        person_t *person = list_get(room->people, i + 1);

        sprintf(temp, "name%d", i + 1);
        string_pair_insert(pairs, temp, person->name);

        sprintf(temp, "id%d", i + 1);
        string_pair_insert(pairs, temp, person->id);

        struct tm arrival, departure;
        memcpy(&arrival, localtime(&person->in_time), sizeof(struct tm));
        memcpy(&departure, localtime(&person->out_time), sizeof(struct tm));

        char in_date[12], out_date[12];
        strftime(in_date, sizeof(in_date), "%d/%m/%Y", &arrival);
        strftime(out_date, sizeof(out_date), "%d/%m/%Y", &departure);

        if (room->type == DORM) {
            sprintf(temp, "arrival%d", i + 1);
            string_pair_insert(pairs, temp, in_date);

            sprintf(temp, "departure%d", i + 1);
            string_pair_insert(pairs, temp, out_date);

            sprintf(temp, "keys%d", i + 1);
            sprintf(temp + strlen(temp) + 1, "%d", person->keys);
            string_pair_insert(pairs, temp, temp + strlen(temp) + 1);

            sprintf(temp, "breakfast%d", i + 1);
            sprintf(temp + strlen(temp) + 1, "%d", person->breakfast);
            string_pair_insert(pairs, temp, temp + strlen(temp) + 1);

            sprintf(temp, "payment%d", i + 1);
            sprintf(temp + strlen(temp) + 1, "%.2lf", person->payment);
            string_pair_insert(pairs, temp, temp + strlen(temp) + 1);
        } else {
            if (i > 0)
                continue;

            string_pair_insert(pairs, "arrival", in_date);
            string_pair_insert(pairs, "departure", out_date);

            sprintf(temp, "%d", person->keys);
            string_pair_insert(pairs, "keys", temp);

            sprintf(temp, "%d", person->breakfast);
            string_pair_insert(pairs, "breakfast", temp);

            sprintf(temp, "%.2lf", person->payment);
            string_pair_insert(pairs, "payment", temp);
        }
    }

    return 0;
}

int pair_write_iter(pair_t *pair, FILE *fp) {
    char temp[MAX_STR_LEN];
    char *key = pair->key;
    char *value = pair->value;

    if (strcmp(key, "room") == 0)
        sprintf(temp, "[%s %s]\n", key, value);
    else
        sprintf(temp, "%s = %s\n", key, value);

    string_pair_free(&pair);
    fputs(temp, fp);
    return 0;
}

void save_db(hotel_t *hotel) {
    list_t *pairs = list_init();
    hotel_write(pairs, hotel);
    list_iterate(hotel->rooms, (iter_func_t) room_write_iter, pairs);

    FILE *fp = fopen(hotel->database, "w");
    list_iterate(pairs, (iter_func_t) pair_write_iter, fp);
    fclose(fp);
    list_free(&pairs);
}


int person_cmp_id(person_t *person1, person_t *person2) {
    return strcmp(person1->id, person2->id);
}

int person_cmp_name(person_t *person1, person_t *person2) {
    return strcmp(person1->name, person2->name);
}

int room_free_iter(room_t *room) {
    list_free(&room->people);
    free(room);
    return 0;
}

int person_free_iter(person_t *person) {
    free(person->name);
    free(person->id);
    free(person);
    return 0;
}

hotel_t *hotel_init(const char *database) {
    hotel_t *hotel = malloc(sizeof(hotel_t));
    assert(hotel != NULL);
    memset(hotel, 0, sizeof(hotel_t));

    hotel->database = database;
    hotel->events = list_init();
    hotel->messages = list_init();

    hotel->rooms = list_init();
    hotel->people_id = set_init((cmp_func_t) person_cmp_id);
    hotel->people_name = set_init((cmp_func_t) person_cmp_name);
    hotel->garbage = list_init();

    load_db(hotel);
    return hotel;
}

void hotel_free(hotel_t **pHotel) {
    hotel_t *hotel = *pHotel;
    assert(hotel != NULL);

    list_iterate(hotel->rooms, (iter_func_t) room_free_iter, 0);
    list_free(&hotel->rooms);

    list_iterate(hotel->garbage, (iter_func_t) person_free_iter, 0);
    list_free(&hotel->garbage);

    set_iterate(hotel->people_id, (iter_func_t) person_free_iter, 0);
    set_free(&hotel->people_name);
    set_free(&hotel->people_id);

    list_free(&hotel->events);
    list_free(&hotel->messages);

    free(hotel);
    *pHotel = NULL;
}


int person_update_iter(person_t *person, pair_t *args) {
    room_t *room = args->key;
    hotel_t *hotel = args->value;

    person->type = room->type;
    person->room = room;

    if (room->type != DORM) {
        // People in non-dorm type rooms have the same in and out time.
        person->in_time = person->in_time == 0 ? room->begin : person->in_time;
        person->out_time = person->out_time == 0 ? room->due : person->out_time;
    }
    // Make begin of rooms be the earliest, while due the latest.
    room->begin = room->begin > person->in_time ? person->in_time : room->begin;
    room->due = room->due < person->out_time ? person->out_time : room->due;

    if (set_check(hotel->people_id, person) != person) {
        // The person is not in the sets.
        // map_insert(hotel->people_id, person->id, person);
        set_insert(hotel->people_id, person);
        set_insert(hotel->people_name, person);
    }
    return 0;
}

int room_update_iter(room_t *room, hotel_t *hotel) {
    // Initialize begin and due.
    if (room->type != DORM)
        room->capacity = room->type;
    room->vacant = room->capacity - list_length(room->people);
    // Reset begin and due, so that when people leave, it will not cause damage.
    room->begin = INT_MAX;
    room->due = 0;

    pair_t args = {room, hotel};
    list_iterate(room->people, (iter_func_t) person_update_iter, &args);
    return 0;
}

void update_room(room_t *room, hotel_t *hotel) {
    room_update_iter(room, hotel);
}

// Complete information of every room and person, not including payments.
void update_hotel(hotel_t *hotel) {
    list_iterate(hotel->rooms, (iter_func_t) room_update_iter, hotel);
}


void push_message(const char *message, hotel_t *hotel) {
    char *str = malloc(strlen(message) + 1);
    strcpy(str, message);
    list_insert_end(hotel->messages, str);
}

void pop_message(char *message, hotel_t *hotel) {
    if (list_empty(hotel->messages))
        return;

    char *str = list_get(hotel->messages, 1);
    strcpy(message, str);
    list_remove_begin(hotel->messages);
    free(str);
}


int room_find_iter(room_t *room, pair_t *args) {
    int number = *(int *) args->key;
    room_t **pRoom = args->value;

    if (room->number == number) {
        *pRoom = room;
        return 1;
    }
    return 0;
}

room_t *find_room_by_number(int number, hotel_t *hotel) {
    room_t *room = NULL;
    pair_t args = {&number, &room};
    list_iterate(hotel->rooms, (iter_func_t) room_find_iter, &args);
    return room;
}

person_t *find_person_by_id(const char *id, hotel_t *hotel) {
    person_t *temp = person_init("", id);
    person_t *person = set_check(hotel->people_id, temp);
    person_free_iter(temp);
    return person;
}