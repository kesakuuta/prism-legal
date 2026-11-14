#ifndef BASE_PRISM_CORE_RUST_H
#define BASE_PRISM_CORE_RUST_H

#include <base/color.h>
#include <base/vmath.h>

#ifdef __cplusplus
extern "C" {
#endif

// Version check
uint32_t prism_rust_version();

// Chinese Hat
struct HatPointRust
{
	float x;
	float y;
};

struct ColorRGBARust
{
	float r;
	float g;
	float b;
	float a;
};

struct HatVertexRust
{
	HatPointRust pos;
	ColorRGBARust color;
};

struct HatConfigRust
{
	bool enabled;
	int32_t color_mode;
	ColorRGBARust base_color;
	float facet_alpha;
	float rib_alpha;
	float wave_speed;
	float scale;
	float y_offset;
	float tilt;
};

struct HatFacetRust
{
	HatVertexRust v0;
	HatVertexRust v1;
	HatVertexRust v2;
};

struct HatRibRust
{
	HatPointRust start;
	HatPointRust end;
	ColorRGBARust color;
};

struct HatGeometryRust
{
	HatFacetRust facets[8];
	HatRibRust ribs[8];
	HatPointRust brim_points[9];
	ColorRGBARust brim_color;
};

bool prism_chinese_hat_generate(
	float tee_x,
	float tee_y,
	float tee_radius,
	float dir_angle,
	float time_seconds,
	const HatConfigRust *config,
	HatGeometryRust *out_geom);

// Theme Engine
struct ThemePaletteRust
{
	ColorRGBARust primary;
	ColorRGBARust secondary;
	ColorRGBARust accent;
	ColorRGBARust background;
	ColorRGBARust surface;
	ColorRGBARust border;
	ColorRGBARust text_bright;
	ColorRGBARust text_muted;
};

bool prism_theme_get_palette(
	int32_t preset_index,
	float custom_r,
	float custom_g,
	float custom_b,
	float custom_a,
	ThemePaletteRust *out_palette);

// HUD Engine
struct SpeedometerDataRust
{
	float raw_speed;
	float speed_tiles_per_sec;
	float speed_km_h;
	float speed_mph;
	float heading_angle;
};

bool prism_hud_calculate_speedometer(
	float vel_x,
	float vel_y,
	SpeedometerDataRust *out_data);

// T-Client Rainbow Engine
struct RainbowConfigRust
{
	bool enabled;
	float speed;
	float saturation;
	float lightness;
	float alpha;
	float spread;
};

bool prism_tclient_rainbow_color(
	float time_seconds,
	float offset,
	const RainbowConfigRust *config,
	ColorRGBARust *out_color);

// T-Client Pet Physics Engine
struct PetStateRust
{
	HatPointRust pos;
	HatPointRust vel;
	float orbit_angle;
	HatPointRust eye_dir;
	ColorRGBARust color;
};

struct PetConfigRust
{
	bool enabled;
	float orbit_radius;
	float orbit_speed;
	float spring_stiffness;
	float damping;
	float bobbing_amplitude;
	float bobbing_frequency;
	float scale;
};

bool prism_tclient_pet_update(
	PetStateRust *state,
	float owner_x,
	float owner_y,
	float vel_x,
	float vel_y,
	float delta_time,
	float time_seconds,
	const PetConfigRust *config);

#ifdef __cplusplus
}
#endif

#endif // BASE_PRISM_CORE_RUST_H
