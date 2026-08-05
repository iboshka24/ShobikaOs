#include <gtk/gtk.h>
#include <gio/gio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *stack;
    GtkWidget *back_btn;
    GtkWidget *next_btn;
    
    // Config fields
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
    
    // Widgets for inputs
    GtkWidget *user_entry;
    GtkWidget *full_entry;
    GtkWidget *pass_entry;
    GtkWidget *host_entry;
    GtkWidget *lang_combo;
    GtkWidget *kb_combo;
    GtkWidget *disk_combo;
    GtkWidget *de_combo;
    GtkWidget *dm_combo;
    GtkWidget *wp_combo;
    
    // Installation screen widgets
    GtkWidget *install_start_btn;
    GtkWidget *progress_bar;
    GtkWidget *status_label;
    GtkWidget *log_text_view;
    GtkTextBuffer *log_buffer;
    GtkWidget *log_scrolled_win;
    
    int current_step;
    guint log_timer_id;
    long log_file_pos;
} InstallerApp;

static InstallerApp app_state;

const char *HIGH_TECH_CSS = 
"window {\n"
"    background-color: #090a0f;\n"
"    color: #e0e6ed;\n"
"    font-family: 'Noto Sans', 'Roboto', sans-serif;\n"
"}\n"
".header-box {\n"
"    background: linear-gradient(90deg, #0d0f18 0%, #151928 50%, #0d0f18 100%);\n"
"    border-bottom: 2px solid #00f3ff;\n"
"    padding: 16px 24px;\n"
"}\n"
".logo-title {\n"
"    font-size: 22px;\n"
"    font-weight: 800;\n"
"    color: #00f3ff;\n"
"    letter-spacing: 2px;\n"
"}\n"
".logo-sub {\n"
"    font-size: 13px;\n"
"    color: #9d4edd;\n"
"    font-weight: 600;\n"
"}\n"
".step-card {\n"
"    background: rgba(18, 22, 36, 0.85);\n"
"    border: 1px solid rgba(0, 243, 255, 0.25);\n"
"    border-radius: 12px;\n"
"    padding: 24px;\n"
"    margin: 20px;\n"
"    box-shadow: 0 8px 32px rgba(0, 0, 0, 0.5);\n"
"}\n"
".step-title {\n"
"    font-size: 20px;\n"
"    font-weight: 700;\n"
"    color: #ffffff;\n"
"    margin-bottom: 6px;\n"
"}\n"
".step-sub {\n"
"    font-size: 13px;\n"
"    color: #8a99ad;\n"
"    margin-bottom: 16px;\n"
"}\n"
"button {\n"
"    background: #141824;\n"
"    color: #00f3ff;\n"
"    border: 1px solid #00f3ff;\n"
"    border-radius: 8px;\n"
"    padding: 10px 20px;\n"
"    font-weight: 600;\n"
"    font-size: 14px;\n"
"}\n"
"button:hover {\n"
"    background: #00f3ff;\n"
"    color: #090a0f;\n"
"}\n"
"button.suggested-action {\n"
"    background: linear-gradient(135deg, #00f3ff 0%, #7000ff 100%);\n"
"    color: #ffffff;\n"
"    border: none;\n"
"    font-size: 15px;\n"
"    font-weight: 700;\n"
"}\n"
"button.suggested-action:hover {\n"
"    background: linear-gradient(135deg, #33f5ff 0%, #8a2be2 100%);\n"
"}\n"
"entry, passwordentry {\n"
"    background-color: #101420;\n"
"    color: #00f3ff;\n"
"    border: 1px solid rgba(0, 243, 255, 0.4);\n"
"    border-radius: 6px;\n"
"    padding: 10px;\n"
"    font-size: 14px;\n"
"}\n"
"combobox {\n"
"    background-color: #101420;\n"
"    color: #ffffff;\n"
"    border: 1px solid rgba(0, 243, 255, 0.4);\n"
"    border-radius: 6px;\n"
"}\n"
"textview.log-view {\n"
"    background-color: #050608;\n"
"    color: #00ff66;\n"
"    font-family: monospace;\n"
"    font-size: 12px;\n"
"    border-radius: 8px;\n"
"    border: 1px solid rgba(0, 255, 102, 0.4);\n"
"}\n"
"progressbar > progress {\n"
"    background: linear-gradient(90deg, #00f3ff 0%, #7000ff 100%);\n"
"    border-radius: 4px;\n"
"}\n";

static void apply_custom_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider, HIGH_TECH_CSS);
    GdkDisplay *display = gdk_display_get_default();
    if (display) {
        gtk_style_context_add_provider_for_display(display, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
}

static void save_installer_config(InstallerApp *app) {
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

static gboolean update_install_log(gpointer user_data) {
    InstallerApp *app = (InstallerApp*)user_data;
    FILE *f = fopen("/tmp/shobika-install.log", "r");
    if (!f) return TRUE;
    
    fseek(f, app->log_file_pos, SEEK_SET);
    char line[512];
    gboolean updated = FALSE;
    
    while (fgets(line, sizeof(line), f)) {
        updated = TRUE;
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(app->log_buffer, &end);
        gtk_text_buffer_insert(app->log_buffer, &end, line, -1);
        
        if (strstr(line, "Formatting")) gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.15);
        else if (strstr(line, "Pacstrap")) gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.35);
        else if (strstr(line, "User creation")) gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.65);
        else if (strstr(line, "Desktop Environment")) gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.85);
        else if (strstr(line, "COMPLETE")) {
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 1.0);
            gtk_label_set_text(GTK_LABEL(app->status_label), "🎉 Установка ShobikaOs завершена! Перезагрузите систему.");
        }
    }
    
    app->log_file_pos = ftell(f);
    fclose(f);
    
    if (updated && app->log_text_view) {
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(app->log_buffer, &end);
        GtkTextMark *mark = gtk_text_buffer_create_mark(app->log_buffer, NULL, &end, FALSE);
        gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(app->log_text_view), mark, 0.0, TRUE, 0.0, 1.0);
    }
    
    return TRUE;
}

static void on_start_installation_clicked(GtkButton *btn, gpointer user_data) {
    InstallerApp *app = (InstallerApp*)user_data;
    gtk_widget_set_sensitive(GTK_WIDGET(btn), FALSE);
    
    // Extract input fields
    const char *u = gtk_editable_get_text(GTK_EDITABLE(app->user_entry));
    if (u && strlen(u) > 0) strcpy(app->username, u);
    const char *f = gtk_editable_get_text(GTK_EDITABLE(app->full_entry));
    if (f && strlen(f) > 0) strcpy(app->fullname, f);
    const char *p = gtk_editable_get_text(GTK_EDITABLE(app->pass_entry));
    if (p && strlen(p) > 0) strcpy(app->password, p);
    const char *h = gtk_editable_get_text(GTK_EDITABLE(app->host_entry));
    if (h && strlen(h) > 0) strcpy(app->hostname, h);
    
    const char *lang = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->lang_combo));
    if (lang) strcpy(app->language, lang);
    const char *kb = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->kb_combo));
    if (kb) strcpy(app->keyboard, kb);
    const char *dk = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->disk_combo));
    if (dk) strcpy(app->disk, dk);
    const char *de = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->de_combo));
    if (de) strcpy(app->desktop, de);
    const char *wp = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->wp_combo));
    if (wp) strcpy(app->wallpaper, wp);
    
    save_installer_config(app);
    
    // Clear old log file
    FILE *logf = fopen("/tmp/shobika-install.log", "w");
    if (logf) { fputs("=== ShobikaOs Installation Started ===\n", logf); fclose(logf); }
    app->log_file_pos = 0;
    
    gtk_label_set_text(GTK_LABEL(app->status_label), "⚡ Установка выполняется... Нажмите для просмотра лога.");
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.05);
    
    // Spawn backend installer script with output redirection
    int res = system("/usr/bin/shobika-install-backend /tmp/shobika-installer.conf > /tmp/shobika-install.log 2>&1 &");
    (void)res;
    
    // Start live log monitoring timer
    app->log_timer_id = g_timeout_add(500, update_install_log, app);
}

static GtkWidget* create_card_container(const char *title_text, const char *sub_text, GtkWidget *content) {
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(card, "step-card");
    
    GtkWidget *title = gtk_label_new(title_text);
    gtk_widget_add_css_class(title, "step-title");
    gtk_widget_set_halign(title, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(card), title);
    
    if (sub_text) {
        GtkWidget *sub = gtk_label_new(sub_text);
        gtk_widget_add_css_class(sub, "step-sub");
        gtk_widget_set_halign(sub, GTK_ALIGN_START);
        gtk_box_append(GTK_BOX(card), sub);
    }
    
    if (content) {
        gtk_box_append(GTK_BOX(card), content);
    }
    return card;
}

static void on_next_step(GtkButton *btn, gpointer user_data) {
    (void)btn;
    InstallerApp *app = (InstallerApp*)user_data;
    if (app->current_step < 6) {
        app->current_step++;
        char step_id[16];
        snprintf(step_id, sizeof(step_id), "step-%d", app->current_step);
        gtk_stack_set_visible_child_name(GTK_STACK(app->stack), step_id);
    }
}

static void on_back_step(GtkButton *btn, gpointer user_data) {
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
    
    apply_custom_css();
    
    app->window = gtk_application_window_new(gtk_app);
    gtk_window_set_title(GTK_WINDOW(app->window), "ShobikaOs Cyberpunk Installer");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 980, 660);
    
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    // Header
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_add_css_class(header_box, "header-box");
    
    GtkWidget *logo_title = gtk_label_new("⚡ SHOBIKA OS");
    gtk_widget_add_css_class(logo_title, "logo-title");
    GtkWidget *logo_sub = gtk_label_new("HIGH-TECH GRAPHICAL INSTALLER v2.0");
    gtk_widget_add_css_class(logo_sub, "logo-sub");
    
    gtk_box_append(GTK_BOX(header_box), logo_title);
    gtk_box_append(GTK_BOX(header_box), logo_sub);
    gtk_box_append(GTK_BOX(main_box), header_box);
    
    app->stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(app->stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    
    // Step 0: Language
    app->lang_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->lang_combo), "ru_RU.UTF-8", "🇷🇺 Русский (Russian)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->lang_combo), "en_US.UTF-8", "🇺🇸 English (US)");
    gtk_combo_box_text_set_active_id(GTK_COMBO_BOX_TEXT(app->lang_combo), "ru_RU.UTF-8");
    gtk_stack_add_named(GTK_STACK(app->stack), create_card_container("1. Выбор Языка Система", "Выберите основной язык для интерфейса ShobikaOs", app->lang_combo), "step-0");
    
    // Step 1: Keyboard
    app->kb_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->kb_combo), "us", "US English");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->kb_combo), "ru", "Russian (us,ru)");
    gtk_combo_box_text_set_active_id(GTK_COMBO_BOX_TEXT(app->kb_combo), "us");
    gtk_stack_add_named(GTK_STACK(app->stack), create_card_container("2. Раскладка Клавиатуры", "Выберите раскладку для ввода", app->kb_combo), "step-1");
    
    // Step 2: Disk Selection
    app->disk_combo = gtk_combo_box_text_new();
    DIR *d = opendir("/sys/block");
    if (d) {
        struct dirent *dir;
        while ((dir = readdir(d)) != NULL) {
            if (strncmp(dir->d_name, "sd", 2) == 0 || strncmp(dir->d_name, "nvme", 4) == 0 || strncmp(dir->d_name, "vd", 2) == 0) {
                char dev_path[64];
                snprintf(dev_path, sizeof(dev_path), "/dev/%s", dir->d_name);
                gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->disk_combo), dev_path, dev_path);
            }
        }
        closedir(d);
    }
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->disk_combo), "/dev/sda", "/dev/sda (Virtual Disk)");
    gtk_combo_box_text_set_active_id(GTK_COMBO_BOX_TEXT(app->disk_combo), "/dev/sda");
    gtk_stack_add_named(GTK_STACK(app->stack), create_card_container("3. Выбор Целевого Диска", "⚠️ Внимание: Все существующие разделы на выбранном диске будут отформатированы!", app->disk_combo), "step-2");
    
    // Step 3: User Setup
    GtkWidget *user_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    app->user_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->user_entry), "Имя пользователя (username)");
    app->full_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->full_entry), "Полное имя (Full Name)");
    app->pass_entry = gtk_password_entry_new();
    app->host_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->host_entry), "Имя компьютера (hostname)");
    gtk_editable_set_text(GTK_EDITABLE(app->host_entry), "shobika-pc");
    
    gtk_box_append(GTK_BOX(user_box), app->user_entry);
    gtk_box_append(GTK_BOX(user_box), app->full_entry);
    gtk_box_append(GTK_BOX(user_box), app->pass_entry);
    gtk_box_append(GTK_BOX(user_box), app->host_entry);
    gtk_stack_add_named(GTK_STACK(app->stack), create_card_container("4. Учетная Запись и Безопасность", "Настройка имени пользователя, пароля и имени хоста", user_box), "step-3");
    
    // Step 4: Desktop Environment
    app->de_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->de_combo), "cinnamon", "🖥️ Cinnamon (Классический флагман ShobikaOs)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->de_combo), "gnome", "📱 GNOME 46 (Современный)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->de_combo), "kde", "⚡ KDE Plasma 6 (Гибкий)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->de_combo), "xfce", "🚀 XFCE 4.18 (Лёгкий)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->de_combo), "hyprland", "🔷 Hyprland (Wayland Tiling)");
    gtk_combo_box_text_set_active_id(GTK_COMBO_BOX_TEXT(app->de_combo), "cinnamon");
    gtk_stack_add_named(GTK_STACK(app->stack), create_card_container("5. Окружение Рабочего Стола", "Выберите графическую оболочку для вашей системы", app->de_combo), "step-4");
    
    // Step 5: Wallpaper
    app->wp_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->wp_combo), "shobikaos-aurora.png", "🌌 Aurora (Северное сияние)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->wp_combo), "shobikaos-geometric.png", "🔷 Geometric (Киберпространство)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->wp_combo), "shobikaos-minimal.png", "🟣 Minimal (Темный стильный)");
    gtk_combo_box_text_set_active_id(GTK_COMBO_BOX_TEXT(app->wp_combo), "shobikaos-aurora.png");
    gtk_stack_add_named(GTK_STACK(app->stack), create_card_container("6. Обои Рабочего Стола", "Выберите стартовые фирменные обои ShobikaOs", app->wp_combo), "step-5");
    
    // Step 6: Installation Progress & Live Terminal Output
    GtkWidget *install_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(install_card, "step-card");
    
    app->status_label = gtk_label_new("🚀 Всё готово к инсталляции ShobikaOs! Нажмите кнопк ниже.");
    gtk_widget_add_css_class(app->status_label, "step-title");
    gtk_box_append(GTK_BOX(install_card), app->status_label);
    
    app->progress_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.0);
    gtk_box_append(GTK_BOX(install_card), app->progress_bar);
    
    app->log_buffer = gtk_text_buffer_new(NULL);
    app->log_text_view = gtk_text_view_new_with_buffer(app->log_buffer);
    gtk_widget_add_css_class(app->log_text_view, "log-view");
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->log_text_view), FALSE);
    
    app->log_scrolled_win = gtk_scrolled_window_new();
    gtk_widget_set_size_request(app->log_scrolled_win, -1, 240);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(app->log_scrolled_win), app->log_text_view);
    gtk_box_append(GTK_BOX(install_card), app->log_scrolled_win);
    
    app->install_start_btn = gtk_button_new_with_label("⚡ НАЧАТЬ УСТАНОВКУ SHOBIKA OS");
    gtk_widget_add_css_class(app->install_start_btn, "suggested-action");
    gtk_widget_set_size_request(app->install_start_btn, -1, 48);
    g_signal_connect(app->install_start_btn, "clicked", G_CALLBACK(on_start_installation_clicked), app);
    gtk_box_append(GTK_BOX(install_card), app->install_start_btn);
    
    gtk_stack_add_named(GTK_STACK(app->stack), install_card, "step-6");
    
    gtk_widget_set_vexpand(app->stack, TRUE);
    gtk_box_append(GTK_BOX(main_box), app->stack);
    
    // Navigation bar
    GtkWidget *nav_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_set_margin_start(nav_box, 24);
    gtk_widget_set_margin_end(nav_box, 24);
    gtk_widget_set_margin_bottom(nav_box, 16);
    
    app->back_btn = gtk_button_new_with_label("◀ Назад");
    g_signal_connect(app->back_btn, "clicked", G_CALLBACK(on_back_step), app);
    
    app->next_btn = gtk_button_new_with_label("Далее ▶");
    gtk_widget_add_css_class(app->next_btn, "suggested-action");
    g_signal_connect(app->next_btn, "clicked", G_CALLBACK(on_next_step), app);
    
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
