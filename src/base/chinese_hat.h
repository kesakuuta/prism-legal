#ifndef BASE_CHINESE_HAT_H
#define BASE_CHINESE_HAT_H

#include <base/color.h>
#include <base/vmath.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ChineseHatConfig
{
	bool m_Enabled;
	int m_Mode;          // 0 = static, 1 = rainbow, 2 = shimmering monochrome
	ColorRGBA m_Color;
	ColorRGBA m_RibColor;
	float m_Alpha;       // 0.0 .. 1.0
	float m_RibAlpha;    // 0.0 .. 1.0
	float m_Speed;       // animation speed multiplier
	float m_Scale;       // size scale (1.0 = 100%)
	float m_OffsetY;     // vertical offset
	bool m_Tilt;         // tilt with movement & aim
};

struct HatPoint
{
	float x;
	float y;
};

struct HatFacet
{
	HatPoint m_Apex;
	HatPoint m_Left;
	HatPoint m_Right;
	ColorRGBA m_Color;
};

struct HatLine
{
	HatPoint m_From;
	HatPoint m_To;
	ColorRGBA m_Color;
};

#define MAX_HAT_FACETS 12
#define MAX_HAT_LINES 32

struct ChineseHatMesh
{
	size_t m_FacetCount;
	HatFacet m_aFacets[MAX_HAT_FACETS];
	size_t m_LineCount;
	HatLine m_aLines[MAX_HAT_LINES];
};

void generate_chinese_hat_mesh(
	float body_pos_x,
	float body_pos_y,
	float body_angle,
	float aim_dir_x,
	float aim_dir_y,
	float base_size,
	float anim_time_sec,
	const ChineseHatConfig *pConfig,
	ChineseHatMesh *pOutMesh);

#ifdef __cplusplus
}
#endif

#endif // BASE_CHINESE_HAT_H
