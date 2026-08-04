use gtk4::prelude::*;
use gtk4::{Box as GtkBox, Label, ComboBoxText, Orientation, Align, Separator};
use std::sync::{Arc, Mutex};
use crate::installer::Installer;
use crate::steps::{Step, create_step_container};

pub struct KeyboardStep {
    container: GtkBox,
    installer: Arc<Mutex<Installer>>,
    layout: Arc<Mutex<String>>,
    variant: Arc<Mutex<String>>,
}

impl KeyboardStep {
    pub fn new(installer: Arc<Mutex<Installer>>) -> Self {
        let container = create_step_container(
            "Раскладка клавиатуры",
            "Выберите раскладку и вариант клавиатуры"
        );

        let layout_box = GtkBox::new(Orientation::Vertical, 12);
        layout_box.set_halign(Align::Center);
        layout_box.set_valign(Align::Center);
        layout_box.set_margin_top(24);

        let layout_label = Label::new(Some("Раскладка"));
        layout_label.add_css_class("field-label");
        
        let layout_combo = ComboBoxText::new();
        layout_combo.set_hexpand(true);
        layout_combo.set_min_width(300);
        
        let layouts = vec![
            ("us", "US English"),
            ("ru", "Russian"),
            ("de", "German"),
            ("fr", "French"),
            ("es", "Spanish"),
            ("gb", "UK English"),
            ("jp", "Japanese"),
            ("cn", "Chinese"),
        ];
        
        for (code, name) in layouts {
            layout_combo.append(Some(code), name);
        }
        layout_combo.set_active_id(Some("us"));

        let layout_sel = Arc::new(Mutex::new("us".to_string()));
        let layout_sel_clone = layout_sel.clone();
        layout_combo.connect_changed(move |combo| {
            if let Some(id) = combo.active_id() {
                *layout_sel_clone.lock().unwrap() = id.to_string();
            }
        });

        layout_box.append(&layout_label);
        layout_box.append(&layout_combo);

        let sep = Separator::new(Orientation::Horizontal);
        sep.set_margin_top(12);
        sep.set_margin_bottom(12);
        layout_box.append(&sep);

        let variant_label = Label::new(Some("Вариант"));
        variant_label.add_css_class("field-label");
        
        let variant_combo = ComboBoxText::new();
        variant_combo.set_hexpand(true);
        variant_combo.set_min_width(300);
        
        let variants = vec![
            ("", "Default"),
            ("dvorak", "Dvorak"),
            ("colemak", "Colemak"),
            ("workman", "Workman"),
        ];
        
        for (code, name) in variants {
            variant_combo.append(Some(code), name);
        }
        variant_combo.set_active_id(Some(""));

        let variant_sel = Arc::new(Mutex::new("".to_string()));
        let variant_sel_clone = variant_sel.clone();
        variant_combo.connect_changed(move |combo| {
            if let Some(id) = combo.active_id() {
                *variant_sel_clone.lock().unwrap() = id.to_string();
            }
        });

        layout_box.append(&variant_label);
        layout_box.append(&variant_combo);

        container.append(&layout_box);

        Self { container, installer, layout: layout_sel, variant: variant_sel }
    }
}

impl Step for KeyboardStep {
    fn widget(&self) -> gtk4::Widget {
        self.container.clone().upcast()
    }

    fn name(&self) -> &str {
        "keyboard"
    }

    fn validate(&self) -> bool {
        true
    }

    fn apply(&self) -> anyhow::Result<()> {
        let mut inst = self.installer.lock().unwrap();
        inst.config.keyboard_layout = self.layout.lock().unwrap().clone();
        inst.config.keyboard_variant = self.variant.lock().unwrap().clone();
        Ok(())
    }
}