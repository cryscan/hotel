//
// Created by lepet on 7/18/2018.
//

#include <gtk/gtk.h>

#include "hotel.h"
#include "hotel-app.h"

static void hotel_app_init(HotelApp *app) {
}

static void hotel_app_activate(GApplication *app) {
    HotelAppWindow *window;
    window = hotel_app_window_new(HOTEL_APP(app));
    gtk_window_present(GTK_WINDOW(window));
}

static void hotel_app_class_init(HotelAppClass *class) {
    G_APPLICATION_CLASS(class)->activate = hotel_app_activate;
}

HotelApp *hotel_app_new() {
    return g_object_new(HOTEL_APP_TYPE,
                        "application-id", "net.lepet.hotel",
                        NULL);
}


static int rooms_load_model_iter(room_t *room, pair_t *args) {
    GtkListStore *store = args->key;
    GtkTreeIter *iter = args->value;

    char *room_types[4] = {"dorm", "single", "double", "family"};
    char str[16];
    sprintf(str, "%d/%d", room->vacant, room->capacity);

    gint number = room->number;
    gchararray type = room_types[room->type];
    gint available = 100 * room->vacant / room->capacity;
    gchararray vacant = str;

    gtk_list_store_append(store, iter);
    gtk_list_store_set(store, iter,
                       ROOMS_COL_NUMBER, number,
                       ROOMS_COL_TYPE, type,
                       ROOMS_COL_AVAILABLE, available,
                       ROOMS_COL_VACANT, vacant, -1);

    return 0;
}

static int people_load_model_iter(person_t *person, pair_t *args) {
    GtkListStore *store = args->key;
    GtkTreeIter *iter = args->value;

    gchararray name = person->name;
    gchararray id = person->id;
    gint number = person->room->number;

    gtk_list_store_append(store, iter);
    gtk_list_store_set(store, iter,
                       PEOPLE_COL_NAME, name,
                       PEOPLE_COL_ID, id,
                       PEOPLE_COL_NUMBER, number, -1);

    return 0;
}

static GtkWidget *create_model_view_rooms(HotelAppWindow *window) {
    GtkCellRenderer *renderer;
    GtkTreeModel *model;
    GtkWidget *view;
    GtkListStore *store;
    GtkTreeIter iter;

    view = gtk_tree_view_new();

    // Column #1
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(
            GTK_TREE_VIEW(view), -1, "Number", renderer, "text", ROOMS_COL_NUMBER, NULL);

    // Column #2
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 0.5, NULL);
    gtk_tree_view_insert_column_with_attributes(
            GTK_TREE_VIEW(view), -1, "Room Type", renderer, "text", ROOMS_COL_TYPE, NULL);

    // Column #3
    renderer = gtk_cell_renderer_progress_new();
    gtk_tree_view_insert_column_with_attributes(
            GTK_TREE_VIEW(view), -1, "Available", renderer, "value", ROOMS_COL_AVAILABLE,
            "text", ROOMS_COL_VACANT, NULL);

    store = gtk_list_store_new(ROOMS_COL_LAST, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING);
    pair_t args = {store, &iter};
    list_iterate(window->hotel->rooms, (iter_func_t) rooms_load_model_iter, &args);

    gtk_tree_view_set_model(GTK_TREE_VIEW(view), GTK_TREE_MODEL(store));
    return view;
}

static GtkWidget *create_model_view_people(HotelAppWindow *window) {
    GtkCellRenderer *renderer;
    GtkTreeModel *model;
    GtkWidget *view;
    GtkListStore *store;
    GtkTreeIter iter;

    view = gtk_tree_view_new();

    // Column #1
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(
            GTK_TREE_VIEW(view), -1, "Name", renderer, "text", PEOPLE_COL_NAME, NULL);

    // Column #2
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(
            GTK_TREE_VIEW(view), -1, "ID", renderer, "text", PEOPLE_COL_ID, NULL);

    // Column #3
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 1.0, NULL);
    gtk_tree_view_insert_column_with_attributes(
            GTK_TREE_VIEW(view), -1, "Room", renderer, "text", PEOPLE_COL_NUMBER, NULL);

    store = gtk_list_store_new(PEOPLE_COL_LAST, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT);
    pair_t args = {store, &iter};
    set_iterate(window->hotel->people_name, (iter_func_t) people_load_model_iter, &args);

    gtk_tree_view_set_model(GTK_TREE_VIEW(view), GTK_TREE_MODEL(store));
    return view;
}

static void hotel_app_window_init(HotelAppWindow *window) {
    GtkWidget *scrolled, *view;

    window->hotel = hotel_init("data/hotel.db");

    gtk_widget_init_template(GTK_WIDGET(window));

    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_hexpand(scrolled, TRUE);
    gtk_widget_set_vexpand(scrolled, TRUE);
    g_object_set(scrolled, "hscrollbar_policy", GTK_POLICY_NEVER, NULL);

    view = create_model_view_rooms(window);
    gtk_container_add(GTK_CONTAINER(scrolled), view);

    gtk_stack_add_titled(GTK_STACK(window->stack), scrolled, "Rooms", "Rooms");

    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_hexpand(scrolled, TRUE);
    gtk_widget_set_vexpand(scrolled, TRUE);
    g_object_set(scrolled, "hscrollbar_policy", GTK_POLICY_NEVER, NULL);

    view = create_model_view_people(window);
    gtk_container_add(GTK_CONTAINER(scrolled), view);

    gtk_stack_add_titled(GTK_STACK(window->stack), scrolled, "People", "People");

    gtk_widget_show_all(GTK_WIDGET(window));
}

static void hotel_app_window_class_init(HotelAppWindowClass *class) {
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
                                                "/net/lepet/hotel/ui/main.glade");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), HotelAppWindow, stack);
}

HotelAppWindow *hotel_app_window_new(HotelApp *app) {
    return g_object_new(HOTEL_APP_WINDOW_TYPE, "application", app, NULL);
}


int main(int argc, char *argv[]) {
    return g_application_run(G_APPLICATION(hotel_app_new()), argc, argv);
}