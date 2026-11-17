use std::path::PathBuf;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AssetTab {
    Entities,
    Game,
    Emoticons,
    Particles,
    Hud,
    Extras,
}

#[derive(Debug, Clone)]
#[allow(dead_code)]
pub struct AssetItem {
    pub name: String,
    pub path: PathBuf,
    pub is_dir: bool,
    pub size_bytes: u64,
}

pub struct AssetsManager;

impl AssetsManager {
    pub fn get_tab_dir(tab: AssetTab) -> PathBuf {
        let sub = match tab {
            AssetTab::Entities => "entities",
            AssetTab::Game => "game",
            AssetTab::Emoticons => "emoticons",
            AssetTab::Particles => "particles",
            AssetTab::Hud => "hud",
            AssetTab::Extras => "extras",
        };

        // Try local repo/game data or appdata
        let local_path = PathBuf::from(format!("data/{}", sub));
        if local_path.exists() {
            return local_path;
        }

        let parent_path = PathBuf::from(format!("../data/{}", sub));
        if parent_path.exists() {
            return parent_path;
        }

        if let Some(appdata) = std::env::var_os("APPDATA") {
            let app_path = PathBuf::from(appdata).join("DDNet").join(sub);
            if app_path.exists() {
                return app_path;
            }
        }

        local_path
    }

    pub fn scan_tab(tab: AssetTab) -> Vec<AssetItem> {
        let dir = Self::get_tab_dir(tab);
        let mut items = Vec::new();

        if let Ok(entries) = std::fs::read_dir(&dir) {
            for entry in entries.flatten() {
                let path = entry.path();
                let name = path.file_name().unwrap_or_default().to_string_lossy().to_string();
                if name.starts_with('.') {
                    continue;
                }

                let is_dir = path.is_dir();
                let size_bytes = entry.metadata().map(|m| m.len()).unwrap_or(0);

                items.push(AssetItem {
                    name,
                    path,
                    is_dir,
                    size_bytes,
                });
            }
        }

        items.sort_by(|a, b| a.name.to_lowercase().cmp(&b.name.to_lowercase()));
        items
    }
}
