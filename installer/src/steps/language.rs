use gtk4::prelude::*;
use gtk4::{Box as GtkBox, Label, ListBox, ListBoxRow, Orientation, ScrolledWindow, Align, Button, Image, Separator};
use std::sync::{Arc, Mutex};
use crate::installer::Installer;
use crate::steps::{Step, create_step_container};

pub struct LanguageStep {
    container: GtkBox,
    installer: Arc<Mutex<Installer>>,
    selected: Arc<Mutex<Option<String>>>,
}

impl LanguageStep {
    pub fn new(installer: Arc<Mutex<Installer>>) -> Self {
        let container = create_step_container(
            "Выберите язык",
            "Выберите язык установки и системы"
        );

        let list = ListBox::new();
        list.set_selection_mode(gtk4::SelectionMode::Single);
        list.add_css_class("language-list");

        let languages = vec![
            ("en_US", "English (US)", "🇺🇸"),
            ("ru_RU", "Русский", "🇷🇺"),
            ("de_DE", "Deutsch", "🇩🇪"),
            ("fr_FR", "Français", "🇫🇷"),
            ("es_ES", "Español", "🇪🇸"),
            ("zh_CN", "中文 (简体)", "🇨🇳"),
            ("ja_JP", "日本語", "🇯🇵"),
            ("ko_KR", "한국어", "🇰🇷"),
        ];

        let selected = Arc::new(Mutex::new(None::<String>));

        for (code, name, flag) in languages {
            let row = ListBoxRow::new();
            row.add_css_class("language-row");
            
            let row_box = GtkBox::new(Orientation::Horizontal, 16);
            row_box.set_margin_top(12);
            row_box.set_margin_bottom(12);
            row_box.set_margin_start(16);
            row_box.set_margin_end(16);
            
            let flag_label = Label::new(Some(flag));
            flag_label.add_css_class("flag");
            flag_label.set_markup(&format!("<span size='xx-large'>{}</span>", flag));
            
            let name_label = Label::new(Some(name));
            name_label.add_css_class("language-name");
            name_label.set_halign(Align::Start);
            name_label.set_hexpand(true);
            
            row_box.append(&flag_label);
            row_box.append(&name_label);
            row.set_child(Some(&row_box));
            
            let sel = selected.clone();
            let code_clone = code.to_string();
            row.connect_activate(move |_| {
                *sel.lock().unwrap() = Some(code_clone.clone());
            });
            
            list.append(&row);
        }

        let scrolled = ScrolledWindow::new();
        scrolled.set_child(Some(&list));
        scrolled.set_vexpand(true);
        scrolled.set_policy(gtk4::PolicyType::Never, gtk4::PolicyType::Automatic);
        scrolled.set_min_content_height(300);

        container.append(&scrolled);

        Self { container, installer, selected }
    }
}

impl Step for LanguageStep {
    fn widget(&self) -> &gtk4::Widget {
        self.container.upcast_ref()
    }

    fn name(&self) -> &str {
        "language"
    }

    fn validate(&self) -> bool {
        self.selected.lock().unwrap().is_some()
    }

    fn apply(&self) -> anyhow::Result<()> {
        if let Some(lang) = self.selected.lock().unwrap().as_ref() {
            self.installer.lock().unwrap().config.language = lang.clone();
        }
        Ok(())
    }
}