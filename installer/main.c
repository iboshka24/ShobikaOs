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
    char wallpaper[128];
    
    // Widgets for inputs
    GtkWidget *user_entry;
    GtkWidget *full_entry;
    GtkWidget *pass_entry;
    GtkWidget *host_entry;
    GtkWidget *disk_entry;
    GtkWidget *de_dropdown;
    GtkWidget *lang_dropdown;
    GtkWidget *kb_dropdown;
    
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
"entry {\n"
"    background-color: #101420;\n"
"    color: #00f3ff;\n"
"    border: 1px solid rgba(0, 243, 255, 0.4);\n"
"    border-radius: 6px;\n"
"    padding: 10px;\n"
"    font-size: 14px;\n"
"}\n"
"dropdown {\n"
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
    gtk_css_provider_load_from_data(provider, HIGH_TECH_CSS, -1);
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
    fprintf(f, "DISPLAY_MANAGER=\"%s\"\n", "auto");
    fprintf(f, "WALLPAPER=\"%s\"\n", "shobikaos-aurora.png");
    
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
    
    const char *u = gtk_editable_get_text(GTK_EDITABLE(app->user_entry));
    if (u && strlen(u) > 0) strncpy(app->username, u, sizeof(app->username) - 1);
    const char *f = gtk_editable_get_text(GTK_EDITABLE(app->full_entry));
    if (f && strlen(f) > 0) strncpy(app->fullname, f, sizeof(app->fullname) - 1);
    const char *p = gtk_editable_get_text(GTK_EDITABLE(app->pass_entry));
    if (p && strlen(p) > 0) strncpy(app->password, p, sizeof(app->password) - 1);
    const char *h = gtk_editable_get_text(GTK_EDITABLE(app->host_entry));
    if (h && strlen(h) > 0) strncpy(app->hostname, h, sizeof(app->hostname) - 1);
    const char *d = gtk_editable_get_text(GTK_EDITABLE(app->disk_entry));
    if (d && strlen(d) > 0) strncpy(app->disk, d, sizeof(app->disk) - 1);
    
    save_installer_config(app);
    
    FILE *logf = fopen("/tmp/shobika-install.log", "w");
    if (logf) { fputs("=== ShobikaOs Installation Started ===\n", logf); fclose(logf); }
    app->log_file_pos = 0;
    
    gtk_label_set_text(GTK_LABEL(app->status_label), "⚡ Установка выполняется... Идёт форматирование и pacstrap.");
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.05);
    
    int res = system("/usr/bin/shobika-install-backend /tmp/shobika-installer.conf > /tmp/shobika-install.log 2>&1 &");
    (void)res;
    
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
    if (app->current_step < 5) {
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
    
    // Set default values
    strncpy(app->language, "ru_RU.UTF-8", sizeof(app->language) - 1);
    strncpy(app->keyboard, "us", sizeof(app->keyboard) - 1);
    strncpy(app->disk, "/dev/sda", sizeof(app->disk) - 1);
    strncpy(app->username, "shobika", sizeof(app->username) - 1);
    strncpy(app->fullname, "Shobika User", sizeof(app->fullname) - 1);
    strncpy(app->password, "shobikaos", sizeof(app->password) - 1);
    strncpy(app->hostname, "shobika-pc", sizeof(app->hostname) - 1);
    strncpy(app->desktop, "cinnamon", sizeof(app->desktop) - 1);
    
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
    
    // Step 0: Language Selection
    const char *langs[] = {"🇷🇺 Русский (ru_RU.UTF-8)", "🇺🇸 English (en_US.UTF-8)", NULL};
    app->lang_dropdown = gtk_drop_down_new_from_strings(langs);
    gtk_stack_add_named(GTK_STACK(app->stack), create_card_container("1. Выбор Языка Системы", "Выберите основной язык для интерфейса ShobikaOs", app->lang_dropdown), "step-0");
    
    // Step 1: Disk Selection Entry
    app->disk_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(app->disk_entry), "/dev/sda");
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->disk_entry), "Путь к диску (например /dev/sda или /dev/nvme0n1)");
    gtk_stack_add_named(GTK_STACK(app->stack), create_card_container("2. Выбор Целевого Диска", "⚠️ Внимание: Все существующие разделы на выбранном диске будут отформатированы!", app->disk_entry), "step-1");
    
    // Step 2: User Setup
    GtkWidget *user_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    app->user_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->user_entry), "Имя пользователя (username)");
    gtk_editable_set_text(GTK_EDITABLE(app->user_entry), "shobika");
    
    app->full_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->full_entry), "Полное имя (Full Name)");
    gtk_editable_set_text(GTK_EDITABLE(app->full_entry), "Shobika User");
    
    app->pass_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->pass_entry), "Пароль (password)");
    gtk_editable_set_text(GTK_EDITABLE(app->pass_entry), "shobikaos");
    
    app->host_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->host_entry), "Имя компьютера (hostname)");
    gtk_editable_set_text(GTK_EDITABLE(app->host_entry), "shobika-pc");
    
    gtk_box_append(GTK_BOX(user_box), app->user_entry);
    gtk_box_append(GTK_BOX(user_box), app->full_entry);
    gtk_box_append(GTK_BOX(user_box), app->pass_entry);
    gtk_box_append(GTK_BOX(user_box), app->host_entry);
    gtk_stack_add_named(GTK_STACK(app->stack), create_card_container("3. Учетная Запись и Безопасность", "Настройка имени пользователя, пароля и имени хоста", user_box), "step-2");
    
    // Step 3: Desktop Environment Selection
    const char *desktops[] = {"🖥️ Cinnamon (Классический флагман)", "📱 GNOME 46", "⚡ KDE Plasma 6", "🚀 XFCE 4.18", "🔷 Hyprland Wayland", NULL};
    app->de_dropdown = gtk_drop_down_new_from_strings(desktops);
    gtk_stack_add_named(GTK_STACK(app->stack), create_card_container("4. Окружение Рабочего Стола", "Выберите графическую оболочку для вашей системы", app->de_dropdown), "step-3");
    
    // Step 4: Confirm Installation
    GtkWidget *confirm_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *confirm_lbl = gtk_label_new("Система ShobikaOs будет установлена со следующими параметрами:\n• Диск: /dev/sda (ext4 + EFI)\n• Рабочий стол: Cinnamon Desktop\n• Пользователь: shobika\n\nНажмите 'Далее' для перехода к окну установки.");
    gtk_widget_add_css_class(confirm_lbl, "step-sub");
    gtk_box_append(GTK_BOX(confirm_box), confirm_lbl);
    gtk_stack_add_named(GTK_STACK(app->stack), create_card_container("5. Подтверждение Настроек", "Проверьте параметры перед запуском форматирования", confirm_box), "step-4");
    
    // Step 5: Installation Progress & Live Terminal Output
    GtkWidget *install_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(install_card, "step-card");
    
    app->status_label = gtk_label_new("🚀 Всё готово к инсталляции ShobikaOs! Нажмите кнопку ниже.");
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
    
    gtk_stack_add_named(GTK_STACK(app->stack), install_card, "step-5");
    
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
