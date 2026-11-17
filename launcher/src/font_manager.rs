use std::path::PathBuf;

#[derive(Debug, Clone)]
pub struct FontItem {
    pub name: String,
    pub path: PathBuf,
    pub size_kb: u64,
}

pub struct FontManager;

impl FontManager {
    pub fn scan_fonts() -> Vec<FontItem> {
        let mut fonts = Vec::new();

        let search_dirs = [
            PathBuf::from("data/fonts"),
            PathBuf::from("../data/fonts"),
            if let Some(appdata) = std::env::var_os("APPDATA") {
                PathBuf::from(appdata).join("DDNet/fonts")
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
                    let ext = path.extension().unwrap_or_default().to_string_lossy().to_lowercase();
                    if ext == "ttf" || ext == "otf" {
                        let name = path.file_stem().unwrap_or_default().to_string_lossy().to_string();
                        if fonts.iter().any(|f: &FontItem| f.name == name) {
                            continue;
                        }

                        let size_kb = entry.metadata().map(|m| m.len() / 1024).unwrap_or(0);
                        fonts.push(FontItem {
                            name,
                            path,
                            size_kb,
                        });
                    }
                }
            }
        }

        fonts.sort_by(|a, b| a.name.to_lowercase().cmp(&b.name.to_lowercase()));
        fonts
    }
}
