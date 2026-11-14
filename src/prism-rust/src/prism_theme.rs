//! Prism Theme Engine written in Rust.
//! Calculates color palettes, UI accents, background gradients, and button state colors.

use crate::prism_math::ColorRGBA;

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq)]
pub enum ThemePreset {
    Crimson = 0,
    Sapphire = 1,
    Emerald = 2,
    Amethyst = 3,
    Sunset = 4,
    Custom = 5,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct ThemePalette {
    pub primary: ColorRGBA,
    pub secondary: ColorRGBA,
    pub accent: ColorRGBA,
    pub background: ColorRGBA,
    pub surface: ColorRGBA,
    pub border: ColorRGBA,
    pub text_bright: ColorRGBA,
    pub text_muted: ColorRGBA,
}

impl ThemePalette {
    pub fn for_preset(preset: ThemePreset, custom_color: ColorRGBA) -> Self {
        match preset {
            ThemePreset::Crimson => ThemePalette {
                primary: ColorRGBA::new(0.85, 0.15, 0.25, 1.0),
                secondary: ColorRGBA::new(0.60, 0.10, 0.18, 1.0),
                accent: ColorRGBA::new(1.00, 0.30, 0.40, 1.0),
                background: ColorRGBA::new(0.08, 0.05, 0.06, 0.95),
                surface: ColorRGBA::new(0.14, 0.09, 0.11, 0.85),
                border: ColorRGBA::new(0.85, 0.15, 0.25, 0.40),
                text_bright: ColorRGBA::new(1.0, 1.0, 1.0, 1.0),
                text_muted: ColorRGBA::new(0.7, 0.65, 0.67, 1.0),
            },
            ThemePreset::Sapphire => ThemePalette {
                primary: ColorRGBA::new(0.15, 0.45, 0.95, 1.0),
                secondary: ColorRGBA::new(0.10, 0.30, 0.70, 1.0),
                accent: ColorRGBA::new(0.35, 0.65, 1.00, 1.0),
                background: ColorRGBA::new(0.05, 0.07, 0.10, 0.95),
                surface: ColorRGBA::new(0.09, 0.12, 0.18, 0.85),
                border: ColorRGBA::new(0.15, 0.45, 0.95, 0.40),
                text_bright: ColorRGBA::new(1.0, 1.0, 1.0, 1.0),
                text_muted: ColorRGBA::new(0.65, 0.70, 0.75, 1.0),
            },
            ThemePreset::Emerald => ThemePalette {
                primary: ColorRGBA::new(0.15, 0.80, 0.45, 1.0),
                secondary: ColorRGBA::new(0.10, 0.55, 0.30, 1.0),
                accent: ColorRGBA::new(0.30, 0.95, 0.60, 1.0),
                background: ColorRGBA::new(0.05, 0.09, 0.07, 0.95),
                surface: ColorRGBA::new(0.08, 0.15, 0.11, 0.85),
                border: ColorRGBA::new(0.15, 0.80, 0.45, 0.40),
                text_bright: ColorRGBA::new(1.0, 1.0, 1.0, 1.0),
                text_muted: ColorRGBA::new(0.65, 0.75, 0.70, 1.0),
            },
            ThemePreset::Amethyst => ThemePalette {
                primary: ColorRGBA::new(0.65, 0.25, 0.90, 1.0),
                secondary: ColorRGBA::new(0.45, 0.15, 0.65, 1.0),
                accent: ColorRGBA::new(0.80, 0.40, 1.00, 1.0),
                background: ColorRGBA::new(0.07, 0.05, 0.10, 0.95),
                surface: ColorRGBA::new(0.12, 0.09, 0.17, 0.85),
                border: ColorRGBA::new(0.65, 0.25, 0.90, 0.40),
                text_bright: ColorRGBA::new(1.0, 1.0, 1.0, 1.0),
                text_muted: ColorRGBA::new(0.70, 0.65, 0.75, 1.0),
            },
            ThemePreset::Sunset => ThemePalette {
                primary: ColorRGBA::new(0.95, 0.45, 0.15, 1.0),
                secondary: ColorRGBA::new(0.70, 0.30, 0.10, 1.0),
                accent: ColorRGBA::new(1.00, 0.65, 0.25, 1.0),
                background: ColorRGBA::new(0.10, 0.06, 0.05, 0.95),
                surface: ColorRGBA::new(0.17, 0.10, 0.08, 0.85),
                border: ColorRGBA::new(0.95, 0.45, 0.15, 0.40),
                text_bright: ColorRGBA::new(1.0, 1.0, 1.0, 1.0),
                text_muted: ColorRGBA::new(0.75, 0.68, 0.65, 1.0),
            },
            ThemePreset::Custom => {
                let p = custom_color;
                ThemePalette {
                    primary: p,
                    secondary: ColorRGBA::new(p.r * 0.7, p.g * 0.7, p.b * 0.7, 1.0),
                    accent: ColorRGBA::new(
                        (p.r * 1.3).min(1.0),
                        (p.g * 1.3).min(1.0),
                        (p.b * 1.3).min(1.0),
                        1.0,
                    ),
                    background: ColorRGBA::new(0.08, 0.08, 0.08, 0.95),
                    surface: ColorRGBA::new(0.14, 0.14, 0.14, 0.85),
                    border: ColorRGBA::new(p.r, p.g, p.b, 0.40),
                    text_bright: ColorRGBA::new(1.0, 1.0, 1.0, 1.0),
                    text_muted: ColorRGBA::new(0.7, 0.7, 0.7, 1.0),
                }
            }
        }
    }

    /// Calculate dynamic button state color (idle, hover, active) with smooth interpolation.
    pub fn button_color(&self, is_hovered: bool, is_active: bool, anim_progress: f32) -> ColorRGBA {
        if is_active {
            self.primary
        } else if is_hovered {
            self.surface.lerp(&self.accent, anim_progress * 0.65)
        } else {
            self.surface
        }
    }
}
