pub mod language;
pub mod keyboard;
pub mod disk;
pub mod user;
pub mod desktop;
pub mod wallpaper;
pub mod install;

pub use language::LanguageStep;
pub use keyboard::KeyboardStep;
pub use disk::DiskStep;
pub use user::UserStep;
pub use desktop::DesktopStep;
pub use wallpaper::WallpaperStep;
pub use install::InstallStep;

use gtk4::prelude::*;
use gtk4::Widget;
use std::sync::{Arc, Mutex};
use crate::installer::Installer;

pub trait Step: Send + Sync {
    fn widget(&self) -> &Widget;
    fn name(&self) -> &str;
    fn validate(&self) -> bool;
    fn apply(&self) -> anyhow::Result<()>;
}

fn create_step_container(title: &str, subtitle: &str) -> gtk4::Box {
    let container = gtk4::Box::new(gtk4::Orientation::Vertical, 24);
    container.set_margin_top(48);
    container.set_margin_bottom(48);
    container.set_margin_start(64);
    container.set_margin_end(64);
    container.set_halign(gtk4::Align::Center);
    container.set_valign(gtk4::Align::Center);

    let title_label = gtk4::Label::new(Some(title));
    title_label.add_css_class("step-title");
    title_label.set_halign(gtk4::Align::Center);

    let subtitle_label = gtk4::Label::new(Some(subtitle));
    subtitle_label.add_css_class("step-subtitle");
    subtitle_label.set_halign(gtk4::Align::Center);
    subtitle_label.set_wrap(true);
    subtitle_label.set_max_width_chars(60);

    container.append(&title_label);
    container.append(&subtitle_label);

    container
}