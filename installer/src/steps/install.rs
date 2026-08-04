use gtk4::prelude::*;
use gtk4::{Box as GtkBox, Label, ProgressBar, Orientation, Align, Separator, Button, Spinner};
use std::sync::{Arc, Mutex};
use crate::installer::Installer;
use crate::steps::{Step, create_step_container};
use std::thread;
use glib::MainContext;

pub struct InstallStep {
    container: GtkBox,
    installer: Arc<Mutex<Installer>>,
    progress: Arc<Mutex<f64>>,
    status: Arc<Mutex<String>>,
    finished: Arc<Mutex<bool>>,
    error: Arc<Mutex<Option<String>>>,
}

impl InstallStep {
    pub fn new(installer: Arc<Mutex<Installer>>) -> Self {
        let container = create_step_container(
            "Установка ShobikaOs",
            "Подготовка к установке. Нажмите \"Установить\" для начала."
        );

        let content = GtkBox::new(Orientation::Vertical, 24);
        content.set_halign(Align::Center);
        content.set_valign(Align::Center);

        let status_label = Label::new(Some("Готов к установке"));
        status_label.add_css_class("install-status");
        status_label.set_halign(Align::Center);
        content.append(&status_label);

        let progress = ProgressBar::new();
        progress.set_fraction(0.0);
        progress.set_show_text(true);
        progress.set_hexpand(true);
        progress.set_min_width(400);
        progress.add_css_class("install-progress");
        content.append(&progress);

        let details = Label::new(Some(""));
        details.add_css_class("install-details");
        details.set_halign(Align::Center);
        details.set_wrap(true);
        details.set_max_width_chars(60);
        content.append(&details);

        let spinner = Spinner::new();
        spinner.add_css_class("install-spinner");
        content.append(&spinner);

        let install_btn = Button::with_label("Установить");
        install_btn.add_css_class("install-button");
        install_btn.add_css_class("suggested-action");
        install_btn.set_halign(Align::Center);
        install_btn.set_min_width(200);
        content.append(&install_btn);

        let progress_arc = Arc::new(Mutex::new(0.0));
        let status_arc = Arc::new(Mutex::new("Готов к установке".to_string()));
        let details_arc = Arc::new(Mutex::new(String::new()));
        let finished_arc = Arc::new(Mutex::new(false));
        let error_arc = Arc::new(Mutex::new(None::<String>));
        let installer_clone = installer.clone();

        let progress_ui = progress.clone();
        let status_ui = status_label.clone();
        let details_ui = details.clone();
        let spinner_ui = spinner.clone();
        let btn_ui = install_btn.clone();

        install_btn.connect_clicked(move |_| {
            btn_ui.set_sensitive(false);
            spinner_ui.start();
            
            let installer = installer_clone.clone();
            let progress = progress_arc.clone();
            let status = status_arc.clone();
            let details = details_arc.clone();
            let finished = finished_arc.clone();
            let error = error_arc.clone();
            
            thread::spawn(move || {
                let result = Self::run_install(installer, progress, status, details);
                
                MainContext::default().invoke(move || {
                    spinner_ui.stop();
                    match result {
                        Ok(_) => {
                            *finished.lock().unwrap() = true;
                            progress_ui.set_fraction(1.0);
                            progress_ui.set_text(Some("Установка завершена!"));
                            status_ui.set_text("Установка успешно завершена");
                            details_ui.set_text("Перезагрузите компьютер для входа в систему");
                            btn_ui.set_label("Перезагрузить");
                            btn_ui.set_sensitive(true);
                            btn_ui.connect_clicked(|_| {
                                std::process::Command::new("reboot").status().ok();
                            });
                        }
                        Err(e) => {
                            *error.lock().unwrap() = Some(e.to_string());
                            status_ui.set_text("Ошибка установки");
                            status_ui.add_css_class("error");
                            details_ui.set_text(&format!("Ошибка: {}", e));
                            btn_ui.set_label("Повторить");
                            btn_ui.set_sensitive(true);
                            spinner_ui.stop();
                        }
                    }
                });
            });
        });

        let progress_clone = progress_arc.clone();
        let status_clone = status_arc.clone();
        let details_clone = details_arc.clone();
        let progress_ui_clone = progress.clone();
        let status_ui_clone = status_label.clone();
        let details_ui_clone = details.clone();

        glib::timeout_add_local(std::time::Duration::from_millis(100), move || {
            let p = *progress_clone.lock().unwrap();
            let s = status_clone.lock().unwrap().clone();
            let d = details_clone.lock().unwrap().clone();
            
            progress_ui_clone.set_fraction(p);
            progress_ui_clone.set_text(Some(&format!("{:.0}%", p * 100.0)));
            status_ui_clone.set_text(&s);
            details_ui_clone.set_text(&d);
            
            if *finished_arc.lock().unwrap() || error_arc.lock().unwrap().is_some() {
                glib::ControlFlow::Break
            } else {
                glib::ControlFlow::Continue
            }
        });

        container.append(&content);

        Self { container, installer, progress: progress_arc, status: status_arc, finished: finished_arc, error: error_arc }
    }

    fn run_install(
        installer: Arc<Mutex<Installer>>,
        progress: Arc<Mutex<f64>>,
        status: Arc<Mutex<String>>,
        details: Arc<Mutex<String>>,
    ) -> anyhow::Result<()> {
        let mut inst = installer.lock().unwrap();
        
        *status.lock().unwrap() = "Разметка диска".to_string();
        *details.lock().unwrap() = "Создание разделов и файловых систем...".to_string();
        *progress.lock().unwrap() = 0.05;
        inst.partition_disk()?;
        
        *status.lock().unwrap() = "Установка базовой системы".to_string();
        *details.lock().unwrap() = "pacstrap: base, linux, firmware, утилиты...".to_string();
        *progress.lock().unwrap() = 0.10;
        inst.install_base()?;
        
        *status.lock().unwrap() = "Определение оборудования".to_string();
        *details.lock().unwrap() = "Установка драйверов GPU, CPU микрокода...".to_string();
        *progress.lock().unwrap() = 0.25;
        inst.install_hardware_drivers()?;
        
        *status.lock().unwrap() = "Мультимедиа стек".to_string();
        *details.lock().unwrap() = "PipeWire, кодеки, ffmpeg...".to_string();
        *progress.lock().unwrap() = 0.35;
        inst.install_multimedia_stack()?;
        
        *status.lock().unwrap() = "Установка окружения".to_string();
        *details.lock().unwrap() = format!("Установка {}...", inst.config.desktop);
        *progress.lock().unwrap() = 0.45;
        inst.install_desktop()?;
        
        *status.lock().unwrap() = "Игровой стек".to_string();
        *details.lock().unwrap() = "Steam, Lutris, MangoHud, gamemode...".to_string();
        *progress.lock().unwrap() = 0.55;
        inst.install_gaming_stack()?;
        
        *status.lock().unwrap() = "Дополнительные пакеты".to_string();
        *details.lock().unwrap() = "Firefox, Flatpak, Bluetooth...".to_string();
        *progress.lock().unwrap() = 0.63;
        inst.install_optional_packages()?;
        
        *status.lock().unwrap() = "Настройка системы".to_string();
        *details.lock().unwrap() = "fstab, locale, users, сервисы...".to_string();
        *progress.lock().unwrap() = 0.72;
        inst.configure_system()?;
        
        *status.lock().unwrap() = "Применение обоев".to_string();
        *details.lock().unwrap() = format!("Установка {}...", inst.config.wallpaper);
        *progress.lock().unwrap() = 0.82;
        inst.apply_wallpaper()?;
        
        *status.lock().unwrap() = "Оптимизация".to_string();
        *details.lock().unwrap() = "sysctl, systemd, zram, зеркала...".to_string();
        *progress.lock().unwrap() = 0.88;
        inst.apply_performance_tweaks()?;
        
        *status.lock().unwrap() = "Финализация".to_string();
        *details.lock().unwrap() = "Очистка кэша, синхронизация...".to_string();
        *progress.lock().unwrap() = 0.95;
        inst.finalize()?;
        
        Ok(())
    }
}

impl Step for InstallStep {
    fn widget(&self) -> gtk4::Widget {
        self.container.clone().upcast()
    }

    fn name(&self) -> &str {
        "install"
    }

    fn validate(&self) -> bool {
        true
    }

    fn apply(&self) -> anyhow::Result<()> {
        Ok(())
    }
}