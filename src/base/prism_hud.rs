//! Prism Modular HUD Telemetry and calculation engine in Rust.

use crate::prism_math::Vec2;

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct SpeedometerData {
    pub raw_speed: f32,
    pub speed_tiles_per_sec: f32,
    pub speed_km_h: f32,
    pub speed_mph: f32,
    pub heading_angle: f32,
}

pub fn calculate_speedometer(vel: Vec2) -> SpeedometerData {
    let raw_speed = vel.length();
    // In DDNet, 1 tile = 32 units, 50 ticks per second
    let speed_tiles_per_sec = (raw_speed * 50.0) / 32.0;
    // Approximating 1 tile as 1 meter
    let speed_km_h = speed_tiles_per_sec * 3.6;
    let speed_mph = speed_km_h * 0.621371;
    let heading_angle = vel.y.atan2(vel.x);

    SpeedometerData {
        raw_speed,
        speed_tiles_per_sec,
        speed_km_h,
        speed_mph,
        heading_angle,
    }
}

/// Exponential Moving Average filter for smooth FPS and Ping display.
#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct EmaFilter {
    pub value: f32,
    pub alpha: f32,
}

impl EmaFilter {
    pub fn new(initial: f32, alpha: f32) -> Self {
        EmaFilter {
            value: initial,
            alpha: alpha.clamp(0.01, 1.0),
        }
    }

    pub fn update(&mut self, sample: f32) -> f32 {
        self.value = self.alpha * sample + (1.0 - self.alpha) * self.value;
        self.value
    }
}

/// Keystrokes UI Key layout state
#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct KeyState {
    pub is_pressed: bool,
    pub press_alpha: f32,
}

pub fn update_key_alpha(current_alpha: f32, is_pressed: bool, delta_time: f32) -> f32 {
    let target = if is_pressed { 1.0 } else { 0.0 };
    let speed = if is_pressed { 14.0 } else { 7.0 };
    let new_alpha = current_alpha + (target - current_alpha) * (speed * delta_time).min(1.0);
    new_alpha.clamp(0.0, 1.0)
}
