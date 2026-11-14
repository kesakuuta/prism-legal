//! Chinese Conical Hat (斗笠 - Dǒulì) procedural 3D projection, triangulation,
//! and dynamic color wave engine implemented in Rust.

use crate::prism_math::{ColorRGBA, Vec2};
use std::f32::consts::PI;

pub const CHINESE_HAT_FACET_COUNT: usize = 8;
pub const CHINESE_HAT_VERTICES_PER_FACET: usize = 3;

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct HatVertex {
    pub pos: Vec2,
    pub color: ColorRGBA,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct HatConfig {
    pub enabled: bool,
    pub color_mode: i32, // 0 = Static, 1 = Rainbow, 2 = Shimmering Monochrome
    pub base_color: ColorRGBA,
    pub facet_alpha: f32,
    pub rib_alpha: f32,
    pub wave_speed: f32,
    pub scale: f32,
    pub y_offset: f32,
    pub tilt: f32,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct HatFacet {
    pub v0: HatVertex,
    pub v1: HatVertex,
    pub v2: HatVertex,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct HatRib {
    pub start: Vec2,
    pub end: Vec2,
    pub color: ColorRGBA,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct HatGeometry {
    pub facets: [HatFacet; CHINESE_HAT_FACET_COUNT],
    pub ribs: [HatRib; CHINESE_HAT_FACET_COUNT],
    pub brim_points: [Vec2; CHINESE_HAT_FACET_COUNT + 1],
    pub brim_color: ColorRGBA,
}

/// Calculate Chinese Hat geometry, facet triangulation, and procedural color waves.
pub fn generate_hat_geometry(
    tee_pos: Vec2,
    tee_radius: f32,
    dir_angle: f32,
    time_seconds: f32,
    config: &HatConfig,
) -> HatGeometry {
    let scale = (tee_radius / 28.0) * config.scale;
    let base_radius_x = 36.0 * scale;
    let base_radius_y = 10.5 * scale;
    let cone_height = 24.0 * scale;

    let total_tilt = config.tilt + (dir_angle.sin() * 0.12);
    let hat_center = Vec2::new(
        tee_pos.x,
        tee_pos.y + config.y_offset * (tee_radius / 28.0),
    );

    let apex = Vec2::new(
        hat_center.x + total_tilt * 4.0,
        hat_center.y - cone_height,
    );

    let mut base_pts = [Vec2::ZERO; CHINESE_HAT_FACET_COUNT + 1];
    let start_angle = total_tilt * 0.5;

    for i in 0..=CHINESE_HAT_FACET_COUNT {
        let t = i as f32 / CHINESE_HAT_FACET_COUNT as f32;
        let angle = start_angle + t * PI;

        let bx = -angle.cos() * base_radius_x;
        let by = angle.sin() * base_radius_y * 0.45;

        let cos_tilt = total_tilt.cos();
        let sin_tilt = total_tilt.sin();
        let rx = bx * cos_tilt - by * sin_tilt;
        let ry = bx * sin_tilt + by * cos_tilt;

        base_pts[i] = Vec2::new(hat_center.x + rx, hat_center.y + ry);
    }

    let default_facet = HatFacet {
        v0: HatVertex { pos: Vec2::ZERO, color: ColorRGBA::WHITE },
        v1: HatVertex { pos: Vec2::ZERO, color: ColorRGBA::WHITE },
        v2: HatVertex { pos: Vec2::ZERO, color: ColorRGBA::WHITE },
    };
    let default_rib = HatRib {
        start: Vec2::ZERO,
        end: Vec2::ZERO,
        color: ColorRGBA::WHITE,
    };

    let mut facets = [default_facet; CHINESE_HAT_FACET_COUNT];
    let mut ribs = [default_rib; CHINESE_HAT_FACET_COUNT];

    for i in 0..CHINESE_HAT_FACET_COUNT {
        let facet_color = compute_facet_color(i, CHINESE_HAT_FACET_COUNT, time_seconds, config);
        let rib_color = compute_rib_color(i, CHINESE_HAT_FACET_COUNT, time_seconds, config);

        facets[i] = HatFacet {
            v0: HatVertex { pos: apex, color: facet_color },
            v1: HatVertex { pos: base_pts[i], color: facet_color },
            v2: HatVertex { pos: base_pts[i + 1], color: facet_color },
        };

        ribs[i] = HatRib {
            start: apex,
            end: base_pts[i],
            color: rib_color,
        };
    }

    let brim_color = compute_brim_color(time_seconds, config);

    HatGeometry {
        facets,
        ribs,
        brim_points: base_pts,
        brim_color,
    }
}

fn compute_facet_color(index: usize, total: usize, time: f32, config: &HatConfig) -> ColorRGBA {
    match config.color_mode {
        1 => {
            // Rainbow Gradient Wave
            let t = index as f32 / total as f32;
            let hue = (time * config.wave_speed * 0.35 + t * 0.8) % 1.0;
            ColorRGBA::from_hsv(hue, 0.85, 0.95, config.facet_alpha)
        }
        2 => {
            // Shimmering Monochromatic Luster
            let t = index as f32 / total as f32;
            let phase = time * config.wave_speed * 2.5 + t * 4.0;
            let wave = (phase.sin() + 1.0) * 0.5;
            let specular = (phase.cos() * 2.0).clamp(0.0, 1.0).powf(3.0) * 0.35;

            let luma = (0.35 + wave * 0.45 + specular).clamp(0.0, 1.0);
            ColorRGBA {
                r: config.base_color.r * luma,
                g: config.base_color.g * luma,
                b: config.base_color.b * luma,
                a: config.facet_alpha,
            }
        }
        _ => {
            // Custom Static Color with Transparency
            let mut c = config.base_color;
            c.a = config.facet_alpha;
            c
        }
    }
}

fn compute_rib_color(index: usize, total: usize, time: f32, config: &HatConfig) -> ColorRGBA {
    match config.color_mode {
        1 => {
            let t = index as f32 / total as f32;
            let hue = (time * config.wave_speed * 0.35 + t * 0.8 + 0.05) % 1.0;
            ColorRGBA::from_hsv(hue, 0.95, 1.0, config.rib_alpha)
        }
        2 => {
            let t = index as f32 / total as f32;
            let phase = time * config.wave_speed * 2.5 + t * 4.0;
            let wave = (phase.sin() + 1.0) * 0.5;
            let luma = (0.55 + wave * 0.45).clamp(0.0, 1.0);
            ColorRGBA {
                r: (config.base_color.r * luma + 0.15).clamp(0.0, 1.0),
                g: (config.base_color.g * luma + 0.15).clamp(0.0, 1.0),
                b: (config.base_color.b * luma + 0.15).clamp(0.0, 1.0),
                a: config.rib_alpha,
            }
        }
        _ => {
            ColorRGBA {
                r: (config.base_color.r * 1.3).clamp(0.0, 1.0),
                g: (config.base_color.g * 1.3).clamp(0.0, 1.0),
                b: (config.base_color.b * 1.3).clamp(0.0, 1.0),
                a: config.rib_alpha,
            }
        }
    }
}

fn compute_brim_color(time: f32, config: &HatConfig) -> ColorRGBA {
    match config.color_mode {
        1 => {
            let hue = (time * config.wave_speed * 0.35) % 1.0;
            ColorRGBA::from_hsv(hue, 1.0, 1.0, config.rib_alpha)
        }
        2 => {
            let wave = (time * config.wave_speed * 2.5).sin() * 0.2 + 0.8;
            ColorRGBA {
                r: (config.base_color.r * wave).clamp(0.0, 1.0),
                g: (config.base_color.g * wave).clamp(0.0, 1.0),
                b: (config.base_color.b * wave).clamp(0.0, 1.0),
                a: config.rib_alpha,
            }
        }
        _ => {
            let mut c = config.base_color;
            c.a = config.rib_alpha;
            c
        }
    }
}
