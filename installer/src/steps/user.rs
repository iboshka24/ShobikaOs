use gtk4::prelude::*;
use gtk4::{Box as GtkBox, Label, Entry, Orientation, Align, Separator, PasswordEntry};
use std::sync::{Arc, Mutex};
use crate::installer::Installer;
use crate::steps::{Step, create_step_container};

pub struct UserStep {
    container: GtkBox,
    installer: Arc<Mutex<Installer>>,
    username: Arc<Mutex<String>>,
    fullname: Arc<Mutex<String>>,
    password: Arc<Mutex<String>>,
    confirm_password: Arc<Mutex<String>>,
    hostname: Arc<Mutex<String>>,
}

impl UserStep {
    pub fn new(installer: Arc<Mutex<Installer>>) -> Self {
        let container = create_step_container(
            "Пользователь и сеть",
            "Создайте пользователя и задайте имя компьютера"
        );

        let form = GtkBox::new(Orientation::Vertical, 16);
        form.set_halign(Align::Center);
        form.set_valign(Align::Center);
        form.set_margin_top(24);
        form.set_size_request(400, -1);

        let username = Arc::new(Mutex::new(String::new()));
        let fullname = Arc::new(Mutex::new(String::new()));
        let password = Arc::new(Mutex::new(String::new()));
        let confirm_password = Arc::new(Mutex::new(String::new()));
        let hostname = Arc::new(Mutex::new("shobikaos".to_string()));

        macro_rules! add_field {
            ($label:expr, $entry:expr, $placeholder:expr, $store:expr, $is_password:expr) => {{
                let lbl = Label::new(Some($label));
                lbl.add_css_class("field-label");
                lbl.set_halign(Align::Start);
                
                if $is_password {
                    let pass_entry = PasswordEntry::new();
                    pass_entry.set_placeholder_text(Some($placeholder));
                    pass_entry.set_hexpand(true);
                    let store = $store.clone();
                    pass_entry.connect_text_changed(move |e| {
                        *store.lock().unwrap() = e.text().to_string();
                    });
                    form.append(&lbl);
                    form.append(&pass_entry);
                } else {
                    let entry = Entry::new();
                    entry.set_placeholder_text(Some($placeholder));
                    entry.set_hexpand(true);
                    let store = $store.clone();
                    entry.connect_changed(move |e| {
                        *store.lock().unwrap() = e.text().to_string();
                    });
                    form.append(&lbl);
                    form.append(&entry);
                }
            }};
        }

        add_field!("Имя пользователя", "", "user", username, false);
        add_field!("Полное имя", "", "User Name", fullname, false);
        add_field!("Пароль", "", "••••••••", password, true);
        add_field!("Подтвердите пароль", "", "••••••••", confirm_password, true);
        
        let sep = Separator::new(Orientation::Horizontal);
        sep.set_margin_top(8);
        sep.set_margin_bottom(8);
        form.append(&sep);

        add_field!("Имя компьютера (hostname)", "", "shobikaos", hostname, false);

        container.append(&form);

        Self { container, installer, username, fullname, password, confirm_password, hostname }
    }
}

impl Step for UserStep {
    fn widget(&self) -> &gtk4::Widget {
        self.container.upcast_ref()
    }

    fn name(&self) -> &str {
        "user"
    }

    fn validate(&self) -> bool {
        let u = self.username.lock().unwrap();
        let p = self.password.lock().unwrap();
        let cp = self.confirm_password.lock().unwrap();
        !u.is_empty() && !p.is_empty() && p == *cp && u.chars().all(|c| c.is_ascii_lowercase() || c.is_ascii_digit() || c == '_')
    }

    fn apply(&self) -> anyhow::Result<()> {
        let mut inst = self.installer.lock().unwrap();
        inst.config.username = self.username.lock().unwrap().clone();
        inst.config.fullname = self.fullname.lock().unwrap().clone();
        inst.config.password = self.password.lock().unwrap().clone();
        inst.config.hostname = self.hostname.lock().unwrap().clone();
        Ok(())
    }
}