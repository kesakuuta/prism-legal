use std::path::{Path, PathBuf};
use serde::{Deserialize, Serialize};
use crate::i18n::Language;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum BgMode {
    Fill,
    Fit,
    Stretch,
    Center,
}

impl Default for BgMode {
    fn default() -> Self {
        BgMode::Fill
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum ThemeMode {
    Dark,
    Light,
    System,
}

impl Default for ThemeMode {
    fn default() -> Self {
        ThemeMode::Dark
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum HorizonTheme {
    Chroma,
    FirstLight,
}

impl Default for HorizonTheme {
    fn default() -> Self {
        HorizonTheme::Chroma
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct LauncherConfig {
    pub language: Language,
    pub theme_mode: ThemeMode,
    pub horizon_theme: HorizonTheme,
    pub accent_color: [u8; 3],
    pub glass_opacity: f32,
    pub blur_radius: f32,
    pub performance_mode: bool,
    pub background_path: Option<String>,
    pub background_mode: BgMode,
    pub adaptive_text: bool,
    pub minimize_on_launch: bool,
    pub show_splash: bool,
    pub game_path: String,
    pub game_args: String,
    pub selected_theme: String,
    pub selected_shader: String,
    pub selected_font: String,
}

impl Default for LauncherConfig {
    fn default() -> Self {
        Self {
            language: Language::Russian,
            theme_mode: ThemeMode::Dark,
            horizon_theme: HorizonTheme::Chroma,
            accent_color: [190, 16, 93], // #be105d Horizon crimson
            glass_opacity: 0.85,
            blur_radius: 2.0,
            performance_mode: false,
            background_path: None,
            background_mode: BgMode::Fill,
            adaptive_text: true,
            minimize_on_launch: true,
            show_splash: true,
            game_path: detect_default_game_path(),
            game_args: String::new(),
            selected_theme: "chroma_dark".to_string(),
            selected_shader: "atmospheric_amber".to_string(),
            selected_font: "default".to_string(),
        }
    }
}

pub fn detect_default_game_path() -> String {
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

    for candidate in candidates {
        if Path::new(candidate).exists() {
            return candidate.to_string();
        }
    }

    "chroma.exe".to_string()
}

impl LauncherConfig {
    pub fn config_path() -> PathBuf {
        if let Some(appdata) = std::env::var_os("APPDATA") {
            let mut dir = PathBuf::from(appdata);
            dir.push("chroma_launcher");
            let _ = std::fs::create_dir_all(&dir);
            dir.push("config.json");
            dir
        } else {
            PathBuf::from("launcher_config.json")
        }
    }

    pub fn load() -> Self {
        let path = Self::config_path();
        if let Ok(bytes) = std::fs::read(&path) {
            if let Ok(cfg) = serde_json::from_slice(&bytes) {
                return cfg;
            }
        }
        Self::default()
    }

    pub fn save(&self) {
        let path = Self::config_path();
        if let Ok(json) = serde_json::to_string_pretty(self) {
            let _ = std::fs::write(path, json);
        }
    }
}
