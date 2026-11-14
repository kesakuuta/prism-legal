#include "chinese_hat.h"

#if defined(NO_RUST)

#include <base/color.h>
#include <base/vmath.h>

#include <algorithm>
#include <cmath>

static inline ColorRGBA HsvToRgba(float h, float s, float v, float a)
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
	return ColorRGBA(r, g, b, a);
}

static inline HatPoint TransformPoint(HatPoint pt, float cos_t, float sin_t, HatPoint center)
{
	HatPoint out;
	out.x = center.x + pt.x * cos_t - pt.y * sin_t;
	out.y = center.y + pt.x * sin_t + pt.y * cos_t;
	return out;
}

extern "C" void generate_chinese_hat_mesh(
	float body_pos_x,
	float body_pos_y,
	float body_angle,
	float aim_dir_x,
	float aim_dir_y,
	float base_size,
	float anim_time_sec,
	const ChineseHatConfig *pConfig,
	ChineseHatMesh *pOutMesh)
{
	if(!pConfig || !pOutMesh || !pConfig->m_Enabled)
	{
		if(pOutMesh)
		{
			pOutMesh->m_FacetCount = 0;
			pOutMesh->m_LineCount = 0;
		}
		return;
	}

	const float Scale = std::clamp(pConfig->m_Scale, 0.4f, 2.5f);
	const float HatWidth = base_size * 1.45f * Scale;
	const float HatHeight = base_size * 0.58f * Scale;
	const float HatSag = HatHeight * 0.16f; // Parabolic arc along the brim

	// Dynamic tilt
	float TotalAngle = body_angle * 2.0f * pi * 0.65f;
	if(pConfig->m_Tilt)
	{
		float AimLen = std::sqrt(aim_dir_x * aim_dir_x + aim_dir_y * aim_dir_y);
		if(AimLen > 0.001f)
		{
			float AimNormX = aim_dir_x / AimLen;
			TotalAngle += AimNormX * 0.14f;
		}
	}

	const float CosT = std::cos(TotalAngle);
	const float SinT = std::sin(TotalAngle);

	// Anchor resting on top of the Tee's head
	HatPoint Anchor;
	Anchor.x = body_pos_x;
	Anchor.y = body_pos_y - base_size * 0.28f + pConfig->m_OffsetY;

	HatPoint LocalApex = {0.0f, -HatHeight};
	HatPoint WorldApex = TransformPoint(LocalApex, CosT, SinT, Anchor);

	// 8 triangular facets (9 rim points)
	const int NumSegments = 8;
	HatPoint aWorldRim[NumSegments + 1];

	for(int i = 0; i <= NumSegments; ++i)
	{
		float t = static_cast<float>(i) / NumSegments;
		float lx = -HatWidth * 0.5f + HatWidth * t;
		float ArcFactor = 1.0f - (2.0f * t - 1.0f) * (2.0f * t - 1.0f);
		float ly = ArcFactor * HatSag;
		aWorldRim[i] = TransformPoint({lx, ly}, CosT, SinT, Anchor);
	}

	const float Speed = std::clamp(pConfig->m_Speed, 0.1f, 10.0f);
	pOutMesh->m_FacetCount = NumSegments;

	for(int i = 0; i < NumSegments; ++i)
	{
		float t = (i + 0.5f) / NumSegments;
		ColorRGBA FacetColor;

		if(pConfig->m_Mode == 1) // Rainbow flowing gradient wave
		{
			float Hue = std::fmod(anim_time_sec * Speed * 0.35f + t * 0.40f, 1.0f);
			FacetColor = HsvToRgba(Hue, 0.88f, 0.98f, pConfig->m_Alpha);
		}
		else if(pConfig->m_Mode == 2) // Shimmering monochrome luster wave
		{
			float Phase = 2.0f * pi * (anim_time_sec * Speed * 0.80f - t * 0.75f);
			float Shimmer = 0.5f + 0.5f * std::sin(Phase);
			float r = std::min(1.0f, pConfig->m_Color.r * (0.65f + 0.35f * Shimmer) + 0.30f * Shimmer);
			float g = std::min(1.0f, pConfig->m_Color.g * (0.65f + 0.35f * Shimmer) + 0.30f * Shimmer);
			float b = std::min(1.0f, pConfig->m_Color.b * (0.65f + 0.35f * Shimmer) + 0.30f * Shimmer);
			FacetColor = ColorRGBA(r, g, b, pConfig->m_Alpha);
		}
		else // Mode 0: Custom static color with subtle 3D facet shading
		{
			float Shade = 0.92f + 0.16f * (i % 2);
			FacetColor = ColorRGBA(
				std::min(1.0f, pConfig->m_Color.r * Shade),
				std::min(1.0f, pConfig->m_Color.g * Shade),
				std::min(1.0f, pConfig->m_Color.b * Shade),
				pConfig->m_Alpha);
		}

		pOutMesh->m_aFacets[i].m_Apex = WorldApex;
		pOutMesh->m_aFacets[i].m_Left = aWorldRim[i];
		pOutMesh->m_aFacets[i].m_Right = aWorldRim[i + 1];
		pOutMesh->m_aFacets[i].m_Color = FacetColor;
	}

	size_t LineIdx = 0;
	ColorRGBA RibCol(pConfig->m_RibColor.r, pConfig->m_RibColor.g, pConfig->m_RibColor.b, pConfig->m_RibAlpha);

	// Radial ribs from Apex to every rim vertex
	for(int i = 0; i <= NumSegments && LineIdx < MAX_HAT_LINES; ++i)
	{
		pOutMesh->m_aLines[LineIdx].m_From = WorldApex;
		pOutMesh->m_aLines[LineIdx].m_To = aWorldRim[i];
		pOutMesh->m_aLines[LineIdx].m_Color = RibCol;
		++LineIdx;
	}

	// Brim rim segments
	for(int i = 0; i < NumSegments && LineIdx < MAX_HAT_LINES; ++i)
	{
		pOutMesh->m_aLines[LineIdx].m_From = aWorldRim[i];
		pOutMesh->m_aLines[LineIdx].m_To = aWorldRim[i + 1];
		pOutMesh->m_aLines[LineIdx].m_Color = RibCol;
		++LineIdx;
	}

	// Silhouette slopes
	if(LineIdx + 1 < MAX_HAT_LINES)
	{
		pOutMesh->m_aLines[LineIdx].m_From = WorldApex;
		pOutMesh->m_aLines[LineIdx].m_To = aWorldRim[0];
		pOutMesh->m_aLines[LineIdx].m_Color = RibCol;
		++LineIdx;

		pOutMesh->m_aLines[LineIdx].m_From = WorldApex;
		pOutMesh->m_aLines[LineIdx].m_To = aWorldRim[NumSegments];
		pOutMesh->m_aLines[LineIdx].m_Color = RibCol;
		++LineIdx;
	}

	pOutMesh->m_LineCount = LineIdx;
}

#endif // NO_RUST
