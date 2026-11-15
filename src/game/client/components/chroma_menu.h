#ifndef GAME_CLIENT_COMPONENTS_CHROMA_MENU_H
#define GAME_CLIENT_COMPONENTS_CHROMA_MENU_H

#include <base/color.h>
#include <base/vmath.h>
#include <engine/graphics.h>
#include <game/client/ui_rect.h>
#include <string>
#include <vector>

class CGameClient;

class CChromaMenu
{
public:
	enum EButtonAction
	{
		ACTION_PLAY = 0,
		ACTION_SETTINGS,
		ACTION_SKINS,
		ACTION_DEMOS,
		ACTION_EDITOR,
		ACTION_SERVER,
		ACTION_QUIT,
		ACTION_UNKNOWN,
	};

	struct SThemeButton
	{
		std::string m_Key;
		std::string m_Label;
		EButtonAction m_Action;
	};

	struct SMenuTheme
	{
		std::string m_Name;
		std::string m_BackgroundPath;
		std::string m_Font;
		std::string m_FontFamily;
		ColorRGBA m_BackgroundFill = ColorRGBA(0.04f, 0.04f, 0.05f, 1.0f);
		std::string m_Greeting;
		float m_GreetingFontSize = 26.0f;
		ColorRGBA m_GreetingColor = ColorRGBA(0.9f, 0.9f, 0.9f, 1.0f);
		float m_GreetingX = 0.12f;
		float m_GreetingY = 0.38f;
		float m_MenuX = 0.12f;
		float m_MenuY = 0.46f;
		float m_ButtonFontSize = 14.0f;
		float m_ButtonSpacing = 18.0f;
		ColorRGBA m_TextColor = ColorRGBA(0.75f, 0.75f, 0.78f, 1.0f);
		ColorRGBA m_HoverColor = ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
		ColorRGBA m_ActiveColor = ColorRGBA(0.4f, 0.75f, 1.0f, 1.0f);
		bool m_ShowVersion = false;
		std::string m_WatermarkText;
		std::vector<SThemeButton> m_vButtons;
	};

	struct SDiscoveredTheme
	{
		std::string m_Name;
		std::string m_Path;
	};

	static bool LoadTheme(class IStorage *pStorage, const char *pFilePath, SMenuTheme &Theme);
	static void Render(CGameClient *pGameClient, CUIRect MainView);
	static void ApplyPreset(int Preset);
	static void Reset();
	static void ScanThemes(class IStorage *pStorage);
	static int GetDiscoveredThemeCount();
	static const SDiscoveredTheme *GetDiscoveredTheme(int Index);
	static void ApplyThemePath(const char *pPath);
};

#endif // GAME_CLIENT_COMPONENTS_CHROMA_MENU_H
