#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *stack;
    GtkWidget *back_btn;
    GtkWidget *next_btn;
    
    char language[32];
    char keyboard[32];
    char disk[256];
    char username[64];
    char fullname[128];
    char password[64];
    char hostname[64];
    char desktop[32];
    char display_manager[32];
    char wallpaper[128];
    
    int current_step;
} InstallerApp;

static InstallerApp app_state;

static void save_config(InstallerApp *app) {
    FILE *f = fopen("/tmp/shobika-installer.conf", "w");
    if (!f) return;
    fprintf(f, "LANGUAGE=\"%s\"\n", app->language[0] ? app->language : "ru_RU.UTF-8");
    fprintf(f, "KEYBOARD=\"%s\"\n", app->keyboard[0] ? app->keyboard : "us");
    fprintf(f, "DISK=\"%s\"\n", app->disk[0] ? app->disk : "/dev/sda");
    fprintf(f, "USERNAME=\"%s\"\n", app->username[0] ? app->username : "user");
    fprintf(f, "FULLNAME=\"%s\"\n", app->fullname[0] ? app->fullname : "Shobika User");
    fprintf(f, "PASSWORD=\"%s\"\n", app->password[0] ? app->password : "shobikaos");
    fprintf(f, "HOSTNAME=\"%s\"\n", app->hostname[0] ? app->hostname : "shobika-pc");
    fprintf(f, "DESKTOP=\"%s\"\n", app->desktop[0] ? app->desktop : "cinnamon");
    fprintf(f, "DISPLAY_MANAGER=\"%s\"\n", app->display_manager[0] ? app->display_manager : "auto");
    fprintf(f, "WALLPAPER=\"%s\"\n", app->wallpaper[0] ? app->wallpaper : "shobikaos-aurora.png");
    fclose(f);
}

static void on_install_start_clicked(GtkButton *btn, gpointer user_data) {
    InstallerApp *app = (InstallerApp*)user_data;
    gtk_widget_set_sensitive(GTK_WIDGET(btn), FALSE);
    save_config(app);
    int res = system("sudo /usr/bin/shobika-install-backend /tmp/shobika-installer.conf &");
    (void)res;
}

static GtkWidget* create_step(const char *title_text, const char *sub_text, GtkWidget *content) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    
    GtkWidget *title = gtk_label_new(title_text);
    gtk_widget_add_css_class(title, "title");
    gtk_box_append(GTK_BOX(box), title);
    
    if (sub_text) {
        GtkWidget *sub = gtk_label_new(sub_text);
        gtk_box_append(GTK_BOX(box), sub);
    }
    
    if (content) {
        gtk_box_append(GTK_BOX(box), content);
    }
    return box;
}

static void on_next_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    InstallerApp *app = (InstallerApp*)user_data;
    if (app->current_step < 6) {
        app->current_step++;
        char step_id[16];
        snprintf(step_id, sizeof(step_id), "step-%d", app->current_step);
        gtk_stack_set_visible_child_name(GTK_STACK(app->stack), step_id);
    }
}

static void on_back_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    InstallerApp *app = (InstallerApp*)user_data;
    if (app->current_step > 0) {
        app->current_step--;
        char step_id[16];
        snprintf(step_id, sizeof(step_id), "step-%d", app->current_step);
        gtk_stack_set_visible_child_name(GTK_STACK(app->stack), step_id);
    }
}

static void activate(GtkApplication *gtk_app, gpointer user_data) {
    (void)user_data;
    InstallerApp *app = &app_state;
    memset(app, 0, sizeof(InstallerApp));
    strcpy(app->language, "ru_RU.UTF-8");
    strcpy(app->keyboard, "us");
    strcpy(app->disk, "/dev/sda");
    strcpy(app->username, "user");
    strcpy(app->password, "shobikaos");
    strcpy(app->desktop, "cinnamon");
    strcpy(app->display_manager, "auto");
    strcpy(app->wallpaper, "shobikaos-aurora.png");
    
    app->window = gtk_application_window_new(gtk_app);
    gtk_window_set_title(GTK_WINDOW(app->window), "ShobikaOs Installer");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 900, 600);
    
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    GtkWidget *header = gtk_label_new("⚡ ShobikaOs Graphical Installer");
    gtk_box_append(GTK_BOX(main_box), header);
    
    app->stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(app->stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    
    // Step 0: Language
    GtkWidget *lang_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(lang_combo), "ru_RU.UTF-8", "🇷🇺 Русский");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(lang_combo), "en_US.UTF-8", "🇺🇸 English");
    gtk_combo_box_text_set_active_id(GTK_COMBO_BOX_TEXT(lang_combo), "ru_RU.UTF-8");
    gtk_stack_add_named(GTK_STACK(app->stack), create_step("Выберите язык", NULL, lang_combo), "step-0");
    
    // Step 1: Keyboard
    GtkWidget *kb_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(kb_combo), "us", "US English");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(kb_combo), "ru", "Russian (us,ru)");
    gtk_combo_box_text_set_active_id(GTK_COMBO_BOX_TEXT(kb_combo), "us");
    gtk_stack_add_named(GTK_STACK(app->stack), create_step("Раскладка клавиатуры", NULL, kb_combo), "step-1");
    
    // Step 2: Disk
    GtkWidget *disk_combo = gtk_combo_box_text_new();
    DIR *d = opendir("/sys/block");
    if (d) {
        struct dirent *dir;
        while ((dir = readdir(d)) != NULL) {
            if (strncmp(dir->d_name, "sd", 2) == 0 || strncmp(dir->d_name, "nvme", 4) == 0 || strncmp(dir->d_name, "vd", 2) == 0) {
                char dev_path[64];
                snprintf(dev_path, sizeof(dev_path), "/dev/%s", dir->d_name);
                gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(disk_combo), dev_path, dev_path);
            }
        }
        closedir(d);
    }
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(disk_combo), "/dev/sda", "/dev/sda (Virtual Disk)");
    gtk_combo_box_text_set_active_id(GTK_COMBO_BOX_TEXT(disk_combo), "/dev/sda");
    gtk_stack_add_named(GTK_STACK(app->stack), create_step("Выберите диск", "⚠️ Все данные на диске будут удалены!", disk_combo), "step-2");
    
    // Step 3: User
    GtkWidget *user_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *u_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(u_entry), "Имя пользователя");
    GtkWidget *p_entry = gtk_password_entry_new();
    gtk_box_append(GTK_BOX(user_box), u_entry);
    gtk_box_append(GTK_BOX(user_box), p_entry);
    gtk_stack_add_named(GTK_STACK(app->stack), create_step("Пользователь и пароль", NULL, user_box), "step-3");
    
    // Step 4: Desktop
    GtkWidget *de_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(de_combo), "cinnamon", "🖥️ Cinnamon");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(de_combo), "gnome", "📱 GNOME");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(de_combo), "kde", "⚡ KDE Plasma");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(de_combo), "xfce", "🚀 XFCE");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(de_combo), "hyprland", "🔷 Hyprland");
    gtk_combo_box_text_set_active_id(GTK_COMBO_BOX_TEXT(de_combo), "cinnamon");
    gtk_stack_add_named(GTK_STACK(app->stack), create_step("Рабочий стол", NULL, de_combo), "step-4");
    
    // Step 5: Wallpaper
    GtkWidget *wp_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(wp_combo), "shobikaos-aurora.png", "🌌 Aurora (По умолчанию)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(wp_combo), "shobikaos-geometric.png", "🔷 Geometric");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(wp_combo), "shobikaos-minimal.png", "🟣 Minimal");
    gtk_combo_box_text_set_active_id(GTK_COMBO_BOX_TEXT(wp_combo), "shobikaos-aurora.png");
    gtk_stack_add_named(GTK_STACK(app->stack), create_step("Выбор обоев", NULL, wp_combo), "step-5");
    
    // Step 6: Install
    GtkWidget *install_btn = gtk_button_new_with_label("🚀 Начать установку");
    g_signal_connect(install_btn, "clicked", G_CALLBACK(on_install_start_clicked), app);
    gtk_stack_add_named(GTK_STACK(app->stack), create_step("Установка системы", "Нажмите для запуска pacstrap и настройки", install_btn), "step-6");
    
    gtk_widget_set_vexpand(app->stack, TRUE);
    gtk_box_append(GTK_BOX(main_box), app->stack);
    
    // Navigation bar
    GtkWidget *nav_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    app->back_btn = gtk_button_new_with_label("Назад");
    g_signal_connect(app->back_btn, "clicked", G_CALLBACK(on_back_clicked), app);
    app->next_btn = gtk_button_new_with_label("Далее");
    g_signal_connect(app->next_btn, "clicked", G_CALLBACK(on_next_clicked), app);
    
    gtk_box_append(GTK_BOX(nav_box), app->back_btn);
    gtk_box_append(GTK_BOX(nav_box), app->next_btn);
    gtk_box_append(GTK_BOX(main_box), nav_box);
    
    gtk_window_set_child(GTK_WINDOW(app->window), main_box);
    gtk_window_present(GTK_WINDOW(app->window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("org.shobikaos.installer", 0);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
