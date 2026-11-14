/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "menus.h"

#include <engine/shared/config.h>
#include <game/client/gameclient.h>
#include <game/client/ui.h>
#include <game/localization.h>

void CMenus::RenderSettingsTClient(CUIRect MainView)
{
	CUIRect Left, Right, Button, Label;
	MainView.VSplitMid(&Left, &Right, 20.0f);

	// Left Column: Rainbow & Trails & Pet
	{
		// Rainbow Section
		Left.HSplitTop(25.0f, &Label, &Left);
		Ui()->DoLabel(&Label, Localize("Rainbow Effects"), 18.0f, TEXTALIGN_ML);
		Left.HSplitTop(5.0f, nullptr, &Left);

		Left.HSplitTop(20.0f, &Button, &Left);
		if(DoButton_CheckBox(&g_Config.m_TcRainbowTees, Localize("Enable rainbow tee"), g_Config.m_TcRainbowTees, &Button))
			g_Config.m_TcRainbowTees ^= 1;

		Left.HSplitTop(20.0f, &Button, &Left);
		if(DoButton_CheckBox(&g_Config.m_TcRainbowHook, Localize("Enable rainbow hook"), g_Config.m_TcRainbowHook, &Button))
			g_Config.m_TcRainbowHook ^= 1;

		Left.HSplitTop(20.0f, &Button, &Left);
		if(DoButton_CheckBox(&g_Config.m_TcRainbowWeapon, Localize("Enable rainbow weapon"), g_Config.m_TcRainbowWeapon, &Button))
			g_Config.m_TcRainbowWeapon ^= 1;

		Left.HSplitTop(20.0f, &Button, &Left);
		if(DoButton_CheckBox(&g_Config.m_TcRainbowOthers, Localize("Rainbow for other tees"), g_Config.m_TcRainbowOthers, &Button))
			g_Config.m_TcRainbowOthers ^= 1;

		Left.HSplitTop(5.0f, nullptr, &Left);
		Left.HSplitTop(20.0f, &Button, &Left);
		Ui()->DoScrollbarOption(&g_Config.m_TcRainbowSpeed, &g_Config.m_TcRainbowSpeed, &Button, Localize("Rainbow speed"), 10, 500, &CUi::ms_LinearScrollbarScale, 0u, "%");

		Left.HSplitTop(15.0f, nullptr, &Left);

		// Trails Section
		Left.HSplitTop(25.0f, &Label, &Left);
		Ui()->DoLabel(&Label, Localize("Tee Movement Trails"), 18.0f, TEXTALIGN_ML);
		Left.HSplitTop(5.0f, nullptr, &Left);

		Left.HSplitTop(20.0f, &Button, &Left);
		if(DoButton_CheckBox(&g_Config.m_TcTeeTrail, Localize("Enable tee trails"), g_Config.m_TcTeeTrail, &Button))
			g_Config.m_TcTeeTrail ^= 1;

		Left.HSplitTop(20.0f, &Button, &Left);
		if(DoButton_CheckBox(&g_Config.m_TcTeeTrailOthers, Localize("Show other players' trails"), g_Config.m_TcTeeTrailOthers, &Button))
			g_Config.m_TcTeeTrailOthers ^= 1;

		Left.HSplitTop(20.0f, &Button, &Left);
		if(DoButton_CheckBox(&g_Config.m_TcTeeTrailFade, Localize("Fade trail over length"), g_Config.m_TcTeeTrailFade, &Button))
			g_Config.m_TcTeeTrailFade ^= 1;

		Left.HSplitTop(5.0f, nullptr, &Left);
		Left.HSplitTop(20.0f, &Button, &Left);
		Ui()->DoScrollbarOption(&g_Config.m_TcTeeTrailLength, &g_Config.m_TcTeeTrailLength, &Button, Localize("Trail length"), 5, 100, &CUi::ms_LinearScrollbarScale);

		Left.HSplitTop(5.0f, nullptr, &Left);
		Left.HSplitTop(20.0f, &Button, &Left);
		Ui()->DoScrollbarOption(&g_Config.m_TcTeeTrailAlpha, &g_Config.m_TcTeeTrailAlpha, &Button, Localize("Trail opacity"), 10, 100, &CUi::ms_LinearScrollbarScale, 0u, "%");

		Left.HSplitTop(15.0f, nullptr, &Left);

		// Pet Companion Section
		Left.HSplitTop(25.0f, &Label, &Left);
		Ui()->DoLabel(&Label, Localize("Companion Pet"), 18.0f, TEXTALIGN_ML);
		Left.HSplitTop(5.0f, nullptr, &Left);

		Left.HSplitTop(20.0f, &Button, &Left);
		if(DoButton_CheckBox(&g_Config.m_TcPetShow, Localize("Enable companion pet"), g_Config.m_TcPetShow, &Button))
			g_Config.m_TcPetShow ^= 1;

		Left.HSplitTop(5.0f, nullptr, &Left);
		Left.HSplitTop(20.0f, &Button, &Left);
		Ui()->DoScrollbarOption(&g_Config.m_TcPetSize, &g_Config.m_TcPetSize, &Button, Localize("Pet size"), 20, 150, &CUi::ms_LinearScrollbarScale, 0u, "%");

		Left.HSplitTop(5.0f, nullptr, &Left);
		Left.HSplitTop(20.0f, &Button, &Left);
		Ui()->DoScrollbarOption(&g_Config.m_TcPetAlpha, &g_Config.m_TcPetAlpha, &Button, Localize("Pet opacity"), 10, 100, &CUi::ms_LinearScrollbarScale, 0u, "%");
	}

	// Right Column: Status Bar, War List, Quality of Life
	{
		// Status Bar Section
		Right.HSplitTop(25.0f, &Label, &Right);
		Ui()->DoLabel(&Label, Localize("Status Bar"), 18.0f, TEXTALIGN_ML);
		Right.HSplitTop(5.0f, nullptr, &Right);

		Right.HSplitTop(20.0f, &Button, &Right);
		if(DoButton_CheckBox(&g_Config.m_TcStatusBar, Localize("Enable status bar"), g_Config.m_TcStatusBar, &Button))
			g_Config.m_TcStatusBar ^= 1;

		Right.HSplitTop(20.0f, &Button, &Right);
		if(DoButton_CheckBox(&g_Config.m_TcStatusBar12HourClock, Localize("12-hour clock format"), g_Config.m_TcStatusBar12HourClock, &Button))
			g_Config.m_TcStatusBar12HourClock ^= 1;

		Right.HSplitTop(20.0f, &Button, &Right);
		if(DoButton_CheckBox(&g_Config.m_TcStatusBarLocalTimeSeconds, Localize("Show seconds in clock"), g_Config.m_TcStatusBarLocalTimeSeconds, &Button))
			g_Config.m_TcStatusBarLocalTimeSeconds ^= 1;

		Right.HSplitTop(20.0f, &Button, &Right);
		if(DoButton_CheckBox(&g_Config.m_TcStatusBarLabels, Localize("Show status bar labels"), g_Config.m_TcStatusBarLabels, &Button))
			g_Config.m_TcStatusBarLabels ^= 1;

		Right.HSplitTop(15.0f, nullptr, &Right);

		// War List Section
		Right.HSplitTop(25.0f, &Label, &Right);
		Ui()->DoLabel(&Label, Localize("War List & Indicators"), 18.0f, TEXTALIGN_ML);
		Right.HSplitTop(5.0f, nullptr, &Right);

		Right.HSplitTop(20.0f, &Button, &Right);
		if(DoButton_CheckBox(&g_Config.m_TcWarList, Localize("Enable war list"), g_Config.m_TcWarList, &Button))
			g_Config.m_TcWarList ^= 1;

		Right.HSplitTop(20.0f, &Button, &Right);
		if(DoButton_CheckBox(&g_Config.m_TcWarListReason, Localize("Show war reason"), g_Config.m_TcWarListReason, &Button))
			g_Config.m_TcWarListReason ^= 1;

		Right.HSplitTop(20.0f, &Button, &Right);
		if(DoButton_CheckBox(&g_Config.m_TcWarListChat, Localize("Show war colors in chat"), g_Config.m_TcWarListChat, &Button))
			g_Config.m_TcWarListChat ^= 1;

		Right.HSplitTop(20.0f, &Button, &Right);
		if(DoButton_CheckBox(&g_Config.m_TcWarListScoreboard, Localize("Show war colors in scoreboard"), g_Config.m_TcWarListScoreboard, &Button))
			g_Config.m_TcWarListScoreboard ^= 1;

		Right.HSplitTop(20.0f, &Button, &Right);
		if(DoButton_CheckBox(&g_Config.m_TcWarListIndicator, Localize("War list radial indicators"), g_Config.m_TcWarListIndicator, &Button))
			g_Config.m_TcWarListIndicator ^= 1;

		Right.HSplitTop(15.0f, nullptr, &Right);

		// Quality of Life
		Right.HSplitTop(25.0f, &Label, &Right);
		Ui()->DoLabel(&Label, Localize("Quality of Life"), 18.0f, TEXTALIGN_ML);
		Right.HSplitTop(5.0f, nullptr, &Right);

		Right.HSplitTop(20.0f, &Button, &Right);
		if(DoButton_CheckBox(&g_Config.m_TcShowFrozenHud, Localize("Show frozen tees HUD"), g_Config.m_TcShowFrozenHud, &Button))
			g_Config.m_TcShowFrozenHud ^= 1;

		Right.HSplitTop(20.0f, &Button, &Right);
		if(DoButton_CheckBox(&g_Config.m_TcNameplatePingCircle, Localize("Ping indicator circle in nameplates"), g_Config.m_TcNameplatePingCircle, &Button))
			g_Config.m_TcNameplatePingCircle ^= 1;

		Right.HSplitTop(20.0f, &Button, &Right);
		if(DoButton_CheckBox(&g_Config.m_TcNameplateCountry, Localize("Country flag in nameplates"), g_Config.m_TcNameplateCountry, &Button))
			g_Config.m_TcNameplateCountry ^= 1;

		Right.HSplitTop(20.0f, &Button, &Right);
		if(DoButton_CheckBox(&g_Config.m_TcResetBindWheelMouse, Localize("Reset mouse when opening bind wheel"), g_Config.m_TcResetBindWheelMouse, &Button))
			g_Config.m_TcResetBindWheelMouse ^= 1;

		Right.HSplitTop(20.0f, &Button, &Right);
		if(DoButton_CheckBox(&g_Config.m_TcTranslateAuto, Localize("Auto-translate chat messages"), g_Config.m_TcTranslateAuto, &Button))
			g_Config.m_TcTranslateAuto ^= 1;
	}
}
