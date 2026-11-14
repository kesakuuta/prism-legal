//! T-Client Pet companion physics and animation simulation in Rust.

use crate::prism_math::{ColorRGBA, Vec2};

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct PetState {
    pub pos: Vec2,
    pub vel: Vec2,
    pub orbit_angle: f32,
    pub eye_dir: Vec2,
    pub color: ColorRGBA,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct PetConfig {
    pub enabled: bool,
    pub orbit_radius: f32,
    pub orbit_speed: f32,
    pub spring_stiffness: f32,
    pub damping: f32,
    pub bobbing_amplitude: f32,
    pub bobbing_frequency: f32,
    pub scale: f32,
}

impl Default for PetConfig {
    fn default() -> Self {
        PetConfig {
            enabled: true,
            orbit_radius: 54.0,
            orbit_speed: 1.8,
            spring_stiffness: 12.0,
            damping: 4.5,
            bobbing_amplitude: 8.0,
            bobbing_frequency: 3.2,
            scale: 0.55,
        }
    }
}

pub fn update_pet_physics(
    current_state: &mut PetState,
    owner_pos: Vec2,
    owner_vel: Vec2,
    delta_time: f32,
    time_seconds: f32,
    config: &PetConfig,
) {
    if !config.enabled {
        return;
    }

    current_state.orbit_angle = (current_state.orbit_angle + config.orbit_speed * delta_time)
        % (std::f32::consts::PI * 2.0);

    let bobbing = (time_seconds * config.bobbing_frequency).sin() * config.bobbing_amplitude;

    let target_offset = Vec2::new(
        current_state.orbit_angle.cos() * config.orbit_radius,
        current_state.orbit_angle.sin() * (config.orbit_radius * 0.4) - 30.0 + bobbing,
    );

    let target_pos = Vec2::new(
        owner_pos.x + target_offset.x + owner_vel.x * 0.12,
        owner_pos.y + target_offset.y + owner_vel.y * 0.12,
    );

    // Spring-damper force: F = -k * dx - c * v
    let displacement = Vec2::new(
        current_state.pos.x - target_pos.x,
        current_state.pos.y - target_pos.y,
    );

    let spring_force_x = -config.spring_stiffness * displacement.x - config.damping * current_state.vel.x;
    let spring_force_y = -config.spring_stiffness * displacement.y - config.damping * current_state.vel.y;

    current_state.vel.x += spring_force_x * delta_time;
    current_state.vel.y += spring_force_y * delta_time;

    current_state.pos.x += current_state.vel.x * delta_time;
    current_state.pos.y += current_state.vel.y * delta_time;

    // Looking towards cursor / motion
    current_state.eye_dir = Vec2::new(
        owner_pos.x - current_state.pos.x,
        owner_pos.y - current_state.pos.y,
    )
    .normalize();
}
