use gtk4::prelude::*;
use gtk4::{Box as GtkBox, Label, ListBox, ListBoxRow, Orientation, ScrolledWindow, Align, Image, Separator, Button, CheckButton, ComboBoxText};
use std::sync::{Arc, Mutex};
use crate::installer::Installer;
use crate::steps::{Step, create_step_container};

#[derive(Clone)]
struct DesktopOption {
    id: &'static str,
    name: &'static str,
    description: &'static str,
    icon: &'static str,
    packages: &'static [&'static str],
}

pub struct DesktopStep {
    container: GtkBox,
    installer: Arc<Mutex<Installer>>,
    selected: Arc<Mutex<Option<String>>>,
    display_manager: Arc<Mutex<String>>,
    install_gaming: Arc<Mutex<bool>>,
    install_multimedia: Arc<Mutex<bool>>,
    install_browser: Arc<Mutex<bool>>,
    install_flatpak: Arc<Mutex<bool>>,
    install_bluetooth: Arc<Mutex<bool>>,
    install_office: Arc<Mutex<bool>>,
    install_printing: Arc<Mutex<bool>>,
}

impl DesktopStep {
    pub fn new(installer: Arc<Mutex<Installer>>) -> Self {
        let container = create_step_container(
            "Окружение рабочего стола",
            "Выберите графическое окружение и дисплейный менеджер"
        );

        let list = ListBox::new();
        list.set_selection_mode(gtk4::SelectionMode::Single);
        list.add_css_class("desktop-list");

        let desktops = vec![
            DesktopOption {
                id: "cinnamon",
                name: "Cinnamon",
                description: "Классический интерфейс с панелью снизу. Настроен: тёмная тема Arc, иконки Papirus, кастомные цвета терминала.",
                icon: "🖥️",
                packages: &["cinnamon", "nemo", "gnome-terminal", "lightdm"],
            },
            DesktopOption {
                id: "gnome",
                name: "GNOME",
                description: "Современный минималистичный интерфейс. Настроен: тёмная тема, Dash to Dock, кастомные расширения, акцентные цвета.",
                icon: "📱",
                packages: &["gnome", "gnome-extra", "gnome-tweaks", "gdm"],
            },
            DesktopOption {
                id: "kde",
                name: "KDE Plasma",
                description: "Мощный и настраиваемый интерфейс. Настроен: Breeze Dark, Kvantum, кастомные виджеты, иконки Papirus.",
                icon: "⚡",
                packages: &["plasma", "kde-applications", "sddm", "kvantum"],
            },
            DesktopOption {
                id: "xfce",
                name: "XFCE",
                description: "Лёгкий и быстрый интерфейс. Настроен: тёмная тема, кастомная панель, композитинг, терминал.",
                icon: "🚀",
                packages: &["xfce4", "xfce4-goodies", "lightdm", "thunar"],
            },
            DesktopOption {
                id: "hyprland",
                name: "Hyprland",
                description: "Динамический тайловый Wayland-композитор. Настроен: blur, анимации, waybar, wofi, foot, hyprpaper.",
                icon: "🔷",
                packages: &["hyprland", "waybar", "wofi", "foot", "hyprpaper", "sddm"],
            },
        ];

        let selected = Arc::new(Mutex::new(None::<String>));

        for de in desktops {
            let row = ListBoxRow::new();
            row.add_css_class("desktop-row");
            
            let row_box = GtkBox::new(Orientation::Horizontal, 20);
            row_box.set_margin_top(20);
            row_box.set_margin_bottom(20);
            row_box.set_margin_start(20);
            row_box.set_margin_end(20);
            
            let icon_label = Label::new(Some(de.icon));
            icon_label.add_css_class("desktop-icon");
            icon_label.set_markup(&format!("<span size='xx-large'>{}</span>", de.icon));
            
            let info_box = GtkBox::new(Orientation::Vertical, 8);
            info_box.set_hexpand(true);
            
            let name_label = Label::new(Some(de.name));
            name_label.add_css_class("desktop-name");
            name_label.set_halign(Align::Start);
            
            let desc_label = Label::new(Some(de.description));
            desc_label.add_css_class("desktop-description");
            desc_label.set_halign(Align::Start);
            desc_label.set_wrap(true);
            desc_label.set_max_width_chars(50);
            desc_label.set_hexpand(true);
            
            info_box.append(&name_label);
            info_box.append(&desc_label);
            
            row_box.append(&icon_label);
            row_box.append(&info_box);
            row.set_child(Some(&row_box));
            
            let sel = selected.clone();
            let id = de.id;
            row.connect_activate(move |_| {
                *sel.lock().unwrap() = Some(id.to_string());
            });
            
            list.append(&row);
        }

        let scrolled = ScrolledWindow::new();
        scrolled.set_child(Some(&list));
        scrolled.set_vexpand(true);
        scrolled.set_policy(gtk4::PolicyType::Never, gtk4::PolicyType::Automatic);
        scrolled.set_min_content_height(300);

        container.append(&scrolled);

        let sep1 = Separator::new(Orientation::Horizontal);
        sep1.set_margin_top(12);
        sep1.set_margin_bottom(12);
        container.append(&sep1);

        let dm_box = GtkBox::new(Orientation::Horizontal, 16);
        dm_box.set_margin_start(16);
        dm_box.set_margin_end(16);
        dm_box.set_halign(Align::Start);

        let dm_label = Label::new(Some("🔑 Дисплейный менеджер (Display Manager):"));
        dm_label.add_css_class("field-label");

        let dm_combo = ComboBoxText::new();
        dm_combo.append(Some("auto"), "Автоматически (Рекомендуемый для DE)");
        dm_combo.append(Some("sddm"), "SDDM (SDDM Catppuccin theme)");
        dm_combo.append(Some("gdm"), "GDM (GNOME Display Manager)");
        dm_combo.append(Some("lightdm"), "LightDM (Webkit2 Greeter)");
        dm_combo.append(Some("ly"), "Ly (Лёгкий TUI дисплей менеджер)");
        dm_combo.set_active_id(Some("auto"));

        let display_manager = Arc::new(Mutex::new("auto".to_string()));
        let dm_clone = display_manager.clone();
        dm_combo.connect_changed(move |combo| {
            if let Some(id) = combo.active_id() {
                *dm_clone.lock().unwrap() = id.to_string();
            }
        });

        dm_box.append(&dm_label);
        dm_box.append(&dm_combo);
        container.append(&dm_box);

        let sep2 = Separator::new(Orientation::Horizontal);
        sep2.set_margin_top(12);
        sep2.set_margin_bottom(12);
        container.append(&sep2);

        let extras_title = Label::new(Some("Дополнительные пакеты"));
        extras_title.add_css_class("step-subtitle");
        extras_title.set_halign(Align::Start);
        container.append(&extras_title);

        let extras_box = GtkBox::new(Orientation::Vertical, 8);
        extras_box.set_margin_start(16);
        extras_box.set_margin_end(16);

        let install_gaming = Arc::new(Mutex::new(true));
        let install_multimedia = Arc::new(Mutex::new(true));
        let install_browser = Arc::new(Mutex::new(true));
        let install_flatpak = Arc::new(Mutex::new(true));
        let install_bluetooth = Arc::new(Mutex::new(true));
        let install_office = Arc::new(Mutex::new(false));
        let install_printing = Arc::new(Mutex::new(false));

        let add_check = |box_: &GtkBox, label: &str, state: Arc<Mutex<bool>>| {
            let cb = CheckButton::builder().label(label).active(*state.lock().unwrap()).build();
            cb.add_css_class("extras-check");
            let state_clone = state.clone();
            cb.connect_toggled(move |btn| {
                *state_clone.lock().unwrap() = btn.is_active();
            });
            box_.append(&cb);
        };

        add_check(&extras_box, "🎮 Игры (Steam, Lutris, MangoHud, gamemode)", install_gaming.clone());
        add_check(&extras_box, "🎬 Мультимедиа (PipeWire, кодеки, ffmpeg)", install_multimedia.clone());
        add_check(&extras_box, "🌐 Браузер (Firefox)", install_browser.clone());
        add_check(&extras_box, "📦 Flatpak + Flathub", install_flatpak.clone());
        add_check(&extras_box, "📶 Bluetooth поддержка", install_bluetooth.clone());
        add_check(&extras_box, "📝 Офис (LibreOffice)", install_office.clone());
        add_check(&extras_box, "🖨️ Принтеры (CUPS)", install_printing.clone());

        container.append(&extras_box);

        Self { 
            container, installer, selected, display_manager,
            install_gaming, install_multimedia, install_browser,
            install_flatpak, install_bluetooth, install_office, install_printing
        }
    }
}

impl Step for DesktopStep {
    fn widget(&self) -> gtk4::Widget {
        self.container.clone().upcast()
    }

    fn name(&self) -> &str {
        "desktop"
    }

    fn validate(&self) -> bool {
        self.selected.lock().unwrap().is_some()
    }

    fn apply(&self) -> anyhow::Result<()> {
        let mut inst = self.installer.lock().unwrap();
        if let Some(de) = self.selected.lock().unwrap().as_ref() {
            inst.config.desktop = de.clone();
        }
        inst.config.display_manager = self.display_manager.lock().unwrap().clone();
        inst.config.install_gaming = *self.install_gaming.lock().unwrap();
        inst.config.install_multimedia = *self.install_multimedia.lock().unwrap();
        inst.config.install_browser = *self.install_browser.lock().unwrap();
        inst.config.install_flatpak = *self.install_flatpak.lock().unwrap();
        inst.config.install_bluetooth = *self.install_bluetooth.lock().unwrap();
        inst.config.install_office = *self.install_office.lock().unwrap();
        inst.config.install_printing = *self.install_printing.lock().unwrap();
        Ok(())
    }
}