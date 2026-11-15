// chroma. custom main menu and JSON theme engine
#include "chroma_menu.h"

#include <base/color.h>
#include <base/io.h>
#include <engine/graphics.h>
#include <engine/keys.h>
#include <engine/shared/config.h>
#include <engine/shared/json.h>
#include <engine/storage.h>
#include <engine/textrender.h>
#include <game/client/components/menus.h>
#include <game/client/gameclient.h>
#include <game/localization.h>
#include <game/version.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

static CChromaMenu::SMenuTheme s_ActiveTheme;
static int s_LoadedPreset = -1;
static std::string s_LoadedPath = "";
static IGraphics::CTextureHandle s_BackgroundTex;
static float s_aButtonHover[16] = {};

static std::vector<CChromaMenu::SDiscoveredTheme> s_vDiscoveredThemes;

static int ThemeScanCallback(const char *pName, int IsDir, int StorageType, void *pUser)
{
	if(!IsDir || pName[0] == '.')
		return 0;
	IStorage *pStorage = (IStorage *)pUser;
	char aThemeJson[512];
	str_format(aThemeJson, sizeof(aThemeJson), "chroma/themes/%s/theme.json", pName);
	if(pStorage->FileExists(aThemeJson, IStorage::TYPE_ALL))
	{
		CChromaMenu::SDiscoveredTheme Item;
		Item.m_Name = pName;
		Item.m_Path = aThemeJson;
		s_vDiscoveredThemes.push_back(Item);
	}
	return 0;
}

void CChromaMenu::ScanThemes(IStorage *pStorage)
{
	s_vDiscoveredThemes.clear();
	if(!pStorage)
		return;
	pStorage->ListDirectory(IStorage::TYPE_ALL, "chroma/themes", ThemeScanCallback, pStorage);
}

int CChromaMenu::GetDiscoveredThemeCount()
{
	return (int)s_vDiscoveredThemes.size();
}

const CChromaMenu::SDiscoveredTheme *CChromaMenu::GetDiscoveredTheme(int Index)
{
	if(Index < 0 || Index >= (int)s_vDiscoveredThemes.size())
		return nullptr;
	return &s_vDiscoveredThemes[Index];
}

void CChromaMenu::ApplyThemePath(const char *pPath)
{
	if(!pPath || !pPath[0])
		return;
	str_copy(g_Config.m_ChromaThemePath, pPath, sizeof(g_Config.m_ChromaThemePath));
	g_Config.m_ChromaMenuTheme = 4; // custom
	Reset();
}

static ColorRGBA ParseColor(const json_value *pVal, ColorRGBA DefaultCol)
{
	if(!pVal || pVal->type != json_array || pVal->u.array.length < 3)
		return DefaultCol;
	float r = (float)pVal->u.array.values[0]->u.dbl;
	float g = (float)pVal->u.array.values[1]->u.dbl;
	float b = (float)pVal->u.array.values[2]->u.dbl;
	float a = pVal->u.array.length >= 4 ? (float)pVal->u.array.values[3]->u.dbl : 1.0f;
	return ColorRGBA(r, g, b, a);
}

bool CChromaMenu::LoadTheme(IStorage *pStorage, const char *pFilePath, SMenuTheme &Theme)
{
	if(!pStorage || !pFilePath || !pFilePath[0])
		return false;

	IOHANDLE File = pStorage->OpenFile(pFilePath, IOFLAG_READ, IStorage::TYPE_ALL);
	if(!File)
		return false;

	const int64_t Length = io_length(File);
	if(Length <= 0 || Length > 1024 * 1024)
	{
		io_close(File);
		return false;
	}
	const size_t FileSize = (size_t)Length;

	std::string Buffer(FileSize, '\0');
	io_read(File, &Buffer[0], FileSize);
	io_close(File);

	std::unique_ptr<json_value, decltype(&json_value_free)> Root(JsonParse(Buffer.c_str(), Buffer.size()), json_value_free);
	if(!Root || Root->type != json_object)
		return false;

	Theme.m_vButtons.clear();

	for(unsigned i = 0; i < Root->u.object.length; ++i)
	{
		const char *pKey = Root->u.object.values[i].name;
		const json_value *pVal = Root->u.object.values[i].value;

		if(!str_comp(pKey, "name") && pVal->type == json_string)
			Theme.m_Name = pVal->u.string.ptr;
		else if(!str_comp(pKey, "background") && pVal->type == json_string)
			Theme.m_BackgroundPath = pVal->u.string.ptr;
		else if(!str_comp(pKey, "font") && pVal->type == json_string)
			Theme.m_Font = pVal->u.string.ptr;
		else if(!str_comp(pKey, "font_family") && pVal->type == json_string)
			Theme.m_FontFamily = pVal->u.string.ptr;
		else if(!str_comp(pKey, "background_fill"))
			Theme.m_BackgroundFill = ParseColor(pVal, Theme.m_BackgroundFill);
		else if(!str_comp(pKey, "greeting") && pVal->type == json_string)
			Theme.m_Greeting = pVal->u.string.ptr;
		else if(!str_comp(pKey, "greeting_font_size") && (pVal->type == json_integer || pVal->type == json_double))
			Theme.m_GreetingFontSize = (float)(pVal->type == json_integer ? pVal->u.integer : pVal->u.dbl);
		else if(!str_comp(pKey, "greeting_color"))
			Theme.m_GreetingColor = ParseColor(pVal, Theme.m_GreetingColor);
		else if(!str_comp(pKey, "greeting_x") && (pVal->type == json_integer || pVal->type == json_double))
			Theme.m_GreetingX = (float)(pVal->type == json_integer ? pVal->u.integer : pVal->u.dbl);
		else if(!str_comp(pKey, "greeting_y") && (pVal->type == json_integer || pVal->type == json_double))
			Theme.m_GreetingY = (float)(pVal->type == json_integer ? pVal->u.integer : pVal->u.dbl);
		else if(!str_comp(pKey, "menu_x") && (pVal->type == json_integer || pVal->type == json_double))
			Theme.m_MenuX = (float)(pVal->type == json_integer ? pVal->u.integer : pVal->u.dbl);
		else if(!str_comp(pKey, "menu_y") && (pVal->type == json_integer || pVal->type == json_double))
			Theme.m_MenuY = (float)(pVal->type == json_integer ? pVal->u.integer : pVal->u.dbl);
		else if(!str_comp(pKey, "button_font_size") && (pVal->type == json_integer || pVal->type == json_double))
			Theme.m_ButtonFontSize = (float)(pVal->type == json_integer ? pVal->u.integer : pVal->u.dbl);
		else if(!str_comp(pKey, "button_spacing") && (pVal->type == json_integer || pVal->type == json_double))
			Theme.m_ButtonSpacing = (float)(pVal->type == json_integer ? pVal->u.integer : pVal->u.dbl);
		else if(!str_comp(pKey, "text_color"))
			Theme.m_TextColor = ParseColor(pVal, Theme.m_TextColor);
		else if(!str_comp(pKey, "hover_color"))
			Theme.m_HoverColor = ParseColor(pVal, Theme.m_HoverColor);
		else if(!str_comp(pKey, "active_color"))
			Theme.m_ActiveColor = ParseColor(pVal, Theme.m_ActiveColor);
		else if(!str_comp(pKey, "show_version") && pVal->type == json_boolean)
			Theme.m_ShowVersion = pVal->u.boolean;
		else if(!str_comp(pKey, "watermark_text") && pVal->type == json_string)
			Theme.m_WatermarkText = pVal->u.string.ptr;
		else if(!str_comp(pKey, "buttons") && pVal->type == json_array)
		{
			for(unsigned b = 0; b < pVal->u.array.length; ++b)
			{
				if(pVal->u.array.values[b]->type != json_string)
					continue;
				const char *pBtn = pVal->u.array.values[b]->u.string.ptr;
				SThemeButton Btn;
				Btn.m_Key = pBtn;
				if(!str_comp(pBtn, "play")) Btn.m_Action = ACTION_PLAY;
				else if(!str_comp(pBtn, "settings")) Btn.m_Action = ACTION_SETTINGS;
				else if(!str_comp(pBtn, "skins") || !str_comp(pBtn, "dressing_room")) Btn.m_Action = ACTION_SKINS;
				else if(!str_comp(pBtn, "demos") || !str_comp(pBtn, "achievements")) Btn.m_Action = ACTION_DEMOS;
				else if(!str_comp(pBtn, "editor") || !str_comp(pBtn, "marketplace")) Btn.m_Action = ACTION_EDITOR;
				else if(!str_comp(pBtn, "server") || !str_comp(pBtn, "local_server")) Btn.m_Action = ACTION_SERVER;
				else if(!str_comp(pBtn, "quit") || !str_comp(pBtn, "exit") || !str_comp(pBtn, "save_and_quit")) Btn.m_Action = ACTION_QUIT;
				else Btn.m_Action = ACTION_UNKNOWN;
				Theme.m_vButtons.push_back(Btn);
			}
		}
	}
	return true;
}

void CChromaMenu::ApplyPreset(int Preset)
{
	g_Config.m_ChromaMenuTheme = Preset;
	Reset();
}

void CChromaMenu::Reset()
{
	s_LoadedPreset = -1;
	s_LoadedPath = "";
}

void CChromaMenu::Render(CGameClient *pGameClient, CUIRect MainView)
{
	if(!pGameClient || !g_Config.m_ChromaCustomMenu)
		return;

	// Check if theme needs reloading
	const int TargetPreset = g_Config.m_ChromaMenuTheme;
	const std::string TargetPath = TargetPreset == 1 ? "chroma/themes/chrome_dark/theme.json" :
	                              TargetPreset == 2 ? "chroma/themes/chrome_sword/theme.json" :
	                              TargetPreset == 3 ? "chroma/themes/fuji_sakura/theme.json" :
	                              g_Config.m_ChromaThemePath;

	if(s_LoadedPreset != TargetPreset || s_LoadedPath != TargetPath)
	{
		if(LoadTheme(pGameClient->Storage(), TargetPath.c_str(), s_ActiveTheme))
		{
			s_LoadedPreset = TargetPreset;
			s_LoadedPath = TargetPath;
			if(!s_ActiveTheme.m_BackgroundPath.empty())
			{
				s_BackgroundTex = pGameClient->Graphics()->LoadTexture(s_ActiveTheme.m_BackgroundPath.c_str(), IStorage::TYPE_ALL);
			}
		}
		else
		{
			// Fallback default
			s_ActiveTheme.m_Name = "Default";
			s_ActiveTheme.m_BackgroundFill = ColorRGBA(0.04f, 0.04f, 0.05f, 1.0f);
			s_ActiveTheme.m_Greeting = "Hello, {player}";
			s_ActiveTheme.m_GreetingFontSize = 26.0f;
			s_ActiveTheme.m_GreetingColor = ColorRGBA(0.9f, 0.9f, 0.92f, 1.0f);
			s_ActiveTheme.m_GreetingX = 0.12f;
			s_ActiveTheme.m_GreetingY = 0.36f;
			s_ActiveTheme.m_MenuX = 0.12f;
			s_ActiveTheme.m_MenuY = 0.44f;
			s_ActiveTheme.m_ButtonFontSize = 14.0f;
			s_ActiveTheme.m_ButtonSpacing = 20.0f;
			s_ActiveTheme.m_TextColor = ColorRGBA(0.72f, 0.74f, 0.78f, 1.0f);
			s_ActiveTheme.m_HoverColor = ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
			s_ActiveTheme.m_vButtons = {
				{"play", "Play", ACTION_PLAY},
				{"settings", "Settings", ACTION_SETTINGS},
				{"skins", "Skins", ACTION_SKINS},
				{"demos", "Demos", ACTION_DEMOS},
				{"editor", "Editor", ACTION_EDITOR},
				{"quit", "Save & Quit", ACTION_QUIT}
			};
			s_LoadedPreset = TargetPreset;
			s_LoadedPath = TargetPath;
		}
	}

	// 1. Render Full-Viewport Background (covers the entire screen with 0 margin leakage)
	const CUIRect Screen = *pGameClient->Ui()->Screen();
	pGameClient->Graphics()->DrawRect(Screen.x, Screen.y, Screen.w, Screen.h, s_ActiveTheme.m_BackgroundFill, IGraphics::CORNER_NONE, 0.0f);
	if(s_BackgroundTex.IsValid())
	{
		pGameClient->Graphics()->TextureSet(s_BackgroundTex);
		pGameClient->Graphics()->QuadsBegin();
		pGameClient->Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		IGraphics::CQuadItem Quad(Screen.x, Screen.y, Screen.w, Screen.h);
		pGameClient->Graphics()->QuadsDrawTL(&Quad, 1);
		pGameClient->Graphics()->QuadsEnd();
	}

	// Dynamic text contrast adaptation to background brightness
	const float Luma = s_ActiveTheme.m_BackgroundFill.r * 0.299f + s_ActiveTheme.m_BackgroundFill.g * 0.587f + s_ActiveTheme.m_BackgroundFill.b * 0.114f;
	const bool BrightBg = Luma > 0.55f;

	ColorRGBA GreetingColor = s_ActiveTheme.m_GreetingColor;
	if(BrightBg && GreetingColor.r > 0.7f && GreetingColor.g > 0.7f && GreetingColor.b > 0.7f)
		GreetingColor = ColorRGBA(0.08f, 0.08f, 0.09f, 1.0f);
	else if(!BrightBg && GreetingColor.r < 0.25f && GreetingColor.g < 0.25f && GreetingColor.b < 0.25f)
		GreetingColor = ColorRGBA(0.92f, 0.92f, 0.95f, 1.0f);

	ColorRGBA BaseTextColor = s_ActiveTheme.m_TextColor;
	ColorRGBA BaseHoverColor = s_ActiveTheme.m_HoverColor;
	if(BrightBg && BaseTextColor.r > 0.7f && BaseTextColor.g > 0.7f && BaseTextColor.b > 0.7f)
	{
		BaseTextColor = ColorRGBA(0.12f, 0.12f, 0.14f, 1.0f);
		BaseHoverColor = ColorRGBA(0.35f, 0.35f, 0.38f, 1.0f);
	}
	else if(!BrightBg && BaseTextColor.r < 0.25f && BaseTextColor.g < 0.25f && BaseTextColor.b < 0.25f)
	{
		BaseTextColor = ColorRGBA(0.75f, 0.77f, 0.82f, 1.0f);
		BaseHoverColor = ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
	}

	const ColorRGBA OutlineCol = BrightBg ? ColorRGBA(1.0f, 1.0f, 1.0f, 0.40f) : ColorRGBA(0.0f, 0.0f, 0.0f, 0.65f);

	// 1.5. Set font preset if theme requests custom/pixel font
	const bool UsePixelFont = !s_ActiveTheme.m_FontFamily.empty() ||
		s_ActiveTheme.m_Font.find("pixel") != std::string::npos ||
		s_ActiveTheme.m_Font.find("Pixel") != std::string::npos ||
		s_ActiveTheme.m_Name.find("Pixel") != std::string::npos;
	if(UsePixelFont)
		pGameClient->TextRender()->SetFontPreset(EFontPreset::PIXEL_FONT);

	// 2. Render Greeting with dynamic player nickname ("Hello, <Nick>")
	std::string Nick = pGameClient->Client()->PlayerName();
	if(Nick.empty() || !str_comp(Nick.c_str(), "nameless tee"))
		Nick = "Tee";

	std::string GreetingText = s_ActiveTheme.m_Greeting;
	if(!GreetingText.empty())
	{
		size_t Pos = GreetingText.find("{player}");
		if(Pos != std::string::npos)
		{
			GreetingText.replace(Pos, 8, Nick);
		}
		else if((Pos = GreetingText.find("chrome haert")) != std::string::npos)
		{
			GreetingText.replace(Pos, 12, Nick);
		}
		else if(GreetingText == "Hello, " || GreetingText == "Hello")
		{
			GreetingText = "Hello, " + Nick;
		}
	}
	else if(s_ActiveTheme.m_GreetingFontSize > 0.001f)
	{
		GreetingText = "Hello, " + Nick;
	}

	if(!GreetingText.empty())
	{
		float Size = s_ActiveTheme.m_GreetingFontSize;
		const float MaxWidth = Screen.w * 0.75f;
		const float TextW = pGameClient->TextRender()->TextWidth(Size, GreetingText.c_str());
		if(TextW > MaxWidth && TextW > 0.001f)
			Size *= (MaxWidth / TextW);

		const float GX = Screen.x + Screen.w * s_ActiveTheme.m_GreetingX;
		const float GY = Screen.y + Screen.h * s_ActiveTheme.m_GreetingY;
		pGameClient->TextRender()->TextColor(GreetingColor);
		pGameClient->TextRender()->TextOutlineColor(OutlineCol);
		pGameClient->TextRender()->Text(GX, GY, Size, GreetingText.c_str());
		pGameClient->TextRender()->TextColor(pGameClient->TextRender()->DefaultTextColor());
		pGameClient->TextRender()->TextOutlineColor(pGameClient->TextRender()->DefaultTextOutlineColor());
	}

	// 3. Render Buttons with Auto-Fit Text
	const float BaseX = Screen.x + Screen.w * s_ActiveTheme.m_MenuX;
	float CurY = Screen.y + Screen.h * s_ActiveTheme.m_MenuY;
	const float Dt = std::clamp(pGameClient->Client()->RenderFrameTime(), 0.001f, 0.1f);
	const bool IsRu = g_Config.m_ChromaLanguage == 1;

	for(size_t i = 0; i < s_ActiveTheme.m_vButtons.size(); ++i)
	{
		const auto &Btn = s_ActiveTheme.m_vButtons[i];
		const char *pLabel = "";

		switch(Btn.m_Action)
		{
		case ACTION_PLAY: pLabel = Localize("Play", "Start menu"); break;
		case ACTION_SETTINGS: pLabel = Localize("Settings"); break;
		case ACTION_SKINS: pLabel = IsRu ? "Гардероб (Скины)" : "Skins"; break;
		case ACTION_DEMOS: pLabel = IsRu ? "Демо" : "Demos"; break;
		case ACTION_EDITOR: pLabel = IsRu ? "Редактор" : "Editor"; break;
		case ACTION_SERVER:
			pLabel = pGameClient->m_LocalServer.IsServerRunning() ? Localize("Stop server") : Localize("Run server");
			break;
		case ACTION_QUIT: pLabel = IsRu ? "Выход из игры" : "Save & Quit"; break;
		default: pLabel = Btn.m_Key.c_str(); break;
		}

		// Calculate text width & auto-fit
		float FontSize = s_ActiveTheme.m_ButtonFontSize;
		const float MaxBtnW = Screen.w * 0.32f;
		const float TextWidth = pGameClient->TextRender()->TextWidth(FontSize, pLabel);
		if(TextWidth > MaxBtnW && TextWidth > 0.001f)
			FontSize *= (MaxBtnW / TextWidth);

		const float BoxW = std::max(TextWidth + 24.0f, 140.0f);
		const float BoxH = s_ActiveTheme.m_ButtonSpacing + 2.0f;
		CUIRect BtnRect = {BaseX, CurY, BoxW, BoxH};

		// Hover smooth animation
		const bool Hover = pGameClient->Ui()->MouseInside(&BtnRect);
		const size_t Slot = std::min(i, (size_t)15);
		s_aButtonHover[Slot] += ((Hover ? 1.0f : 0.0f) - s_aButtonHover[Slot]) * (1.0f - std::exp(-Dt * 14.0f));
		const float H = s_aButtonHover[Slot];

		// Text Color blend with background adaptation
		const ColorRGBA TextCol(
			BaseTextColor.r + (BaseHoverColor.r - BaseTextColor.r) * H,
			BaseTextColor.g + (BaseHoverColor.g - BaseTextColor.g) * H,
			BaseTextColor.b + (BaseHoverColor.b - BaseTextColor.b) * H,
			BaseTextColor.a + (BaseHoverColor.a - BaseTextColor.a) * H);

		// Subtle hover bar
		if(H > 0.01f)
		{
			pGameClient->Graphics()->DrawRect(BaseX - 6.0f, CurY + 2.0f, 2.5f * H, BoxH - 4.0f,
				BaseHoverColor.WithAlpha(0.75f * H), IGraphics::CORNER_ALL, 1.0f);
		}

		pGameClient->TextRender()->TextColor(TextCol);
		pGameClient->TextRender()->TextOutlineColor(OutlineCol);
		pGameClient->TextRender()->Text(BaseX + (H * 4.0f), CurY + 2.0f, FontSize, pLabel);
		pGameClient->TextRender()->TextColor(pGameClient->TextRender()->DefaultTextColor());
		pGameClient->TextRender()->TextOutlineColor(pGameClient->TextRender()->DefaultTextOutlineColor());

		// Button logic click
		if(pGameClient->Ui()->MouseButtonClicked(0) && Hover)
		{
			switch(Btn.m_Action)
			{
			case ACTION_PLAY:
				pGameClient->m_Menus.SetShowStart(false);
				pGameClient->m_Menus.SetMenuPage(CMenus::PAGE_INTERNET);
				break;
			case ACTION_SETTINGS:
				pGameClient->m_Menus.SetShowStart(false);
				pGameClient->m_Menus.SetMenuPage(CMenus::PAGE_SETTINGS);
				break;
			case ACTION_SKINS:
				pGameClient->m_Menus.SetShowStart(false);
				g_Config.m_UiSettingsPage = CMenus::SETTINGS_TEE;
				pGameClient->m_Menus.SetMenuPage(CMenus::PAGE_SETTINGS);
				break;
			case ACTION_DEMOS:
				pGameClient->m_Menus.SetShowStart(false);
				pGameClient->m_Menus.SetMenuPage(CMenus::PAGE_DEMOS);
				break;
			case ACTION_EDITOR:
				g_Config.m_ClEditor = 1;
				pGameClient->Input()->MouseModeRelative();
				break;
			case ACTION_SERVER:
				if(pGameClient->m_LocalServer.IsServerRunning())
					pGameClient->m_LocalServer.KillServer();
				else
					pGameClient->m_LocalServer.RunServer({});
				break;
			case ACTION_QUIT:
				pGameClient->m_Menus.ShowQuitPopup();
				break;
			default: break;
			}
		}

		CurY += s_ActiveTheme.m_ButtonSpacing + 5.0f;
	}

	// 4. Watermark & Escape key handling
	if(pGameClient->Ui()->ConsumeHotkey(CUi::HOTKEY_ESCAPE) || pGameClient->Input()->KeyPress(KEY_ESCAPE))
	{
		pGameClient->m_Menus.ShowQuitPopup();
	}

	if(s_ActiveTheme.m_ShowVersion || !s_ActiveTheme.m_WatermarkText.empty())
	{
		const char *pWatermark = !s_ActiveTheme.m_WatermarkText.empty() ? s_ActiveTheme.m_WatermarkText.c_str() : "chroma. | DDNet 20.3";
		pGameClient->TextRender()->TextColor(BrightBg ? ColorRGBA(0.15f, 0.15f, 0.18f, 0.70f) : ColorRGBA(0.85f, 0.85f, 0.88f, 0.70f));
		pGameClient->TextRender()->Text(Screen.x + 18.0f, Screen.y + Screen.h - 26.0f, 10.0f, pWatermark);
		pGameClient->TextRender()->TextColor(pGameClient->TextRender()->DefaultTextColor());
	}

	if(UsePixelFont)
		pGameClient->TextRender()->SetFontPreset(EFontPreset::DEFAULT_FONT);
}
