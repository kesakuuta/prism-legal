//! Mathematical utilities, vector operations, color space conversions for Prism & DDNet.

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

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct ColorRGBA {
    pub r: f32,
    pub g: f32,
    pub b: f32,
    pub a: f32,
}

impl ColorRGBA {
    pub const WHITE: ColorRGBA = ColorRGBA { r: 1.0, g: 1.0, b: 1.0, a: 1.0 };
    pub const BLACK: ColorRGBA = ColorRGBA { r: 0.0, g: 0.0, b: 0.0, a: 1.0 };
    pub const TRANSPARENT: ColorRGBA = ColorRGBA { r: 0.0, g: 0.0, b: 0.0, a: 0.0 };

    pub fn new(r: f32, g: f32, b: f32, a: f32) -> Self {
        ColorRGBA { r, g, b, a }
    }

    pub fn from_hsv(hue: f32, sat: f32, val: f32, alpha: f32) -> Self {
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

    pub fn lerp(&self, other: &ColorRGBA, t: f32) -> Self {
        let t = t.clamp(0.0, 1.0);
        ColorRGBA {
            r: self.r + (other.r - self.r) * t,
            g: self.g + (other.g - self.g) * t,
            b: self.b + (other.b - self.b) * t,
            a: self.a + (other.a - self.a) * t,
        }
    }
}
