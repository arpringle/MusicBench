#ifndef INTERFACE_H
#define INTERFACE_H

/*
The "includes":
*/

// Include GTK
#include <gtk/gtk.h>

/*
The function prototypes:
*/

int ConstructUI(int argc, char *argv[]);
static GtkWidget *ConstructWindow();
static GtkWidget *ConstructTitleBar();
static GtkWidget *ConstructAbout();
void WindowClosed(GtkWidget *widget, gpointer data);
void PaneResized(GObject *object, GParamSpec *pspec, gpointer user_data);
void AddTrack(GtkWidget *button, gpointer data);

#endif