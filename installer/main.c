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
    
    // Inputs & Selection
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
    
    gboolean install_gaming;
    gboolean install_multimedia;
    gboolean install_browser;
    gboolean install_flatpak;
    gboolean install_bluetooth;
    
    GtkWidget *user_entry;
    GtkWidget *full_entry;
    GtkWidget *pass_entry;
    GtkWidget *host_entry;
    GtkWidget *dm_combo;
    GtkWidget *progress_bar;
    GtkWidget *status_label;
    GtkWidget *spinner;
    
    int current_step;
} InstallerApp;

static InstallerApp app_state;

static void save_config_and_run_install(InstallerApp *app) {
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
    fprintf(f, "INSTALL_GAMING=%d\n", app->install_gaming);
    fprintf(f, "INSTALL_MULTIMEDIA=%d\n", app->install_multimedia);
    fprintf(f, "INSTALL_BROWSER=%d\n", app->install_browser);
    fprintf(f, "INSTALL_FLATPAK=%d\n", app->install_flatpak);
    fprintf(f, "INSTALL_BLUETOOTH=%d\n", app->install_bluetooth);
    
    fclose(f);
}

static GtkWidget* create_header(void) {
    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_add_css_class(header, "header");
    
    GtkWidget *logo = gtk_label_new("⚡ ShobikaOs");
    gtk_widget_add_css_class(logo, "logo");
    
    GtkWidget *title = gtk_label_new("Graphical Installer");
    gtk_widget_add_css_class(title, "title");
    
    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer, TRUE);
    
    gtk_box_append(GTK_BOX(header), logo);
    gtk_box_append(GTK_BOX(header), title);
    gtk_box_append(GTK_BOX(header), spacer);
    
    return header;
}

// 1. Step: Language
static GtkWidget* create_language_step(InstallerApp *app) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    
    GtkWidget *title = gtk_label_new("Выберите язык системы / Select Language");
    gtk_widget_add_css_class(title, "step-title");
    gtk_box_append(GTK_BOX(box), title);
    
    GtkWidget *combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), "ru_RU.UTF-8", "🇷🇺 Русский (Russian)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), "en_US.UTF-8", "🇺🇸 English (US)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), "de_DE.UTF-8", "🇩🇪 Deutsch (German)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), "fr_FR.UTF-8", "🇫🇷 Français (French)");
    gtk_combo_box_text_set_active_id(GTK_COMBO_BOX_TEXT(combo), "ru_RU.UTF-8");
    strcpy(app->language, "ru_RU.UTF-8");
    
    gtk_box_append(GTK_BOX(box), combo);
    return box;
}

// 2. Step: Keyboard
static GtkWidget* create_keyboard_step(InstallerApp *app) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    
    GtkWidget *title = gtk_label_new("Раскладка клавиатуры / Keyboard Layout");
    gtk_widget_add_css_class(title, "step-title");
    gtk_box_append(GTK_BOX(box), title);
    
    GtkWidget *combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), "us", "US English");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), "ru", "Russian (us,ru)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), "de", "German");
    gtk_combo_box_text_set_active_id(GTK_COMBO_BOX_TEXT(combo), "us");
    strcpy(app->keyboard, "us");
    
    gtk_box_append(GTK_BOX(box), combo);
    return box;
}

// 3. Step: Disk
static GtkWidget* create_disk_step(InstallerApp *app) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    
    GtkWidget *title = gtk_label_new("Выберите диск для установки");
    gtk_widget_add_css_class(title, "step-title");
    gtk_box_append(GTK_BOX(box), title);
    
    GtkWidget *warning = gtk_label_new("⚠️ Внимание: Все данные на выбранном диске будут удалены!");
    gtk_widget_add_css_class(warning, "warning");
    gtk_box_append(GTK_BOX(box), warning);
    
    GtkWidget *combo = gtk_combo_box_text_new();
    
    DIR *d = opendir("/sys/block");
    int count = 0;
    if (d) {
        struct dirent *dir;
        while ((dir = readdir(d)) != NULL) {
            if (strncmp(dir->d_name, "sd", 2) == 0 || strncmp(dir->d_name, "nvme", 4) == 0 || strncmp(dir->d_name, "vd", 2) == 0) {
                char dev_path[64], label[128];
                snprintf(dev_path, sizeof(dev_path), "/dev/%s", dir->d_name);
                snprintf(label, sizeof(label), "💾 %s (Диск)", dev_path);
                gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), dev_path, label);
                if (count == 0) {
                    gtk_combo_box_text_set_active_id(GTK_COMBO_BOX_TEXT(combo), dev_path);
                    strcpy(app->disk, dev_path);
                }
                count++;
            }
        }
        closedir(d);
    }
    
    if (count == 0) {
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), "/dev/sda", "💾 /dev/sda (Virtual Disk)");
        gtk_combo_box_text_set_active_id(GTK_COMBO_BOX_TEXT(combo), "/dev/sda");
        strcpy(app->disk, "/dev/sda");
    }
    
    gtk_box_append(GTK_BOX(box), combo);
    return box;
}

// 4. Step: User
static GtkWidget* create_user_step(InstallerApp *app) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_size_request(box, 360, -1);
    
    GtkWidget *title = gtk_label_new("Настройка пользователя");
    gtk_widget_add_css_class(title, "step-title");
    gtk_box_append(GTK_BOX(box), title);
    
    app->user_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->user_entry), "Имя пользователя (например: user)");
    gtk_box_append(GTK_BOX(box), app->user_entry);
    
    app->full_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->full_entry), "Полное имя (например: Shobika User)");
    gtk_box_append(GTK_BOX(box), app->full_entry);
    
    app->pass_entry = gtk_password_entry_new();
    gtk_password_entry_set_placeholder_text(GTK_PASSWORD_ENTRY(app->pass_entry), "Пароль");
    gtk_box_append(GTK_BOX(box), app->pass_entry);
    
    app->host_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->host_entry), "Имя компьютера (например: shobika-pc)");
    gtk_editable_set_text(GTK_EDITABLE(app->host_entry), "shobika-pc");
    gtk_box_append(GTK_BOX(box), app->host_entry);
    
    return box;
}

// 5. Step: Desktop & DM
static GtkWidget* create_desktop_step(InstallerApp *app) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    
    GtkWidget *title = gtk_label_new("Окружение рабочего стола и Дисплейный менеджер");
    gtk_widget_add_css_class(title, "step-title");
    gtk_box_append(GTK_BOX(box), title);
    
    GtkWidget *de_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(de_combo), "cinnamon", "🖥️ Cinnamon (Классический)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(de_combo), "gnome", "📱 GNOME (Современный)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(de_combo), "kde", "⚡ KDE Plasma (Гибкий)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(de_combo), "xfce", "🚀 XFCE (Лёгкий)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(de_combo), "hyprland", "🔷 Hyprland (Wayland Tiling)");
    gtk_combo_box_text_set_active_id(GTK_COMBO_BOX_TEXT(de_combo), "cinnamon");
    strcpy(app->desktop, "cinnamon");
    gtk_box_append(GTK_BOX(box), de_combo);
    
    GtkWidget *dm_label = gtk_label_new("🔑 Дисплейный менеджер:");
    gtk_box_append(GTK_BOX(box), dm_label);
    
    app->dm_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->dm_combo), "auto", "Автоматически (Рекомендуется)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->dm_combo), "sddm", "SDDM");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->dm_combo), "gdm", "GDM");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->dm_combo), "lightdm", "LightDM");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->dm_combo), "ly", "Ly (TUI)");
    gtk_combo_box_text_set_active_id(GTK_COMBO_BOX_TEXT(app->dm_combo), "auto");
    strcpy(app->display_manager, "auto");
    gtk_box_append(GTK_BOX(box), app->dm_combo);
    
    app->install_gaming = TRUE;
    app->install_multimedia = TRUE;
    app->install_browser = TRUE;
    app->install_flatpak = TRUE;
    app->install_bluetooth = TRUE;
    
    return box;
}

// 6. Step: Wallpaper
static GtkWidget* create_wallpaper_step(InstallerApp *app) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    
    GtkWidget *title = gtk_label_new("Выбор обоев рабочего стола");
    gtk_widget_add_css_class(title, "step-title");
    gtk_box_append(GTK_BOX(box), title);
    
    GtkWidget *combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), "shobikaos-aurora.png", "🌌 Aurora (Северное сияние - По умолчанию)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), "shobikaos-geometric.png", "🔷 Geometric (Геометрия)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), "shobikaos-minimal.png", "🟣 Minimal (Минимализм)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), "shobikaos-cosmic.png", "💎 Cosmic (Космос)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), "shobikaos-mountain.png", "🏔️ Mountain (Горы)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), "shobikaos-circuit.png", "⚡ Circuit (Киберпанк)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), "shobikaos-wave.png", "🌊 Wave (Волна)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), "shobikaos-nebula.png", "🪐 Nebula (Туманность)");
    gtk_combo_box_text_set_active_id(GTK_COMBO_BOX_TEXT(combo), "shobikaos-aurora.png");
    strcpy(app->wallpaper, "shobikaos-aurora.png");
    
    gtk_box_append(GTK_BOX(box), combo);
    return box;
}

// 7. Step: Install Execution
static void on_install_start_clicked(GtkButton *btn, gpointer user_data) {
    InstallerApp *app = (InstallerApp*)user_data;
    gtk_widget_set_sensitive(GTK_WIDGET(btn), FALSE);
    gtk_spinner_start(GTK_SPINNER(app->spinner));
    
    // Save user entry values
    const char *u = gtk_editable_get_text(GTK_EDITABLE(app->user_entry));
    if (u && strlen(u) > 0) strcpy(app->username, u);
    
    const char *f = gtk_editable_get_text(GTK_EDITABLE(app->full_entry));
    if (f && strlen(f) > 0) strcpy(app->fullname, f);
    
    const char *p = gtk_editable_get_text(GTK_EDITABLE(app->pass_entry));
    if (p && strlen(p) > 0) strcpy(app->password, p);
    
    const char *h = gtk_editable_get_text(GTK_EDITABLE(app->host_entry));
    if (h && strlen(h) > 0) strcpy(app->hostname, h);
    
    save_config_and_run_install(app);
    
    gtk_label_set_text(GTK_LABEL(app->status_label), "Установка ShobikaOs запущена! Выполняется pacstrap...");
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.2);
    
    // Spawn background shell script
    system("sudo /usr/bin/shobika-install-backend /tmp/shobika-installer.conf &");
}

static GtkWidget* create_install_step(InstallerApp *app) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    
    GtkWidget *title = gtk_label_new("Готов к установке ShobikaOs");
    gtk_widget_add_css_class(title, "step-title");
    gtk_box_append(GTK_BOX(box), title);
    
    app->status_label = gtk_label_new("Нажмите 'Установить' для начала форматирования и инсталляции.");
    gtk_box_append(GTK_BOX(box), app->status_label);
    
    app->progress_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.0);
    gtk_widget_set_size_request(app->progress_bar, 400, -1);
    gtk_box_append(GTK_BOX(box), app->progress_bar);
    
    app->spinner = gtk_spinner_new();
    gtk_box_append(GTK_BOX(box), app->spinner);
    
    GtkWidget *btn = gtk_button_new_with_label("🚀 Начать установку");
    gtk_widget_add_css_class(btn, "suggested-action");
    gtk_widget_set_size_request(btn, 200, 48);
    g_signal_connect(btn, "clicked", G_CALLBACK(on_install_start_clicked), app);
    gtk_box_append(GTK_BOX(box), btn);
    
    return box;
}

static void on_next_clicked(GtkButton *btn, gpointer user_data) {
    InstallerApp *app = (InstallerApp*)user_data;
    if (app->current_step < 6) {
        app->current_step++;
        char step_id[16];
        snprintf(step_id, sizeof(step_id), "step-%d", app->current_step);
        gtk_stack_set_visible_child_name(GTK_STACK(app->stack), step_id);
    }
}

static void on_back_clicked(GtkButton *btn, gpointer user_data) {
    InstallerApp *app = (InstallerApp*)user_data;
    if (app->current_step > 0) {
        app->current_step--;
        char step_id[16];
        snprintf(step_id, sizeof(step_id), "step-%d", app->current_step);
        gtk_stack_set_visible_child_name(GTK_STACK(app->stack), step_id);
    }
}

static void activate(GtkApplication *gtk_app, gpointer user_data) {
    InstallerApp *app = &app_state;
    memset(app, 0, sizeof(InstallerApp));
    
    app->window = gtk_application_window_new(gtk_app);
    gtk_window_set_title(GTK_WINDOW(app->window), "ShobikaOs Installer");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 960, 640);
    
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    GtkWidget *header = create_header();
    gtk_box_append(GTK_BOX(main_box), header);
    
    app->stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(app->stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    
    gtk_stack_add_named(GTK_STACK(app->stack), create_language_step(app), "step-0");
    gtk_stack_add_named(GTK_STACK(app->stack), create_keyboard_step(app), "step-1");
    gtk_stack_add_named(GTK_STACK(app->stack), create_disk_step(app), "step-2");
    gtk_stack_add_named(GTK_STACK(app->stack), create_user_step(app), "step-3");
    gtk_stack_add_named(GTK_STACK(app->stack), create_desktop_step(app), "step-4");
    gtk_stack_add_named(GTK_STACK(app->stack), create_wallpaper_step(app), "step-5");
    gtk_stack_add_named(GTK_STACK(app->stack), create_install_step(app), "step-6");
    
    gtk_widget_set_vexpand(app->stack, TRUE);
    gtk_box_append(GTK_BOX(main_box), app->stack);
    
    // Navigation bar
    GtkWidget *nav_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_set_margin_top(nav_box, 16);
    gtk_widget_set_margin_bottom(nav_box, 16);
    gtk_widget_set_margin_start(nav_box, 24);
    gtk_widget_set_margin_end(nav_box, 24);
    
    app->back_btn = gtk_button_new_with_label("Назад");
    g_signal_connect(app->back_btn, "clicked", G_CALLBACK(on_back_clicked), app);
    
    app->next_btn = gtk_button_new_with_label("Далее");
    gtk_widget_add_css_class(app->next_btn, "suggested-action");
    g_signal_connect(app->next_btn, "clicked", G_CALLBACK(on_next_clicked), app);
    
    gtk_box_append(GTK_BOX(nav_box), app->back_btn);
    gtk_box_append(GTK_BOX(nav_box), app->next_btn);
    
    gtk_box_append(GTK_BOX(main_box), nav_box);
    
    gtk_window_set_child(GTK_WINDOW(app->window), main_box);
    gtk_window_present(GTK_WINDOW(app->window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("org.shobikaos.installer", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
