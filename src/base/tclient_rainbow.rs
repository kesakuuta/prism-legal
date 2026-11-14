//! T-Client Rainbow Engine written in Rust.
//! Fast HSV color cycling, character string rainbow wave, tee outlines, and trail color sweeps.

use crate::prism_math::ColorRGBA;

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct RainbowConfig {
    pub enabled: bool,
    pub speed: f32,
    pub saturation: f32,
    pub lightness: f32,
    pub alpha: f32,
    pub spread: f32,
}

impl Default for RainbowConfig {
    fn default() -> Self {
        RainbowConfig {
            enabled: true,
            speed: 1.0,
            saturation: 0.9,
            lightness: 0.95,
            alpha: 1.0,
            spread: 0.05,
        }
    }
}

pub fn get_rainbow_color(time_seconds: f32, offset: f32, config: &RainbowConfig) -> ColorRGBA {
    if !config.enabled {
        return ColorRGBA::WHITE;
    }
    let hue = (time_seconds * config.speed * 0.2 + offset * config.spread) % 1.0;
    ColorRGBA::from_hsv(hue, config.saturation, config.lightness, config.alpha)
}

pub fn get_rainbow_for_char(char_index: usize, time_seconds: f32, config: &RainbowConfig) -> ColorRGBA {
    get_rainbow_color(time_seconds, char_index as f32, config)
}

pub fn get_rainbow_for_trail(segment_index: usize, total_segments: usize, time_seconds: f32, config: &RainbowConfig) -> ColorRGBA {
    let t = if total_segments > 0 { segment_index as f32 / total_segments as f32 } else { 0.0 };
    let mut c = get_rainbow_color(time_seconds, t * 10.0, config);
    // Quadratic fadeout towards tail
    c.a *= (1.0 - t * t).clamp(0.0, 1.0);
    c
}
