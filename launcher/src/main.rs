#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

mod assets_manager;
mod config;
mod font_manager;
mod i18n;
mod music_manager;
mod shader_manager;
mod theme_manager;

use std::path::Path;
use std::process::Command;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use std::time::Instant;

use assets_manager::{AssetTab, AssetsManager};
use config::{BgMode, LauncherConfig};
use eframe::egui;
use font_manager::FontManager;
use i18n::{get_strings, Language, Strings};
use music_manager::MusicManager;
use shader_manager::ShaderManager;
use theme_manager::ThemeManager;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum ActiveView {
    Home,
    Themes,
    Shaders,
    Fonts,
    Music,
    Assets,
    Settings,
}

struct ChromaLauncherApp {
    config: LauncherConfig,
    active_view: ActiveView,
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

                    // Compute luminance for adaptive text contrast
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
                        &pixels,
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

        let mut path_str = self.config.game_path.clone();
        if !Path::new(&path_str).exists() {
            // Try fallback
            let fallbacks = ["Prism.exe", "chroma.exe", "DDNet.exe", "../Prism.exe", "../chroma.exe"];
            for fb in fallbacks {
                if Path::new(fb).exists() {
                    path_str = fb.to_string();
                    self.config.game_path = fb.to_string();
                    self.config.save();
                    break;
                }
            }
        }

        if !Path::new(&path_str).exists() {
            let s = get_strings(self.config.language);
            self.last_error = Some(s.launch_error.to_string());
            return;
        }

        let mut cmd = Command::new(&path_str);
        if !self.config.game_args.trim().is_empty() {
            for arg in self.config.game_args.split_whitespace() {
                cmd.arg(arg);
            }
        }

        if self.config.fullscreen {
            cmd.arg("gfx_fullscreen 1");
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

        // 1. Splash Screen Display (like Horizon startup screen)
        if self.config.show_splash && !self.splash_done {
            let elapsed = self.splash_start.elapsed().as_secs_f32();
            if elapsed < 1.35 {
                ctx.request_repaint();

                egui::CentralPanel::default()
                    .frame(egui::Frame::none().fill(egui::Color32::from_rgb(13, 12, 24)))
                    .show(ctx, |ui| {
                        let rect = ui.max_rect();
                        let center = rect.center();

                        // Centered bold chroma. text
                        let splash_font = egui::FontId::proportional(46.0);
                        let sub_font = egui::FontId::proportional(14.0);

                        let progress = (elapsed / 1.35).clamp(0.0, 1.0);
                        let alpha = if elapsed < 0.3 {
                            (elapsed / 0.3).clamp(0.0, 1.0)
                        } else if elapsed > 1.05 {
                            (1.0 - (elapsed - 1.05) / 0.3).clamp(0.0, 1.0)
                        } else {
                            1.0
                        };

                        let text_color = egui::Color32::from_white_alpha((alpha * 255.0) as u8);
                        let sub_color = egui::Color32::from_rgba_unmultiplied(180, 180, 200, (alpha * 220.0) as u8);

                        ui.painter().text(
                            egui::pos2(center.x, center.y - 18.0),
                            egui::Align2::CENTER_CENTER,
                            "chroma.",
                            splash_font,
                            text_color,
                        );

                        ui.painter().text(
                            egui::pos2(center.x, center.y + 24.0),
                            egui::Align2::CENTER_CENTER,
                            "DDRaceNetwork 20.3 Client • 100% Legal",
                            sub_font,
                            sub_color,
                        );

                        // Subtle bottom progress bar
                        let bar_w = 180.0;
                        let bar_rect = egui::Rect::from_center_size(
                            egui::pos2(center.x, rect.bottom() - 36.0),
                            egui::vec2(bar_w, 2.5),
                        );
                        ui.painter().rect_filled(bar_rect, 1.0, egui::Color32::from_rgba_unmultiplied(255, 255, 255, 40));
                        let mut fill_rect = bar_rect;
                        fill_rect.set_width(bar_w * progress);
                        let [r, g, b] = self.config.accent_color;
                        ui.painter().rect_filled(fill_rect, 1.0, egui::Color32::from_rgb(r, g, b));
                    });
                return;
            } else {
                self.splash_done = true;
            }
        }

        self.update_bg_texture(ctx);

        // Adaptive text luminance
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

        // Render main view
        egui::CentralPanel::default()
            .frame(egui::Frame::none().fill(egui::Color32::from_rgb(16, 15, 24)))
            .show(ctx, |ui| {
                let rect = ui.max_rect();

                // 1. Draw Background Image if present
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
                }

                // 2. Glass tint & Vignette overlay
                let glass_alpha = (self.config.glass_opacity * 255.0).clamp(0.0, 255.0) as u8;
                let glass_color = egui::Color32::from_rgba_unmultiplied(12, 10, 18, glass_alpha);
                ui.painter().rect_filled(rect, 0.0, glass_color);

                // 3. Header Navigation Bar (Horizon style)
                let header_rect = egui::Rect::from_min_size(
                    rect.min,
                    egui::vec2(rect.width(), 56.0),
                );

                ui.painter().rect_filled(
                    header_rect,
                    0.0,
                    egui::Color32::from_rgba_unmultiplied(18, 16, 26, 210),
                );
                ui.painter().line_segment(
                    [egui::pos2(header_rect.left(), header_rect.bottom()), egui::pos2(header_rect.right(), header_rect.bottom())],
                    egui::Stroke::new(1.0_f32, egui::Color32::from_rgba_unmultiplied(255, 255, 255, 18)),
                );

                ui.allocate_ui_at_rect(header_rect.shrink(10.0), |ui| {
                    ui.horizontal(|ui| {
                        // Brand Logo
                        ui.label(egui::RichText::new("chroma.").color(accent_color).size(20.0).strong());
                        ui.add_space(24.0);

                        // Top Nav buttons: Темы, Шейдеры, Шрифты, Музыка, Ассеты
                        let nav_tabs = [
                            (ActiveView::Home, s.nav_home),
                            (ActiveView::Themes, s.nav_themes),
                            (ActiveView::Shaders, s.nav_shaders),
                            (ActiveView::Fonts, s.nav_fonts),
                            (ActiveView::Music, s.nav_music),
                            (ActiveView::Assets, s.nav_assets),
                        ];

                        for (view, label) in nav_tabs {
                            let selected = self.active_view == view;
                            let btn_color = if selected { accent_color } else { subtle_text_color };
                            let btn_text = egui::RichText::new(label).color(btn_color).size(14.0);
                            let btn = egui::Button::new(btn_text).frame(false);
                            if ui.add(btn).clicked() {
                                self.active_view = view;
                            }
                            ui.add_space(14.0);
                        }

                        // Right-aligned settings & status
                        ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                            let settings_selected = self.active_view == ActiveView::Settings;
                            let s_color = if settings_selected { accent_color } else { subtle_text_color };
                            let settings_btn = egui::Button::new(egui::RichText::new(format!("⚙ {}", s.nav_settings)).color(s_color).size(14.0)).frame(false);
                            if ui.add(settings_btn).clicked() {
                                self.active_view = ActiveView::Settings;
                            }

                            ui.add_space(16.0);

                            // Running / Ready badge
                            let running = self.game_running.load(Ordering::SeqCst);
                            let badge_text = if running { s.status_running } else { s.status_ready };
                            let badge_color = if running { egui::Color32::from_rgb(52, 211, 153) } else { subtle_text_color };
                            ui.label(egui::RichText::new(badge_text).color(badge_color).size(12.0));
                        });
                    });
                });

                // 4. Main Body Content Area
                let body_rect = egui::Rect::from_min_max(
                    egui::pos2(rect.left(), header_rect.bottom()),
                    rect.max,
                );

                ui.allocate_ui_at_rect(body_rect.shrink2(egui::vec2(28.0, 20.0)), |ui| {
                    match self.active_view {
                        ActiveView::Home => self.render_home_view(ui, &s, main_text_color, subtle_text_color, accent_color),
                        ActiveView::Themes => self.render_themes_view(ui, &s, main_text_color, subtle_text_color, accent_color),
                        ActiveView::Shaders => self.render_shaders_view(ui, &s, main_text_color, subtle_text_color, accent_color),
                        ActiveView::Fonts => self.render_fonts_view(ui, &s, main_text_color, subtle_text_color, accent_color),
                        ActiveView::Music => self.render_music_view(ui, &s, main_text_color, subtle_text_color, accent_color),
                        ActiveView::Assets => self.render_assets_view(ui, &s, main_text_color, subtle_text_color, accent_color),
                        ActiveView::Settings => self.render_settings_view(ui, &s, main_text_color, subtle_text_color, accent_color),
                    }
                });
            });
    }
}

impl ChromaLauncherApp {
    fn render_home_view(&mut self, ui: &mut egui::Ui, s: &Strings, main_color: egui::Color32, sub_color: egui::Color32, accent: egui::Color32) {
        ui.add_space(16.0);

        // Hero title & subtitle
        ui.label(egui::RichText::new("chroma.").color(main_color).size(34.0).strong());
        ui.label(egui::RichText::new(s.play_subtitle).color(sub_color).size(14.0));

        ui.add_space(32.0);

        // Horizon-style "Играть" (Play) button
        let running = self.game_running.load(Ordering::SeqCst);
        let play_btn_text = if running { s.status_running } else { s.play };

        let btn_size = egui::vec2(210.0, 52.0);
        let btn = egui::Button::new(
            egui::RichText::new(format!("▶ {}", play_btn_text))
                .color(egui::Color32::WHITE)
                .size(19.0)
                .strong(),
        )
        .fill(accent)
        .rounding(egui::Rounding::same(12.0));

        if ui.add_sized(btn_size, btn).clicked() {
            self.launch_game(ui.ctx());
        }

        if let Some(ref err) = self.last_error {
            ui.add_space(12.0);
            ui.label(egui::RichText::new(err).color(egui::Color32::from_rgb(239, 68, 68)).size(13.0));
        }

        ui.add_space(38.0);

        // Summary cards
        ui.horizontal(|ui| {
            // Theme Card
            ui.group(|ui| {
                ui.set_width(240.0);
                ui.label(egui::RichText::new(s.nav_themes).color(accent).size(13.0).strong());
                ui.label(egui::RichText::new(&self.config.selected_theme).color(main_color).size(15.0));
            });

            ui.add_space(14.0);

            // Shader Card
            ui.group(|ui| {
                ui.set_width(240.0);
                ui.label(egui::RichText::new(s.nav_shaders).color(accent).size(13.0).strong());
                ui.label(egui::RichText::new(&self.config.selected_shader).color(main_color).size(15.0));
            });

            ui.add_space(14.0);

            // Music Card
            ui.group(|ui| {
                ui.set_width(240.0);
                ui.label(egui::RichText::new(s.nav_music).color(accent).size(13.0).strong());
                ui.label(egui::RichText::new(&self.config.selected_track).color(main_color).size(15.0));
            });
        });
    }

    fn render_themes_view(&mut self, ui: &mut egui::Ui, s: &Strings, main_color: egui::Color32, sub_color: egui::Color32, accent: egui::Color32) {
        ui.label(egui::RichText::new(s.themes_title).color(main_color).size(24.0).strong());
        ui.label(egui::RichText::new(s.themes_desc).color(sub_color).size(13.0));
        ui.add_space(16.0);

        let themes = ThemeManager::scan_themes();
        egui::ScrollArea::vertical().show(ui, |ui| {
            for theme in themes {
                let is_selected = self.config.selected_theme == theme.id;
                ui.group(|ui| {
                    ui.horizontal(|ui| {
                        ui.vertical(|ui| {
                            ui.label(egui::RichText::new(&theme.name).color(main_color).size(16.0).strong());
                            ui.label(egui::RichText::new(format!("{} • {} v{}", theme.id, theme.author, theme.version)).color(sub_color).size(12.0));
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
                ui.add_space(8.0);
            }
        });
    }

    fn render_shaders_view(&mut self, ui: &mut egui::Ui, s: &Strings, main_color: egui::Color32, sub_color: egui::Color32, accent: egui::Color32) {
        ui.label(egui::RichText::new(s.shaders_title).color(main_color).size(24.0).strong());
        ui.label(egui::RichText::new(s.shaders_desc).color(sub_color).size(13.0));
        ui.add_space(16.0);

        let shaders = ShaderManager::scan_shaders();
        egui::ScrollArea::vertical().show(ui, |ui| {
            for shader in shaders {
                let is_selected = self.config.selected_shader == shader.id;
                ui.group(|ui| {
                    ui.horizontal(|ui| {
                        ui.vertical(|ui| {
                            ui.label(egui::RichText::new(&shader.name).color(main_color).size(16.0).strong());
                            ui.label(egui::RichText::new(&shader.description).color(sub_color).size(12.5));
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
                ui.add_space(8.0);
            }
        });
    }

    fn render_fonts_view(&mut self, ui: &mut egui::Ui, s: &Strings, main_color: egui::Color32, sub_color: egui::Color32, accent: egui::Color32) {
        ui.label(egui::RichText::new(s.fonts_title).color(main_color).size(24.0).strong());
        ui.label(egui::RichText::new(s.fonts_desc).color(sub_color).size(13.0));
        ui.add_space(16.0);

        let fonts = FontManager::scan_fonts();
        egui::ScrollArea::vertical().show(ui, |ui| {
            for font in fonts {
                let is_selected = self.config.selected_font == font.name;
                ui.group(|ui| {
                    ui.horizontal(|ui| {
                        ui.vertical(|ui| {
                            ui.label(egui::RichText::new(&font.name).color(main_color).size(15.0).strong());
                            ui.label(egui::RichText::new(format!("{} KB • {}", font.size_kb, font.path.display())).color(sub_color).size(11.5));
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
                ui.add_space(8.0);
            }
        });
    }

    fn render_music_view(&mut self, ui: &mut egui::Ui, s: &Strings, main_color: egui::Color32, sub_color: egui::Color32, accent: egui::Color32) {
        ui.label(egui::RichText::new(s.music_title).color(main_color).size(24.0).strong());
        ui.label(egui::RichText::new(s.music_desc).color(sub_color).size(13.0));
        ui.add_space(16.0);

        let tracks = MusicManager::scan_music();
        egui::ScrollArea::vertical().show(ui, |ui| {
            for track in tracks {
                let is_selected = self.config.selected_track == track.title;
                ui.group(|ui| {
                    ui.horizontal(|ui| {
                        ui.vertical(|ui| {
                            ui.label(egui::RichText::new(&track.title).color(main_color).size(15.0).strong());
                            ui.label(egui::RichText::new(format!("{} • .{}", track.artist, track.format)).color(sub_color).size(12.0));
                        });

                        ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                            if is_selected {
                                ui.label(egui::RichText::new(s.active).color(accent).strong());
                            } else if ui.button(s.select).clicked() {
                                self.config.selected_track = track.title.clone();
                                self.config.save();
                            }
                        });
                    });
                });
                ui.add_space(8.0);
            }
        });
    }

    fn render_assets_view(&mut self, ui: &mut egui::Ui, s: &Strings, main_color: egui::Color32, sub_color: egui::Color32, accent: egui::Color32) {
        ui.label(egui::RichText::new(s.nav_assets).color(main_color).size(24.0).strong());
        ui.add_space(12.0);

        // Assets sub-tabs matching DDNet assets settings
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

        ui.add_space(14.0);

        // Open folder button
        let tab_dir = AssetsManager::get_tab_dir(self.active_asset_tab);
        ui.horizontal(|ui| {
            if ui.button(format!("📁 {}", s.open_folder)).clicked() {
                let _ = Command::new("explorer").arg(&tab_dir).spawn();
            }
            ui.label(egui::RichText::new(tab_dir.display().to_string()).color(sub_color).size(12.0));
        });

        ui.add_space(12.0);

        let items = AssetsManager::scan_tab(self.active_asset_tab);
        egui::ScrollArea::vertical().show(ui, |ui| {
            if items.is_empty() {
                ui.label(egui::RichText::new("No custom asset overrides found in this directory.").color(sub_color));
            } else {
                for item in items {
                    ui.group(|ui| {
                        ui.horizontal(|ui| {
                            ui.label(egui::RichText::new(&item.name).color(main_color).strong());
                            ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                                ui.label(egui::RichText::new(format!("{} KB", item.size_bytes / 1024)).color(sub_color).size(11.5));
                            });
                        });
                    });
                    ui.add_space(6.0);
                }
            }
        });
    }

    fn render_settings_view(&mut self, ui: &mut egui::Ui, s: &Strings, main_color: egui::Color32, sub_color: egui::Color32, _accent: egui::Color32) {
        ui.label(egui::RichText::new(s.settings_title).color(main_color).size(24.0).strong());
        ui.label(egui::RichText::new(s.settings_subtitle).color(sub_color).size(13.0));
        ui.add_space(20.0);

        egui::ScrollArea::vertical().show(ui, |ui| {
            // 1. Language selector - EXPLICITLY THE VERY TOP SETTING
            ui.group(|ui| {
                ui.horizontal(|ui| {
                    ui.label(egui::RichText::new(s.settings_language).color(main_color).strong());
                    ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                        let mut lang = self.config.language;
                        egui::ComboBox::from_id_source("lang_cb")
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
            });

            ui.add_space(12.0);

            // 2. Custom Background Image & Modes
            ui.group(|ui| {
                ui.vertical(|ui| {
                    ui.label(egui::RichText::new(s.settings_bg).color(main_color).strong());
                    ui.add_space(4.0);

                    ui.horizontal(|ui| {
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

                        if let Some(ref p) = self.config.background_path {
                            ui.label(egui::RichText::new(p).color(sub_color).size(11.5));
                        }
                    });

                    ui.add_space(8.0);

                    // Mode: Fill, Fit, Stretch, Center
                    ui.horizontal(|ui| {
                        ui.label(egui::RichText::new(s.settings_bg_mode).color(sub_color));
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

                    ui.add_space(8.0);

                    // Dynamic text contrast adaptation toggle
                    let mut adapt = self.config.adaptive_text;
                    if ui.checkbox(&mut adapt, s.settings_adaptive_text).changed() {
                        self.config.adaptive_text = adapt;
                        self.config.save();
                    }
                });
            });

            ui.add_space(12.0);

            // 3. Accent Color Presets (Horizon palette)
            ui.group(|ui| {
                ui.horizontal(|ui| {
                    ui.label(egui::RichText::new(s.settings_accent_color).color(main_color).strong());
                    ui.add_space(12.0);

                    let presets: [([u8; 3], &str); 7] = [
                        ([190, 24, 93], "Rose"),       // #be185d Horizon Pink
                        ([59, 130, 246], "Blue"),      // Blue
                        ([14, 165, 233], "Sky"),       // Sky
                        ([16, 185, 129], "Teal"),      // Teal
                        ([34, 197, 94], "Green"),      // Green
                        ([249, 115, 22], "Orange"),    // Orange
                        ([239, 68, 68], "Red"),        // Red
                    ];

                    for (rgb, _name) in presets {
                        let col = egui::Color32::from_rgb(rgb[0], rgb[1], rgb[2]);
                        let is_active = self.config.accent_color == rgb;
                        let _size = if is_active { egui::vec2(24.0, 24.0) } else { egui::vec2(18.0, 18.0) };
                        if ui.button(egui::RichText::new("").background_color(col)).clicked() {
                            self.config.accent_color = rgb;
                            self.config.save();
                        }
                    }
                });
            });

            ui.add_space(12.0);

            // 4. Glass & Vignette Sliders
            ui.group(|ui| {
                ui.vertical(|ui| {
                    ui.horizontal(|ui| {
                        ui.label(egui::RichText::new(s.settings_glass).color(main_color));
                        let mut val = self.config.glass_opacity;
                        if ui.add(egui::Slider::new(&mut val, 0.1..=1.0)).changed() {
                            self.config.glass_opacity = val;
                            self.config.save();
                        }
                    });

                    ui.add_space(6.0);

                    ui.horizontal(|ui| {
                        ui.label(egui::RichText::new(s.settings_vignette).color(main_color));
                        let mut val = self.config.vignette;
                        if ui.add(egui::Slider::new(&mut val, 0.0..=1.0)).changed() {
                            self.config.vignette = val;
                            self.config.save();
                        }
                    });
                });
            });

            ui.add_space(12.0);

            // 5. Behavior during game: Minimize / Stay
            ui.group(|ui| {
                ui.horizontal(|ui| {
                    ui.label(egui::RichText::new(s.settings_during_game).color(main_color).strong());
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
            });

            ui.add_space(12.0);

            // 6. Game Client Path & Launch Args
            ui.group(|ui| {
                ui.vertical(|ui| {
                    ui.label(egui::RichText::new(s.settings_game_path).color(main_color).strong());
                    ui.horizontal(|ui| {
                        ui.text_edit_singleline(&mut self.config.game_path);
                        if ui.button(s.settings_browse).clicked() {
                            if let Some(file) = rfd::FileDialog::new()
                                .add_filter("Executables", &["exe"])
                                .pick_file()
                            {
                                self.config.game_path = file.display().to_string();
                                self.config.save();
                            }
                        }
                    });

                    ui.add_space(8.0);

                    ui.label(egui::RichText::new(s.settings_args).color(main_color).strong());
                    if ui.text_edit_singleline(&mut self.config.game_args).lost_focus() {
                        self.config.save();
                    }

                    ui.add_space(8.0);

                    let mut fs = self.config.fullscreen;
                    if ui.checkbox(&mut fs, s.settings_fullscreen).changed() {
                        self.config.fullscreen = fs;
                        self.config.save();
                    }
                });
            });

            ui.add_space(16.0);

            // 7. Reset settings button
            if ui.button(egui::RichText::new(s.settings_reset).color(egui::Color32::from_rgb(239, 68, 68))).clicked() {
                self.config = LauncherConfig::default();
                self.config.save();
            }

            ui.add_space(24.0);
        });
    }
}

fn main() -> eframe::Result<()> {
    let native_options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default()
            .with_title("chroma. Launcher")
            .with_inner_size([1020.0, 640.0])
            .with_min_inner_size([840.0, 540.0]),
        ..Default::default()
    };

    eframe::run_native(
        "chroma. Launcher",
        native_options,
        Box::new(|cc| Ok(Box::new(ChromaLauncherApp::new(cc)))),
    )
}
