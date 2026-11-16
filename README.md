<div align="center">

```
  ██████╗██╗  ██╗██████╗  ██████╗ ███╗   ███╗ █████╗        
 ██╔════╝██║  ██║██╔══██╗██╔═══██╗████╗ ████║██╔══██╗       
 ██║     ███████║██████╔╝██║   ██║██╔████╔██║███████║       
 ██║     ██╔══██║██╔══██╗██║   ██║██║╚██╔╝██║██╔══██║  ██╗   
 ╚██████╗██║  ██║██║  ██║╚██████╔╝██║ ╚═╝ ██║██║  ██║  ╚═╝   
  ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚═╝     ╚═╝╚═╝  ╚═╝       
```

### **Aesthetic · Legal · High-Performance**
*The next-generation visual and quality-of-life client for [DDRaceNetwork](https://ddnet.org)*

<br>

[![Release](https://img.shields.io/badge/Release-v1.0.7-8A2BE2?style=for-the-badge&logo=github&logoColor=white)](https://github.com/kesakuuta/prism-legal/releases/tag/v1.0.7)
[![Base](https://img.shields.io/badge/DDNet-20.3-00C853?style=for-the-badge&logo=cplusplus&logoColor=white)](https://github.com/ddnet/ddnet)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-0078D6?style=for-the-badge&logo=windows&logoColor=white)](https://github.com/kesakuuta/prism-legal/releases)
[![Fair Play](https://img.shields.io/badge/Fair--Play-100%25%20Legal-FFD700?style=for-the-badge&logo=shield&logoColor=black)](https://github.com/kesakuuta/prism-legal)
[![License](https://img.shields.io/badge/License-GPLv3%2FBSD-blue?style=for-the-badge)](license.txt)

<br>

[✨ Особенности](#-ключевые-особенности) • [📥 Скачать](#-установка-и-запуск) • [🎨 Создание тем](#-создание-собственных-тем) • [🔮 Шейдеры](#-шейдеры-и-пост-процессинг) • [🎵 Музыка](#-музыкальный-плеер-в-меню) • [🚀 Лаунчер](#-нативный-лаунчер-rust) • [⌨️ Управление](#️-горячие-клавиши) • [🛠️ Сборка](#️-сборка-из-исходников) • [English](#-english-summary)

---

</div>

<br>

## 🌌 О проекте

**chroma.** — это модифицированный легальный клиент **DDRaceNetwork (DDNet 20.3)**, созданный для тех, кто ценит безупречную эстетику, гибкую кастомизацию интерфейса и плавную графику без ущерба для соревновательной честности.

В клиенте **полностью отсутствуют** любые читы, макросы, ассисты и скрытые преимущества: физика, сетевой стек и предсказания оригинального DDNet сохранены на 100%. Все улучшения сосредоточены на визуальной эстетике, персональном оформлении и удобстве игрока.

<br>

---

## ✨ Ключевые особенности

<table>
  <tr>
    <td width="50%">
      <h3>🎨 Динамический JSON-движок тем</h3>
      <ul>
        <li><b>Бесконечная кастомизация:</b> поддержка тем через простой и понятный <code>theme.json</code>.</li>
        <li><b>Любые шрифты и фоны:</b> подключайте свои TTF-шрифты и фоновые изображения в высоком разрешении.</li>
        <li><b>Адаптивный контраст текста:</b> встроенный движок яркости (<i>Luminance Engine</i>) автоматически определяет цвет фона и меняет оттенок текста и теней для идеальной читаемости.</li>
        <li><b>Горячее сканирование:</b> кнопка <i>«Сканировать темы с диска»</i> мгновенно находит новые темы без перезапуска игры.</li>
      </ul>
    </td>
    <td width="50%">
      <h3>👒 Процедурная 3D-панама (斗笠)</h3>
      <ul>
        <li><b>Честная 3D-геометрия:</b> коническая азиатская шляпа из 64 полигональных сегментов в реальном времени.</li>
        <li><b>Режимы окраски:</b> статический цвет, динамическая RGB-радуга, кастомная палитра рёбер и ткани.</li>
        <li><b>Физика движения:</b> реалистичный наклон панамы при беге, прыжках и прицеливании молотом/хуком.</li>
        <li><b>Индивидуальная настройка:</b> раздельное управление прозрачностью ткани, рёбер, масштабом и высотой посадки.</li>
      </ul>
    </td>
  </tr>
  <tr>
    <td width="50%">
      <h3>🎛️ ClickGUI и Color Picker</h3>
      <ul>
        <li><b>Быстрый доступ:</b> меню открывается клавишей <code>Insert</code> и парит над экраном без перебивания окон игры.</li>
        <li><b>Продвинутая пипетка:</b> выбор любого оттенка через спектральную карту, HSV/RGB/Hex слайдеры и альфа-канал.</li>
        <li><b>Экранирование кликов:</b> фон надежно защищен от случайных нажатий по интерфейсу игры позади меню.</li>
        <li><b>Двуязычность:</b> полная и качественная локализация на русский и английский языки.</li>
      </ul>
    </td>
    <td width="50%">
      <h3>📊 Модульный HUD и Визуалы</h3>
      <ul>
        <li><b>Гибкий оверлей:</b> независимый масштаб модулей (Identity, Active Hotkeys, Dummy status, Effects, Input).</li>
        <li><b>Физичный трейл:</b> частицы следа аккуратно затягиваются в тии при остановке, не оставляя статичных артефактов.</li>
        <li><b>Полный экран:</b> 100% покрытие видовым холстом без рамок и просветов оригинального фона.</li>
        <li><b>Динамическое приветствие:</b> отображение реального никнейма игрока в главном меню (<code>Hello, &lt;nickname&gt;</code>).</li>
      </ul>
    </td>
  </tr>
</table>

<br>

---

## 📥 Установка и запуск

Готовые оптимизированные сборки доступны для **Windows** и **Linux**.

| Платформа | Архитектура | Формат | Ссылка на скачивание |
| :--- | :---: | :---: | :--- |
| **Windows** | `x86_64` | `.zip` | [📥 **Скачать chroma. v1.0.7 (Windows)**](https://github.com/kesakuuta/prism-legal/releases/download/v1.0.7/chroma.-Windows-x64.zip) |
| **Linux** | `x86_64` | `.tar.gz` | [📥 **Скачать chroma. v1.0.7 (Linux)**](https://github.com/kesakuuta/prism-legal/releases/download/v1.0.7/chroma.-Linux-x86_64.tar.gz) |

> [!TIP]
> **Быстрый старт на Windows:**
> 1. Скачайте архив `chroma.-Windows-x64.zip`.
> 2. Распакуйте папку в любое удобное место (например, на рабочий стол или диск `D:\Games\chroma`).
> 3. Запустите файл `chroma.exe`.

<br>

---

## 🎨 Создание собственных тем

Клиент позволяет создать собственную тему за пару минут. Все темы хранятся в папке:
```
data/chroma/themes/<название_темы>/
```

### Структура папки темы:

```text
data/chroma/themes/cyberpunk/
├── theme.json         # Файл конфигурации темы
├── background.png     # Фоновое изображение (1080p / 2K / 4K PNG)
└── font.ttf           # (Опционально) Ваш любимый шрифт
```

### Пример `theme.json`:

```json
{
  "name": "Cyberpunk Neon",
  "author": "YourName",
  "version": "1.0",
  "font_file": "fonts/cyber.ttf",
  "background": "background.png",
  "greeting": "Hello, {player}",
  "menu": {
    "play": "Начать игру",
    "editor": "Редактор карт",
    "demos": "Демозаписи",
    "settings": "Настройки",
    "skins": "Гардероб",
    "quit": "Выход"
  },
  "colors": {
    "text": [255, 255, 255, 255],
    "accent": [0, 230, 255, 255],
    "shadow": [0, 0, 0, 180]
  }
}
```

> [!NOTE]
> В строке `"greeting"` переменная `{player}` автоматически подставляет текущий игровой никнейм!  
> В меню настроек `chroma.` перейдите во вкладку **«Темы»** и нажмите **«Сканировать темы с диска»** — ваша тема сразу появится в списке.

<br>

---

## 🔮 Шейдеры и Пост-Процессинг

В `chroma.` интегрирован конвейер пост-обработки **OpenGL 3.3 Core Profile**, преображающий рендеринг карты и динамического освещения в реальном времени. Шейдеры не влияют на соревновательную физику игры и работают полностью аппаратно.

Вы можете включать и настраивать шейдеры прямо в игре:
* **Настройки -> Ассеты -> Шейдеры**: выбор шейдерпака, ползунки радиуса света, интенсивности, плотности тумана, летающих пылинок и виньетки, а также кнопка быстрого перехода в папку.
* **ClickGUI (`Insert`)**: переключение и тюнинг на лету.

<br>

### 📁 Структура папки шейдерпака:

Каждый шейдерпак находится в `data/chroma/shaders/<название_шейдера>/`:
```text
data/chroma/shaders/atmospheric_amber/
├── atmospheric_glow.vert  # Вершинный шейдер (Fullscreen Quad)
├── atmospheric_glow.frag  # Фрагментный шейдер пост-обработки
└── shader.json            # Манифест параметров и значений по умолчанию
```

### ⚙️ Спецификация `shader.json`:

```json
{
  "name": "atmospheric_amber",
  "author": "chroma.",
  "version": "1.0",
  "description": "Cinematic volumetric lighting with warm dust motes and soft vignette",
  "vertex_shader": "atmospheric_glow.vert",
  "fragment_shader": "atmospheric_glow.frag",
  "defaults": {
    "u_LightRadius": 0.45,
    "u_LightIntensity": 1.35,
    "u_AmbientDark": 0.12,
    "u_FogDensity": 0.35,
    "u_DustIntensity": 0.65,
    "u_VignettePower": 0.85
  }
}
```

### 📐 Контракт Uniform-переменных (GLSL Contract):

Движок `chroma.` передает во фрагментный шейдер следующие переменные:

| Uniform | Тип | Диапазон | Описание |
| :--- | :--- | :--- | :--- |
| `u_ScreenTexture` | `sampler2D` | Текстурный слот 0 | Фреймбуфер отрендеренной сцены Teeworlds. |
| `u_Resolution` | `vec2` | Пиксели (напр. `1920, 1080`) | Разрешение окна для коррекции пропорций (Aspect Ratio). |
| `u_PlayerPos` | `vec2` | `[0.0, 1.0]` | Нормализованная позиция тишки игрока на экране (центр света). |
| `u_Time` | `float` | Секунды | Непрерывный таймер для анимации тумана и мерцания пылинок. |
| `u_LightRadius` | `float` | `0.15 – 1.0` | Радиус светового пузыря вокруг игрока. |
| `u_LightIntensity` | `float` | `0.5 – 3.0` | Множитель яркости в эпицентре источника света. |
| `u_LightColor` | `vec3` | RGB `[0.0, 1.0]` | Оттенок света и пылинок (напр. янтарный, неон, лунный). |
| `u_AmbientDark` | `float` | `0.02 – 0.35` | Освещенность неосвещенных зон карты (темнота). |
| `u_FogDensity` | `float` | `0.0 – 1.0` | Плотность процедурного дрейфующего тумана (fBm). |
| `u_DustIntensity` | `float` | `0.0 – 2.0` | Плотность и яркость летающих микрочастиц пыли. |
| `u_VignettePower` | `float` | `0.0 – 2.0` | Сила затемнения по краям и углам видоискателя. |

<br>

---

## 🎵 Музыкальный плеер в меню

В правом нижнем углу главного меню встроен стильный компактный виджет аудиоплеера:
- **Управление:** `Play/Pause`, предыдущий и следующий треки, случайный порядок (`Shuffle`), повтор (`Repeat`).
- **Интерактивный скруббер:** плавная перемотка трека мышью в реальном времени.
- **Список воспроизведения:** выпадающий список треков по нажатию на иконку списка.
- **Громкость:** привязана к глобальному регулятору громкости звуков игры (**Настройки -> Звуки**).
- **Поддержка форматов:** высококачественные аудиофайлы `.wv` (WavPack) и `.opus` (Opus).
- **Добавление треков:** просто скопируйте ваши треки в формате `Исполнитель - Название.wv` или `Название.opus` в папку `data/audio/music/`.

<br>

---

## 🚀 Нативный лаунчер (Rust)

Для удобного запуска клиента, быстрого переключения тем, шейдерпаков, шрифтов и управления ассетами разработан легковесный нативный лаунчер на **Rust**.

> [!NOTE]
> Код лаунчера расположен в приватной ветке разработки **`dev`** в директории `launcher/`.

### Возможности лаунчера:
- **Заставка «chroma.»** при старте лаунчера.
- **Верхняя навигация:** Темы, Шейдеры, Шрифты, Музыка, Ассеты.
- **Категории ассетов DDNet:** сущности (Entities), игровой процесс (Game), эмотиконы (Emoticons), частицы (Particles), HUD, экстра (Extras).
- **Настройки:**
  - Переключатель языка (Русский / English) — в самом верху настроек.
  - Персонализация фона: выбор собственного изображения PNG/JPG/WebP с режимами `fill`, `fit`, `stretch`, `center`.
  - Автоматическая адаптация цвета текста под яркость фонового изображения.
  - Поведение при старте игры: сворачивать лаунчер или оставлять открытым.
  - Путь к исполняемому файлу клиента (`Prism.exe` / `chroma.exe`).
- **Кнопка «Играть»:** мгновенный запуск оптимизированного бинарника клиента с проверкой целостности.

<br>

## ⌨️ Горячие клавиши

| Клавиша | Назначение |
| :---: | :--- |
| <kbd>Insert</kbd> | Открыть / закрыть оверлей меню **chroma.** (ClickGUI) |
| <kbd>Esc</kbd> | Закрыть меню или вернуться на предыдущий экран |
| <kbd>F1</kbd> | Консоль команд DDNet |
| <kbd>F2</kbd> | Встроенный редактор карт DDNet |
| <kbd>F10</kbd> | Быстрый скриншот |
| <kbd>F12</kbd> | Начать / остановить запись демо |

<br>

---

## 🛠️ Сборка из исходников

### Требования:
- **Git**
- **CMake** 3.16+
- **Python** 3.8+
- **Rust** & Cargo (для нативных модулей)
- Компилятор: **MSVC 2022** (Windows) или **GCC / Clang** (Linux)

### Сборка на Windows (PowerShell):

```powershell
# Клонирование репозитория
git clone --recurse-submodules https://github.com/kesakuuta/prism-legal.git
cd prism-legal

# Конфигурация CMake
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release -DVULKAN=OFF -DAUTOUPDATE=OFF -DSTEAM=OFF

# Компиляция клиента и сервера
cmake --build build --config Release --target chroma. DDNet-Server --parallel 4
```

### Сборка на Linux (Bash):

```bash
git clone --recurse-submodules https://github.com/kesakuuta/prism-legal.git
cd prism-legal
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DVULKAN=OFF -DAUTOUPDATE=OFF -DSTEAM=OFF
make -j$(nproc) chroma.
```

<br>

---

## 🌐 English Summary

<details>
<summary><b>Click to expand English summary</b></summary>

<br>

**chroma.** is a fully legal, fair-play modified client for **DDRaceNetwork (DDNet 20.3)** focused on next-generation aesthetics, customizable user interfaces, and procedural 3D cosmetics.

### Key Highlights:
- **Zero Cheats / 100% Legal:** Fully compliant with official DDNet servers. No aimbots, no auto-hook, no prediction cheats.
- **Custom JSON Theme Engine:** Create or drop themes into `data/chroma/themes/` with custom backgrounds, fonts, and button labels. Includes dynamic background luminance contrast adaptation.
- **Atmospheric Post-Processing Shaders:** Fullscreen OpenGL 3.3 pipeline with radial player lights, volumetric fBm fog, glittering dust motes, and cinematic vignette. Configure in Settings -> Assets -> Shaders.
- **In-Menu Music Player:** Floating bottom-right player widget with seek bar, playlist popup, shuffle, and loop. Supports `.wv` and `.opus` in `data/audio/music/`.
- **Procedural 3D Conical Hat (斗笠):** 64-vertex real-time parametric Asian straw hat with custom fabric/rib colors, rainbow hues, and velocity/aim tilting.
- **Sleek ClickGUI (`Insert`):** Floating overlay with interactive Color Picker (Hex, RGBA, HSVA) and input shielding.
- **Modular HUD:** Freely scalable and configurable HUD modules for hotkeys, identity, dummy, and input displays.
- **Native Rust Launcher:** Available in branch `dev`, with asset managers, DDNet launcher settings, custom backgrounds, and play button.
- **Bilingual Interface:** Full Russian and English localization.

</details>

<br>

---

## 📜 Лицензия и благодарности

- Клиент основан на исходном коде [DDRaceNetwork](https://github.com/ddnet/ddnet) и [Teeworlds](https://www.teeworlds.com/).
- Распространяется на условиях лицензий **GPLv3** и **BSD**.
- Все товарные знаки, ресурсы и исходный код принадлежат их уважаемым авторам.
- Отдельная благодарность [Prism](https://github.com/atlasru/prism) и [TClient](https://github.com/TaterClient/TClient)

<br>

<div align="center">

**Made with 💜 for the Teeworlds & DDRaceNetwork Community**

</div>
