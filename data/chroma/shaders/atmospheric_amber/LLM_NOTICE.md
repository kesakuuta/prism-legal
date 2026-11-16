# LLM_NOTICE: Shaderpack Architecture & Developer Specification

> **Target Engine:** `chroma.` client (DDRaceNetwork / DDNet 20.3 & Teeworlds fork)  
> **Pipeline Target:** Fullscreen Quad FBO Post-Processing  
> **GLSL Version:** OpenGL 3.3 Core Profile (`#version 330 core`)  
> **Compatibility:** OpenGL 3.3+, OpenGL ES 3.0, Vulkan SPIR-V (via glslang)

---

## 1. Pipeline Architecture

1. **Offscreen FBO Capture:** The client renders all world layers (background, quads, map tiles, entities, Tees, weapons, hooks, particles) into an offscreen Framebuffer Object texture (`u_ScreenTexture`).
2. **Post-Processing Pass:** A fullscreen textured quad covering normalized device coordinates `[-1, -1]` to `[1, 1]` is rasterized using `atmospheric_glow.vert` and `atmospheric_glow.frag`.
3. **Swapchain Presentation:** The resulting composite buffer is presented on the display swapchain.

---

## 2. Uniform Dictionary & Contract

| Uniform Name | Type | Range / Format | Description & Role |
| :--- | :--- | :--- | :--- |
| `u_ScreenTexture` | `sampler2D` | Texture unit 0 | Raw rendered scene framebuffer. |
| `u_Resolution` | `vec2` | Pixels (e.g. `1920, 1080`) | Viewport dimensions for aspect ratio correction. |
| `u_PlayerPos` | `vec2` | `[0.0, 1.0]` | Normalized screen position of local player Tee. Fallback: `(0.5, 0.5)`. |
| `u_Time` | `float` | Seconds (`>= 0.0`) | Monotonic engine timer driving fog drift and dust sparkle. |
| `u_LightRadius` | `float` | `0.15` – `1.0` | Radius of the warm point-light bubble (default: `0.45`). |
| `u_LightIntensity` | `float` | `0.5` – `3.0` | Core brightness multiplier (default: `1.35`). |
| `u_LightColor` | `vec3` | RGB `[0.0, 1.0]` | Tint color of light and motes (default: `[1.0, 0.78, 0.45]`). |
| `u_AmbientDark` | `float` | `0.02` – `0.35` | Ambient light multiplier in unlit areas (default: `0.12`). |
| `u_FogDensity` | `float` | `0.0` – `1.0` | Opacity of procedural drifting fog (default: `0.35`). |
| `u_DustIntensity` | `float` | `0.0` – `2.0` | Brightness of glittering floating dust motes (default: `0.65`). |
| `u_VignettePower` | `float` | `0.0` – `2.0` | Exponent curve for edge/corner darkening (default: `0.85`). |

---

## 3. Algorithmic Breakdown

* **Aspect Ratio Correction:**  
  `vec2 delta = (uv - u_PlayerPos) * vec2(u_Resolution.x / u_Resolution.y, 1.0);`  
  Guarantees circular light bubbles on widescreen and ultrawide displays.
* **Point Light Falloff:**  
  Inverse-quadratic decay `1.0 / (1.0 + (dist / radius)^2 * 4.0)` combined with `smoothstep(radius * 1.8, 0.0, dist)` to eliminate sharp cutoff borders.
* **Volumetric Fog (fBm):**  
  3-octave Fractional Brownian Motion (`fbm`) driven by `noise()` and scrolling UVs (`u_Time * 0.04, -u_Time * 0.02`).
* **Dust Motes Engine (`renderDustMotes`):**  
  Cellular grid sampling with sinusoidal time pulsation and soft circular falloff. Particles smoothly drift upward with subtle lateral sway.
* **Tone Mapping (S-Curve):**  
  `pow(rgb, 1.15)` for deeper charcoal shadows, followed by filmic shoulder compression `rgb / (rgb + 0.15) * 1.15`.

---

## 4. Extension & Modification Recipes (For AI & Developers)

1. **Cyberpunk / Neon Mode:**  
   Set `u_LightColor = vec3(0.55, 0.15, 1.0)` (neon purple) or `vec3(0.0, 0.85, 1.0)` (cyan).
2. **Directional Flashlight:**  
   Pass uniform `vec2 u_AimDir;` and modulate `lightMask` with `max(0.0, dot(normalize(-delta), u_AimDir))`.
3. **Chromatic Aberration:**  
   Sample `u_ScreenTexture` with offset per channel:
   ```glsl
   float r = texture(u_ScreenTexture, uv + delta * 0.003).r;
   float g = texture(u_ScreenTexture, uv).g;
   float b = texture(u_ScreenTexture, uv - delta * 0.003).b;
   ```
4. **Low-End GPU Optimization:**  
   Reduce `fbm` to 2 octaves and decrease dust grid density from `36.0` to `24.0`.
