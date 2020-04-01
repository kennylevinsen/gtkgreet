#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <gtk/gtk.h>

#include "gtkgreet.h"
#include "config.h"

int config_update_command_selector(GtkWidget *combobox) {
    int entries = 0;
    if (gtkgreet->command != NULL) {
        gtk_combo_box_text_append((GtkComboBoxText*)combobox, NULL, gtkgreet->command);
    	entries++;
    }

    char buffer[255];
    FILE *fp = fopen("/etc/greetd/environments", "r");
    if (fp == NULL) {
        return entries;
    }
    while(fgets(buffer, 255, (FILE*) fp)) {
        size_t len = strnlen(buffer, 255);
        if (len > 0 && len < 255 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
        if (gtkgreet->command != NULL && strcmp(gtkgreet->command, buffer) == 0) {
        	continue;
        }
        gtk_combo_box_text_append((GtkComboBoxText*)combobox, NULL, buffer);
        entries++;
    }

    fclose(fp);
    return entries;
}