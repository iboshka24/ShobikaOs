use gtk4::prelude::*;
use gtk4::{Box as GtkBox, Label, ListBox, ListBoxRow, Orientation, ScrolledWindow, Align, Separator, Image};
use std::sync::{Arc, Mutex};
use crate::installer::Installer;
use crate::steps::{Step, create_step_container};

#[derive(Clone)]
struct WallpaperOption {
    id: &'static str,
    name: &'static str,
    description: &'static str,
    icon: &'static str,
}

pub struct WallpaperStep {
    container: GtkBox,
    installer: Arc<Mutex<Installer>>,
    selected: Arc<Mutex<String>>,
}

impl WallpaperStep {
    pub fn new(installer: Arc<Mutex<Installer>>) -> Self {
        let container = create_step_container(
            "Выбор обоев",
            "Выберите обои рабочего стола. Все обои доступны после установки."
        );

        let list = ListBox::new();
        list.set_selection_mode(gtk4::SelectionMode::Single);
        list.add_css_class("wallpaper-list");

        let wallpapers = vec![
            WallpaperOption {
                id: "shobikaos-aurora.png",
                name: "🌌 Северное сияние",
                description: "Aurora borealis — голубые и бирюзовые переливы на тёмном фоне. По умолчанию.",
                icon: "🌌",
            },
            WallpaperOption {
                id: "shobikaos-geometric.png",
                name: "🔷 Геометрия",
                description: "Абстрактные геометрические формы с голубым свечением и частицами.",
                icon: "🔷",
            },
            WallpaperOption {
                id: "shobikaos-minimal.png",
                name: "🟣 Минимализм",
                description: "Градиентная сфера на тёмном фоне — чистый и спокойный стиль.",
                icon: "🟣",
            },
            WallpaperOption {
                id: "shobikaos-cosmic.png",
                name: "💎 Cosmic",
                description: "Гексагональный кристалл-логотип ShobikaOs с частицами.",
                icon: "💎",
            },
            WallpaperOption {
                id: "shobikaos-mountain.png",
                name: "🏔️ Горы",
                description: "Горный пейзаж с северным сиянием и звёздным небом.",
                icon: "🏔",
            },
            WallpaperOption {
                id: "shobikaos-circuit.png",
                name: "⚡ Circuit",
                description: "Футуристический S-логотип из печатных плат — киберпанк стиль.",
                icon: "⚡",
            },
            WallpaperOption {
                id: "shobikaos-wave.png",
                name: "🌊 Волна",
                description: "Шёлковая жидкая волна с градиентами — элегантный Apple-стиль.",
                icon: "🌊",
            },
            WallpaperOption {
                id: "shobikaos-nebula.png",
                name: "🪐 Туманность",
                description: "Космическая туманность со звёздами — глубокий космос.",
                icon: "🪐",
            },
        ];

        // Default selection is aurora
        let selected = Arc::new(Mutex::new("shobikaos-aurora.png".to_string()));

        for (i, wp) in wallpapers.iter().enumerate() {
            let row = ListBoxRow::new();
            row.add_css_class("wallpaper-row");
            
            let row_box = GtkBox::new(Orientation::Horizontal, 16);
            row_box.set_margin_top(14);
            row_box.set_margin_bottom(14);
            row_box.set_margin_start(20);
            row_box.set_margin_end(20);
            
            let icon_label = Label::new(Some(wp.icon));
            icon_label.add_css_class("wallpaper-icon");
            icon_label.set_markup(&format!("<span size='xx-large'>{}</span>", wp.icon));
            
            let info_box = GtkBox::new(Orientation::Vertical, 4);
            info_box.set_hexpand(true);
            
            let name_label = Label::new(Some(wp.name));
            name_label.add_css_class("wallpaper-name");
            name_label.set_halign(Align::Start);
            
            let desc_label = Label::new(Some(wp.description));
            desc_label.add_css_class("wallpaper-description");
            desc_label.set_halign(Align::Start);
            desc_label.set_wrap(true);
            desc_label.set_max_width_chars(55);
            
            // Default badge for aurora
            if wp.id == "shobikaos-aurora.png" {
                let badge = Label::new(Some("По умолчанию"));
                badge.add_css_class("default-badge");
                info_box.append(&name_label);
                let name_row = GtkBox::new(Orientation::Horizontal, 8);
                name_row.append(&name_label);
                name_row.append(&badge);
                info_box.append(&name_row);
            } else {
                info_box.append(&name_label);
            }
            info_box.append(&desc_label);
            
            row_box.append(&icon_label);
            row_box.append(&info_box);
            row.set_child(Some(&row_box));
            
            list.append(&row);
        }

        // Handle selection change
        let sel = selected.clone();
        let wallpapers_clone = wallpapers.clone();
        list.connect_row_selected(move |_, row| {
            if let Some(row) = row {
                let idx = row.index() as usize;
                if idx < wallpapers_clone.len() {
                    *sel.lock().unwrap() = wallpapers_clone[idx].id.to_string();
                }
            }
        });

        // Pre-select first row (aurora)
        if let Some(first_row) = list.row_at_index(0) {
            list.select_row(Some(&first_row));
        }

        let scrolled = ScrolledWindow::new();
        scrolled.set_child(Some(&list));
        scrolled.set_vexpand(true);
        scrolled.set_policy(gtk4::PolicyType::Never, gtk4::PolicyType::Automatic);
        scrolled.set_min_content_height(400);

        container.append(&scrolled);

        Self { container, installer, selected }
    }
}

impl Step for WallpaperStep {
    fn widget(&self) -> &gtk4::Widget {
        self.container.upcast_ref()
    }

    fn name(&self) -> &str {
        "wallpaper"
    }

    fn validate(&self) -> bool {
        true
    }

    fn apply(&self) -> anyhow::Result<()> {
        let wp = self.selected.lock().unwrap().clone();
        self.installer.lock().unwrap().config.wallpaper = wp;
        Ok(())
    }
}
