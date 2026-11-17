#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

mod assets_manager;
mod config;
mod font_manager;
mod i18n;
mod shader_manager;
mod theme_manager;

use std::path::Path;
use std::process::Command;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use std::time::Instant;

use assets_manager::{AssetTab, AssetsManager};
use config::{BgMode, HorizonTheme, LauncherConfig, ThemeMode};
use eframe::egui;
use font_manager::FontManager;
use i18n::{get_strings, Language, Strings};
use shader_manager::ShaderManager;
use theme_manager::ThemeManager;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum ActiveView {
    Home,
    Themes,
    Shaders,
    Fonts,
    Assets,
    Settings,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum SettingsTab {
    Account,
    Updates,
    Launcher,
    Network,
    Game,
    Ai,
    About,
    DataCollection,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum HomeSubTab {
    Overview,
    Themes,
    Shaders,
    Fonts,
    Assets,
    Screenshots,
}

struct ChromaLauncherApp {
    config: LauncherConfig,
    active_view: ActiveView,
    active_settings_tab: SettingsTab,
    active_home_tab: HomeSubTab,
    active_asset_tab: AssetTab,

    splash_start: Instant,
    splash_done: bool,

    game_running: Arc<AtomicBool>,
    last_error: Option<String>,

    bg_texture: Option<egui::TextureHandle>,
    bg_luma: f32,
    loaded_bg_path: Option<String>,
}

impl ChromaLauncherApp {
    fn new(_cc: &eframe::CreationContext<'_>) -> Self {
        let config = LauncherConfig::load();
        Self {
            config,
            active_view: ActiveView::Home,
            active_settings_tab: SettingsTab::Launcher,
            active_home_tab: HomeSubTab::Overview,
            active_asset_tab: AssetTab::Entities,
            splash_start: Instant::now(),
            splash_done: false,
            game_running: Arc::new(AtomicBool::new(false)),
            last_error: None,
            bg_texture: None,
            bg_luma: 0.1,
            loaded_bg_path: None,
        }
    }

    fn update_bg_texture(&mut self, ctx: &egui::Context) {
        if self.config.background_path == self.loaded_bg_path {
            return;
        }

        self.loaded_bg_path = self.config.background_path.clone();

        if let Some(ref path_str) = self.config.background_path {
            let path = Path::new(path_str);
            if path.exists() {
                if let Ok(img) = image::open(path) {
                    let rgba = img.to_rgba8();
                    let (w, h) = rgba.dimensions();

                    // Calculate average luminance for adaptive contrast
                    let mut sum_luma = 0.0f64;
                    let pixels = rgba.as_raw();
                    let pixel_count = (w * h) as usize;
                    for chunk in pixels.chunks_exact(4) {
                        let r = chunk[0] as f64 / 255.0;
                        let g = chunk[1] as f64 / 255.0;
                        let b = chunk[2] as f64 / 255.0;
                        let l = 0.299 * r + 0.587 * g + 0.114 * b;
                        sum_luma += l;
                    }
                    if pixel_count > 0 {
                        self.bg_luma = (sum_luma / pixel_count as f64) as f32;
                    }

                    let color_img = egui::ColorImage::from_rgba_unmultiplied(
                        [w as usize, h as usize],
                        pixels,
                    );
                    self.bg_texture = Some(ctx.load_texture("bg_image", color_img, egui::TextureOptions::LINEAR));
                    return;
                }
            }
        }

        self.bg_texture = None;
        self.bg_luma = 0.1;
    }

    fn launch_game(&mut self, ctx: &egui::Context) {
        if self.game_running.load(Ordering::SeqCst) {
            return;
        }

        let candidates = [
            "chroma.exe",
            "Prism.exe",
            "DDNet.exe",
            "./chroma.exe",
            "./Prism.exe",
            "../chroma.exe",
            "../Prism.exe",
            "../build/x64/Release/chroma.exe",
            "../build/x64/Release/Prism.exe",
            "../../chroma.exe",
            "../../Prism.exe",
        ];

        let mut chosen_exe: Option<String> = None;
        for c in candidates {
            if Path::new(c).exists() {
                chosen_exe = Some(c.to_string());
                break;
            }
        }

        let exe_path = match chosen_exe {
            Some(p) => p,
            None => {
                let s = get_strings(self.config.language);
                self.last_error = Some(s.launch_error.to_string());
                return;
            }
        };

        let mut cmd = Command::new(&exe_path);
        if !self.config.game_args.trim().is_empty() {
            for arg in self.config.game_args.split_whitespace() {
                cmd.arg(arg);
            }
        }

        let game_running_flag = self.game_running.clone();
        let minimize = self.config.minimize_on_launch;

        match cmd.spawn() {
            Ok(mut child) => {
                self.last_error = None;
                game_running_flag.store(true, Ordering::SeqCst);

                if minimize {
                    ctx.send_viewport_cmd(egui::ViewportCommand::Minimized(true));
                }

                std::thread::spawn(move || {
                    let _ = child.wait();
                    game_running_flag.store(false, Ordering::SeqCst);
                });
            }
            Err(e) => {
                self.last_error = Some(format!("Error: {}", e));
            }
        }
    }
}

impl eframe::App for ChromaLauncherApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        let s = get_strings(self.config.language);

        // 1. Splash Screen Display (Clean centered bold title on dark background)
        if self.config.show_splash && !self.splash_done {
            let elapsed = self.splash_start.elapsed().as_secs_f32();
            if elapsed < 1.15 {
                ctx.request_repaint();

                egui::CentralPanel::default()
                    .frame(egui::Frame::none().fill(egui::Color32::from_rgb(13, 12, 22)))
                    .show(ctx, |ui| {
                        let rect = ui.max_rect();
                        let center = rect.center();

                        let alpha = if elapsed < 0.25 {
                            (elapsed / 0.25).clamp(0.0, 1.0)
                        } else if elapsed > 0.90 {
                            (1.0 - (elapsed - 0.90) / 0.25).clamp(0.0, 1.0)
                        } else {
                            1.0
                        };

                        let text_color = egui::Color32::from_white_alpha((alpha * 255.0) as u8);
                        let splash_font = egui::FontId::proportional(54.0);

                        ui.painter().text(
                            egui::pos2(center.x, center.y),
                            egui::Align2::CENTER_CENTER,
                            "CHROMA",
                            splash_font,
                            text_color,
                        );
                    });
                return;
            } else {
                self.splash_done = true;
            }
        }

        self.update_bg_texture(ctx);

        let bright_bg = self.config.adaptive_text && self.bg_texture.is_some() && self.bg_luma > 0.52;
        let main_text_color = if bright_bg {
            egui::Color32::from_rgb(20, 20, 28)
        } else {
            egui::Color32::from_rgb(240, 242, 248)
        };
        let subtle_text_color = if bright_bg {
            egui::Color32::from_rgb(70, 72, 85)
        } else {
            egui::Color32::from_rgb(175, 178, 195)
        };

        let [ar, ag, ab] = self.config.accent_color;
        let accent_color = egui::Color32::from_rgb(ar, ag, ab);

        egui::CentralPanel::default()
            .frame(egui::Frame::none().fill(egui::Color32::from_rgb(18, 13, 20)))
            .show(ctx, |ui| {
                let rect = ui.max_rect();

                // 1. Wallpaper background
                if let Some(ref tex) = self.bg_texture {
                    let tex_size = tex.size_vec2();
                    let (uv_min, uv_max, draw_rect) = match self.config.background_mode {
                        BgMode::Fill => {
                            let scale = (rect.width() / tex_size.x).max(rect.height() / tex_size.y);
                            let scaled_size = tex_size * scale;
                            let offset = (scaled_size - rect.size()) * 0.5;
                            let u0 = (offset.x / scaled_size.x).clamp(0.0, 1.0);
                            let v0 = (offset.y / scaled_size.y).clamp(0.0, 1.0);
                            let u1 = 1.0 - u0;
                            let v1 = 1.0 - v0;
                            (egui::pos2(u0, v0), egui::pos2(u1, v1), rect)
                        }
                        BgMode::Fit => {
                            let scale = (rect.width() / tex_size.x).min(rect.height() / tex_size.y);
                            let scaled_size = tex_size * scale;
                            let draw = egui::Rect::from_center_size(rect.center(), scaled_size);
                            (egui::pos2(0.0, 0.0), egui::pos2(1.0, 1.0), draw)
                        }
                        BgMode::Stretch => {
                            (egui::pos2(0.0, 0.0), egui::pos2(1.0, 1.0), rect)
                        }
                        BgMode::Center => {
                            let draw = egui::Rect::from_center_size(rect.center(), tex_size);
                            (egui::pos2(0.0, 0.0), egui::pos2(1.0, 1.0), draw)
                        }
                    };

                    ui.painter().image(
                        tex.id(),
                        draw_rect,
                        egui::Rect::from_min_max(uv_min, uv_max),
                        egui::Color32::WHITE,
                    );
                } else {
                    // Default Horizon deep burgundy landscape tint
                    let grad_top = egui::Color32::from_rgb(28, 14, 24);
                    let grad_bot = egui::Color32::from_rgb(46, 16, 32);
                    ui.painter().rect_filled(rect, 0.0, grad_bot);
                    let top_rect = egui::Rect::from_min_size(rect.min, egui::vec2(rect.width(), rect.height() * 0.45));
                    ui.painter().rect_filled(top_rect, 0.0, grad_top);
                }

                // 2. Horizon Frosted Glass Tint
                let glass_alpha = (self.config.glass_opacity * 255.0).clamp(0.0, 255.0) as u8;
                let glass_color = egui::Color32::from_rgba_unmultiplied(16, 10, 18, glass_alpha);
                ui.painter().rect_filled(rect, 0.0, glass_color);

                // 3. Top Header Bar (Horizon Offline exact look)
                let header_h = 48.0;
                let header_rect = egui::Rect::from_min_size(rect.min, egui::vec2(rect.width(), header_h));

                ui.painter().rect_filled(
                    header_rect,
                    0.0,
                    egui::Color32::from_rgba_unmultiplied(16, 11, 20, 210),
                );
                ui.painter().line_segment(
                    [egui::pos2(header_rect.left(), header_rect.bottom()), egui::pos2(header_rect.right(), header_rect.bottom())],
                    egui::Stroke::new(1.0_f32, egui::Color32::from_rgba_unmultiplied(255, 255, 255, 14)),
                );

                ui.allocate_ui_at_rect(header_rect.shrink2(egui::vec2(16.0, 6.0)), |ui| {
                    ui.horizontal(|ui| {
                        // Window Title Left: ++ chroma. Offline
                        ui.label(egui::RichText::new("++ chroma. Offline").color(egui::Color32::from_rgb(220, 222, 235)).size(13.0).strong());
                        ui.add_space(20.0);

                        // Center Nav Tabs: Главная, Темы, Шейдеры, Шрифты, Ассеты, Настройки
                        let nav_tabs = [
                            (ActiveView::Home, s.nav_home),
                            (ActiveView::Themes, s.nav_themes),
                            (ActiveView::Shaders, s.nav_shaders),
                            (ActiveView::Fonts, s.nav_fonts),
                            (ActiveView::Assets, s.nav_assets),
                            (ActiveView::Settings, s.nav_settings),
                        ];

                        for (view, label) in nav_tabs {
                            let selected = self.active_view == view;
                            let text_col = if selected {
                                egui::Color32::WHITE
                            } else {
                                egui::Color32::from_rgb(160, 162, 178)
                            };

                            let btn_text = egui::RichText::new(label).color(text_col).size(13.0);
                            let btn = if selected {
                                egui::Button::new(btn_text)
                                    .fill(egui::Color32::from_rgba_unmultiplied(255, 255, 255, 22))
                                    .rounding(egui::Rounding::same(6.0))
                            } else {
                                egui::Button::new(btn_text).frame(false)
                            };

                            if ui.add(btn).clicked() {
                                self.active_view = view;
                            }
                            ui.add_space(6.0);
                        }

                        // Right Controls: Журнал сборки, User offline card
                        ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                            // User card
                            ui.label(egui::RichText::new("kesakuuta • Офлайн").color(egui::Color32::from_rgb(180, 182, 198)).size(12.0));
                            ui.add_space(10.0);
                            ui.label(egui::RichText::new("Журнал сборки").color(egui::Color32::from_rgb(150, 152, 170)).size(12.0));
                        });
                    });
                });

                // 4. Main Body Area below Header
                let body_rect = egui::Rect::from_min_max(
                    egui::pos2(rect.left(), header_rect.bottom()),
                    rect.max,
                );

                ui.allocate_ui_at_rect(body_rect, |ui| {
                    match self.active_view {
                        ActiveView::Home => self.render_home_view(ui, &s, main_text_color, subtle_text_color, accent_color),
                        ActiveView::Themes => self.render_themes_view(ui, &s, main_text_color, subtle_text_color, accent_color),
                        ActiveView::Shaders => self.render_shaders_view(ui, &s, main_text_color, subtle_text_color, accent_color),
                        ActiveView::Fonts => self.render_fonts_view(ui, &s, main_text_color, subtle_text_color, accent_color),
                        ActiveView::Assets => self.render_assets_view(ui, &s, main_text_color, subtle_text_color, accent_color),
                        ActiveView::Settings => self.render_settings_view(ui, &s, main_text_color, subtle_text_color, accent_color),
                    }
                });
            });
    }
}

impl ChromaLauncherApp {
    fn render_home_view(&mut self, ui: &mut egui::Ui, s: &Strings, main_color: egui::Color32, sub_color: egui::Color32, accent: egui::Color32) {
        let full_rect = ui.max_rect();
        let sidebar_w = 210.0;

        let sidebar_rect = egui::Rect::from_min_size(full_rect.min, egui::vec2(sidebar_w, full_rect.height()));
        let content_rect = egui::Rect::from_min_max(
            egui::pos2(sidebar_rect.right(), full_rect.top()),
            full_rect.max,
        );

        // --- Left Sidebar (Horizon Offline) ---
        ui.painter().rect_filled(
            sidebar_rect,
            0.0,
            egui::Color32::from_rgba_unmultiplied(18, 12, 22, 170),
        );
        ui.painter().line_segment(
            [egui::pos2(sidebar_rect.right(), sidebar_rect.top()), egui::pos2(sidebar_rect.right(), sidebar_rect.bottom())],
            egui::Stroke::new(1.0_f32, egui::Color32::from_rgba_unmultiplied(255, 255, 255, 12)),
        );

        ui.allocate_ui_at_rect(sidebar_rect.shrink(12.0), |ui| {
            ui.vertical(|ui| {
                ui.horizontal(|ui| {
                    ui.label(egui::RichText::new("Сборки").color(sub_color).size(12.0).strong());
                    ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                        let _ = ui.button("+");
                    });
                });

                ui.add_space(8.0);

                // Build Card
                let card_rect = ui.available_rect_before_wrap();
                let card_h = 52.0;
                let card_box = egui::Rect::from_min_size(card_rect.min, egui::vec2(card_rect.width(), card_h));
                ui.painter().rect_filled(card_box, 8.0, egui::Color32::from_rgba_unmultiplied(255, 255, 255, 12));

                ui.allocate_ui_at_rect(card_box.shrink(8.0), |ui| {
                    ui.vertical(|ui| {
                        ui.label(egui::RichText::new("++ chroma.").color(main_color).size(13.5).strong());
                        ui.label(egui::RichText::new("• Нативная сборка...").color(egui::Color32::from_rgb(52, 211, 153)).size(10.5));
                    });
                });
                ui.add_space(card_h + 8.0);

                // Bottom Sidebar buttons
                ui.with_layout(egui::Layout::bottom_up(egui::Align::LEFT), |ui| {
                    ui.label(egui::RichText::new("Офлайн-лаунчер 1.0.7").color(sub_color).size(10.0));
                    ui.add_space(4.0);

                    ui.horizontal(|ui| {
                        ui.label(egui::RichText::new("🌐 Сайт").color(sub_color).size(11.0));
                        ui.add_space(10.0);
                        ui.label(egui::RichText::new("♡ Поддержать").color(sub_color).size(11.0));
                    });
                    ui.add_space(6.0);

                    // Discord button
                    let discord_btn = egui::Button::new(
                        egui::RichText::new("Вступить в Discord").color(egui::Color32::WHITE).size(12.0),
                    )
                    .fill(egui::Color32::from_rgb(88, 101, 242))
                    .rounding(egui::Rounding::same(8.0));
                    let _ = ui.add_sized(egui::vec2(ui.available_width(), 32.0), discord_btn);

                    ui.add_space(8.0);
                });
            });
        });

        // --- Right Content Area ---
        ui.allocate_ui_at_rect(content_rect.shrink2(egui::vec2(28.0, 20.0)), |ui| {
            ui.vertical(|ui| {
                // Header Title ++ chroma.
                ui.label(egui::RichText::new("++ chroma.").color(main_color).size(36.0).strong());
                ui.label(
                    egui::RichText::new("Вы наиграли 1 час! Последняя игра: 8 нояб. • 1,087 сейчас в игре • 38 на официальном SMP")
                        .color(sub_color)
                        .size(12.5),
                );

                ui.add_space(18.0);

                // Action Row: [▶ Играть] [...] [Версия v] [♡] [📁]
                ui.horizontal(|ui| {
                    let running = self.game_running.load(Ordering::SeqCst);
                    let play_text = if running { s.status_running } else { s.play };

                    let play_btn = egui::Button::new(
                        egui::RichText::new(format!("▶  {}", play_text))
                            .color(egui::Color32::WHITE)
                            .size(16.0)
                            .strong(),
                    )
                    .fill(accent)
                    .rounding(egui::Rounding::same(10.0));

                    if ui.add_sized(egui::vec2(160.0, 42.0), play_btn).clicked() {
                        self.launch_game(ui.ctx());
                    }

                    ui.add_space(8.0);

                    // More button ...
                    let _ = ui.add_sized(egui::vec2(36.0, 42.0), egui::Button::new("···").rounding(egui::Rounding::same(8.0)));
                    ui.add_space(8.0);

                    // Version dropdown box
                    let ver_box = egui::Button::new(
                        egui::RichText::new("Версия: chroma. v20.3").color(main_color).size(12.5),
                    )
                    .rounding(egui::Rounding::same(8.0));
                    let _ = ui.add_sized(egui::vec2(190.0, 42.0), ver_box);

                    ui.add_space(8.0);

                    // Open folder button
                    if ui.add_sized(egui::vec2(40.0, 42.0), egui::Button::new("📁").rounding(egui::Rounding::same(8.0))).clicked() {
                        let _ = Command::new("explorer").arg(".").spawn();
                    }
                });

                if let Some(ref err) = self.last_error {
                    ui.add_space(8.0);
                    ui.label(egui::RichText::new(err).color(egui::Color32::from_rgb(239, 68, 68)).size(12.0));
                }

                ui.add_space(24.0);

                // Horizontal Tab Bar: Обзор, Темы, Шейдеры, Шрифты, Ассеты, Скриншоты
                ui.horizontal(|ui| {
                    let sub_tabs = [
                        (HomeSubTab::Overview, "Обзор"),
                        (HomeSubTab::Themes, "Темы 3"),
                        (HomeSubTab::Shaders, "Шейдеры 1"),
                        (HomeSubTab::Fonts, "Шрифты 2"),
                        (HomeSubTab::Assets, "Ассеты"),
                        (HomeSubTab::Screenshots, "Скриншоты 1"),
                    ];

                    for (tab, label) in sub_tabs {
                        let selected = self.active_home_tab == tab;
                        let col = if selected { egui::Color32::WHITE } else { sub_color };
                        let btn_text = egui::RichText::new(label).color(col).size(13.0);
                        let btn = if selected {
                            egui::Button::new(btn_text)
                                .fill(egui::Color32::from_rgba_unmultiplied(255, 255, 255, 18))
                                .rounding(egui::Rounding::same(6.0))
                        } else {
                            egui::Button::new(btn_text).frame(false)
                        };

                        if ui.add(btn).clicked() {
                            self.active_home_tab = tab;
                        }
                        ui.add_space(10.0);
                    }
                });

                ui.add_space(14.0);

                // Tab Content
                match self.active_home_tab {
                    HomeSubTab::Overview => {
                        ui.horizontal_top(|ui| {
                            // Left column: Screenshot Card
                            ui.vertical(|ui| {
                                ui.label(egui::RichText::new("Скриншоты 1").color(main_color).size(13.5).strong());
                                ui.add_space(6.0);
                                let preview_box = egui::Rect::from_min_size(ui.cursor().min, egui::vec2(320.0, 180.0));
                                ui.painter().rect_filled(preview_box, 8.0, egui::Color32::from_rgb(22, 18, 30));
                                ui.painter().text(
                                    preview_box.center(),
                                    egui::Align2::CENTER_CENTER,
                                    "chroma. Game Scene",
                                    egui::FontId::proportional(14.0),
                                    sub_color,
                                );
                                ui.add_space(186.0);
                            });

                            ui.add_space(20.0);

                            // Right column: Changelog
                            ui.vertical(|ui| {
                                ui.label(egui::RichText::new("Журнал изменений").color(main_color).size(13.5).strong());
                                ui.add_space(6.0);
                                ui.group(|ui| {
                                    ui.set_width(280.0);
                                    ui.label(egui::RichText::new("chroma. Release v1.0.8").color(main_color).strong());
                                    ui.label(egui::RichText::new("• Atmospheric Amber cinematic shaders\n• Pure background menu music\n• Start server button in main menu\n• Custom JSON themes\n• Fair-play DDNet 20.3 engine").color(sub_color).size(11.5));
                                });
                            });
                        });
                    }
                    HomeSubTab::Themes => {
                        self.render_themes_view(ui, s, main_color, sub_color, accent);
                    }
                    HomeSubTab::Shaders => {
                        self.render_shaders_view(ui, s, main_color, sub_color, accent);
                    }
                    HomeSubTab::Fonts => {
                        self.render_fonts_view(ui, s, main_color, sub_color, accent);
                    }
                    HomeSubTab::Assets => {
                        self.render_assets_view(ui, s, main_color, sub_color, accent);
                    }
                    HomeSubTab::Screenshots => {
                        ui.label(egui::RichText::new("Скриншоты сохраняются в папку screenshots/").color(sub_color));
                    }
                }
            });
        });
    }

    fn render_themes_view(&mut self, ui: &mut egui::Ui, s: &Strings, main_color: egui::Color32, sub_color: egui::Color32, accent: egui::Color32) {
        ui.label(egui::RichText::new(s.themes_title).color(main_color).size(22.0).strong());
        ui.label(egui::RichText::new(s.themes_desc).color(sub_color).size(12.5));
        ui.add_space(12.0);

        let themes = ThemeManager::scan_themes();
        egui::ScrollArea::vertical().show(ui, |ui| {
            for theme in themes {
                let is_selected = self.config.selected_theme == theme.id;
                ui.group(|ui| {
                    ui.horizontal(|ui| {
                        ui.vertical(|ui| {
                            ui.label(egui::RichText::new(&theme.name).color(main_color).size(15.0).strong());
                            ui.label(egui::RichText::new(format!("{} • {} v{}", theme.id, theme.author, theme.version)).color(sub_color).size(11.5));
                        });

                        ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                            if is_selected {
                                ui.label(egui::RichText::new(s.active).color(accent).strong());
                            } else if ui.button(s.select).clicked() {
                                self.config.selected_theme = theme.id.clone();
                                self.config.save();
                            }
                        });
                    });
                });
                ui.add_space(6.0);
            }
        });
    }

    fn render_shaders_view(&mut self, ui: &mut egui::Ui, s: &Strings, main_color: egui::Color32, sub_color: egui::Color32, accent: egui::Color32) {
        ui.label(egui::RichText::new(s.shaders_title).color(main_color).size(22.0).strong());
        ui.label(egui::RichText::new(s.shaders_desc).color(sub_color).size(12.5));
        ui.add_space(12.0);

        let shaders = ShaderManager::scan_shaders();
        egui::ScrollArea::vertical().show(ui, |ui| {
            for shader in shaders {
                let is_selected = self.config.selected_shader == shader.id;
                ui.group(|ui| {
                    ui.horizontal(|ui| {
                        ui.vertical(|ui| {
                            ui.label(egui::RichText::new(&shader.name).color(main_color).size(15.0).strong());
                            ui.label(egui::RichText::new(&shader.description).color(sub_color).size(12.0));
                            ui.label(egui::RichText::new(format!("{} • {} v{}", shader.id, shader.author, shader.version)).color(sub_color).size(11.0));
                        });

                        ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                            if is_selected {
                                ui.label(egui::RichText::new(s.active).color(accent).strong());
                            } else if ui.button(s.select).clicked() {
                                self.config.selected_shader = shader.id.clone();
                                self.config.save();
                            }
                        });
                    });
                });
                ui.add_space(6.0);
            }
        });
    }

    fn render_fonts_view(&mut self, ui: &mut egui::Ui, s: &Strings, main_color: egui::Color32, sub_color: egui::Color32, accent: egui::Color32) {
        ui.label(egui::RichText::new(s.fonts_title).color(main_color).size(22.0).strong());
        ui.label(egui::RichText::new(s.fonts_desc).color(sub_color).size(12.5));
        ui.add_space(12.0);

        let fonts = FontManager::scan_fonts();
        egui::ScrollArea::vertical().show(ui, |ui| {
            for font in fonts {
                let is_selected = self.config.selected_font == font.name;
                ui.group(|ui| {
                    ui.horizontal(|ui| {
                        ui.vertical(|ui| {
                            ui.label(egui::RichText::new(&font.name).color(main_color).size(14.5).strong());
                            ui.label(egui::RichText::new(format!("{} KB • {}", font.size_kb, font.path.display())).color(sub_color).size(11.0));
                        });

                        ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                            if is_selected {
                                ui.label(egui::RichText::new(s.active).color(accent).strong());
                            } else if ui.button(s.select).clicked() {
                                self.config.selected_font = font.name.clone();
                                self.config.save();
                            }
                        });
                    });
                });
                ui.add_space(6.0);
            }
        });
    }

    fn render_assets_view(&mut self, ui: &mut egui::Ui, s: &Strings, main_color: egui::Color32, sub_color: egui::Color32, accent: egui::Color32) {
        ui.label(egui::RichText::new(s.nav_assets).color(main_color).size(22.0).strong());
        ui.add_space(10.0);

        let tabs = [
            (AssetTab::Entities, s.assets_entities),
            (AssetTab::Game, s.assets_game),
            (AssetTab::Emoticons, s.assets_emoticons),
            (AssetTab::Particles, s.assets_particles),
            (AssetTab::Hud, s.assets_hud),
            (AssetTab::Extras, s.assets_extras),
        ];

        ui.horizontal(|ui| {
            for (tab, label) in tabs {
                let selected = self.active_asset_tab == tab;
                let col = if selected { accent } else { sub_color };
                if ui.button(egui::RichText::new(label).color(col).strong()).clicked() {
                    self.active_asset_tab = tab;
                }
            }
        });

        ui.add_space(10.0);

        let items = AssetsManager::scan_tab(self.active_asset_tab);
        egui::ScrollArea::vertical().show(ui, |ui| {
            if items.is_empty() {
                ui.label(egui::RichText::new("Нет кастомных оверрайдов ассетов в этой категории.").color(sub_color));
            } else {
                for item in items {
                    ui.group(|ui| {
                        ui.horizontal(|ui| {
                            ui.label(egui::RichText::new(&item.name).color(main_color).strong());
                            ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                                ui.label(egui::RichText::new(format!("{} KB", item.size_bytes / 1024)).color(sub_color).size(11.0));
                            });
                        });
                    });
                    ui.add_space(4.0);
                }
            }
        });
    }

    fn render_settings_view(&mut self, ui: &mut egui::Ui, s: &Strings, main_color: egui::Color32, sub_color: egui::Color32, _accent: egui::Color32) {
        let full_rect = ui.max_rect();
        let sidebar_w = 170.0;

        let sidebar_rect = egui::Rect::from_min_size(full_rect.min, egui::vec2(sidebar_w, full_rect.height()));
        let content_rect = egui::Rect::from_min_max(
            egui::pos2(sidebar_rect.right(), full_rect.top()),
            full_rect.max,
        );

        // --- Settings Left Sidebar (Exact Horizon Offline list) ---
        ui.painter().rect_filled(
            sidebar_rect,
            0.0,
            egui::Color32::from_rgba_unmultiplied(16, 11, 20, 180),
        );
        ui.painter().line_segment(
            [egui::pos2(sidebar_rect.right(), sidebar_rect.top()), egui::pos2(sidebar_rect.right(), sidebar_rect.bottom())],
            egui::Stroke::new(1.0_f32, egui::Color32::from_rgba_unmultiplied(255, 255, 255, 12)),
        );

        ui.allocate_ui_at_rect(sidebar_rect.shrink(12.0), |ui| {
            ui.vertical(|ui| {
                let tabs = [
                    (SettingsTab::Account, "👤 Аккаунт"),
                    (SettingsTab::Updates, "🔄 Обновления"),
                    (SettingsTab::Launcher, "🖥️ Лаунчер"),
                    (SettingsTab::Network, "🌐 Сеть"),
                    (SettingsTab::Game, "🎮 Игра"),
                    (SettingsTab::Ai, "🤖 ИИ"),
                    (SettingsTab::About, "ℹ️ О программе"),
                    (SettingsTab::DataCollection, "📊 Data collection"),
                ];

                for (tab, label) in tabs {
                    let selected = self.active_settings_tab == tab;
                    let text_col = if selected { egui::Color32::WHITE } else { sub_color };
                    let btn = if selected {
                        egui::Button::new(egui::RichText::new(label).color(text_col).size(12.5).strong())
                            .fill(egui::Color32::from_rgba_unmultiplied(255, 255, 255, 18))
                            .rounding(egui::Rounding::same(6.0))
                    } else {
                        egui::Button::new(egui::RichText::new(label).color(text_col).size(12.5)).frame(false)
                    };

                    if ui.add(btn).clicked() {
                        self.active_settings_tab = tab;
                    }
                    ui.add_space(4.0);
                }
            });
        });

        // --- Settings Right Panel ---
        ui.allocate_ui_at_rect(content_rect.shrink2(egui::vec2(28.0, 16.0)), |ui| {
            ui.vertical(|ui| {
                ui.label(egui::RichText::new(s.settings_title).color(main_color).size(26.0).strong());
                ui.label(egui::RichText::new(s.settings_subtitle).color(sub_color).size(12.0));
                ui.add_space(16.0);

                egui::ScrollArea::vertical().show(ui, |ui| {
                    match self.active_settings_tab {
                        SettingsTab::Launcher => {
                            // 1. Оформление (Темное, Светлое, Системное)
                            ui.horizontal(|ui| {
                                ui.vertical(|ui| {
                                    ui.label(egui::RichText::new(s.settings_appearance).color(main_color).strong());
                                    ui.label(egui::RichText::new(s.settings_appearance_desc).color(sub_color).size(11.0));
                                });
                                ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                                    let mut mode = self.config.theme_mode;
                                    if ui.selectable_value(&mut mode, ThemeMode::System, s.theme_system).clicked() {
                                        self.config.theme_mode = mode;
                                        self.config.save();
                                    }
                                    if ui.selectable_value(&mut mode, ThemeMode::Light, s.theme_light).clicked() {
                                        self.config.theme_mode = mode;
                                        self.config.save();
                                    }
                                    if ui.selectable_value(&mut mode, ThemeMode::Dark, s.theme_dark).clicked() {
                                        self.config.theme_mode = mode;
                                        self.config.save();
                                    }
                                });
                            });
                            ui.add_space(16.0);

                            // 2. Тема лаунчера (chroma. / Первый свет)
                            ui.horizontal(|ui| {
                                ui.vertical(|ui| {
                                    ui.label(egui::RichText::new(s.settings_launcher_theme).color(main_color).strong());
                                    ui.label(egui::RichText::new(s.settings_launcher_theme_desc).color(sub_color).size(11.0));
                                });
                                ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                                    let mut ht = self.config.horizon_theme;
                                    if ui.selectable_value(&mut ht, HorizonTheme::FirstLight, s.theme_first_light).clicked() {
                                        self.config.horizon_theme = ht;
                                        self.config.accent_color = [37, 99, 235];
                                        self.config.save();
                                    }
                                    if ui.selectable_value(&mut ht, HorizonTheme::Chroma, s.theme_chroma).clicked() {
                                        self.config.horizon_theme = ht;
                                        self.config.accent_color = [190, 16, 93];
                                        self.config.save();
                                    }
                                });
                            });
                            ui.add_space(16.0);

                            // 3. Цвет акцента (Palette dots + Hex input)
                            ui.horizontal(|ui| {
                                ui.vertical(|ui| {
                                    ui.label(egui::RichText::new(s.settings_accent_color).color(main_color).strong());
                                    ui.label(egui::RichText::new(s.settings_accent_desc).color(sub_color).size(11.0));
                                });
                                ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                                    let presets: [[u8; 3]; 8] = [
                                        [190, 16, 93],  // #be105d Crimson
                                        [239, 68, 68],  // Red
                                        [249, 115, 22], // Orange
                                        [34, 197, 94],  // Green
                                        [13, 148, 136], // Teal
                                        [2, 132, 199],  // Sky
                                        [37, 99, 235],  // Blue
                                        [124, 58, 237], // Purple
                                    ];

                                    for rgb in presets {
                                        let col = egui::Color32::from_rgb(rgb[0], rgb[1], rgb[2]);
                                        let active = self.config.accent_color == rgb;
                                        let dot_size = if active { egui::vec2(22.0, 22.0) } else { egui::vec2(16.0, 16.0) };
                                        let btn = egui::Button::new("")
                                            .fill(col)
                                            .rounding(egui::Rounding::same(11.0));
                                        if ui.add_sized(dot_size, btn).clicked() {
                                            self.config.accent_color = rgb;
                                            self.config.save();
                                        }
                                        ui.add_space(3.0);
                                    }
                                });
                            });
                            ui.add_space(16.0);

                            // 4. Glass Slider
                            ui.horizontal(|ui| {
                                ui.vertical(|ui| {
                                    ui.label(egui::RichText::new(s.settings_glass).color(main_color).strong());
                                    ui.label(egui::RichText::new(s.settings_glass_desc).color(sub_color).size(11.0));
                                });
                                ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                                    let mut val = self.config.glass_opacity;
                                    if ui.add(egui::Slider::new(&mut val, 0.05..=1.0).show_value(true)).changed() {
                                        self.config.glass_opacity = val;
                                        self.config.save();
                                    }
                                });
                            });
                            ui.add_space(16.0);

                            // 5. Blur Slider
                            ui.horizontal(|ui| {
                                ui.vertical(|ui| {
                                    ui.label(egui::RichText::new(s.settings_blur).color(main_color).strong());
                                    ui.label(egui::RichText::new(s.settings_blur_desc).color(sub_color).size(11.0));
                                });
                                ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                                    let mut val = self.config.blur_radius;
                                    if ui.add(egui::Slider::new(&mut val, 0.0..=10.0).show_value(true)).changed() {
                                        self.config.blur_radius = val;
                                        self.config.save();
                                    }
                                });
                            });
                            ui.add_space(16.0);

                            // 6. Performance mode Toggle
                            ui.horizontal(|ui| {
                                ui.vertical(|ui| {
                                    ui.label(egui::RichText::new(s.settings_perf).color(main_color).strong());
                                    ui.label(egui::RichText::new(s.settings_perf_desc).color(sub_color).size(11.0));
                                });
                                ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                                    let mut perf = self.config.performance_mode;
                                    if ui.checkbox(&mut perf, "").changed() {
                                        self.config.performance_mode = perf;
                                        self.config.save();
                                    }
                                });
                            });
                            ui.add_space(16.0);

                            // 7. Фон главной (Выбрать картинку... & Режимы Fill, Fit, Stretch, Center)
                            ui.horizontal(|ui| {
                                ui.vertical(|ui| {
                                    ui.label(egui::RichText::new(s.settings_bg).color(main_color).strong());
                                    ui.label(egui::RichText::new(s.settings_bg_desc).color(sub_color).size(11.0));
                                });
                                ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                                    if ui.button(s.settings_bg_btn).clicked() {
                                        if let Some(file) = rfd::FileDialog::new()
                                            .add_filter("Images", &["png", "jpg", "jpeg", "webp"])
                                            .pick_file()
                                        {
                                            self.config.background_path = Some(file.display().to_string());
                                            self.config.save();
                                        }
                                    }
                                    if self.config.background_path.is_some() {
                                        if ui.button(s.settings_bg_reset).clicked() {
                                            self.config.background_path = None;
                                            self.config.save();
                                        }
                                    }
                                });
                            });
                            ui.horizontal(|ui| {
                                ui.label(egui::RichText::new(s.settings_bg_mode).color(sub_color).size(11.5));
                                let mut mode = self.config.background_mode;
                                if ui.selectable_value(&mut mode, BgMode::Fill, s.mode_fill).clicked() {
                                    self.config.background_mode = mode;
                                    self.config.save();
                                }
                                if ui.selectable_value(&mut mode, BgMode::Fit, s.mode_fit).clicked() {
                                    self.config.background_mode = mode;
                                    self.config.save();
                                }
                                if ui.selectable_value(&mut mode, BgMode::Stretch, s.mode_stretch).clicked() {
                                    self.config.background_mode = mode;
                                    self.config.save();
                                }
                                if ui.selectable_value(&mut mode, BgMode::Center, s.mode_center).clicked() {
                                    self.config.background_mode = mode;
                                    self.config.save();
                                }
                            });
                            ui.add_space(16.0);

                            // 8. Язык (Dropdown: Русский / English)
                            ui.horizontal(|ui| {
                                ui.label(egui::RichText::new(s.settings_language).color(main_color).strong());
                                ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                                    let mut lang = self.config.language;
                                    egui::ComboBox::from_id_source("lang_cb_horizon")
                                        .selected_text(match lang {
                                            Language::Russian => "Русский",
                                            Language::English => "English",
                                        })
                                        .show_ui(ui, |ui| {
                                            if ui.selectable_value(&mut lang, Language::Russian, "Русский").clicked() {
                                                self.config.language = Language::Russian;
                                                self.config.save();
                                            }
                                            if ui.selectable_value(&mut lang, Language::English, "English").clicked() {
                                                self.config.language = Language::English;
                                                self.config.save();
                                            }
                                        });
                                });
                            });
                            ui.add_space(16.0);

                            // 9. Во время игры (Оставаться / Свернуть)
                            ui.horizontal(|ui| {
                                ui.vertical(|ui| {
                                    ui.label(egui::RichText::new(s.settings_during_game).color(main_color).strong());
                                    ui.label(egui::RichText::new(s.settings_during_desc).color(sub_color).size(11.0));
                                });
                                ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                                    let mut minimize = self.config.minimize_on_launch;
                                    if ui.selectable_value(&mut minimize, true, s.during_minimize).clicked() {
                                        self.config.minimize_on_launch = true;
                                        self.config.save();
                                    }
                                    if ui.selectable_value(&mut minimize, false, s.during_stay).clicked() {
                                        self.config.minimize_on_launch = false;
                                        self.config.save();
                                    }
                                });
                            });
                            ui.add_space(16.0);

                            // 10. Показывать заставку (Toggle)
                            ui.horizontal(|ui| {
                                ui.vertical(|ui| {
                                    ui.label(egui::RichText::new(s.settings_show_splash).color(main_color).strong());
                                    ui.label(egui::RichText::new(s.settings_show_splash_desc).color(sub_color).size(11.0));
                                });
                                ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                                    let mut splash = self.config.show_splash;
                                    if ui.checkbox(&mut splash, "").changed() {
                                        self.config.show_splash = splash;
                                        self.config.save();
                                    }
                                });
                            });
                            ui.add_space(16.0);

                            // 11. Сбросить вид лаунчера
                            ui.horizontal(|ui| {
                                ui.vertical(|ui| {
                                    ui.label(egui::RichText::new(s.settings_reset_view).color(main_color).strong());
                                    ui.label(egui::RichText::new(s.settings_reset_desc).color(sub_color).size(11.0));
                                });
                                ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                                    if ui.button(s.settings_reset_btn).clicked() {
                                        self.config = LauncherConfig::default();
                                        self.config.save();
                                    }
                                });
                            });
                            ui.add_space(20.0);
                        }
                        SettingsTab::Account => {
                            ui.label(egui::RichText::new("Вход в оффлайн-аккаунт kesakuuta активен.").color(main_color));
                        }
                        SettingsTab::Updates => {
                            ui.label(egui::RichText::new("Установлена актуальная версия chroma. Launcher 1.0.7.").color(main_color));
                        }
                        SettingsTab::Network => {
                            ui.label(egui::RichText::new("Прямое подключение к мастер-серверам DDRaceNetwork.").color(main_color));
                        }
                        SettingsTab::Game => {
                            ui.label(egui::RichText::new("Аргументы командной строки клиента:").color(main_color));
                            ui.add_space(6.0);
                            if ui.text_edit_singleline(&mut self.config.game_args).lost_focus() {
                                self.config.save();
                            }
                        }
                        SettingsTab::Ai => {
                            ui.label(egui::RichText::new("Интеграция искусственного интеллекта не настроена.").color(sub_color));
                        }
                        SettingsTab::About => {
                            ui.label(egui::RichText::new("chroma. Launcher v1.0.7\n100% Fair-Play DDRaceNetwork 20.3 Client").color(main_color));
                        }
                        SettingsTab::DataCollection => {
                            ui.label(egui::RichText::new("Сбор персональных данных отключен.").color(sub_color));
                        }
                    }
                });
            });
        });
    }
}

fn main() -> eframe::Result<()> {
    let native_options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default()
            .with_title("++ chroma. Offline")
            .with_inner_size([1040.0, 660.0])
            .with_min_inner_size([880.0, 560.0]),
        ..Default::default()
    };

    eframe::run_native(
        "chroma. Launcher",
        native_options,
        Box::new(|cc| Ok(Box::new(ChromaLauncherApp::new(cc)))),
    )
}
