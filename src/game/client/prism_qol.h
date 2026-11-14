// Prism QoL and Modular HUD geometry. Distributed under the zlib license in license.txt.
#ifndef GAME_CLIENT_PRISM_QOL_H
#define GAME_CLIENT_PRISM_QOL_H

#include <algorithm>
#include <array>
#include <cmath>

namespace PrismQol
{
inline float HudCoordinate(int Normalized, float Extent, float ElementExtent)
{
	const float MaxOrigin = std::max(0.0f, Extent - ElementExtent);
	return std::clamp(Normalized / 10000.0f * MaxOrigin, 0.0f, MaxOrigin);
}

inline int HudNormalize(float Position, float Extent)
{
	if(Extent <= 0.0f)
		return 0;
	return std::clamp(static_cast<int>(std::round(Position / Extent * 10000.0f)), 0, 10000);
}

static constexpr int NUM_HUD_MODULES = 7;
enum EHudModule { HUD_HOTKEYS, HUD_IDENTITY, HUD_PERFORMANCE, HUD_DUMMY, HUD_STAFF, HUD_INPUT, HUD_EFFECTS };
enum EHudPreset { HUD_MINIMAL, HUD_STREAMING, HUD_COMPETITIVE, HUD_CUSTOM };

struct SHudPreset
{
	std::array<int, NUM_HUD_MODULES> m_aEnabled{};
	std::array<int, NUM_HUD_MODULES> m_aX{{300, 300, 8200, 300, 8200, 300, 7600}};
	std::array<int, NUM_HUD_MODULES> m_aY{{900, 250, 250, 3200, 1800, 7500, 1800}};
};

inline SHudPreset HudPreset(int Preset)
{
	SHudPreset Result;
	if(Preset == HUD_MINIMAL)
		Result.m_aEnabled = {{0, 1, 1, 0, 0, 0, 0}};
	else if(Preset == HUD_STREAMING)
		Result.m_aEnabled = {{0, 1, 0, 0, 0, 1, 0}};
	else if(Preset == HUD_COMPETITIVE)
		Result.m_aEnabled = {{1, 1, 1, 1, 0, 1, 0}};
	else
		Result.m_aEnabled = {{1, 1, 0, 1, 0, 0, 0}};
	return Result;
}

inline float HudSnap(float Position, float Extent, float ElementExtent, float Threshold = 4.0f)
{
	const float MaxOrigin = std::max(0.0f, Extent - ElementExtent);
	if(std::abs(Position) <= Threshold)
		return 0.0f;
	if(std::abs(Position - MaxOrigin) <= Threshold)
		return MaxOrigin;
	const float Center = (Extent - ElementExtent) / 2.0f;
	if(std::abs(Position - Center) <= Threshold)
		return Center;
	return Position;
}

inline void HudModuleExtent(int Module, int GlobalScale, int ModuleScale, int Padding, int FontSize, int KeySize, float &Width, float &Height)
{
	const float Scale = std::clamp(GlobalScale, 65, 150) * std::clamp(ModuleScale, 65, 150) / 10000.0f;
	const float Font = std::clamp(FontSize, 5, 12) * Scale;
	const float Row = Font + 4.0f * Scale;
	const float Pad = std::clamp(Padding, 0, 16) * Scale;
	const float Key = std::clamp(KeySize, 12, 32) * Scale;
	const int aRows[NUM_HUD_MODULES] = {6, 1, 2, 2, 2, 0, 7};
	Module = std::clamp(Module, 0, NUM_HUD_MODULES - 1);
	Width = (Module == HUD_INPUT ? Key * 3.0f + 4.0f : Font * (Module == HUD_IDENTITY ? 19.0f : 24.0f)) + Pad * 2.0f;
	Height = Module == HUD_INPUT ? Key * 2.0f + 2.0f + Row + Pad * 2.0f : Row * aRows[Module] + Pad * 2.0f;
}
}

#endif // GAME_CLIENT_PRISM_QOL_H
