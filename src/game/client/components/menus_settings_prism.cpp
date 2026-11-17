// chroma. additions, distributed under the zlib license in license.txt.
#include "menus.h"
#include <game/client/gameclient.h>

#include <algorithm>
#include <cmath>

#include <engine/client/keyboard.h>
#include <engine/keys.h>
#include <engine/shared/config.h>
#include <game/client/animstate.h>
#include <game/client/components/chroma_menu.h>
#include <game/client/prism.h>
#include <game/client/prism_qol.h>
#include <game/client/prism_theme.h>
#include <game/client/prism_ui.h>
#include <game/client/prism_version.h>
#include <game/client/ui_scrollregion.h>
#include <game/localization.h>

static const char *LocalizeChroma(const char *pText)
{
	if(g_Config.m_ChromaLanguage == 0)
		return pText;

	if(!str_comp(pText, "Visuals")) return "Визуалы";
	if(!str_comp(pText, "Hat")) return "Панама";
	if(!str_comp(pText, "HUD")) return "Интерфейс";
	if(!str_comp(pText, "Themes")) return "Темы";
	if(!str_comp(pText, "Settings")) return "Настройки";
	if(!str_comp(pText, "Visual presets")) return "Пресеты визуалов";
	if(!str_comp(pText, "Default")) return "По умолчанию";
	if(!str_comp(pText, "Clean")) return "Чистый";
	if(!str_comp(pText, "Competitive")) return "Соревновательный";
	if(!str_comp(pText, "Cinematic")) return "Кинематографичный";
	if(!str_comp(pText, "Custom")) return "Пользовательский";
	if(!str_comp(pText, "Tee   /   local player")) return "Тии   /   свой персонаж";
	if(!str_comp(pText, "Outline")) return "Обводка";
	if(!str_comp(pText, "Outline width")) return "Толщина обводки";
	if(!str_comp(pText, "Outline color")) return "Цвет обводки";
	if(!str_comp(pText, "Glow")) return "Свечение";
	if(!str_comp(pText, "Glow strength")) return "Сила свечения";
	if(!str_comp(pText, "Glow color")) return "Цвет свечения";
	if(!str_comp(pText, "Tee   /   other players")) return "Тии   /   другие игроки";
	if(!str_comp(pText, "Hook   /   local player")) return "Хук   /   свой персонаж";
	if(!str_comp(pText, "Custom color")) return "Свой цвет";
	if(!str_comp(pText, "Hook color")) return "Цвет хука";
	if(!str_comp(pText, "Hook   /   other players")) return "Хук   /   другие игроки";
	if(!str_comp(pText, "Hook particles  /  cosmetic only")) return "Частицы хука  /  косметика";
	if(!str_comp(pText, "Local hook")) return "Свой хук";
	if(!str_comp(pText, "Other visible hooks")) return "Хуки других игроков";
	if(!str_comp(pText, "Particle color")) return "Цвет частиц";
	if(!str_comp(pText, "Particle rate")) return "Частота частиц";
	if(!str_comp(pText, "Particle cap")) return "Макс. частиц";
	if(!str_comp(pText, "Particle size")) return "Размер частиц";
	if(!str_comp(pText, "Particle lifetime")) return "Время жизни";
	if(!str_comp(pText, "Particle opacity")) return "Прозрачность";
	if(!str_comp(pText, "Particle spread")) return "Разброс частиц";
	if(!str_comp(pText, "Particle velocity")) return "Скорость частиц";
	if(!str_comp(pText, "Particle fade")) return "Затухание частиц";
	if(!str_comp(pText, "Particle glow")) return "Свечение частиц";
	if(!str_comp(pText, "Pulse frequency")) return "Частота пульсации";
	if(!str_comp(pText, "Pulse intensity")) return "Сила пульсации";
	if(!str_comp(pText, "Player trail")) return "Трейл движения";
	if(!str_comp(pText, "Local trail")) return "Свой трейл";
	if(!str_comp(pText, "Other visible trails")) return "Трейлы других игроков";
	if(!str_comp(pText, "Trail color")) return "Цвет трейла";
	if(!str_comp(pText, "Trail width")) return "Ширина трейла";
	if(!str_comp(pText, "Trail lifetime")) return "Длина трейла";
	if(!str_comp(pText, "Sample interval")) return "Интервал сэмплирования";
	if(!str_comp(pText, "Trail opacity")) return "Прозрачность трейла";
	if(!str_comp(pText, "Trail fade")) return "Затухание трейла";
	if(!str_comp(pText, "Trail glow")) return "Свечение трейла";
	if(!str_comp(pText, "Player highlight  /  normally visible Tees")) return "Подсветка тии  /  видимые игроки";
	if(!str_comp(pText, "Highlight")) return "Подсветка";
	if(!str_comp(pText, "Local Tee")) return "Свой тии";
	if(!str_comp(pText, "Other visible Tees")) return "Другие видимые тии";
	if(!str_comp(pText, "Highlight color")) return "Цвет подсветки";
	if(!str_comp(pText, "Fill opacity")) return "Прозрачность заливки";
	if(!str_comp(pText, "Outline opacity")) return "Прозрачность рамки";
	if(!str_comp(pText, "Box scale")) return "Масштаб рамки";
	if(!str_comp(pText, "Corner radius")) return "Скругление углов";
	if(!str_comp(pText, "Chinese Conical Hat   /   斗笠")) return "Коническая панама   /   斗笠";
	if(!str_comp(pText, "Enable Chinese Hat")) return "Включить панаму";
	if(!str_comp(pText, "Hat base color")) return "Основной цвет панамы";
	if(!str_comp(pText, "Rib lines color")) return "Цвет ребер панамы";
	if(!str_comp(pText, "Facet opacity (%)")) return "Прозрачность ткани (%)";
	if(!str_comp(pText, "Rib lines opacity (%)")) return "Прозрачность ребер (%)";
	if(!str_comp(pText, "Animation speed (%)")) return "Скорость анимации (%)";
	if(!str_comp(pText, "Hat size scale (%)")) return "Размер панамы (%)";
	if(!str_comp(pText, "Vertical offset")) return "Смещение по высоте";
	if(!str_comp(pText, "Tilt with movement & aim")) return "Наклон при движении и прицеле";
	if(!str_comp(pText, "Show on local player")) return "Показывать на себе";
	if(!str_comp(pText, "Show on other players")) return "Показывать на других";
	if(!str_comp(pText, "Modular HUD   /   drag handles to reposition")) return "Модульный HUD   /   перетаскивание";
	if(!str_comp(pText, "Enable all modules")) return "Включить все модули";
	if(!str_comp(pText, "HUD scale (%)")) return "Масштаб HUD (%)";
	if(!str_comp(pText, "HUD opacity (%)")) return "Прозрачность HUD (%)";
	if(!str_comp(pText, "HUD background")) return "Фон виджетов HUD";
	if(!str_comp(pText, "Snap to guides")) return "Прилипание к направляющим";
	if(!str_comp(pText, "Snap to screen edges")) return "Прилипание к краям";
	if(!str_comp(pText, "Lock HUD layout")) return "Закрепить позиции HUD";
	if(!str_comp(pText, "Drag widgets on game screen")) return "Перетаскивать на экране игры";
	if(!str_comp(pText, "Show guides")) return "Показывать линии привязки";
	if(!str_comp(pText, "Padding")) return "Отступы";
	if(!str_comp(pText, "Rounding")) return "Скругление";
	if(!str_comp(pText, "Font size")) return "Размер шрифта";
	if(!str_comp(pText, "Physical input overlay")) return "Отображение нажатий (Keystrokes)";
	if(!str_comp(pText, "Key size")) return "Размер кнопок";
	if(!str_comp(pText, "Key rounding")) return "Скругление кнопок";
	if(!str_comp(pText, "Action labels")) return "Подписи клавиш";
	if(!str_comp(pText, "Key animation")) return "Анимация клавиш";
	if(!str_comp(pText, "Pressed key")) return "Нажатая клавиша";
	if(!str_comp(pText, "Released key")) return "Отпущенная клавиша";
	if(!str_comp(pText, "Layout preview  /  drag enabled modules")) return "Предпросмотр раскладки";
	if(!str_comp(pText, "Reset HUD layout")) return "Сбросить раскладку HUD";
	if(!str_comp(pText, "Themes   /   chroma. identity")) return "Темы оформления главного меню и GUI";
	if(!str_comp(pText, "Custom JSON main menu")) return "Свое главное меню из JSON";
	if(!str_comp(pText, "Select menu theme preset")) return "Выбор темы главного меню";
	if(!str_comp(pText, "Theme: Chroma Dark")) return "Тема: Chroma Dark";
	if(!str_comp(pText, "Theme: Minecraft Sakura")) return "Тема: Minecraft Sakura";
	if(!str_comp(pText, "Theme: Chroma Light")) return "Тема: Chroma Light";
	if(!str_comp(pText, "Theme: Default DDNet")) return "Тема: Стандартное меню DDNet";
	if(!str_comp(pText, "Reload / apply custom JSON theme")) return "Перезагрузить JSON тему из файла";
	if(!str_comp(pText, "Accent")) return "Акцентный цвет";
	if(!str_comp(pText, "Background")) return "Цвет фона";
	if(!str_comp(pText, "Panel")) return "Цвет панелей";
	if(!str_comp(pText, "Text")) return "Цвет текста";
	if(!str_comp(pText, "Heading width (%)")) return "Ширина заголовков (%)";
	if(!str_comp(pText, "Border opacity")) return "Прозрачность рамки";
	if(!str_comp(pText, "Shadow opacity")) return "Прозрачность тени";
	if(!str_comp(pText, "chroma.   /   Control center")) return "chroma.   /   Центр управления";
	if(!str_comp(pText, "Enable chroma. visuals")) return "Включить визуалы chroma.";
	if(!str_comp(pText, "Modular HUD")) return "Модульный HUD";
	if(!str_comp(pText, "Performance   /   measured locally")) return "Производительность";
	if(!str_comp(pText, "FPS and frame-time overlay")) return "Оверлей FPS и времени кадра";
	if(!str_comp(pText, "Performance HUD panel")) return "Панель FPS и Ping";
	if(!str_comp(pText, "Menu scale")) return "Масштаб меню";
	if(!str_comp(pText, "Glass opacity")) return "Прозрачность стекла";
	if(!str_comp(pText, "Background darkness")) return "Затемнение фона";
	if(!str_comp(pText, "Glass tint")) return "Тонировка стекла";
	if(!str_comp(pText, "Animations")) return "Анимации";
	if(!str_comp(pText, "Animation duration (ms)")) return "Длительность анимаций (мс)";
	if(!str_comp(pText, "Reduce animation")) return "Уменьшить движение";
	if(!str_comp(pText, "Lock ClickGUI position")) return "Закрепить позицию меню";
	if(!str_comp(pText, "Snap ClickGUI to edges")) return "Прилипание меню к краям";
	if(!str_comp(pText, "Reset ClickGUI position")) return "Сбросить позицию меню";
	if(!str_comp(pText, "Reset visual presets only")) return "Сбросить только пресеты визуалов";
	if(!str_comp(pText, "Verified Staff")) return "Персонал сервера";
	if(!str_comp(pText, "Input Overlay")) return "Оверлей клавиш";
	if(!str_comp(pText, "Effects Status")) return "Статус эффектов";
	if(!str_comp(pText, "Active Hotkeys")) return "Активные бинды";
	if(!str_comp(pText, "chroma. name/version")) return "Имя и версия chroma.";
	if(!str_comp(pText, "Performance")) return "Производительность";
	if(!str_comp(pText, "Dummy Status")) return "Статус dummy";
	if(!str_comp(pText, "Module scale: Active Hotkeys")) return "Масштаб: Активные бинды";
	if(!str_comp(pText, "Module scale: Identity")) return "Масштаб: Имя и версия";
	if(!str_comp(pText, "Module scale: Performance")) return "Масштаб: Производительность";
	if(!str_comp(pText, "Module scale: Dummy")) return "Масштаб: Статус dummy";
	if(!str_comp(pText, "Module scale: Staff")) return "Масштаб: Персонал";
	if(!str_comp(pText, "Module scale: Input")) return "Масштаб: Ввод (клавиши)";
	if(!str_comp(pText, "Module scale: Effects")) return "Масштаб: Эффекты";
	if(!str_comp(pText, "Minimal")) return "Минимальный";
	if(!str_comp(pText, "Streaming")) return "Стримерский";
	if(!str_comp(pText, "Mode: Custom Static Color")) return "Режим: Свой цвет";
	if(!str_comp(pText, "Mode: Rainbow Gradient Wave")) return "Режим: Радужная волна";
	if(!str_comp(pText, "Mode: Shimmering Monochrome Luster")) return "Режим: Мерцающий блеск";
	if(!str_comp(pText, "Mode: Off")) return "Режим: Выкл";
	if(!str_comp(pText, "Mode: Leaves")) return "Режим: Листья";
	if(!str_comp(pText, "Mode: Sparkles")) return "Режим: Искры";
	if(!str_comp(pText, "Mode: Hearts")) return "Режим: Сердечки";
	if(!str_comp(pText, "Mode: Pulse")) return "Режим: Пульс";
	if(!str_comp(pText, "Trail: Off")) return "Трейл: Выкл";
	if(!str_comp(pText, "Trail: Line")) return "Трейл: Линия";
	if(!str_comp(pText, "Trail: Ribbon")) return "Трейл: Лента";
	if(!str_comp(pText, "Trail: Particles")) return "Трейл: Частицы";
	if(!str_comp(pText, "UI Color Palette")) return "Палитра интерфейса";
	if(!str_comp(pText, "Menu cursor: System")) return "Курсор меню: Системный";
	if(!str_comp(pText, "Menu cursor: DDNet")) return "Курсор меню: DDNet";
	if(!str_comp(pText, "Scan / reload custom themes from disk")) return "Сканировать темы с диска";
	if(!str_comp(pText, "Discovered custom themes")) return "Найденные темы на диске";

	return pText;
}

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
	const float BackdropDarkness = g_Config.m_PrismHudEdit && m_PrismCategory == 2 ? 0.28f : g_Config.m_PrismGlassDarkness / 100.0f;
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
		Grab.h = std::min(32.0f, Panel.h);
		if(!m_PrismDragging && !Ui()->IsPopupOpen() && Ui()->ActiveItem() == nullptr && Ui()->MouseButtonClicked(0) && Ui()->MouseInside(&Grab))
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

	if(m_PrismOpen && m_PrismCategory == 2 && g_Config.m_PrismHudEdit && g_Config.m_PrismHudEnabled)
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

	// Clean layout: No top close button, pure modern panel with bottom footer
	CUIRect Inner = Panel;
	Inner.Margin(11.0f, &Inner);
	CUIRect Body, Footer;
	Inner.HSplitBottom(22.0f, &Body, &Footer);

	CUIRect Sidebar, Content;
	Body.VSplitLeft(std::min(116.0f * Scale, Body.w * 0.25f), &Sidebar, &Content);
	Sidebar.VSplitRight(7.0f, &Sidebar, nullptr);
	Sidebar.Draw(Theme.m_Background.WithAlpha(0.91f * Fade), IGraphics::CORNER_ALL, 8.0f);
	Sidebar.Margin(6.0f, &Sidebar);
	Content.Draw(Theme.m_Background.WithAlpha(0.83f * Fade), IGraphics::CORNER_ALL, 8.0f);
	Content.Margin(8.0f, &Content);

	static const char *s_apTabsEn[5] = {"Visuals", "Hat", "HUD", "Themes", "Settings"};
	static const char *s_apTabsRu[5] = {"Визуалы", "Панама", "Интерфейс", "Темы", "Настройки"};
	static CButtonContainer s_aTabs[5];
	static float s_aTabBlend[5] = {};
	for(int i = 0; i < 5; ++i)
	{
		CUIRect Tab;
		Sidebar.HSplitTop(29.0f, &Tab, &Sidebar);
		Sidebar.HSplitTop(3.0f, nullptr, &Sidebar);
		const float Target = m_PrismCategory == i ? 1.0f : 0.0f;
		s_aTabBlend[i] += (Target - s_aTabBlend[i]) * Motion;
		const float Highlight = s_aTabBlend[i];
		const ColorRGBA ButtonColor(Theme.m_Accent.r, Theme.m_Accent.g, Theme.m_Accent.b, 0.06f + Highlight * 0.36f);
		const char *pTabTitle = g_Config.m_ChromaLanguage == 1 ? s_apTabsRu[i] : s_apTabsEn[i];
		if(DoButton_Menu(&s_aTabs[i], pTabTitle, 0, &Tab, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 6.0f, 0.41f, ButtonColor))
		{
			m_PrismCategory = i;
			Ui()->SetActiveItem(nullptr);
		}
	}

	static CScrollRegion s_aScroll[5];
	CUIRect ScrollView = Content;
	if(m_PrismCategory < 0 || m_PrismCategory >= 5)
		m_PrismCategory = 0;
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
			Ui()->DoLabel(&Row, LocalizeChroma(pLabel), 12.0f, TEXTALIGN_ML);
	};
	static float s_aaToggleProgress[5][96] = {};
	static float s_aaToggleHover[5][96] = {};
	static bool s_aaToggleInitialized[5][96] = {};
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
		Ui()->DoLabel(&LabelRect, LocalizeChroma(pLabel), 11.0f, TEXTALIGN_ML);
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
			Ui()->DoScrollbarOption(pValue, pValue, &Row, LocalizeChroma(pLabel), Min, Max);
		}
	};
	static CButtonContainer s_aColors[64];
	auto Color = [&](const char *pLabel, unsigned *pValue, int Index) {
		CUIRect Row = NextRow(27.0f);
		if(!Scroll.RectClipped(Row))
			DoLine_ColorPicker(&s_aColors[Index], 24.0f, 12.0f, 3.0f, &Row, LocalizeChroma(pLabel), pValue, ColorRGBA(0.65f, 0.78f, 0.91f, 1.0f), false, nullptr, Index == 9 || Index == 10);
	};

	static CButtonContainer s_aActionButtons[64];
	auto Button = [&](const char *pText, int Id, const ColorRGBA &ButtonTint = ColorRGBA(0.37f, 0.47f, 0.58f, 0.25f)) {
		CUIRect Row = NextRow(29.0f);
		return !Scroll.RectClipped(Row) && DoButton_Menu(&s_aActionButtons[Id], LocalizeChroma(pText), 0, &Row, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 6.0f, 0.41f, ButtonTint);
	};

	switch(m_PrismCategory)
	{
	case 0: // Visuals
	{
		Label("Visual presets");
		static const char *s_apPresets[] = {"Default", "Clean", "Competitive", "Cinematic", "Custom"};
		static CButtonContainer s_aPresetButtons[5];
		for(int i = 0; i < 5; ++i)
		{
			CUIRect Row = NextRow(32.0f);
			if(!Scroll.RectClipped(Row) && DoButton_Menu(&s_aPresetButtons[i], LocalizeChroma(s_apPresets[i]), 0, &Row, BUTTONFLAG_LEFT, nullptr,
				IGraphics::CORNER_ALL, 7.0f, 0.45f, g_Config.m_PrismPreset == i ? ColorRGBA(0.39f, 0.56f, 0.71f, 0.43f) : ColorRGBA(0.34f, 0.42f, 0.50f, 0.18f)))
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

	case 2: // HUD + Physical Input Overlay
	{
		Label("Modular HUD   /   drag handles to reposition");
		Toggle("Enable all modules", &g_Config.m_PrismHudEnabled);
		Slider("HUD scale (%)", &g_Config.m_PrismHudScale, 65, 150);
		Slider("HUD opacity (%)", &g_Config.m_PrismHudOpacity, 20, 100);
		static const char *s_apModuleNames[] = {"Active Hotkeys", "chroma. name/version", "Performance", "Dummy Status", "Verified Staff", "Input Overlay", "Effects Status"};
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
		CUIRect Preview = NextRow(155.0f);
		if(!Scroll.RectClipped(Preview))
		{
			Preview.Draw(ColorRGBA(0.018f, 0.031f, 0.047f, 0.75f), IGraphics::CORNER_ALL, 8.0f);
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
		break;
	}

	case 3: // Themes
	{
		Label("Themes   /   chroma. identity");
		Toggle("Custom JSON main menu", &g_Config.m_ChromaCustomMenu);
		Label("Select menu theme preset");
		if(Button("Theme: Chroma Dark", 18, g_Config.m_ChromaMenuTheme == 1 ? ColorRGBA(0.42f, 0.62f, 0.82f, 0.55f) : ColorRGBA(0.32f, 0.40f, 0.48f, 0.22f)))
		{
			CChromaMenu::ApplyPreset(1);
		}
		if(Button("Theme: Minecraft Sakura", 19, g_Config.m_ChromaMenuTheme == 2 ? ColorRGBA(0.42f, 0.62f, 0.82f, 0.55f) : ColorRGBA(0.32f, 0.40f, 0.48f, 0.22f)))
		{
			CChromaMenu::ApplyPreset(2);
		}
		if(Button("Theme: Chroma Light", 20, g_Config.m_ChromaMenuTheme == 3 ? ColorRGBA(0.42f, 0.62f, 0.82f, 0.55f) : ColorRGBA(0.32f, 0.40f, 0.48f, 0.22f)))
		{
			CChromaMenu::ApplyPreset(3);
		}
		if(Button("Theme: Default DDNet", 21, g_Config.m_ChromaMenuTheme == 0 ? ColorRGBA(0.42f, 0.62f, 0.82f, 0.55f) : ColorRGBA(0.32f, 0.40f, 0.48f, 0.22f)))
		{
			CChromaMenu::ApplyPreset(0);
		}
		if(Button("Scan / reload custom themes from disk", 22))
		{
			CChromaMenu::ScanThemes(Storage());
			CChromaMenu::Reset();
		}

		// Display any custom themes discovered in chroma/themes/
		const int DiscoveredCount = CChromaMenu::GetDiscoveredThemeCount();
		if(DiscoveredCount > 0)
		{
			Label("Discovered custom themes");
			for(int d = 0; d < DiscoveredCount; ++d)
			{
				const auto *pTh = CChromaMenu::GetDiscoveredTheme(d);
				if(!pTh) continue;
				char aThemeBtn[256];
				str_format(aThemeBtn, sizeof(aThemeBtn), "Theme: %s", pTh->m_Name.c_str());
				const bool Active = g_Config.m_ChromaMenuTheme == 4 && !str_comp(g_Config.m_ChromaThemePath, pTh->m_Path.c_str());
				if(Button(aThemeBtn, 31 + d, Active ? ColorRGBA(0.42f, 0.62f, 0.82f, 0.55f) : ColorRGBA(0.32f, 0.40f, 0.48f, 0.22f)))
				{
					CChromaMenu::ApplyThemePath(pTh->m_Path.c_str());
				}
			}
		}

		Label("UI Color Palette");
		Color("Accent", &g_Config.m_PrismThemeAccent, 11);
		Color("Background", &g_Config.m_PrismThemeBackground, 12);
		Color("Panel", &g_Config.m_PrismThemePanel, 13);
		Color("Text", &g_Config.m_PrismThemeText, 14);
		Slider("Corner radius", &g_Config.m_PrismThemeRounding, 0, 20);
		Slider("Border opacity", &g_Config.m_PrismThemeBorder, 0, 100);
		Slider("Shadow opacity", &g_Config.m_PrismThemeShadow, 0, 100);
		break;
	}

	case 4: // Settings and language
	{
		Label("chroma.   /   Control center");
		if(Button(g_Config.m_ChromaLanguage == 1 ? "Language: Русский  (Нажмите для English)" : "Language: English  (Click for Русский)", 30, ColorRGBA(0.35f, 0.55f, 0.75f, 0.45f)))
		{
			g_Config.m_ChromaLanguage ^= 1;
		}

		Toggle("Enable chroma. visuals", &g_Config.m_PrismEnabled);
		Toggle("Modular HUD", &g_Config.m_PrismHudEnabled);

		char aStatus[128];
		str_format(aStatus, sizeof(aStatus), "%s %s (DDNet 20.3)   |   Legit Edition", CHROMA_NAME, CHROMA_VERSION);
		Label(aStatus);

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
		if(Button("Reset visual presets only", 15))
			Prism::Reset(g_Config);
		break;
	}
	}

	Scroll.End();
	Footer.Draw(Theme.m_Text.WithAlpha(0.04f), IGraphics::CORNER_ALL, 5.0f);
	Ui()->DoLabel(&Footer, "chroma. v" CHROMA_VERSION, 10.5f, TEXTALIGN_MC);
	TextRender()->SetFontPreset(EFontPreset::DEFAULT_FONT);
	Prism::Validate(g_Config);
}
