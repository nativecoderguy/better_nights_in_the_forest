# Better nights in the forest

A lightweight, native C utility for macOS that intercepts low-level mouse events to create advanced macros and key bindings for FPS gaming.  Adds mouse wheel to switch items, side mouse buttons for 'use' and 'put in sack'.  If your sack is not equiped side mouse button 2 will switch to the sack, store the item, and switch back to your original item in the blink of an eye.

Built to replace bloated, electron-based mouse driver software (Logitech G Hub, Razer Synapse) with a single, efficient binary.

## ⚡ Performance
- **Language:** C 
- **Dependencies:** None (uses native macOS `ApplicationServices` framework).
- **Latency:** Zero-copy event tapping via `CGEventTap`.

## 🎮 Features

### 1. Smart Side Buttons
- **Side Button 1 (Back):** Maps to `e` (Interact/Use).
- **Side Button 2 (Forward):** **"Smart Return" Macro**
    - *Scenario A (Holding Primary):* Just presses `f`.
    - *Scenario B (Holding Secondary/Other):* Instantly switches to Primary (`1`), presses `f`, then switches back to your previous weapon.
    - *Perfect for quick-casting abilities or interacting without losing your weapon state.*

### 2. Scroll Wheel Weapon Cycling
- **Scroll Up/Down:** Cycles number keys `1` through `N`.
- **Caps Lock Safety:** If Caps Lock is **ON**, the scroller functionality is disabled (native scrolling passes through), allowing you to browse menus or web pages without closing the app.

### 3. Dynamic Range Configuration
- **Auto-Sync:** Passively listens for keyboard presses (1-9) to keep the internal state in sync with the game.
- **Double-Tap Config:** Double-tap any number key (1-9) to set the **Max Scroll Range**.
    - *Example:* Double-tap `4` -> Scroll wheel now cycles `1-2-3-4`.

## 🛠 Building

No makefiles or package managers required. Just compile with Clang.

```bash
clang -Os -framework ApplicationServices -o 99nights_rebind 99nights_rebind.c
