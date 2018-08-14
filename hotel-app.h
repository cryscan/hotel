//
// Created by lepet on 7/18/2018.
//

#ifndef HOTEL_HOTEL_APP_H
#define HOTEL_HOTEL_APP_H

gpointer __atomic_load_8(volatile gsize *, int);

// HotelApp Object
#define HOTEL_APP_TYPE (hotel_app_get_type())

G_DECLARE_FINAL_TYPE(HotelApp, hotel_app, HOTEL, APP, GtkApplication)

struct _HotelApp {
    GtkApplication parent;
};

G_DEFINE_TYPE(HotelApp, hotel_app, GTK_TYPE_APPLICATION)

HotelApp *hotel_app_new();


// HotelAppWindow Object
#define HOTEL_APP_WINDOW_TYPE (hotel_app_window_get_type())

G_DECLARE_FINAL_TYPE(HotelAppWindow, hotel_app_window, HOTEL, APP_WINDOW, GtkApplicationWindow)

struct _HotelAppWindow {
    GtkApplicationWindow parent;
    GtkWidget *stack;

    hotel_t *hotel;
};

G_DEFINE_TYPE(HotelAppWindow, hotel_app_window, GTK_TYPE_APPLICATION_WINDOW)

HotelAppWindow *hotel_app_window_new(HotelApp *app);


// For Tree view
enum {
    ROOMS_COL_NUMBER = 0, ROOMS_COL_TYPE, ROOMS_COL_AVAILABLE, ROOMS_COL_VACANT, ROOMS_COL_LAST
};

enum {
    PEOPLE_COL_NAME = 0, PEOPLE_COL_ID, PEOPLE_COL_NUMBER, PEOPLE_COL_LAST
};

#endif //HOTEL_HOTEL_APP_H
