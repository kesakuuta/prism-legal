#include "prism_core_rust.h"
#include <cmath>
#include <algorithm>

// Fallback implementations if Rust static library is linked weakly or compiled with -DNO_RUST
#if defined(NO_RUST)

extern "C" {

uint32_t prism_rust_version()
{
	return 20300;
}

static inline ColorRGBARust HsvToRgbaFallback(float h, float s, float v, float a)
{
	float h_scaled = std::fmod(std::fmod(h, 1.0f) + 1.0f, 1.0f) * 6.0f;
	int i = static_cast<int>(std::floor(h_scaled));
	float f = h_scaled - i;
	float p = v * (1.0f - s);
	float q = v * (1.0f - s * f);
	float t = v * (1.0f - s * (1.0f - f));
	float r = 0, g = 0, b = 0;
	switch(i % 6)
	{
	case 0: r = v; g = t; b = p; break;
	case 1: r = q; g = v; b = p; break;
	case 2: r = p; g = v; b = t; break;
	case 3: r = p; g = q; b = v; break;
	case 4: r = t; g = p; b = v; break;
	default: r = v; g = p; b = q; break;
	}
	return ColorRGBARust{r, g, b, a};
}

bool prism_chinese_hat_generate(
	float tee_x,
	float tee_y,
	float tee_radius,
	float dir_angle,
	float time_seconds,
	const HatConfigRust *config,
	HatGeometryRust *out_geom)
{
	if(!config || !out_geom || !config->enabled)
		return false;

	const float scale = (tee_radius / 28.0f) * config->scale;
	const float base_r_x = 36.0f * scale;
	const float base_r_y = 10.5f * scale;
	const float cone_h = 24.0f * scale;
	const float total_tilt = config->tilt + std::sin(dir_angle) * 0.12f;

	HatPointRust center{tee_x, tee_y + config->y_offset * (tee_radius / 28.0f)};
	HatPointRust apex{center.x + total_tilt * 4.0f, center.y - cone_h};

	const float pi = 3.1415926535f;
	for(int i = 0; i <= 8; ++i)
	{
		float t = static_cast<float>(i) / 8.0f;
		float angle = total_tilt * 0.5f + t * pi;
		float bx = -std::cos(angle) * base_r_x;
		float by = std::sin(angle) * base_r_y * 0.45f;
		float cos_tilt = std::cos(total_tilt);
		float sin_tilt = std::sin(total_tilt);
		out_geom->brim_points[i] = HatPointRust{
			center.x + bx * cos_tilt - by * sin_tilt,
			center.y + bx * sin_tilt + by * cos_tilt
		};
	}

	for(int i = 0; i < 8; ++i)
	{
		ColorRGBARust col = config->base_color;
		col.a = config->facet_alpha;
		if(config->color_mode == 1)
		{
			float t = static_cast<float>(i) / 8.0f;
			float hue = std::fmod(time_seconds * config->wave_speed * 0.35f + t * 0.8f, 1.0f);
			col = HsvToRgbaFallback(hue, 0.85f, 0.95f, config->facet_alpha);
		}
		out_geom->facets[i].v0 = HatVertexRust{apex, col};
		out_geom->facets[i].v1 = HatVertexRust{out_geom->brim_points[i], col};
		out_geom->facets[i].v2 = HatVertexRust{out_geom->brim_points[i + 1], col};

		ColorRGBARust rib_col = col;
		rib_col.a = config->rib_alpha;
		out_geom->ribs[i] = HatRibRust{apex, out_geom->brim_points[i], rib_col};
	}

	out_geom->brim_color = config->base_color;
	out_geom->brim_color.a = config->rib_alpha;
	return true;
}

bool prism_theme_get_palette(
	int32_t preset_index,
	float custom_r,
	float custom_g,
	float custom_b,
	float custom_a,
	ThemePaletteRust *out_palette)
{
	if(!out_palette) return false;
	out_palette->primary = ColorRGBARust{custom_r, custom_g, custom_b, custom_a};
	out_palette->secondary = ColorRGBARust{custom_r * 0.7f, custom_g * 0.7f, custom_b * 0.7f, 1.0f};
	out_palette->accent = ColorRGBARust{std::min(custom_r * 1.3f, 1.0f), std::min(custom_g * 1.3f, 1.0f), std::min(custom_b * 1.3f, 1.0f), 1.0f};
	out_palette->background = ColorRGBARust{0.08f, 0.08f, 0.08f, 0.95f};
	out_palette->surface = ColorRGBARust{0.14f, 0.14f, 0.14f, 0.85f};
	out_palette->border = ColorRGBARust{custom_r, custom_g, custom_b, 0.40f};
	out_palette->text_bright = ColorRGBARust{1.0f, 1.0f, 1.0f, 1.0f};
	out_palette->text_muted = ColorRGBARust{0.7f, 0.7f, 0.7f, 1.0f};
	return true;
}

bool prism_hud_calculate_speedometer(
	float vel_x,
	float vel_y,
	SpeedometerDataRust *out_data)
{
	if(!out_data) return false;
	out_data->raw_speed = std::sqrt(vel_x * vel_x + vel_y * vel_y);
	out_data->speed_tiles_per_sec = (out_data->raw_speed * 50.0f) / 32.0f;
	out_data->speed_km_h = out_data->speed_tiles_per_sec * 3.6f;
	out_data->speed_mph = out_data->speed_km_h * 0.621371f;
	out_data->heading_angle = std::atan2(vel_y, vel_x);
	return true;
}

bool prism_tclient_rainbow_color(
	float time_seconds,
	float offset,
	const RainbowConfigRust *config,
	ColorRGBARust *out_color)
{
	if(!config || !out_color) return false;
	if(!config->enabled) { *out_color = ColorRGBARust{1.0f, 1.0f, 1.0f, 1.0f}; return true; }
	float hue = std::fmod(time_seconds * config->speed * 0.2f + offset * config->spread, 1.0f);
	*out_color = HsvToRgbaFallback(hue, config->saturation, config->lightness, config->alpha);
	return true;
}

bool prism_tclient_pet_update(
	PetStateRust *state,
	float owner_x,
	float owner_y,
	float vel_x,
	float vel_y,
	float delta_time,
	float time_seconds,
	const PetConfigRust *config)
{
	if(!state || !config || !config->enabled) return false;
	const float pi = 3.1415926535f;
	state->orbit_angle = std::fmod(state->orbit_angle + config->orbit_speed * delta_time, pi * 2.0f);
	float bob = std::sin(time_seconds * config->bobbing_frequency) * config->bobbing_amplitude;
	float target_x = owner_x + std::cos(state->orbit_angle) * config->orbit_radius + vel_x * 0.12f;
	float target_y = owner_y + std::sin(state->orbit_angle) * (config->orbit_radius * 0.4f) - 30.0f + bob + vel_y * 0.12f;
	float dx = state->pos.x - target_x;
	float dy = state->pos.y - target_y;
	state->vel.x += (-config->spring_stiffness * dx - config->damping * state->vel.x) * delta_time;
	state->vel.y += (-config->spring_stiffness * dy - config->damping * state->vel.y) * delta_time;
	state->pos.x += state->vel.x * delta_time;
	state->pos.y += state->vel.y * delta_time;
	return true;
}

} // extern "C"

#endif // NO_RUST
