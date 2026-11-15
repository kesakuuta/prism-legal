// Prism additions, distributed under the zlib license in license.txt.
#include "menus.h"
#include <game/client/gameclient.h>

#include <algorithm>
#include <cmath>

#include <engine/client/keyboard.h>
#include <engine/keys.h>
#include <engine/shared/config.h>
#include <game/client/prism.h>
#include <game/client/prism_qol.h>
#include <game/client/prism_theme.h>
#include <game/client/prism_ui.h>
#include <game/client/prism_version.h>
#include <game/client/ui_scrollregion.h>
#include <game/localization.h>

void CMenus::RenderSettingsPrism(CUIRect Screen)
{
	Prism::Validate(g_Config);
	const bool Animate = g_Config.m_PrismAnimations && !g_Config.m_PrismReducedMotion && g_Config.m_PrismThemeAnimation > 0;
	const float Duration = std::max(0.01f, g_Config.m_PrismThemeAnimation / 1000.0f);
	const float Motion = Animate ? 1.0f - std::exp(-std::clamp(Client()->RenderFrameTime(), 0.0f, 0.1f) * 4.0f / Duration) : 1.0f;
	m_PrismTransition += ((m_PrismOpen ? 1.0f : 0.0f) - m_PrismTransition) * Motion;
	if(!m_PrismOpen && m_PrismTransition < 0.01f)
	{
		m_PrismTransition = 0.0f;
		return;
	}

	const float Fade = m_PrismTransition * m_PrismTransition * (3.0f - 2.0f * m_PrismTransition);
	const PrismUi::STheme Theme(g_Config);
	const float Opacity = g_Config.m_PrismPanelOpacity / 100.0f;
	const float Scale = g_Config.m_PrismMenuScale / 100.0f;
	const float BackdropDarkness = g_Config.m_PrismHudEdit && (m_PrismCategory == 2 || m_PrismCategory == 3) ? 0.28f : g_Config.m_PrismGlassDarkness / 100.0f;
	Screen.Draw(ColorRGBA(0.025f, 0.029f, 0.034f, BackdropDarkness * Fade), IGraphics::CORNER_NONE, 0.0f);

	CUIRect Panel = Screen;
	Panel.w = std::max(1.0f, std::min(Screen.w - 24.0f, 620.0f * Scale));
	Panel.h = std::max(1.0f, std::min(Screen.h - 24.0f, 410.0f * Scale));
	const float TravelX = std::max(0.0f, Screen.w - Panel.w);
	const float TravelY = std::max(0.0f, Screen.h - Panel.h);
	Panel.x = Screen.x + TravelX * g_Config.m_PrismMenuX / 10000.0f;
	Panel.y = Screen.y + TravelY * g_Config.m_PrismMenuY / 10000.0f;
	if(m_PrismOpen && !g_Config.m_PrismMenuLock)
	{
		CUIRect Grab = Panel;
		Grab.h = std::min(38.0f, Panel.h);
		Grab.w = std::max(0.0f, Grab.w - 75.0f);
		if(!m_PrismDragging && Ui()->ActiveItem() == nullptr && Ui()->MouseButtonClicked(0) && Ui()->MouseInside(&Grab))
		{
			m_PrismDragging = true;
			m_PrismDragOffset = Ui()->MousePos() - Panel.TopLeft();
			Ui()->SetActiveItem(&m_PrismDragging);
		}
		if(m_PrismDragging && Ui()->MouseButton(0))
		{
			Ui()->CheckActiveItem(&m_PrismDragging);
			float X = std::clamp(Ui()->MouseX() - m_PrismDragOffset.x - Screen.x, 0.0f, TravelX);
			float Y = std::clamp(Ui()->MouseY() - m_PrismDragOffset.y - Screen.y, 0.0f, TravelY);
			if(g_Config.m_PrismMenuSnap)
			{
				X = PrismQol::HudSnap(X, Screen.w, Panel.w, 8.0f);
				Y = PrismQol::HudSnap(Y, Screen.h, Panel.h, 8.0f);
			}
			g_Config.m_PrismMenuX = TravelX > 0 ? std::clamp((int)std::round(X / TravelX * 10000.0f), 0, 10000) : 5000;
			g_Config.m_PrismMenuY = TravelY > 0 ? std::clamp((int)std::round(Y / TravelY * 10000.0f), 0, 10000) : 5000;
			Panel.x = Screen.x + X;
			Panel.y = Screen.y + Y;
		}
		else if(m_PrismDragging)
		{
			m_PrismDragging = false;
			Ui()->SetActiveItem(nullptr);
		}
	}
	else if(m_PrismDragging)
	{
		m_PrismDragging = false;
		Ui()->SetActiveItem(nullptr);
	}
	const float VisualY = (1.0f - Fade) * 9.0f;
	Panel.y += VisualY;
	CUIRect Shadow = Panel;
	Shadow.x -= 8.0f;
	Shadow.y -= 3.0f;
	Shadow.w += 16.0f;
	Shadow.h += 17.0f;
	const float Radius = g_Config.m_PrismThemeRounding;
	Shadow.Draw(ColorRGBA(0.0f, 0.0f, 0.0f, g_Config.m_PrismThemeShadow / 100.0f * Fade), IGraphics::CORNER_ALL, Radius + 3.0f);
	CUIRect Edge = Panel;
	Edge.Margin(-1.0f, &Edge);
	Edge.Draw(Theme.m_Text.WithAlpha(g_Config.m_PrismThemeBorder / 500.0f * Fade), IGraphics::CORNER_ALL, Radius + 1.0f);
	Panel.Draw(Theme.m_Panel.WithAlpha((0.38f + 0.61f * Opacity) * Fade), IGraphics::CORNER_ALL, Radius);
	CUIRect Sheen;
	Panel.HSplitTop(41.0f, &Sheen, nullptr);
	const float Tint = g_Config.m_PrismGlassTint / 100.0f;
	Sheen.Draw4(Theme.m_Accent.WithAlpha(Tint * 0.20f * Fade), Theme.m_Accent.WithAlpha(Tint * 0.07f * Fade),
		Theme.m_Panel.WithAlpha(0.02f * Fade), Theme.m_Panel.WithAlpha(0.02f * Fade), IGraphics::CORNER_T, 11.0f);
	if(m_PrismOpen && (m_PrismCategory == 2 || m_PrismCategory == 3) && g_Config.m_PrismHudEdit && g_Config.m_PrismHudEnabled)
	{
		int *apEnabled[] = {&g_Config.m_PrismHudHotkeys, &g_Config.m_PrismHudIdentity, &g_Config.m_PrismHudPerformance,
			&g_Config.m_PrismHudDummy, &g_Config.m_PrismHudStaff, &g_Config.m_PrismHudInput, &g_Config.m_PrismHudEffects};
		int *apX[] = {&g_Config.m_PrismHudHotkeysX, &g_Config.m_PrismHudIdentityX, &g_Config.m_PrismHudPerformanceX,
			&g_Config.m_PrismHudDummyX, &g_Config.m_PrismHudStaffX, &g_Config.m_PrismHudInputX, &g_Config.m_PrismHudEffectsX};
		int *apY[] = {&g_Config.m_PrismHudHotkeysY, &g_Config.m_PrismHudIdentityY, &g_Config.m_PrismHudPerformanceY,
			&g_Config.m_PrismHudDummyY, &g_Config.m_PrismHudStaffY, &g_Config.m_PrismHudInputY, &g_Config.m_PrismHudEffectsY};
		int *apScale[] = {&g_Config.m_PrismHudHotkeysScale, &g_Config.m_PrismHudIdentityScale,
			&g_Config.m_PrismHudPerformanceScale, &g_Config.m_PrismHudDummyScale, &g_Config.m_PrismHudStaffScale,
			&g_Config.m_PrismHudInputScale, &g_Config.m_PrismHudEffectsScale};
		static int s_aHudDragIds[PrismQol::NUM_HUD_MODULES] = {};
		const float ToUi = Screen.h / 300.0f;
		if(!Ui()->MouseButton(0))
		{
			if(m_PrismHudDragIndex >= 0)
				Ui()->SetActiveItem(nullptr);
			m_PrismHudDragIndex = -1;
		}
		for(int i = PrismQol::NUM_HUD_MODULES - 1; i >= 0; --i)
		{
			if(!*apEnabled[i])
				continue;
			float Width, Height;
			PrismQol::HudModuleExtent(i, g_Config.m_PrismHudScale, *apScale[i], g_Config.m_PrismHudPadding,
				g_Config.m_PrismHudFontSize, g_Config.m_PrismInputKeySize, Width, Height);
			CUIRect Handle;
			Handle.w = std::min(Width * ToUi, Screen.w);
			Handle.h = std::min(Height * ToUi, Screen.h);
			Handle.x = Screen.x + PrismQol::HudCoordinate(*apX[i], Screen.w, Handle.w);
			Handle.y = Screen.y + PrismQol::HudCoordinate(*apY[i], Screen.h, Handle.h);
			if(!g_Config.m_PrismHudLayoutLock && m_PrismHudDragIndex == -1 && Ui()->ActiveItem() == nullptr &&
				Ui()->MouseButtonClicked(0) && !Panel.Inside(Ui()->MousePos()) && Ui()->MouseInside(&Handle))
			{
				m_PrismHudDragIndex = i;
				m_PrismHudDragOffset = Ui()->MousePos() - Handle.TopLeft();
				Ui()->SetActiveItem(&s_aHudDragIds[i]);
			}
			if(m_PrismHudDragIndex == i && Ui()->MouseButton(0) && !g_Config.m_PrismHudLayoutLock)
			{
				Ui()->CheckActiveItem(&s_aHudDragIds[i]);
				float X = std::clamp(Ui()->MouseX() - m_PrismHudDragOffset.x - Screen.x, 0.0f, Screen.w - Handle.w);
				float Y = std::clamp(Ui()->MouseY() - m_PrismHudDragOffset.y - Screen.y, 0.0f, Screen.h - Handle.h);
				if(g_Config.m_PrismHudEdgeSnap)
				{
					X = PrismQol::HudSnap(X, Screen.w, Handle.w, 7.0f);
					Y = PrismQol::HudSnap(Y, Screen.h, Handle.h, 7.0f);
				}
				if(g_Config.m_PrismHudSnap)
				{
					for(int Other = 0; Other < PrismQol::NUM_HUD_MODULES; ++Other)
					{
						if(Other == i || !*apEnabled[Other])
							continue;
						float OtherWidth, OtherHeight;
						PrismQol::HudModuleExtent(Other, g_Config.m_PrismHudScale, *apScale[Other], g_Config.m_PrismHudPadding,
							g_Config.m_PrismHudFontSize, g_Config.m_PrismInputKeySize, OtherWidth, OtherHeight);
						OtherWidth = std::min(OtherWidth * ToUi, Screen.w);
						OtherHeight = std::min(OtherHeight * ToUi, Screen.h);
						const float OtherX = PrismQol::HudCoordinate(*apX[Other], Screen.w, OtherWidth);
						const float OtherY = PrismQol::HudCoordinate(*apY[Other], Screen.h, OtherHeight);
						const float GuidesX[] = {OtherX, OtherX + (OtherWidth - Handle.w) / 2.0f, OtherX + OtherWidth - Handle.w};
						const float GuidesY[] = {OtherY, OtherY + (OtherHeight - Handle.h) / 2.0f, OtherY + OtherHeight - Handle.h};
						for(float Guide : GuidesX)
							if(std::abs(X - Guide) < 4.0f)
								X = std::clamp(Guide, 0.0f, Screen.w - Handle.w);
						for(float Guide : GuidesY)
							if(std::abs(Y - Guide) < 4.0f)
								Y = std::clamp(Guide, 0.0f, Screen.h - Handle.h);
					}
				}
				*apX[i] = PrismQol::HudNormalize(X, Screen.w);
				*apY[i] = PrismQol::HudNormalize(Y, Screen.h);
				g_Config.m_PrismHudPreset = PrismQol::HUD_CUSTOM;
				Handle.x = Screen.x + PrismQol::HudCoordinate(*apX[i], Screen.w, Handle.w);
				Handle.y = Screen.y + PrismQol::HudCoordinate(*apY[i], Screen.h, Handle.h);
			}
			const ColorRGBA OutlineColor = Theme.m_Accent.WithAlpha(m_PrismHudDragIndex == i ? 0.75f : 0.38f);
			const float Stroke = 1.0f;
			CUIRect Border = {Handle.x, Handle.y, Handle.w, Stroke};
			Border.Draw(OutlineColor, IGraphics::CORNER_NONE, 0.0f);
			Border.y = Handle.y + Handle.h - Stroke;
			Border.Draw(OutlineColor, IGraphics::CORNER_NONE, 0.0f);
			Border = {Handle.x, Handle.y, Stroke, Handle.h};
			Border.Draw(OutlineColor, IGraphics::CORNER_NONE, 0.0f);
			Border.x = Handle.x + Handle.w - Stroke;
			Border.Draw(OutlineColor, IGraphics::CORNER_NONE, 0.0f);
		}
	}
	else
		m_PrismHudDragIndex = -1;

	// Closing is purely visual: release interactive items immediately.
	if(!m_PrismOpen)
		return;

	CUIRect Inner = Panel;
	Inner.Margin(11.0f, &Inner);
	CUIRect Header, Body, Footer;
	Inner.HSplitTop(35.0f, &Header, &Inner);
	Inner.HSplitBottom(19.0f, &Body, &Footer);
	CUIRect Title, Close;
	Header.VSplitRight(67.0f, &Title, &Close);
	Ui()->DoLabel(&Title, "PRISM", 15.0f, TEXTALIGN_ML);
	static CButtonContainer s_Close;
	if(DoButton_Menu(&s_Close, "Close", 0, &Close, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 6.0f, 0.46f, ColorRGBA(0.35f, 0.43f, 0.51f, 0.22f)))
	{
		Ui()->ClosePopupMenus();
		m_PrismOpen = false;
	}

	CUIRect Sidebar, Content;
	Body.VSplitLeft(std::min(116.0f * Scale, Body.w * 0.25f), &Sidebar, &Content);
	Sidebar.VSplitRight(7.0f, &Sidebar, nullptr);
	Sidebar.Draw(Theme.m_Background.WithAlpha(0.91f * Fade), IGraphics::CORNER_ALL, 8.0f);
	Sidebar.Margin(6.0f, &Sidebar);
	Content.Draw(Theme.m_Background.WithAlpha(0.83f * Fade), IGraphics::CORNER_ALL, 8.0f);
	Content.Margin(8.0f, &Content);

	static const char *s_apTabs[] = {"Visuals", "Hat", "HUD", "Input", "Themes", "Settings"};
	static CButtonContainer s_aTabs[6];
	static float s_aTabBlend[6] = {};
	for(int i = 0; i < 6; ++i)
	{
		CUIRect Tab;
		Sidebar.HSplitTop(29.0f, &Tab, &Sidebar);
		Sidebar.HSplitTop(3.0f, nullptr, &Sidebar);
		const float Target = m_PrismCategory == i ? 1.0f : 0.0f;
		s_aTabBlend[i] += (Target - s_aTabBlend[i]) * Motion;
		const float Highlight = s_aTabBlend[i];
		const ColorRGBA ButtonColor(Theme.m_Accent.r, Theme.m_Accent.g, Theme.m_Accent.b, 0.06f + Highlight * 0.36f);
		if(DoButton_Menu(&s_aTabs[i], s_apTabs[i], 0, &Tab, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 6.0f, 0.41f, ButtonColor))
		{
			m_PrismCategory = i;
			Ui()->SetActiveItem(nullptr);
		}
	}

	static CScrollRegion s_aScroll[6];
	CUIRect ScrollView = Content;
	CScrollRegion &Scroll = s_aScroll[m_PrismCategory];
	Scroll.Begin(&ScrollView);
	auto NextRow = [&](float Height = 28.0f) {
		CUIRect Row;
		ScrollView.HSplitTop(Height, &Row, &ScrollView);
		Scroll.AddRect(Row);
		ScrollView.HSplitTop(3.0f, nullptr, &ScrollView);
		return Row;
	};
	auto Label = [&](const char *pLabel) {
		CUIRect Row = NextRow(26.0f);
		if(!Scroll.RectClipped(Row))
			Ui()->DoLabel(&Row, pLabel, 12.0f, TEXTALIGN_ML);
	};
	static float s_aaToggleProgress[6][96] = {};
	static float s_aaToggleHover[6][96] = {};
	static bool s_aaToggleInitialized[6][96] = {};
	int ToggleIndex = 0;
	auto Toggle = [&](const char *pLabel, int *pValue) {
		CUIRect Row = NextRow(29.0f);
		const int Index = ToggleIndex++;
		if(Scroll.RectClipped(Row))
			return;
		const int Slot = std::min(Index, 95);
		float &Hover = s_aaToggleHover[m_PrismCategory][Slot];
		Hover += ((Ui()->MouseInside(&Row) ? 1.0f : 0.0f) - Hover) * Motion;
		Row.Draw(Theme.m_Panel.WithAlpha(0.72f + 0.16f * Hover), IGraphics::CORNER_ALL, 6.0f);
		CUIRect LabelRect, Switch;
		Row.VSplitRight(39.0f, &LabelRect, &Switch);
		LabelRect.VMargin(8.0f, &LabelRect);
		Ui()->DoLabel(&LabelRect, pLabel, 11.0f, TEXTALIGN_ML);
		Switch.VMargin(5.0f, &Switch);
		Switch.HMargin(6.0f, &Switch);
		const float Target = *pValue ? 1.0f : 0.0f;
		float &Progress = s_aaToggleProgress[m_PrismCategory][Slot];
		if(!s_aaToggleInitialized[m_PrismCategory][Slot])
		{
			Progress = Target;
			s_aaToggleInitialized[m_PrismCategory][Slot] = true;
		}
		Progress += (Target - Progress) * Motion;
		const float Position = Progress;
		Switch.Draw(ColorRGBA(Theme.m_Panel.r + (Theme.m_Accent.r - Theme.m_Panel.r) * Position,
			Theme.m_Panel.g + (Theme.m_Accent.g - Theme.m_Panel.g) * Position,
			Theme.m_Panel.b + (Theme.m_Accent.b - Theme.m_Panel.b) * Position, 0.9f), IGraphics::CORNER_ALL, 6.0f);
		CUIRect Knob = Switch;
		Knob.w = 11.0f;
		Knob.h = 11.0f;
		Knob.x += 2.0f + Position * (Switch.w - Knob.w - 4.0f);
		Knob.y = Switch.y + (Switch.h - Knob.h) * 0.5f;
		Knob.Draw(ColorRGBA(0.94f, 0.96f, 0.98f, 1.0f), IGraphics::CORNER_ALL, 5.0f);
		if(Ui()->DoButtonLogic(pValue, *pValue, &Row, BUTTONFLAG_LEFT))
			*pValue ^= 1;
	};
	auto Slider = [&](const char *pLabel, int *pValue, int Min, int Max) {
		CUIRect Row = NextRow(31.0f);
		if(!Scroll.RectClipped(Row))
		{
			Row.Draw(Theme.m_Panel.WithAlpha(0.72f), IGraphics::CORNER_ALL, 6.0f);
			Row.Margin(5.0f, &Row);
			Ui()->DoScrollbarOption(pValue, pValue, &Row, pLabel, Min, Max);
		}
	};
	static CButtonContainer s_aColors[64];
	auto Color = [&](const char *pLabel, unsigned *pValue, int Index) {
		CUIRect Row = NextRow(27.0f);
		if(!Scroll.RectClipped(Row))
			DoLine_ColorPicker(&s_aColors[Index], 24.0f, 12.0f, 3.0f, &Row, pLabel, pValue, ColorRGBA(0.65f, 0.78f, 0.91f, 1.0f), false, nullptr, Index == 9 || Index == 10);
	};


    // Phase 3 navigation and editors. All controls edit saved DDNet configuration.
    // Existing visual presets affect visuals only; all QoL preferences are separate.
    static CButtonContainer s_aActionButtons[64];
    auto Button = [&](const char *pText, int Id, const ColorRGBA &Tint = ColorRGBA(0.37f, 0.47f, 0.58f, 0.25f)) {
        CUIRect Row = NextRow(29.0f);
        return !Scroll.RectClipped(Row) && DoButton_Menu(&s_aActionButtons[Id], pText, 0, &Row, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 6.0f, 0.41f, Tint);
    };
    auto Bind = [&](const char *pLabel, int *pValue, int Target, int ButtonId) {
        char aText[128];
        str_format(aText, sizeof(aText), "%s: %s", pLabel,
            m_PrismCaptureBind == Target ? "PRESS A KEY (Esc unbinds)" : (*pValue ? KeyName(*pValue) : "Unbound"));
        if(Button(aText, ButtonId)) m_PrismCaptureBind = Target;
    };
    	switch(m_PrismCategory)
	{
case 0: // Visuals, including presets and all Tee/Hook adjustments
    {
        Label("Visual presets");
        static const char *s_apPresets[] = {"Default", "Clean", "Competitive", "Cinematic", "Custom"};
        static CButtonContainer s_aPresetButtons[5];
        for(int i = 0; i < 5; ++i)
        {
            CUIRect Row = NextRow(36.0f);
            if(!Scroll.RectClipped(Row) && DoButton_Menu(&s_aPresetButtons[i], s_apPresets[i], 0, &Row, BUTTONFLAG_LEFT, nullptr,
                IGraphics::CORNER_ALL, 9.0f, 0.45f, g_Config.m_PrismPreset == i ? ColorRGBA(0.39f, 0.56f, 0.71f, 0.43f) : ColorRGBA(0.34f, 0.42f, 0.50f, 0.18f)))
                Prism::ApplyPreset(g_Config, i);
        }
        Label("Tee   /   local player");
        Toggle("Outline", &g_Config.m_PrismLocalOutline);
        Slider("Outline width", &g_Config.m_PrismLocalOutlineWidth, 1, 8);
        Color("Outline color", &g_Config.m_PrismLocalOutlineColor, 0);
        Toggle("Glow", &g_Config.m_PrismLocalGlow);
        Slider("Glow strength", &g_Config.m_PrismLocalGlowIntensity, 0, 100);
        Color("Glow color", &g_Config.m_PrismLocalGlowColor, 1);
        Label("Tee   /   other players");
        Toggle("Outline", &g_Config.m_PrismOtherOutline);
        Slider("Outline width", &g_Config.m_PrismOtherOutlineWidth, 1, 8);
        Color("Outline color", &g_Config.m_PrismOtherOutlineColor, 2);
        Toggle("Glow", &g_Config.m_PrismOtherGlow);
        Slider("Glow strength", &g_Config.m_PrismOtherGlowIntensity, 0, 100);
        Color("Glow color", &g_Config.m_PrismOtherGlowColor, 3);
        Label("Hook   /   local player");
        Toggle("Custom color", &g_Config.m_PrismLocalHook);
        Color("Hook color", &g_Config.m_PrismLocalHookColor, 4);
        Toggle("Glow", &g_Config.m_PrismLocalHookGlow);
        Slider("Glow strength", &g_Config.m_PrismLocalHookIntensity, 0, 100);
        Label("Hook   /   other players");
        Toggle("Custom color", &g_Config.m_PrismOtherHook);
        Color("Hook color", &g_Config.m_PrismOtherHookColor, 5);
        Toggle("Glow", &g_Config.m_PrismOtherHookGlow);
        Slider("Glow strength", &g_Config.m_PrismOtherHookIntensity, 0, 100);
        Label("Hook particles  /  cosmetic only");
        if(Button(g_Config.m_PrismHookEffect == 0 ? "Mode: Off" :
            g_Config.m_PrismHookEffect == 1 ? "Mode: Leaves" :
            g_Config.m_PrismHookEffect == 2 ? "Mode: Sparkles" :
            g_Config.m_PrismHookEffect == 3 ? "Mode: Hearts" : "Mode: Pulse", 16))
            g_Config.m_PrismHookEffect = (g_Config.m_PrismHookEffect + 1) % 5;
        Toggle("Local hook", &g_Config.m_PrismHookEffectLocal);
        Toggle("Other visible hooks", &g_Config.m_PrismHookEffectOthers);
        Color("Particle color", &g_Config.m_PrismHookParticleColor, 6);
        Slider("Particle rate", &g_Config.m_PrismHookParticleRate, 1, 120);
        Slider("Particle cap", &g_Config.m_PrismHookParticleCap, 0, 512);
        Slider("Particle size", &g_Config.m_PrismHookParticleSize, 1, 24);
        Slider("Particle lifetime", &g_Config.m_PrismHookParticleLifetime, 100, 3000);
        Slider("Particle opacity", &g_Config.m_PrismHookParticleOpacity, 0, 100);
        Slider("Particle spread", &g_Config.m_PrismHookParticleSpread, 0, 80);
        Slider("Particle velocity", &g_Config.m_PrismHookParticleSpeed, 0, 100);
        Toggle("Particle fade", &g_Config.m_PrismHookParticleFade);
        Toggle("Particle glow", &g_Config.m_PrismHookParticleGlow);
        Slider("Pulse frequency", &g_Config.m_PrismHookPulseFrequency, 1, 60);
        Slider("Pulse intensity", &g_Config.m_PrismHookPulseIntensity, 0, 100);
        Label("Player trail");
        if(Button(g_Config.m_PrismTrail == 0 ? "Trail: Off" :
            g_Config.m_PrismTrail == 1 ? "Trail: Line" :
            g_Config.m_PrismTrail == 2 ? "Trail: Ribbon" : "Trail: Particles", 17))
            g_Config.m_PrismTrail = (g_Config.m_PrismTrail + 1) % 4;
        Toggle("Local trail", &g_Config.m_PrismTrailLocal);
        Toggle("Other visible trails", &g_Config.m_PrismTrailOthers);
        Color("Trail color", &g_Config.m_PrismTrailColor, 7);
        Slider("Trail width", &g_Config.m_PrismTrailWidth, 1, 24);
        Slider("Trail lifetime", &g_Config.m_PrismTrailLength, 50, 3000);
        Slider("Sample interval", &g_Config.m_PrismTrailInterval, 8, 100);
        Slider("Trail opacity", &g_Config.m_PrismTrailOpacity, 0, 100);
        Toggle("Trail fade", &g_Config.m_PrismTrailFade);
        Toggle("Trail glow", &g_Config.m_PrismTrailGlow);
        Label("Player highlight  /  normally visible Tees");
        Toggle("Highlight", &g_Config.m_PrismHighlight);
        Toggle("Local Tee", &g_Config.m_PrismHighlightLocal);
        Toggle("Other visible Tees", &g_Config.m_PrismHighlightOthers);
        Color("Highlight color", &g_Config.m_PrismHighlightColor, 8);
        Slider("Fill opacity", &g_Config.m_PrismHighlightFill, 0, 100);
        Slider("Outline opacity", &g_Config.m_PrismHighlightOpacity, 0, 100);
        Slider("Outline width", &g_Config.m_PrismHighlightWidth, 1, 8);
        Slider("Box scale", &g_Config.m_PrismHighlightScale, 50, 200);
        Slider("Corner radius", &g_Config.m_PrismHighlightRounding, 0, 24);
        Label("Effects quality");
        Slider("Quality: low / medium / high", &g_Config.m_PrismEffectQuality, 0, 2);
        break;
    }

	case 1: // Chinese Conical Hat (斗笠)
	{
		Label("Chinese Conical Hat   /   斗笠");
		Toggle("Enable Chinese Hat", &g_Config.m_PrismHatEnabled);
		if(Button(g_Config.m_PrismHatMode == 0 ? "Mode: Custom Static Color" :
			g_Config.m_PrismHatMode == 1 ? "Mode: Rainbow Gradient Wave" : "Mode: Shimmering Monochrome Luster", 29))
			g_Config.m_PrismHatMode = (g_Config.m_PrismHatMode + 1) % 3;

		if(g_Config.m_PrismHatMode != 1)
			Color("Hat base color", &g_Config.m_PrismHatColor, 15);

		Color("Rib lines color", &g_Config.m_PrismHatRibColor, 16);

		Slider("Facet opacity (%)", &g_Config.m_PrismHatAlpha, 10, 100);
		Slider("Rib lines opacity (%)", &g_Config.m_PrismHatRibAlpha, 10, 100);

		if(g_Config.m_PrismHatMode != 0)
			Slider("Animation speed (%)", &g_Config.m_PrismHatSpeed, 10, 500);

		Slider("Hat size scale (%)", &g_Config.m_PrismHatScale, 50, 200);
		Slider("Vertical offset", &g_Config.m_PrismHatOffsetY, -50, 50);

		Toggle("Tilt with movement & aim", &g_Config.m_PrismHatTilt);
		Toggle("Show on local player", &g_Config.m_PrismHatLocal);
		Toggle("Show on other players", &g_Config.m_PrismHatOthers);
		break;
	}

case 2: // HUD drag/drop + layout and per-module visibility
    {
        Label("Modular HUD   /   drag handles to reposition");
        Toggle("Enable all modules", &g_Config.m_PrismHudEnabled);
        Slider("HUD scale (%)", &g_Config.m_PrismHudScale, 65, 150);
        Slider("HUD opacity (%)", &g_Config.m_PrismHudOpacity, 20, 100);
        static const char *s_apModuleNames[] = {"Active Hotkeys", "Prism name/version", "Performance", "Dummy Status", "Verified Staff", "Input Overlay", "Effects Status"};
        int *apEnabled[] = {&g_Config.m_PrismHudHotkeys, &g_Config.m_PrismHudIdentity,
            &g_Config.m_PrismHudPerformance, &g_Config.m_PrismHudDummy, &g_Config.m_PrismHudStaff,
            &g_Config.m_PrismHudInput, &g_Config.m_PrismHudEffects};
        int *apX[] = {&g_Config.m_PrismHudHotkeysX, &g_Config.m_PrismHudIdentityX,
            &g_Config.m_PrismHudPerformanceX, &g_Config.m_PrismHudDummyX, &g_Config.m_PrismHudStaffX,
            &g_Config.m_PrismHudInputX, &g_Config.m_PrismHudEffectsX};
        int *apY[] = {&g_Config.m_PrismHudHotkeysY, &g_Config.m_PrismHudIdentityY,
            &g_Config.m_PrismHudPerformanceY, &g_Config.m_PrismHudDummyY, &g_Config.m_PrismHudStaffY,
            &g_Config.m_PrismHudInputY, &g_Config.m_PrismHudEffectsY};
        int *apScale[] = {&g_Config.m_PrismHudHotkeysScale, &g_Config.m_PrismHudIdentityScale,
            &g_Config.m_PrismHudPerformanceScale, &g_Config.m_PrismHudDummyScale, &g_Config.m_PrismHudStaffScale,
            &g_Config.m_PrismHudInputScale, &g_Config.m_PrismHudEffectsScale};
        static const char *s_apHudPresets[] = {"Minimal", "Streaming", "Competitive", "Custom"};
        for(int Preset = 0; Preset < 4; ++Preset)
        {
            if(Button(s_apHudPresets[Preset], 24 + Preset))
            {
                if(Preset != PrismQol::HUD_CUSTOM)
                {
                    const auto Layout = PrismQol::HudPreset(Preset);
                    for(int i = 0; i < PrismQol::NUM_HUD_MODULES; ++i)
                    {
                        *apEnabled[i] = Layout.m_aEnabled[i];
                        *apX[i] = Layout.m_aX[i];
                        *apY[i] = Layout.m_aY[i];
                    }
                }
                g_Config.m_PrismHudPreset = Preset;
            }
        }
        for(int i = 0; i < 7; ++i) Toggle(s_apModuleNames[i], apEnabled[i]);
        Slider("Module scale: Active Hotkeys", apScale[0], 65, 150);
        Slider("Module scale: Identity", apScale[1], 65, 150);
        Slider("Module scale: Performance", apScale[2], 65, 150);
        Slider("Module scale: Dummy", apScale[3], 65, 150);
        Slider("Module scale: Staff", apScale[4], 65, 150);
        Slider("Module scale: Input", apScale[5], 65, 150);
        Slider("Module scale: Effects", apScale[6], 65, 150);
        Toggle("HUD background", &g_Config.m_PrismHudBackground);
        Toggle("Snap to guides", &g_Config.m_PrismHudSnap);
        Toggle("Snap to screen edges", &g_Config.m_PrismHudEdgeSnap);
        Toggle("Lock HUD layout", &g_Config.m_PrismHudLayoutLock);
        Toggle("Drag widgets on game screen", &g_Config.m_PrismHudEdit);
        Toggle("Show guides", &g_Config.m_PrismHudGuides);
        Slider("Padding", &g_Config.m_PrismHudPadding, 0, 16);
        Slider("Rounding", &g_Config.m_PrismHudRounding, 0, 12);
        Slider("Font size", &g_Config.m_PrismHudFontSize, 5, 12);
        Label("Physical input overlay");
        Slider("Key size", &g_Config.m_PrismInputKeySize, 12, 32);
        Slider("Key rounding", &g_Config.m_PrismInputRounding, 0, 12);
        Toggle("Action labels", &g_Config.m_PrismInputLabels);
        Toggle("Key animation", &g_Config.m_PrismInputAnimation);
        Color("Pressed key", &g_Config.m_PrismInputActiveColor, 9);
        Color("Released key", &g_Config.m_PrismInputInactiveColor, 10);
        Label("Layout preview  /  drag enabled modules");
        CUIRect Preview = NextRow(165.0f);
        if(!Scroll.RectClipped(Preview))
        {
            Preview.Draw(ColorRGBA(0.018f, 0.031f, 0.047f, 0.75f), IGraphics::CORNER_ALL, 9.0f);
            static CButtonContainer s_aPins[7];
            static int s_DragIndex = -1;
            static float s_DragOffsetX = 0.0f, s_DragOffsetY = 0.0f;
            if(!Ui()->MouseButton(0))
            { s_DragIndex = -1; }
            for(int i = 0; i < 7; ++i)
            {
                if(!*apEnabled[i]) continue;
                CUIRect Pin;
                Pin.w = 82.0f; Pin.h = 18.0f;
                Pin.x = Preview.x + PrismQol::HudCoordinate(*apX[i], Preview.w, Pin.w);
                Pin.y = Preview.y + PrismQol::HudCoordinate(*apY[i], Preview.h, Pin.h);
				if(!g_Config.m_PrismHudLayoutLock && s_DragIndex == -1 && Ui()->MouseButtonClicked(0) && Ui()->MouseInside(&Pin))
                {
                    s_DragIndex = i;
                    s_DragOffsetX = Ui()->MouseX() - Pin.x;
                    s_DragOffsetY = Ui()->MouseY() - Pin.y;
                    Ui()->SetActiveItem(&s_aPins[i]);
                }
				if(s_DragIndex == i && Ui()->MouseButton(0) && !g_Config.m_PrismHudLayoutLock)
                {
                    Ui()->CheckActiveItem(&s_aPins[i]);
                    float NewX = Ui()->MouseX() - s_DragOffsetX - Preview.x;
                    float NewY = Ui()->MouseY() - s_DragOffsetY - Preview.y;
                    if(g_Config.m_PrismHudSnap)
                    {
                        NewX = PrismQol::HudSnap(NewX, Preview.w, Pin.w);
                        NewY = PrismQol::HudSnap(NewY, Preview.h, Pin.h);
                    }
                    *apX[i] = PrismQol::HudNormalize(NewX, Preview.w);
                    *apY[i] = PrismQol::HudNormalize(NewY, Preview.h);
                    g_Config.m_PrismHudPreset = PrismQol::HUD_CUSTOM;
                    Pin.x = Preview.x + PrismQol::HudCoordinate(*apX[i], Preview.w, Pin.w);
                    Pin.y = Preview.y + PrismQol::HudCoordinate(*apY[i], Preview.h, Pin.h);
                }
                Pin.Draw(s_DragIndex == i ? ColorRGBA(0.54f, 0.70f, 0.85f, 0.65f) :
                    ColorRGBA(0.29f, 0.40f, 0.51f, 0.80f), IGraphics::CORNER_ALL, 5.0f);
                Ui()->DoLabel(&Pin, s_apModuleNames[i], 8.0f, TEXTALIGN_MC);
            }
        }
        if(Button("Reset HUD layout", 14))
        {
            const int aX[] = {300, 300, 8200, 300, 8200, 300, 7600};
            const int aY[] = {900, 250, 250, 1600, 950, 7500, 1800};
            for(int i = 0; i < 7; ++i) { *apX[i] = aX[i]; *apY[i] = aY[i]; }
            g_Config.m_PrismHudScale = 100;
            g_Config.m_PrismHudOpacity = 85;
        }
        Label("Staff appears only with server-supplied auth data.");
        break;
    }

case 3: // Physical input overlay
        Label("Input overlay  /  physical controls");
        Toggle("Show input overlay", &g_Config.m_PrismHudInput);
        Slider("Key size", &g_Config.m_PrismInputKeySize, 12, 32);
        Slider("Key rounding", &g_Config.m_PrismInputRounding, 0, 12);
        Toggle("Action labels", &g_Config.m_PrismInputLabels);
        Toggle("Key animation", &g_Config.m_PrismInputAnimation);
        Color("Pressed key", &g_Config.m_PrismInputActiveColor, 9);
        Color("Released key", &g_Config.m_PrismInputInactiveColor, 10);
        Label("Movement, Jump, Hook and Fire use physical input.");
        break;

case 4: // Themes
        Label("Themes   /   Prism identity");
        if(Button("Theme: Orange graphite", 18)) PrismUi::ApplyTheme(g_Config, 0);
        if(Button("Theme: Glacier", 19)) PrismUi::ApplyTheme(g_Config, 1);
        if(Button("Theme: Violet", 20)) PrismUi::ApplyTheme(g_Config, 2);
        Color("Accent", &g_Config.m_PrismThemeAccent, 11);
        Color("Background", &g_Config.m_PrismThemeBackground, 12);
        Color("Panel", &g_Config.m_PrismThemePanel, 13);
        Color("Text", &g_Config.m_PrismThemeText, 14);
        Slider("Heading width (%)", &g_Config.m_PrismThemeHeadingWidth, 100, 140);
        Slider("Corner radius", &g_Config.m_PrismThemeRounding, 0, 20);
        Slider("Border opacity", &g_Config.m_PrismThemeBorder, 0, 100);
        Slider("Shadow opacity", &g_Config.m_PrismThemeShadow, 0, 100);
        if(Button("Export theme to prism/theme.json", 21)) PrismTheme::Save(Storage(), g_Config);
        if(Button("Import theme from prism/theme.json", 22)) PrismTheme::Load(Storage(), g_Config);
        Label("Visual presets do not change macros or HUD layouts.");
        break;

	case 5: // Settings and status
	{
		Label("Prism   /   Control center");
		Toggle("Enable Prism visuals", &g_Config.m_PrismEnabled);
		Toggle("Modular HUD", &g_Config.m_PrismHudEnabled);
		char aStatus[128];
		str_copy(aStatus, "Client: Prism Legit Edition (DDNet 20.3)   |   Status: Active");
		Label(aStatus);
		Label("Insert / Esc: close overlay");
		Label("Performance   /   measured locally");
		Toggle("FPS and frame-time overlay", &g_Config.m_PrismOverlay);
		Toggle("Performance HUD panel", &g_Config.m_PrismHudPerformance);
		Slider("Menu scale", &g_Config.m_PrismMenuScale, 80, 120);
		Slider("Glass opacity", &g_Config.m_PrismPanelOpacity, 50, 100);
		Slider("Background darkness", &g_Config.m_PrismGlassDarkness, 0, 100);
		Slider("Glass tint", &g_Config.m_PrismGlassTint, 0, 100);
		Toggle("Animations", &g_Config.m_PrismAnimations);
		Slider("Animation duration (ms)", &g_Config.m_PrismThemeAnimation, 0, 5000);
		Toggle("Reduce animation", &g_Config.m_PrismReducedMotion);
		Toggle("Lock ClickGUI position", &g_Config.m_PrismMenuLock);
		Toggle("Snap ClickGUI to edges", &g_Config.m_PrismMenuSnap);
		if(Button("Reset ClickGUI position", 23))
		{
			g_Config.m_PrismMenuX = 5000;
			g_Config.m_PrismMenuY = 5000;
		}
		if(Button(g_Config.m_PrismMenuCursor ? "Menu cursor: System" : "Menu cursor: DDNet", 28))
			g_Config.m_PrismMenuCursor ^= 1;
		if(Button("Reset visual presets only", 15)) Prism::Reset(g_Config);
		break;
	}
	}

	Scroll.End();
	Footer.Draw(Theme.m_Text.WithAlpha(0.04f), IGraphics::CORNER_ALL, 5.0f);
	Ui()->DoLabel(&Footer, "PRISM " PRISM_VERSION "  /  Insert or Esc", 9.0f, TEXTALIGN_MC);
	TextRender()->SetFontPreset(EFontPreset::DEFAULT_FONT);
	Prism::Validate(g_Config);
}

void CMenus::RenderSettingsPrismTab(CUIRect MainView)
{
	Prism::Validate(g_Config);

	CUIRect TopBar, Left, Right;
	MainView.HSplitTop(35.0f, &TopBar, &MainView);
	MainView.HSplitTop(10.0f, nullptr, &MainView);

	// Top Bar: Info + Button to open floating ClickGUI
	{
		CUIRect Title, OpenButton;
		TopBar.VSplitRight(240.0f, &Title, &OpenButton);
		Ui()->DoLabel(&Title, "Prism Client Legit Edition (DDNet 20.3)", 18.0f, TEXTALIGN_ML);
		static CButtonContainer s_OpenClickGui;
		if(DoButton_Menu(&s_OpenClickGui, "Open ClickGUI [Insert]", 0, &OpenButton, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 6.0f, 0.45f, ColorRGBA(0.2f, 0.5f, 0.8f, 0.6f)))
		{
			TogglePrism();
		}
	}

	MainView.VSplitMid(&Left, &Right, 20.0f);

	static CScrollRegion s_LeftScroll, s_RightScroll;
	s_LeftScroll.Begin(&Left);
	s_RightScroll.Begin(&Right);

	auto LeftRow = [&](float Height = 22.0f, float Spacing = 4.0f) {
		CUIRect Row;
		Left.HSplitTop(Height, &Row, &Left);
		s_LeftScroll.AddRect(Row);
		Left.HSplitTop(Spacing, nullptr, &Left);
		return Row;
	};

	auto RightRow = [&](float Height = 22.0f, float Spacing = 4.0f) {
		CUIRect Row;
		Right.HSplitTop(Height, &Row, &Right);
		s_RightScroll.AddRect(Row);
		Right.HSplitTop(Spacing, nullptr, &Right);
		return Row;
	};

	auto LeftCheckBox = [&](int *pVal, const char *pText) {
		CUIRect Row = LeftRow(22.0f, 4.0f);
		if(!s_LeftScroll.RectClipped(Row))
		{
			if(DoButton_CheckBox(pVal, pText, *pVal, &Row))
				*pVal ^= 1;
		}
	};

	auto RightCheckBox = [&](int *pVal, const char *pText) {
		CUIRect Row = RightRow(22.0f, 4.0f);
		if(!s_RightScroll.RectClipped(Row))
		{
			if(DoButton_CheckBox(pVal, pText, *pVal, &Row))
				*pVal ^= 1;
		}
	};

	auto LeftSlider = [&](int *pVal, const char *pText, int Min, int Max, const char *pSuffix = "%") {
		CUIRect Row = LeftRow(22.0f, 4.0f);
		if(!s_LeftScroll.RectClipped(Row))
			Ui()->DoScrollbarOption(pVal, pVal, &Row, pText, Min, Max, &CUi::ms_LinearScrollbarScale, 0u, pSuffix);
	};

	auto RightSlider = [&](int *pVal, const char *pText, int Min, int Max, const char *pSuffix = "%") {
		CUIRect Row = RightRow(22.0f, 4.0f);
		if(!s_RightScroll.RectClipped(Row))
			Ui()->DoScrollbarOption(pVal, pVal, &Row, pText, Min, Max, &CUi::ms_LinearScrollbarScale, 0u, pSuffix);
	};

	auto LeftColor = [&](CButtonContainer *pReset, const char *pText, unsigned *pVal, ColorRGBA DefCol, bool Alpha = false) {
		CUIRect Row = LeftRow(24.0f, 4.0f);
		if(!s_LeftScroll.RectClipped(Row))
			DoLine_ColorPicker(pReset, 24.0f, 12.0f, 0.0f, &Row, pText, pVal, DefCol, false, nullptr, Alpha);
	};

	auto RightColor = [&](CButtonContainer *pReset, const char *pText, unsigned *pVal, ColorRGBA DefCol, bool Alpha = false) {
		CUIRect Row = RightRow(24.0f, 4.0f);
		if(!s_RightScroll.RectClipped(Row))
			DoLine_ColorPicker(pReset, 24.0f, 12.0f, 0.0f, &Row, pText, pVal, DefCol, false, nullptr, Alpha);
	};

	// ---------------- LEFT COLUMN: Interactive Preview + Chinese Hat + Visuals ----------------
	{
		// Live Interactive Tee Preview
		CUIRect PreviewBox = LeftRow(115.0f, 10.0f);
		if(!s_LeftScroll.RectClipped(PreviewBox))
		{
			PreviewBox.Draw(ColorRGBA(0.0f, 0.0f, 0.0f, 0.28f), IGraphics::CORNER_ALL, 8.0f);
			const CSkin *pDefaultSkin = GameClient()->m_Skins.Find("default");
			const CSkins::CSkinContainer *pOwnSkinContainer = GameClient()->m_Skins.FindContainerOrNullptr(g_Config.m_ClPlayerSkin[0] == '\0' ? "default" : g_Config.m_ClPlayerSkin);
			CTeeRenderInfo OwnSkinInfo;
			OwnSkinInfo.Apply(pOwnSkinContainer == nullptr || pOwnSkinContainer->Skin() == nullptr ? pDefaultSkin : pOwnSkinContainer->Skin().get());
			OwnSkinInfo.ApplyColors(g_Config.m_ClPlayerUseCustomColor, g_Config.m_ClPlayerColorBody, g_Config.m_ClPlayerColorFeet);
			OwnSkinInfo.m_Size = 56.0f;

			vec2 OffsetToMid;
			CRenderTools::GetRenderTeeOffsetToRenderedTee(CAnimState::GetIdle(), &OwnSkinInfo, OffsetToMid);
			const vec2 TeePos = vec2(PreviewBox.x + PreviewBox.w / 2.0f, PreviewBox.y + PreviewBox.h / 2.0f + OffsetToMid.y);
			const vec2 DeltaPos = Ui()->MousePos() - TeePos;
			const float Dist = length(DeltaPos);
			const vec2 TeeDir = Dist < 20.0f ? normalize(vec2(DeltaPos.x, std::max(DeltaPos.y, 0.5f))) : normalize(DeltaPos);
			const int Emote = Dist < 20.0f ? EMOTE_HAPPY : EMOTE_NORMAL;

			RenderTools()->RenderPrismTee(CAnimState::GetIdle(), &OwnSkinInfo, TeeDir, TeePos, 1.0f, true);
			RenderTools()->RenderTee(CAnimState::GetIdle(), &OwnSkinInfo, Emote, TeeDir, TeePos);
			if(g_Config.m_PrismHatEnabled)
				RenderTools()->RenderChineseHat(CAnimState::GetIdle(), &OwnSkinInfo, TeeDir, TeePos);
		}

		// Chinese Conical Hat (斗笠)
		CUIRect HatLabel = LeftRow(26.0f, 4.0f);
		if(!s_LeftScroll.RectClipped(HatLabel))
			Ui()->DoLabel(&HatLabel, Localize("Chinese Conical Hat (斗笠)"), 17.0f, TEXTALIGN_ML);

		LeftCheckBox(&g_Config.m_PrismHatEnabled, Localize("Enable Chinese Hat"));

		CUIRect ModeRow = LeftRow(24.0f, 4.0f);
		if(!s_LeftScroll.RectClipped(ModeRow))
		{
			const char *apHatModes[] = {"Mode: Custom Static Color", "Mode: Rainbow Gradient Wave", "Mode: Shimmering Monochrome Luster"};
			static CButtonContainer s_HatModeBtn;
			if(DoButton_Menu(&s_HatModeBtn, apHatModes[std::clamp(g_Config.m_PrismHatMode, 0, 2)], 0, &ModeRow, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 5.0f, 0.35f))
				g_Config.m_PrismHatMode = (g_Config.m_PrismHatMode + 1) % 3;
		}

		static CButtonContainer s_HatColorReset, s_HatRibColorReset;
		if(g_Config.m_PrismHatMode != 1)
			LeftColor(&s_HatColorReset, Localize("Hat base color"), &g_Config.m_PrismHatColor, ColorRGBA(0.95f, 0.82f, 0.55f, 1.0f));
		LeftColor(&s_HatRibColorReset, Localize("Rib lines color"), &g_Config.m_PrismHatRibColor, ColorRGBA(0.15f, 0.12f, 0.08f, 1.0f));

		LeftSlider(&g_Config.m_PrismHatAlpha, Localize("Facet opacity"), 10, 100);
		LeftSlider(&g_Config.m_PrismHatRibAlpha, Localize("Rib lines opacity"), 10, 100);
		LeftSlider(&g_Config.m_PrismHatScale, Localize("Hat size scale"), 50, 200);
		LeftSlider(&g_Config.m_PrismHatOffsetY, Localize("Vertical offset"), -50, 50, " px");

		if(g_Config.m_PrismHatMode != 0)
			LeftSlider(&g_Config.m_PrismHatSpeed, Localize("Animation speed"), 10, 500);

		LeftCheckBox(&g_Config.m_PrismHatTilt, Localize("Tilt with movement & aim"));
		LeftCheckBox(&g_Config.m_PrismHatLocal, Localize("Show on local player"));
		LeftCheckBox(&g_Config.m_PrismHatOthers, Localize("Show on other players"));

		// Visuals & Cosmetic Effects
		CUIRect VisLabel = LeftRow(26.0f, 4.0f);
		if(!s_LeftScroll.RectClipped(VisLabel))
			Ui()->DoLabel(&VisLabel, Localize("Visuals & Cosmetic Effects"), 17.0f, TEXTALIGN_ML);

		static CButtonContainer s_LocOutlineReset, s_LocGlowReset, s_OthOutlineReset, s_OthGlowReset;
		static CButtonContainer s_LocHookReset, s_OthHookReset, s_TrailReset, s_HighlightReset, s_HookPartReset;

		LeftCheckBox(&g_Config.m_PrismLocalOutline, Localize("Local tee outline"));
		LeftSlider(&g_Config.m_PrismLocalOutlineWidth, Localize("Local outline width"), 1, 8, " px");
		LeftColor(&s_LocOutlineReset, Localize("Local outline color"), &g_Config.m_PrismLocalOutlineColor, ColorRGBA(0.35f, 0.70f, 1.0f, 1.0f));

		LeftCheckBox(&g_Config.m_PrismLocalGlow, Localize("Local tee glow"));
		LeftSlider(&g_Config.m_PrismLocalGlowIntensity, Localize("Local glow strength"), 0, 100);
		LeftColor(&s_LocGlowReset, Localize("Local glow color"), &g_Config.m_PrismLocalGlowColor, ColorRGBA(0.35f, 0.70f, 1.0f, 1.0f));

		LeftCheckBox(&g_Config.m_PrismOtherOutline, Localize("Other tees outline"));
		LeftSlider(&g_Config.m_PrismOtherOutlineWidth, Localize("Other outline width"), 1, 8, " px");
		LeftColor(&s_OthOutlineReset, Localize("Other outline color"), &g_Config.m_PrismOtherOutlineColor, ColorRGBA(1.0f, 0.40f, 0.40f, 1.0f));

		LeftCheckBox(&g_Config.m_PrismOtherGlow, Localize("Other tees glow"));
		LeftSlider(&g_Config.m_PrismOtherGlowIntensity, Localize("Other glow strength"), 0, 100);
		LeftColor(&s_OthGlowReset, Localize("Other glow color"), &g_Config.m_PrismOtherGlowColor, ColorRGBA(1.0f, 0.40f, 0.40f, 1.0f));

		LeftCheckBox(&g_Config.m_PrismLocalHook, Localize("Local hook custom color"));
		LeftColor(&s_LocHookReset, Localize("Local hook color"), &g_Config.m_PrismLocalHookColor, ColorRGBA(0.40f, 0.85f, 1.0f, 1.0f));
		LeftCheckBox(&g_Config.m_PrismLocalHookGlow, Localize("Local hook glow"));
		LeftSlider(&g_Config.m_PrismLocalHookIntensity, Localize("Local hook glow intensity"), 0, 100);

		LeftCheckBox(&g_Config.m_PrismOtherHook, Localize("Other hooks custom color"));
		LeftColor(&s_OthHookReset, Localize("Other hook color"), &g_Config.m_PrismOtherHookColor, ColorRGBA(1.0f, 0.50f, 0.50f, 1.0f));
		LeftCheckBox(&g_Config.m_PrismOtherHookGlow, Localize("Other hook glow"));
		LeftSlider(&g_Config.m_PrismOtherHookIntensity, Localize("Other hook glow intensity"), 0, 100);

		CUIRect PartBtnRow = LeftRow(24.0f, 4.0f);
		if(!s_LeftScroll.RectClipped(PartBtnRow))
		{
			const char *apHookModes[] = {"Hook Particles: Off", "Hook Particles: Leaves", "Hook Particles: Sparkles", "Hook Particles: Hearts", "Hook Particles: Pulse"};
			static CButtonContainer s_HookModeBtn;
			if(DoButton_Menu(&s_HookModeBtn, apHookModes[std::clamp(g_Config.m_PrismHookEffect, 0, 4)], 0, &PartBtnRow, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 5.0f, 0.35f))
				g_Config.m_PrismHookEffect = (g_Config.m_PrismHookEffect + 1) % 5;
		}
		LeftColor(&s_HookPartReset, Localize("Hook particle color"), &g_Config.m_PrismHookParticleColor, ColorRGBA(0.45f, 0.90f, 0.50f, 1.0f));
		LeftSlider(&g_Config.m_PrismHookParticleSize, Localize("Hook particle size"), 2, 24, " px");
		LeftSlider(&g_Config.m_PrismHookParticleOpacity, Localize("Hook particle opacity"), 10, 100);

		LeftCheckBox(&g_Config.m_PrismTrailLocal, Localize("Local player movement trail"));
		CUIRect TrailBtnRow = LeftRow(24.0f, 4.0f);
		if(!s_LeftScroll.RectClipped(TrailBtnRow))
		{
			const char *apTrailModes[] = {"Trail Mode: Off", "Trail Mode: Line", "Trail Mode: Ribbon", "Trail Mode: Particles"};
			static CButtonContainer s_TrailModeBtn;
			if(DoButton_Menu(&s_TrailModeBtn, apTrailModes[std::clamp(g_Config.m_PrismTrail, 0, 3)], 0, &TrailBtnRow, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 5.0f, 0.35f))
				g_Config.m_PrismTrail = (g_Config.m_PrismTrail + 1) % 4;
		}
		LeftSlider(&g_Config.m_PrismTrailWidth, Localize("Trail width"), 1, 24, " px");
		LeftSlider(&g_Config.m_PrismTrailOpacity, Localize("Trail opacity"), 10, 100);
		LeftColor(&s_TrailReset, Localize("Trail color"), &g_Config.m_PrismTrailColor, ColorRGBA(0.40f, 0.85f, 1.0f, 1.0f));
		LeftCheckBox(&g_Config.m_PrismTrailOthers, Localize("Other player trails"));

		LeftCheckBox(&g_Config.m_PrismHighlight, Localize("Player highlight box"));
		LeftSlider(&g_Config.m_PrismHighlightWidth, Localize("Highlight border width"), 1, 8, " px");
		LeftSlider(&g_Config.m_PrismHighlightFill, Localize("Highlight fill opacity"), 0, 100);
		LeftColor(&s_HighlightReset, Localize("Highlight color"), &g_Config.m_PrismHighlightColor, ColorRGBA(0.40f, 0.85f, 1.0f, 1.0f));
	}

	// ---------------- RIGHT COLUMN: Modular HUD + ClickGUI & Themes ----------------
	{
		CUIRect HudLabel = RightRow(26.0f, 4.0f);
		if(!s_RightScroll.RectClipped(HudLabel))
			Ui()->DoLabel(&HudLabel, Localize("Prism Modular HUD"), 17.0f, TEXTALIGN_ML);

		RightCheckBox(&g_Config.m_PrismHudEnabled, Localize("Enable modular HUD"));
		RightSlider(&g_Config.m_PrismHudScale, Localize("HUD scale"), 65, 150);
		RightSlider(&g_Config.m_PrismHudOpacity, Localize("HUD opacity"), 20, 100);

		RightCheckBox(&g_Config.m_PrismHudHotkeys, Localize("Active Hotkeys module"));
		RightCheckBox(&g_Config.m_PrismHudIdentity, Localize("Identity module"));
		RightCheckBox(&g_Config.m_PrismHudPerformance, Localize("Performance HUD (FPS & Ping)"));
		RightCheckBox(&g_Config.m_PrismHudDummy, Localize("Dummy status module"));
		RightCheckBox(&g_Config.m_PrismHudStaff, Localize("Staff indicator module"));
		RightCheckBox(&g_Config.m_PrismHudInput, Localize("Keystrokes & Input HUD"));
		RightCheckBox(&g_Config.m_PrismHudEffects, Localize("Active cosmetic effects list"));

		static CButtonContainer s_InpActReset, s_InpInactReset;
		RightColor(&s_InpActReset, Localize("Pressed key color"), &g_Config.m_PrismInputActiveColor, ColorRGBA(0.2f, 0.7f, 1.0f, 0.85f), true);
		RightColor(&s_InpInactReset, Localize("Released key color"), &g_Config.m_PrismInputInactiveColor, ColorRGBA(0.1f, 0.1f, 0.15f, 0.65f), true);

		RightCheckBox(&g_Config.m_PrismHudEdit, Localize("Drag widgets on game screen [HUD tab]"));
		RightCheckBox(&g_Config.m_PrismHudBackground, Localize("HUD widget background"));
		RightCheckBox(&g_Config.m_PrismHudSnap, Localize("Snap to guide lines"));
		RightCheckBox(&g_Config.m_PrismHudEdgeSnap, Localize("Snap to screen edges"));
		RightCheckBox(&g_Config.m_PrismHudLayoutLock, Localize("Lock HUD widget positions"));

		CUIRect ResetHudRow = RightRow(24.0f, 10.0f);
		if(!s_RightScroll.RectClipped(ResetHudRow))
		{
			static CButtonContainer s_ResetHudBtn;
			if(DoButton_Menu(&s_ResetHudBtn, Localize("Reset HUD layout to default"), 0, &ResetHudRow, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 5.0f, 0.35f))
			{
				g_Config.m_PrismHudHotkeysX = 300; g_Config.m_PrismHudHotkeysY = 900;
				g_Config.m_PrismHudIdentityX = 300; g_Config.m_PrismHudIdentityY = 250;
				g_Config.m_PrismHudPerformanceX = 8200; g_Config.m_PrismHudPerformanceY = 250;
				g_Config.m_PrismHudDummyX = 300; g_Config.m_PrismHudDummyY = 1600;
				g_Config.m_PrismHudStaffX = 8200; g_Config.m_PrismHudStaffY = 950;
				g_Config.m_PrismHudInputX = 300; g_Config.m_PrismHudInputY = 7500;
				g_Config.m_PrismHudEffectsX = 7600; g_Config.m_PrismHudEffectsY = 1800;
			}
		}

		// ClickGUI & Appearance Section
		CUIRect GuiLabel = RightRow(26.0f, 4.0f);
		if(!s_RightScroll.RectClipped(GuiLabel))
			Ui()->DoLabel(&GuiLabel, Localize("ClickGUI & Appearance"), 17.0f, TEXTALIGN_ML);

		CUIRect ThemeRow = RightRow(24.0f, 4.0f);
		if(!s_RightScroll.RectClipped(ThemeRow))
		{
			static int s_ThemeIdx = 0;
			const char *apThemes[] = {"Theme: Orange Graphite", "Theme: Glacier", "Theme: Violet"};
			static CButtonContainer s_ThemeBtn;
			if(DoButton_Menu(&s_ThemeBtn, apThemes[std::clamp(s_ThemeIdx, 0, 2)], 0, &ThemeRow, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 5.0f, 0.35f))
			{
				s_ThemeIdx = (s_ThemeIdx + 1) % 3;
				PrismUi::ApplyTheme(g_Config, s_ThemeIdx);
			}
		}

		RightSlider(&g_Config.m_PrismMenuScale, Localize("ClickGUI menu scale"), 80, 120);
		RightSlider(&g_Config.m_PrismPanelOpacity, Localize("Glass opacity"), 50, 100);
		RightSlider(&g_Config.m_PrismGlassDarkness, Localize("Backdrop darkness"), 0, 100);
		RightSlider(&g_Config.m_PrismGlassTint, Localize("Glass tint"), 0, 100);
		RightCheckBox(&g_Config.m_PrismMenuLock, Localize("Lock ClickGUI position"));
		RightCheckBox(&g_Config.m_PrismMenuSnap, Localize("Snap ClickGUI to edges"));

		CUIRect ResetGuiRow = RightRow(24.0f, 4.0f);
		if(!s_RightScroll.RectClipped(ResetGuiRow))
		{
			static CButtonContainer s_ResetGuiBtn;
			if(DoButton_Menu(&s_ResetGuiBtn, Localize("Reset ClickGUI position"), 0, &ResetGuiRow, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 5.0f, 0.35f))
			{
				g_Config.m_PrismMenuX = 5000;
				g_Config.m_PrismMenuY = 5000;
			}
		}
	}

	s_LeftScroll.End();
	s_RightScroll.End();
}
