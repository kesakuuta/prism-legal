use std::path::PathBuf;
use serde::Deserialize;

#[derive(Debug, Clone, Deserialize)]
#[allow(dead_code)]
pub struct ThemeJson {
    pub name: Option<String>,
    pub author: Option<String>,
    pub version: Option<String>,
    pub font_file: Option<String>,
    pub background: Option<String>,
}

#[derive(Debug, Clone)]
#[allow(dead_code)]
pub struct ThemeItem {
    pub id: String,
    pub name: String,
    pub author: String,
    pub version: String,
    pub path: PathBuf,
    pub bg_path: Option<PathBuf>,
}

pub struct ThemeManager;

impl ThemeManager {
    pub fn scan_themes() -> Vec<ThemeItem> {
        let mut themes = Vec::new();

        let search_dirs = [
            PathBuf::from("data/chroma/themes"),
            PathBuf::from("../data/chroma/themes"),
            if let Some(appdata) = std::env::var_os("APPDATA") {
                PathBuf::from(appdata).join("DDNet/chroma/themes")
            } else {
                PathBuf::new()
            },
        ];

        for base in search_dirs {
            if !base.exists() {
                continue;
            }

            if let Ok(entries) = std::fs::read_dir(&base) {
                for entry in entries.flatten() {
                    let path = entry.path();
                    if path.is_dir() {
                        let id = path.file_name().unwrap_or_default().to_string_lossy().to_string();
                        if id.starts_with('.') || themes.iter().any(|t: &ThemeItem| t.id == id) {
                            continue;
                        }

                        let mut name = id.clone();
                        let mut author = "chroma.".to_string();
                        let mut version = "1.0".to_string();

                        let json_file = path.join("theme.json");
                        if let Ok(bytes) = std::fs::read(&json_file) {
                            if let Ok(parsed) = serde_json::from_slice::<ThemeJson>(&bytes) {
                                if let Some(n) = parsed.name { name = n; }
                                if let Some(a) = parsed.author { author = a; }
                                if let Some(v) = parsed.version { version = v; }
                            }
                        }

                        let bg_path = [
                            path.join("background.png"),
                            path.join("background.jpg"),
                            path.join("background.jpeg"),
                        ].into_iter().find(|p| p.exists());

                        themes.push(ThemeItem {
                            id,
                            name,
                            author,
                            version,
                            path,
                            bg_path,
                        });
                    }
                }
            }
        }

        themes.sort_by(|a, b| a.name.to_lowercase().cmp(&b.name.to_lowercase()));
        themes
    }
}
