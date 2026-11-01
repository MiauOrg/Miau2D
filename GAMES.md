# Adding a Game

The engine runner in `src/main.cpp` only owns the window and main loop. A game provides one `Game` implementation and one factory.

## Layout

```text
src/games/<name>/
  <Name>.hpp
  <Name>.cpp
  GameEntry.cpp
```

## Contract

`src/Game.hpp` defines the interface:

```cpp
class Game {
public:
    virtual ~Game() = default;

    [[nodiscard]] virtual GameConfig Config() const { return {}; }
    virtual bool Load() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Draw() = 0;
    virtual void Unload() = 0;
};
```

`GameConfig` controls the window size, title, and target FPS. It does not set a virtual resolution.

`GameEntry.cpp` defines the factory:

```cpp
#include "Game.hpp"
#include "MyGame.hpp"

std::unique_ptr<Game> CreateGame() {
    return std::make_unique<MyGame>();
}
```

## Viewport

The engine does not fix a virtual resolution. Each game owns one or more `Viewport` instances and chooses their size.

```cpp
Viewport viewport_;
viewport_.Load(640, 352);

void Draw() {
    viewport_.Begin();
    ClearBackground(BLACK);
    // draw in virtual coordinates
    viewport_.End();
    viewport_.Present();
}
```

`Viewport` handles the render texture, point filtering, and letterboxed scaling to the window. A game can use several viewports, for example one per camera. `Camera2D` is only the world transform; resolution belongs to the viewport.

## Build

```powershell
.\scripts\build.ps1 -Config debug -Game <name> -Run
```

The default game is `rpg`. The active game is selected with the `HESAN_GAME` CMake variable.

## Rules

- Do not add a `main.cpp` under `src/games/`; the engine owns `src/main.cpp`.
- Do not edit files under `src/` outside `src/games/`.
- Use raylib directly for input, audio, and drawing.
- Keep game state and rules in the game folder.
