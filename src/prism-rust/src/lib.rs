//! Prism Core Rust Engine
//! High-performance geometry, math, telemetry, physics, and shader pipelines for DDNet & Prism.

pub mod prism_math;
pub mod chinese_hat;
pub mod prism_theme;
pub mod prism_hud;
pub mod tclient_rainbow;
pub mod tclient_pet;
pub mod tclient_trails;
pub mod tclient_statusbar;

pub use prism_math::*;
pub use chinese_hat::*;
pub use prism_theme::*;
pub use prism_hud::*;
pub use tclient_rainbow::*;
pub use tclient_pet::*;
pub use tclient_trails::*;
pub use tclient_statusbar::*;

// =========================================================================
// C-ABI EXPORTS (Zero-overhead FFI to C++ game client)
// =========================================================================

#[no_mangle]
pub extern "C" fn prism_rust_version() -> u32 {
    20300
}

// --- Chinese Hat FFI ---

#[no_mangle]
pub extern "C" fn prism_chinese_hat_generate(
    tee_x: f32,
    tee_y: f32,
    tee_radius: f32,
    dir_angle: f32,
    time_seconds: f32,
    config: *const HatConfig,
    out_geom: *mut HatGeometry,
) -> bool {
    if config.is_null() || out_geom.is_null() {
        return false;
    }
    unsafe {
        let conf = &*config;
        let tee_pos = Vec2::new(tee_x, tee_y);
        let geom = generate_hat_geometry(tee_pos, tee_radius, dir_angle, time_seconds, conf);
        *out_geom = geom;
    }
    true
}

// --- Theme FFI ---

#[no_mangle]
pub extern "C" fn prism_theme_get_palette(
    preset_index: i32,
    custom_r: f32,
    custom_g: f32,
    custom_b: f32,
    custom_a: f32,
    out_palette: *mut ThemePalette,
) -> bool {
    if out_palette.is_null() {
        return false;
    }
    let preset = match preset_index {
        0 => ThemePreset::Crimson,
        1 => ThemePreset::Sapphire,
        2 => ThemePreset::Emerald,
        3 => ThemePreset::Amethyst,
        4 => ThemePreset::Sunset,
        _ => ThemePreset::Custom,
    };
    let custom = ColorRGBA::new(custom_r, custom_g, custom_b, custom_a);
    let pal = ThemePalette::for_preset(preset, custom);
    unsafe {
        *out_palette = pal;
    }
    true
}

// --- HUD FFI ---

#[no_mangle]
pub extern "C" fn prism_hud_calculate_speedometer(
    vel_x: f32,
    vel_y: f32,
    out_data: *mut SpeedometerData,
) -> bool {
    if out_data.is_null() {
        return false;
    }
    let data = calculate_speedometer(Vec2::new(vel_x, vel_y));
    unsafe {
        *out_data = data;
    }
    true
}

// --- T-Client Rainbow FFI ---

#[no_mangle]
pub extern "C" fn prism_tclient_rainbow_color(
    time_seconds: f32,
    offset: f32,
    config: *const RainbowConfig,
    out_color: *mut ColorRGBA,
) -> bool {
    if config.is_null() || out_color.is_null() {
        return false;
    }
    unsafe {
        let conf = &*config;
        *out_color = get_rainbow_color(time_seconds, offset, conf);
    }
    true
}

// --- T-Client Pet FFI ---

#[no_mangle]
pub extern "C" fn prism_tclient_pet_update(
    state: *mut PetState,
    owner_x: f32,
    owner_y: f32,
    vel_x: f32,
    vel_y: f32,
    delta_time: f32,
    time_seconds: f32,
    config: *const PetConfig,
) -> bool {
    if state.is_null() || config.is_null() {
        return false;
    }
    unsafe {
        let conf = &*config;
        let st = &mut *state;
        update_pet_physics(
            st,
            Vec2::new(owner_x, owner_y),
            Vec2::new(vel_x, vel_y),
            delta_time,
            time_seconds,
            conf,
        );
    }
    true
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_version() {
        assert_eq!(prism_rust_version(), 20300);
    }

    #[test]
    fn test_chinese_hat_calc() {
        let config = HatConfig {
            enabled: true,
            color_mode: 1,
            base_color: ColorRGBA::WHITE,
            facet_alpha: 0.65,
            rib_alpha: 0.90,
            wave_speed: 1.0,
            scale: 1.0,
            y_offset: -26.0,
            tilt: 0.0,
        };
        let geom = generate_hat_geometry(Vec2::new(100.0, 100.0), 28.0, 0.0, 1.5, &config);
        assert_eq!(geom.facets.len(), CHINESE_HAT_FACET_COUNT);
        assert!(geom.facets[0].v0.color.a > 0.0);
    }

    #[test]
    fn test_speedometer() {
        let sp = calculate_speedometer(Vec2::new(32.0, 0.0));
        assert!(sp.speed_tiles_per_sec > 0.0);
    }
}
