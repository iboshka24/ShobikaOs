use gtk4::prelude::*;
use gtk4::{Box as GtkBox, Label, ListBox, ListBoxRow, Orientation, ScrolledWindow, Align, Button, Separator};
use std::sync::{Arc, Mutex};
use crate::installer::Installer;
use crate::steps::{Step, create_step_container};
use blockdev::BlockDevice;

pub struct DiskStep {
    container: GtkBox,
    installer: Arc<Mutex<Installer>>,
    selected: Arc<Mutex<Option<String>>>,
    use_btrfs: Arc<Mutex<bool>>,
    use_luks: Arc<Mutex<bool>>,
    luks_password: Arc<Mutex<String>>,
}

impl DiskStep {
    pub fn new(installer: Arc<Mutex<Installer>>) -> Self {
        let container = create_step_container(
            "Выбор диска",
            "Выберите диск для установки. Все данные на диске будут удалены!"
        );

        let warning = Label::new(Some("⚠ Внимание: все данные на выбранном диске будут уничтожены"));
        warning.add_css_class("warning");
        warning.set_halign(Align::Center);
        warning.set_wrap(true);
        container.append(&warning);

        let list = ListBox::new();
        list.set_selection_mode(gtk4::SelectionMode::Single);
        list.add_css_class("disk-list");

        let devices = BlockDevice::list_all().unwrap_or_default();
        let selected = Arc::new(Mutex::new(None::<String>));

        for dev in devices {
            if dev.size == 0 { continue; }
            
            let row = ListBoxRow::new();
            row.add_css_class("disk-row");
            
            let row_box = GtkBox::new(Orientation::Horizontal, 16);
            row_box.set_margin_top(16);
            row_box.set_margin_bottom(16);
            row_box.set_margin_start(16);
            row_box.set_margin_end(16);
            
            let icon = Label::new(Some("💾"));
            icon.add_css_class("disk-icon");
            icon.set_markup("<span size='x-large'>💾</span>");
            
            let info_box = GtkBox::new(Orientation::Vertical, 4);
            info_box.set_hexpand(true);
            
            let name = Label::new(Some(&format!("{} ({})", dev.path.display(), dev.model.unwrap_or_default())));
            name.add_css_class("disk-name");
            name.set_halign(Align::Start);
            
            let size = Label::new(Some(&format!("Размер: {:.1} GB", dev.size as f64 / 1e9)));
            size.add_css_class("disk-size");
            size.set_halign(Align::Start);
            
            info_box.append(&name);
            info_box.append(&size);
            
            row_box.append(&icon);
            row_box.append(&info_box);
            row.set_child(Some(&row_box));
            
            let sel = selected.clone();
            let path = dev.path.to_string_lossy().to_string();
            row.connect_activate(move |_| {
                *sel.lock().unwrap() = Some(path.clone());
            });
            
            list.append(&row);
        }

        let scrolled = ScrolledWindow::new();
        scrolled.set_child(Some(&list));
        scrolled.set_vexpand(true);
        scrolled.set_policy(gtk4::PolicyType::Never, gtk4::PolicyType::Automatic);
        scrolled.set_min_content_height(250);
        container.append(&scrolled);

        let sep = Separator::new(Orientation::Horizontal);
        sep.set_margin_top(12);
        sep.set_margin_bottom(12);
        container.append(&sep);

        let options_box = GtkBox::new(Orientation::Vertical, 12);
        options_box.set_halign(Align::Center);
        options_box.set_margin_top(12);

        let use_btrfs = Arc::new(Mutex::new(true));
        let btrfs_check = Button::new();
        btrfs_check.add_css_class("check-button");
        let btrfs_label = Label::new(Some("Использовать Btrfs с сжатием (zstd) + сабволюмы"));
        btrfs_label.set_halign(Align::Start);
        btrfs_label.set_hexpand(true);
        let btrfs_box = GtkBox::new(Orientation::Horizontal, 12);
        btrfs_box.set_margin_start(16);
        btrfs_box.set_margin_end(16);
        btrfs_box.append(&btrfs_check);
        btrfs_box.append(&btrfs_label);
        let btrfs_sel = use_btrfs.clone();
        btrfs_check.connect_clicked(move |btn| {
            let active = !*btrfs_sel.lock().unwrap();
            *btrfs_sel.lock().unwrap() = active;
            btn.add_css_class(if active { "active" } else { "" });
        });
        options_box.append(&btrfs_box);

        let use_luks = Arc::new(Mutex::new(false));
        let luks_check = Button::new();
        luks_check.add_css_class("check-button");
        let luks_label = Label::new(Some("Шифровать диск (LUKS2)"));
        luks_label.set_halign(Align::Start);
        luks_label.set_hexpand(true);
        let luks_box = GtkBox::new(Orientation::Horizontal, 12);
        luks_box.set_margin_start(16);
        luks_box.set_margin_end(16);
        luks_box.append(&luks_check);
        luks_box.append(&luks_label);
        let luks_sel = use_luks.clone();
        luks_check.connect_clicked(move |btn| {
            let active = !*luks_sel.lock().unwrap();
            *luks_sel.lock().unwrap() = active;
            btn.add_css_class(if active { "active" } else { "" });
        });
        options_box.append(&luks_box);

        container.append(&options_box);

        Self { container, installer, selected, use_btrfs, use_luks, luks_password: Arc::new(Mutex::new(String::new())) }
    }
}

impl Step for DiskStep {
    fn widget(&self) -> &gtk4::Widget {
        self.container.upcast_ref()
    }

    fn name(&self) -> &str {
        "disk"
    }

    fn validate(&self) -> bool {
        self.selected.lock().unwrap().is_some()
    }

    fn apply(&self) -> anyhow::Result<()> {
        let mut inst = self.installer.lock().unwrap();
        inst.config.disk = self.selected.lock().unwrap().clone().unwrap();
        inst.config.use_btrfs = *self.use_btrfs.lock().unwrap();
        inst.config.use_luks = *self.use_luks.lock().unwrap();
        Ok(())
    }
}