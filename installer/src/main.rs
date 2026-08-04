use anyhow::Result;
use clap::Parser;
use gtk4::prelude::*;
use gtk4::{Application, ApplicationWindow, Box as GtkBox, Button, Label, Orientation, Stack, StackSwitcher, ScrolledWindow, ListBox, ListBoxRow, CheckButton, Entry, ComboBoxText, ProgressBar, Separator, Frame, Align, CssProvider, StyleContext, gdk4::Display};
use glib::{MainContext, clone};
use std::process::Command;
use std::sync::{Arc, Mutex};

mod steps;
mod installer;
mod utils;

use steps::{LanguageStep, KeyboardStep, DiskStep, UserStep, DesktopStep, WallpaperStep, InstallStep};
use installer::Installer;

#[derive(Parser)]
#[command(name = "shobika-installer")]
struct Args {
    #[arg(short, long)]
    debug: bool,
}

fn main() -> Result<()> {
    let args = Args::parse();
    
    if args.debug {
        std::env::set_var("RUST_LOG", "debug");
    }

    let app = Application::builder()
        .application_id("org.shobikaos.installer")
        .flags(gio::ApplicationFlags::NON_UNIQUE)
        .build();

    app.connect_activate(build_ui);
    app.run();
    Ok(())
}

fn build_ui(app: &Application) {
    let css = CssProvider::new();
    css.load_from_string(include_str!("style.css"));
    if let Some(display) = Display::default() {
        StyleContext::add_provider_for_display(&display, &css, gtk4::STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

    let window = ApplicationWindow::builder()
        .application(app)
        .title("ShobikaOs Installer")
        .default_width(1000)
        .default_height(700)
        .resizable(false)
        .decorated(false)
        .build();

    let main_box = GtkBox::new(Orientation::Vertical, 0);
    main_box.add_css_class("main-window");

    let header = create_header();
    main_box.append(&header);

    let stack = Stack::new();
    stack.set_transition_type(gtk4::StackTransitionType::SlideLeftRight);
    stack.set_transition_duration(300);

    let installer = Arc::new(Mutex::new(Installer::new()));

    let steps: Vec<Box<dyn Step>> = vec![
        Box::new(LanguageStep::new(installer.clone())),
        Box::new(KeyboardStep::new(installer.clone())),
        Box::new(DiskStep::new(installer.clone())),
        Box::new(UserStep::new(installer.clone())),
        Box::new(DesktopStep::new(installer.clone())),
        Box::new(WallpaperStep::new(installer.clone())),
        Box::new(InstallStep::new(installer.clone())),
    ];

    for (i, step) in steps.iter().enumerate() {
        let page = step.widget();
        page.set_name(&format!("step-{}", i));
        stack.add_named(page, Some(&format!("step-{}", i)));
    }

    let switcher = StackSwitcher::new();
    switcher.set_stack(Some(&stack));
    switcher.add_css_class("step-switcher");
    main_box.append(&switcher);

    let scrolled = ScrolledWindow::new();
    scrolled.set_child(Some(&stack));
    scrolled.set_vexpand(true);
    scrolled.set_policy(gtk4::PolicyType::Never, gtk4::PolicyType::Automatic);
    main_box.append(&scrolled);

    let nav_box = create_navigation(&stack, steps.len());
    main_box.append(&nav_box);

    window.set_child(Some(&main_box));
    window.present();
}

fn create_header() -> GtkBox {
    let header = GtkBox::new(Orientation::Horizontal, 16);
    header.add_css_class("header");

    let logo = Label::new(Some("ShobikaOs"));
    logo.add_css_class("logo");
    
    let title = Label::new(Some("Graphical Installer"));
    title.add_css_class("title");

    let spacer = GtkBox::new(Orientation::Horizontal, 0);
    spacer.set_hexpand(true);

    header.append(&logo);
    header.append(&title);
    header.append(&spacer);

    header
}

fn create_navigation(stack: &Stack, total_steps: usize) -> GtkBox {
    let nav = GtkBox::new(Orientation::Horizontal, 12);
    nav.add_css_class("navigation");

    let back_btn = Button::with_label("Назад");
    back_btn.add_css_class("nav-button");
    back_btn.connect_clicked(clone!(@weak stack => move |_| {
        let current = stack.visible_child_name().unwrap_or_default();
        if let Some(num_str) = current.strip_prefix("step-") {
            if let Ok(num) = num_str.parse::<usize>() {
                if num > 0 {
                    stack.set_visible_child_name(&format!("step-{}", num - 1));
                }
            }
        }
    }));

    let next_btn = Button::with_label("Далее");
    next_btn.add_css_class("nav-button");
    next_btn.add_css_class("suggested-action");
    next_btn.connect_clicked(clone!(@weak stack => move |_| {
        let current = stack.visible_child_name().unwrap_or_default();
        if let Some(num_str) = current.strip_prefix("step-") {
            if let Ok(num) = num_str.parse::<usize>() {
                if num < total_steps - 1 {
                    stack.set_visible_child_name(&format!("step-{}", num + 1));
                }
            }
        }
    }));

    nav.append(&back_btn);
    nav.append(&next_btn);

    nav
}

trait Step: Send + Sync {
    fn widget(&self) -> gtk4::Widget;
    fn name(&self) -> &str;
    fn validate(&self) -> bool;
    fn apply(&self) -> Result<()>;
}