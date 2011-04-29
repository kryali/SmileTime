#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "recorder_server.h"

#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 350
#define MAX_LINES 15

GtkWidget *window;
GtkWidget *container;
GtkWidget *button;
GtkTextView *messagebox;
GtkTextBuffer * messagebuffer;
GtkTextIter messageIter;
GtkEntry *chatbox;

void println(char* username, int usernamesize, char* str, int size);
void button_event(GtkButton *button, gpointer user_data);
void * chatwindow_init();
