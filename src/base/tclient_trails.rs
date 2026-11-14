//! T-Client Movement and Weapon Trails geometry generator in Rust.

use crate::prism_math::{ColorRGBA, Vec2};

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct TrailVertex {
    pub pos: Vec2,
    pub color: ColorRGBA,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct TrailConfig {
    pub enabled: bool,
    pub width: f32,
    pub decay_rate: f32,
    pub base_color: ColorRGBA,
}

/// Calculate ribbon vertices for a trail segment between p0 and p1
pub fn compute_trail_segment(
    p0: Vec2,
    p1: Vec2,
    width0: f32,
    width1: f32,
    color0: ColorRGBA,
    color1: ColorRGBA,
) -> (TrailVertex, TrailVertex, TrailVertex, TrailVertex) {
    let dir = Vec2::new(p1.x - p0.x, p1.y - p0.y).normalize();
    // Normal vector perpendicular to direction
    let normal = Vec2::new(-dir.y, dir.x);

    let v0 = TrailVertex {
        pos: Vec2::new(p0.x + normal.x * width0 * 0.5, p0.y + normal.y * width0 * 0.5),
        color: color0,
    };
    let v1 = TrailVertex {
        pos: Vec2::new(p0.x - normal.x * width0 * 0.5, p0.y - normal.y * width0 * 0.5),
        color: color0,
    };
    let v2 = TrailVertex {
        pos: Vec2::new(p1.x + normal.x * width1 * 0.5, p1.y + normal.y * width1 * 0.5),
        color: color1,
    };
    let v3 = TrailVertex {
        pos: Vec2::new(p1.x - normal.x * width1 * 0.5, p1.y - normal.y * width1 * 0.5),
        color: color1,
    };

    (v0, v1, v2, v3)
}
