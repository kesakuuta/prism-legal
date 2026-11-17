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

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct LauncherConfig {
    pub language: Language,
    pub background_path: Option<String>,
    pub background_mode: BgMode,
    pub adaptive_text: bool,
    pub accent_color: [u8; 3],
    pub glass_opacity: f32,
    pub vignette: f32,
    pub minimize_on_launch: bool,
    pub game_path: String,
    pub game_args: String,
    pub fullscreen: bool,
    pub show_splash: bool,
    pub selected_theme: String,
    pub selected_shader: String,
    pub selected_font: String,
    pub selected_track: String,
}

impl Default for LauncherConfig {
    fn default() -> Self {
        Self {
            language: Language::Russian,
            background_path: None,
            background_mode: BgMode::Fill,
            adaptive_text: true,
            accent_color: [190, 24, 93], // #be185d Horizon crimson/rose
            glass_opacity: 0.85,
            vignette: 0.40,
            minimize_on_launch: true,
            game_path: detect_default_game_path(),
            game_args: String::new(),
            fullscreen: false,
            show_splash: true,
            selected_theme: "chrome_sword".to_string(),
            selected_shader: "atmospheric_amber".to_string(),
            selected_font: "default".to_string(),
            selected_track: "Leanin' (Slowed)".to_string(),
        }
    }
}

fn detect_default_game_path() -> String {
    let candidates = [
        "Prism.exe",
        "chroma.exe",
        "DDNet.exe",
        "../Prism.exe",
        "../chroma.exe",
        "../build/x64/Release/Prism.exe",
        "../../Prism.exe",
    ];

    for candidate in candidates {
        if Path::new(candidate).exists() {
            return candidate.to_string();
        }
    }

    "Prism.exe".to_string()
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
