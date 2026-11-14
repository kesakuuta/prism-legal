//! Mathematical utilities, vector operations, color space conversions for Prism & DDNet.

use crate::color::ColorRGBA;

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Vec2 {
    pub x: f32,
    pub y: f32,
}

impl Vec2 {
    pub const ZERO: Vec2 = Vec2 { x: 0.0, y: 0.0 };

    pub fn new(x: f32, y: f32) -> Self {
        Vec2 { x, y }
    }

    pub fn length_sq(&self) -> f32 {
        self.x * self.x + self.y * self.y
    }

    pub fn length(&self) -> f32 {
        self.length_sq().sqrt()
    }

    pub fn normalize(&self) -> Self {
        let len = self.length();
        if len > 1e-6 {
            Vec2 {
                x: self.x / len,
                y: self.y / len,
            }
        } else {
            Vec2::ZERO
        }
    }

    pub fn dot(&self, other: &Vec2) -> f32 {
        self.x * other.x + self.y * other.y
    }

    pub fn distance(&self, other: &Vec2) -> f32 {
        Vec2 {
            x: self.x - other.x,
            y: self.y - other.y,
        }
        .length()
    }

    pub fn lerp(&self, target: &Vec2, factor: f32) -> Self {
        Vec2 {
            x: self.x + (target.x - self.x) * factor,
            y: self.y + (target.y - self.y) * factor,
        }
    }
}

pub fn color_from_hsv(hue: f32, sat: f32, val: f32, alpha: f32) -> ColorRGBA {
    let h = (hue % 1.0 + 1.0) % 1.0 * 6.0;
    let c = val * sat;
    let x = c * (1.0 - ((h % 2.0) - 1.0).abs());
    let m = val - c;

    let (r1, g1, b1) = if h < 1.0 {
        (c, x, 0.0)
    } else if h < 2.0 {
        (x, c, 0.0)
    } else if h < 3.0 {
        (0.0, c, x)
    } else if h < 4.0 {
        (0.0, x, c)
    } else if h < 5.0 {
        (x, 0.0, c)
    } else {
        (c, 0.0, x)
    };

    ColorRGBA {
        r: r1 + m,
        g: g1 + m,
        b: b1 + m,
        a: alpha,
    }
}

pub fn color_lerp(c1: &ColorRGBA, c2: &ColorRGBA, t: f32) -> ColorRGBA {
    let t = t.clamp(0.0, 1.0);
    ColorRGBA {
        r: c1.r + (c2.r - c1.r) * t,
        g: c1.g + (c2.g - c1.g) * t,
        b: c1.b + (c2.b - c1.b) * t,
        a: c1.a + (c2.a - c1.a) * t,
    }
}
