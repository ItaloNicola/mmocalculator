#include <gtk/gtk.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "unicode.h"
#include "networking.h"

// TODO: use fixed-size sizes

#define global static

typedef struct update_output_data_ {
    GtkEntry *output;
    char *expression;
} update_output_data;

global GtkWidget* window;
global GtkWidget* layout;

// this function is called periodically
// it updates the current output with the
// latest expression we received from the server
int updateOutput(void *data_) {
    update_output_data *data = (update_output_data *) data_;
    gtk_entry_set_text(data->output, data->expression);
    return 1;
}

// 'clicked' button event handler
static void button_clicked(GtkWidget *button, gpointer data) {
    gchar operator[8];

    // get operator
    strcpy(operator, gtk_button_get_label((GtkButton*) button));

    // send message to server
    int n = writeToServer(operator, strlen(operator));
    if (n < 0) {
        fprintf(stderr, "ERROR: Failed to communicate with server.\n");
    }
}

// creates a button with given label and color
static void create_button_at(char *label, char *color, int x, int y, int width, int height) {
    GtkWidget *button;

    // create button and attach it to grid
    button = gtk_button_new_with_label(label);
    gtk_widget_set_hexpand(button, TRUE);
    gtk_widget_set_vexpand(button, TRUE);
    gtk_grid_attach((GtkGrid*) layout, button, x, y, width, height);

    // set button style
    {
        char style[256];
        GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(button));
        GtkCssProvider *provider = gtk_css_provider_new();
        gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
        sprintf(style, "button {color: white; background: %s;}", color);
        gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(provider), style, -1, NULL);

        // clean up
        g_object_unref(provider);
    }

    // add handlers
    g_signal_connect(button, "clicked", G_CALLBACK(button_clicked), NULL);
}

// 'activate' event handler
static void activate(GtkApplication* app, gpointer user_data) {

    // create window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Calculator");

    // and set its layout as grid
    layout = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), layout);

    // create output box
    {
        GtkWidget* output_box;
        output_box = gtk_entry_new();
        gtk_widget_set_hexpand(output_box, TRUE);
        gtk_entry_set_alignment((GtkEntry*) output_box, 1);
        gtk_grid_attach((GtkGrid*) layout, output_box, 0, 0, 5, 1);
    }

    // create result label
    {
        GtkWidget* result_label;
        result_label = gtk_label_new(NULL);
        gtk_widget_set_hexpand(result_label, TRUE);
        gtk_grid_attach((GtkGrid*) layout, result_label, 0, 1, 5, 1);
    }

    // create buttons
    {
        int i, j;

        // buttons data
        char *buttons[5][5] = {
            {"1", "2", "3", "←", "C"},
            {"4", "5", "6", "*", "÷"},
            {"7", "8", "9", "+", "-"},
            {"(", "0", ")", "^", "√"},
            {"%", " ", ",", " ", "="}
        };

        // buttons layout
        for (i = 2; i < 6; i++) {
            for (j = 0; j < 5; j++) {
                if (!strcmp("C", buttons[i-2][j])) create_button_at("C", "#990000", j, i, 1, 1);
                else create_button_at(buttons[i-2][j], "black", j, i, 1, 1);
            }
        }
        for (i = 0; i < 3; i++) {
            create_button_at(buttons[4][i], "black", i, 6, 1, 1);
        }
        create_button_at("=", "#990000", 3, 6, 2, 1);
    }

    // show window and all each of its widgets
    gtk_widget_show_all(window);

    // read current expression from server (PERIODICALLY)
    // this expression must be updated regularly
    // but when it happens this function will have already returned
    // this is why 'current_expression' must be on the heap
    char *current_expression = calloc(256, sizeof(char));
    current_expression[0] = '\0';
    g_timeout_add(20, pullUpdates255, current_expression);

    // update output to match current expression (PERIODICALLY)
    // the output must be updated regularly
    // but when it happens this function will have already returned
    // this is why 'data' must be on the heap
    GtkEntry *output = (GtkEntry*) gtk_grid_get_child_at((GtkGrid *) layout, 0, 0);
    update_output_data *data = malloc(sizeof(data));
    data->output = output;
    data->expression = current_expression;
    g_timeout_add(20, updateOutput, data);
}

int main (int argc, char **argv) {
    GtkApplication *app;
    int status;

    // create new application
    app = gtk_application_new("calc.main.app", G_APPLICATION_FLAGS_NONE);

    // set handler for activate event
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    // try to connect to server
    int er = connectToServer();
    if (er) {
        fprintf(stderr, "ERROR: Could not connect to server.\n");
        exit(1);
    }

    // run application
    status = g_application_run(G_APPLICATION(app), argc, argv);

    // return status code
    return status;
}
