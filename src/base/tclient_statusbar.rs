//! T-Client Statusbar metric and layout engine in Rust.

use crate::prism_math::ColorRGBA;

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct StatusbarMetrics {
    pub ping: i32,
    pub fps: i32,
    pub health: i32,
    pub armor: i32,
    pub speed: f32,
}

pub fn get_health_color(health: i32) -> ColorRGBA {
    let t = (health as f32 / 10.0).clamp(0.0, 1.0);
    if t > 0.6 {
        ColorRGBA::new(0.2, 0.9, 0.3, 1.0)
    } else if t > 0.3 {
        ColorRGBA::new(0.95, 0.8, 0.2, 1.0)
    } else {
        ColorRGBA::new(0.95, 0.2, 0.2, 1.0)
    }
}

pub fn get_ping_color(ping: i32) -> ColorRGBA {
    if ping < 40 {
        ColorRGBA::new(0.2, 0.9, 0.3, 1.0)
    } else if ping < 90 {
        ColorRGBA::new(0.95, 0.8, 0.2, 1.0)
    } else {
        ColorRGBA::new(0.95, 0.2, 0.2, 1.0)
    }
}
