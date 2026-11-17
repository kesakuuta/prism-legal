use std::path::PathBuf;
use serde::Deserialize;

#[derive(Debug, Clone, Deserialize)]
pub struct ShaderJson {
    pub name: Option<String>,
    pub author: Option<String>,
    pub version: Option<String>,
    pub description: Option<String>,
}

#[derive(Debug, Clone)]
#[allow(dead_code)]
pub struct ShaderItem {
    pub id: String,
    pub name: String,
    pub author: String,
    pub version: String,
    pub description: String,
    pub path: PathBuf,
}

pub struct ShaderManager;

impl ShaderManager {
    pub fn scan_shaders() -> Vec<ShaderItem> {
        let mut shaders = Vec::new();

        let search_dirs = [
            PathBuf::from("data/chroma/shaders"),
            PathBuf::from("../data/chroma/shaders"),
            if let Some(appdata) = std::env::var_os("APPDATA") {
                PathBuf::from(appdata).join("DDNet/chroma/shaders")
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
                        if id.starts_with('.') || shaders.iter().any(|s: &ShaderItem| s.id == id) {
                            continue;
                        }

                        let mut name = id.clone();
                        let mut author = "chroma.".to_string();
                        let mut version = "1.0".to_string();
                        let mut description = "Atmospheric post-processing shaderpack".to_string();

                        let json_file = path.join("shader.json");
                        if let Ok(bytes) = std::fs::read(&json_file) {
                            if let Ok(parsed) = serde_json::from_slice::<ShaderJson>(&bytes) {
                                if let Some(n) = parsed.name { name = n; }
                                if let Some(a) = parsed.author { author = a; }
                                if let Some(v) = parsed.version { version = v; }
                                if let Some(d) = parsed.description { description = d; }
                            }
                        }

                        shaders.push(ShaderItem {
                            id,
                            name,
                            author,
                            version,
                            description,
                            path,
                        });
                    }
                }
            }
        }

        shaders.sort_by(|a, b| a.name.to_lowercase().cmp(&b.name.to_lowercase()));
        shaders
    }
}
