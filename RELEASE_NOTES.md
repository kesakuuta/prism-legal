# Prism Legit Edition v0.3.0

Prism v0.3.0 is a complete modernisation and legitimacy overhaul of the Windows-focused DDNet client.

## What's included

- **Upgraded to DDNet 20.3**: Rebased onto the latest upstream DDNet release with all engine improvements, protocol updates, and optimizations.
- **100% Legit Client**: All cheat features have been completely removed:
  - Removed Hook Assist (aimbot / aim correction)
  - Removed Freeze Avoid (automated hazard prediction and dodging)
  - Removed Multi-action Macros and Double Tee automated assistants
  - Removed cheat tabs and synthetic input pulse composition
- **Dedicated T-Client Settings**:
  - Added new "T-Client" tab in main Settings menu
  - Ported TClient Rainbow engine (dynamic hue cycling for Tee, Hook, Weapon)
  - Ported TClient Outlines & Colored Parts
  - Ported TClient Trails & Statusbar HUD
  - Ported TClient Pet System, Player Indicator, Bindwheel, Bindchat, and Warlist
- **Chinese Conical Hat (斗笠)**:
  - Custom 3D-perspective conical hat worn naturally on the Tee's head with dynamic sway and aim tilt.
  - 8 triangular translucent facets fanned out with structural rib lines matching authentic Asian coolie hats.
  - Three visual modes:
    1. Custom Static Color with facet depth shading
    2. Rainbow Gradient Wave with flowing color cycle across facets
    3. Shimmering Monochrome Luster with moving metallic/silk sheen wave
  - Fully customizable via dedicated "Hat" tab in the Prism ClickGUI (`Insert` key): colors, facet alpha, rib alpha, speed, scale, and offset.
- **Rust Core Module**:
  - Implemented Chinese Hat math, procedural geometry generation, and color shaders in Rust (`src/base/chinese_hat.rs`) integrated into DDNet's official Rust workspace (`ddnet-base`) with zero-allocation render loop.
  - Dual Rust + C++ bindings for seamless cross-platform compilation.

## Controls

- **Insert**: Open/close Prism ClickGUI overlay.
- **Esc**: Close overlay or return to game.
- **F1**: DDNet console.
- **Settings Menu**: Native DDNet settings now includes a dedicated "T-Client" section.
