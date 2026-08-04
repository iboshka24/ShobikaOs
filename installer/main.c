#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

static void on_install_click(GtkButton *btn, gpointer user_data) {
    (void)btn;
    (void)user_data;
    int res = system("sudo /usr/bin/shobika-install-backend /tmp/shobika-installer.conf &");
    (void)res;
}

static void activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "ShobikaOs Installer");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 500);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

    GtkWidget *label = gtk_label_new("⚡ Welcome to ShobikaOs Installer");
    gtk_box_append(GTK_BOX(box), label);

    GtkWidget *btn = gtk_button_new_with_label("🚀 Start Installation");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_install_click), NULL);
    gtk_box_append(GTK_BOX(box), btn);

    gtk_window_set_child(GTK_WINDOW(window), box);
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("org.shobikaos.installer", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
