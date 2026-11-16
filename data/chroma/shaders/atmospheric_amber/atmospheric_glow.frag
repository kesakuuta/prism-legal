#version 330 core

// Fullscreen quad texture coordinates
in vec2 oTexCoord;
out vec4 FragColor;

// Scene buffer and uniforms
uniform sampler2D u_ScreenTexture;  // Rendered game scene
uniform vec2      u_Resolution;     // Viewport resolution (e.g. 1920x1080)
uniform vec2      u_PlayerPos;      // Normalized Tee screen position [0.0 - 1.0]
uniform float     u_Time;           // Elapsed time in seconds

// Atmosphere parameters
uniform float     u_LightRadius;    // Light bubble radius (~0.45)
uniform float     u_LightIntensity; // Core brightness (~1.35)
uniform vec3      u_LightColor;     // Warm amber: vec3(1.0, 0.78, 0.45)
uniform float     u_AmbientDark;    // Depth of shadow ambiance (~0.12)
uniform float     u_FogDensity;     // Volumetric fog density (~0.35)
uniform float     u_DustIntensity;  // Floating dust motes density (~0.65)
uniform float     u_VignettePower;  // Edge vignette darkness (~0.85)

// --- Procedural Noise & Particle Generators ---

float hash21(vec2 p)
{
    p = fract(p * vec2(234.34, 435.345));
    p += dot(p, p + 34.23);
    return fract(p.x * p.y);
}

float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = hash21(i);
    float b = hash21(i + vec2(1.0, 0.0));
    float c = hash21(i + vec2(0.0, 1.0));
    float d = hash21(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

// 3-Octave Fractional Brownian Motion for drifting fog
float fbm(vec2 p)
{
    float v = 0.0;
    v += 0.500 * noise(p); p *= 2.02;
    v += 0.250 * noise(p); p *= 2.03;
    v += 0.125 * noise(p);
    return v;
}

// Procedural floating dust motes with soft bokeh and shimmer
float renderDustMotes(vec2 uv, float time)
{
    float dust = 0.0;
    vec2 st = uv * 36.0;
    
    // Upward drift and subtle lateral swaying
    st.y -= time * 0.75;
    st.x += sin(st.y * 0.35 + time * 0.5) * 0.35;
    
    vec2 i_st = floor(st);
    vec2 f_st = fract(st);
    
    for(int y = -1; y <= 1; y++)
    {
        for(int x = -1; x <= 1; x++)
        {
            vec2 neighbor = vec2(float(x), float(y));
            vec2 point = vec2(hash21(i_st + neighbor), hash21(i_st + neighbor + 17.8));
            
            // Sparkle and pulsation over time
            float sparkle = sin(time * 3.2 + point.x * 6.28) * 0.5 + 0.5;
            vec2 diff = neighbor + point - f_st;
            float dist = length(diff);
            
            // Soft circular particle core
            dust += smoothstep(0.18, 0.01, dist) * sparkle * 0.85;
        }
    }
    return dust;
}

void main()
{
    vec2 uv = oTexCoord;
    vec2 aspect = vec2(u_Resolution.x / u_Resolution.y, 1.0);
    
    vec4 sceneColor = texture(u_ScreenTexture, uv);

    // 1. Distance to player
    vec2 lightOrigin = (u_PlayerPos.x > 0.001 || u_PlayerPos.y > 0.001) ? u_PlayerPos : vec2(0.5, 0.5);
    vec2 delta = (uv - lightOrigin) * aspect;
    float distToPlayer = length(delta);

    // 2. Point light falloff
    float radius = (u_LightRadius > 0.01) ? u_LightRadius : 0.45;
    float intensity = (u_LightIntensity > 0.01) ? u_LightIntensity : 1.35;
    vec3 lightColor = (length(u_LightColor) > 0.01) ? u_LightColor : vec3(1.0, 0.78, 0.45);
    float ambientDark = (u_AmbientDark > 0.001) ? u_AmbientDark : 0.12;

    float lightFalloff = 1.0 / (1.0 + (distToPlayer / radius) * (distToPlayer / radius) * 4.0);
    float lightCutoff = smoothstep(radius * 1.8, 0.0, distToPlayer);
    float lightMask = lightFalloff * lightCutoff * intensity;

    // 3. Volumetric fog in light cone
    vec2 fogUv = uv * aspect * 2.5 + vec2(u_Time * 0.04, -u_Time * 0.02);
    float fogDensity = (u_FogDensity > 0.001) ? u_FogDensity : 0.35;
    float fog = fbm(fogUv) * fogDensity * lightMask;

    // 4. Floating golden dust motes
    float dustDensity = (u_DustIntensity > 0.001) ? u_DustIntensity : 0.65;
    float dust = renderDustMotes(uv * aspect, u_Time) * lightMask * dustDensity;

    // 5. Cinematic Vignette
    float vigPower = (u_VignettePower > 0.001) ? u_VignettePower : 0.85;
    float vignette = uv.x * uv.y * (1.0 - uv.x) * (1.0 - uv.y);
    vignette = clamp(pow(16.0 * vignette, 0.25 * vigPower), 0.0, 1.0);

    // 6. Composition
    vec3 baseColor = sceneColor.rgb;
    vec3 litColor = baseColor * lightColor * (lightMask + fog);
    vec3 ambientColor = baseColor * ambientDark;
    vec3 finalRgb = mix(ambientColor, litColor, clamp(lightMask * 1.25, 0.0, 1.0));
    finalRgb += lightColor * dust;
    finalRgb *= vignette;

    // 7. Tone mapping and S-Curve contrast
    finalRgb = pow(finalRgb, vec3(1.15));
    finalRgb = finalRgb / (finalRgb + vec3(0.15)) * 1.15;

    FragColor = vec4(finalRgb, sceneColor.a);
}
