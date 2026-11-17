use std::path::PathBuf;

#[derive(Debug, Clone)]
#[allow(dead_code)]
pub struct TrackItem {
    pub title: String,
    pub artist: String,
    pub path: PathBuf,
    pub format: String,
}

pub struct MusicManager;

impl MusicManager {
    pub fn scan_music() -> Vec<TrackItem> {
        let mut tracks = Vec::new();

        // Built-in default track
        tracks.push(TrackItem {
            title: "Leanin' (Slowed)".to_string(),
            artist: "cormill".to_string(),
            path: PathBuf::from("audio/music_menu.wv"),
            format: "wv".to_string(),
        });

        let search_dirs = [
            PathBuf::from("data/audio/music"),
            PathBuf::from("../data/audio/music"),
            if let Some(appdata) = std::env::var_os("APPDATA") {
                PathBuf::from(appdata).join("DDNet/audio/music")
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
                    if ext == "wv" || ext == "opus" {
                        let stem = path.file_stem().unwrap_or_default().to_string_lossy().to_string();
                        let (artist, title) = if let Some((a, t)) = stem.split_once(" - ") {
                            (a.trim().to_string(), t.trim().to_string())
                        } else {
                            ("Unknown Artist".to_string(), stem.clone())
                        };

                        if tracks.iter().any(|t: &TrackItem| t.title == title && t.artist == artist) {
                            continue;
                        }

                        tracks.push(TrackItem {
                            title,
                            artist,
                            path,
                            format: ext,
                        });
                    }
                }
            }
        }

        tracks
    }
}
