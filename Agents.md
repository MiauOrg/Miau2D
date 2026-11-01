# Agents.md

## Project Overview
A lightweight 2D game engine written in C++.

The primary goal is to provide a foundation for games such as:
- JRPGs
- Colony simulators
- Top-down RPGs
- Tactical and strategy games
- Tile-based games
and more.

The engine should prioritize simplicity, performance, maintainability and ease of development.
The engine is intended to be a practical, fast and simple tool for building games.

## 1. Development Philosophy
### Keep It Simple
Write straightforward C++.

The preferred programming style is similar to C, enhanced with useful C++ features such as:
- Classes and structs
- Namespaces
- Constructors and destructors
- RAII
- References and pointers
- STL containers where appropriate
- Basic operator overloading when useful

Avoid turning simple problems into complicated abstractions.

Code should be easy to read, understand, debug and modify.

### Avoid Overengineering
- Do not introduce abstractions without a concrete use case.
- Do not create generic systems when a simple implementation is sufficient.
- Do not implement features for hypothetical future requirements.
- Do not introduce unnecessary design patterns.
- Avoid excessive inheritance and deep class hierarchies.
- Prefer composition over inheritance.
- Avoid unnecessary interfaces and virtual functions.
- Avoid excessive templates and metaprogramming.
- Do not introduce an ECS architecture unless explicitly justified.

A small amount of duplication is preferable to an unnecessarily complicated abstraction.

### Explicit Is Better
Prefer explicit control flow, ownership and dependencies.
Avoid hidden behavior, excessive indirection and unnecessary magic.
Do not introduce complicated dependency injection systems or service locators.

## 2. Code Style
### File Structure
- Header files: `.hpp`
- Implementation files: `.cpp`
- Do not use `.h` or `.cc`.

Separate declarations and implementations where appropriate.
Do not create separate files for trivial classes or functionality without a good reason.

### Naming Conventions
- Classes and structs: `PascalCase`
- Functions and methods: `PascalCase`
- Variables: `camelCase`
- Member variables: `camelCase`
- Constants: `UPPER_SNAKE_CASE`
- Namespaces: `lowercase`

### Formatting
- Use consistent indentation.
- Use braces for control structures.
- Avoid excessively long functions.
- Keep functions focused on one responsibility.
- Avoid deeply nested control flow.
- Prefer early returns when appropriate.

### Comments
**Do not add comments to source code.**
This includes:
- Inline comments
- Function comments
- Class comments
- Documentation comments
- TODO comments

Code should communicate its intent through clear naming and straightforward implementation.

## 3. C++ Guidelines
- Prefer stack allocation and value semantics where practical.
- Use RAII for resource management.
- Avoid unnecessary dynamic allocations.
- Avoid raw owning pointers.
- Use raw pointers for non-owning references when appropriate.
- Use references when null is not a valid state.
- Use `std::unique_ptr` when dynamic ownership is necessary.
- Use `std::shared_ptr` only when shared ownership is genuinely required.
- Avoid unnecessary copies.
- Use `const` wherever appropriate.
- Avoid global mutable state.
- Avoid unnecessary `static` state.
- Use STL containers instead of implementing custom containers without a specific reason.

Do not sacrifice readability for micro-optimizations.
Performance optimizations should be based on actual requirements or profiling.

## 4. Architecture
Keep the engine modular, but do not overcomplicate its architecture.
Systems should have clear responsibilities and minimal dependencies.
Prefer small, independent components that can be combined when needed.
Avoid creating unnecessary layers between the game and the underlying engine functionality.
Game-specific functionality should not be unnecessarily coupled to the engine.

## 5. General Rule

When choosing between a simple implementation and a complicated abstraction, choose the simple implementation unless there is a concrete technical reason not to.
The goal is to build a functional game engine, not a framework for building frameworks.

---

## Miau2D development guide

## Purpose and scope

This file guides AI agents working anywhere in `Miau2D/`. Read the relevant
source before editing, and keep this guide aligned with verified behavior.
The architecture below describes the current code. Circle-vs-circle,
circle-vs-AABB, AABB-vs-circle collision queries and movement response are
implemented. Navigation, animation, keyboard/touch input, and a wandering NPC
are composed around the shared `Actor` movement code.

Miau2D is a small raylib-based 2D game/engine written in **C++20 with a C-style
development direction**. Favor straightforward data, functions, and explicit
control flow. Keep raylib as the platform/rendering layer. The CMake project,
executable, window title, and build variables currently retain the older
`Hesan2D` / `HESAN_*` names; do not rename them incidentally.

## Coding direction: C-style C++ with raylib

- Compile as C++; this is not a request to convert the project to C or `.c` files.
- Prefer plain structs and free functions for new geometry and systems. Keep
  state visible and algorithms small. Use simple enums or separate collections
  when multiple shape types are actually needed.
- Existing `AABB`, `PhysicsWorld`, `Actor`, `Movement`, `Scene`, and `TiledMap`
  are the current integration points. Extend them where useful; avoid a
  wholesale rewrite just to change style. Keep algorithms in free functions or
  small components with thin adapters to these classes.
- Use raylib types such as `Vector2`, `Rectangle`, `Color`, and `Texture2D`.
  Use raylib/raymath functions or clear scalar math instead of a parallel math
  library or a C++ raylib wrapper.
- Existing `std::vector`, `std::string`, `std::optional`, references, and standard
  math/algorithm helpers are acceptable. C-style does not require replacing safe
  containers with manual allocation.
- Avoid inheritance trees, virtual collider hierarchies, template frameworks,
  exception-driven gameplay flow, ECS infrastructure, and new dependencies
  unless a concrete requested feature needs them.
- Keep collision queries deterministic and independent of input, frame time,
  drawing, and resource loading. Avoid allocations inside individual shape tests.
- Follow nearby formatting: four spaces, braces on the same line, PascalCase
  types/functions, camelCase variables, `float` values with `f` suffixes,
  initialized data, and `#pragma once` headers. Existing private-member suffixes
  differ by class; preserve local conventions.
- Keep ownership explicit. Release textures/render targets while the raylib
  window/context is still alive. Do not copy resource-owning objects without
  defining ownership; `TiledMap` currently requires explicit `Unload()`.

## Repository map and runtime

| Location | Responsibility |
| --- | --- |
| `src/main.cpp` | Window lifecycle, scene setup, render target, debug controls, cleanup. |
| `src/GameObject.hpp`, `src/Transform.hpp` | Shared scene objects, drawable sprites, and 2D transforms. |
| `src/Actor.hpp`, `src/Actor.cpp` | Shared actor collider, velocity, grid state, movement update and animation selection. |
| `src/Movement.hpp`, `src/Movement.cpp` | Free movement functions for grid snapping, step buffering, and continuous movement. |
| `src/Player.hpp`, `src/Player.cpp` | Keyboard/touch player controller composed around `Actor`. |
| `src/SpriteAnimation.hpp`, `src/SpriteAnimation.cpp` | Named frame clips and playback timing without owning textures. |
| `src/PlayerInput.hpp`, `src/PlayerInput.cpp` | Keyboard movement commands, separate from `Actor`. |
| `src/TouchInput.hpp`, `src/TouchInput.cpp` | Mouse/tap targets transformed through the letterboxed viewport and camera. |
| `src/Physics/GridPathfinder.hpp`, `src/Physics/GridPathfinder.cpp` | Configurable grid conversion, actor-footprint walkability and cardinal A*. |
| `src/GridPathFollower.hpp`, `src/GridPathFollower.cpp` | Feeds path steps to the shared grid-moving `Actor`. |
| `src/WanderingBrain.hpp`, `src/WanderingBrain.cpp` | Seeded random goal selection and A* route requests. |
| `src/WanderingNPC.hpp`, `src/WanderingNPC.cpp` | Actor-based NPC that composes a wandering brain. |
| `src/Scene.hpp`, `src/Scene.cpp` | Map, physics, object lifetime, pathfinder, camera, and y-sorted drawing. |
| `src/Physics/AABB.hpp`, `src/Physics/AABB.cpp` | Box geometry, overlap, depenetration, raycast, swept AABB helper, debug drawing. |
| `src/Physics/Circle.hpp`, `src/Physics/Circle.cpp` | Circle data, strict circle-vs-circle overlap, and contact normal/depth. |
| `src/Physics/PhysicsWorld.hpp`, `src/Physics/PhysicsWorld.cpp` | Static AABBs/circles, optional world bounds, linear queries, box movement, and debug drawing. |
| `src/TiledMap.hpp`, `src/TiledMap.cpp` | tmxlite loading, tileset texture ownership, tile rendering and collider extraction. |
| `tiled/Overworld01.tmx` | Map loaded by `main.cpp`; 30 x 20 cells, each 16 x 16 world units. |
| `tiled/*.tsx`, `assets/` | Tileset definitions and image assets. `assets/maps/demo.tmx` is a separate map, not the startup map. |
| `CMakeLists.txt`, `CMakePresets.json` | C++20 executable, dependencies, warnings and debug/release builds. |
| `scripts/` | Windows PowerShell build/run helpers. |

Startup creates a 1280 x 720 window, loads the map, copies its colliders into
`PhysicsWorld`, sets world bounds, and snaps the player to the map grid. The
default pathfinding grid matches the map tile size. Keyboard and touch input are
read separately from `Actor`; touch destinations use A* only in grid mode.
`WanderingNPC` reuses the same `Actor` movement with `WanderingBrain`. The map,
player, and NPC are drawn into a 640 x 352 render texture, scaled to the window
with aspect preservation. Physics uses world coordinates, never window
coordinates or camera zoom. The render camera target is rounded down for pixel
alignment; physical positions remain floating point.

Tiles without a `sortOffsetY` tileset property are drawn before sprites, including
flat terrain and grass on `Level2`. Raised props carry a numeric `sortOffsetY`
in `tiled/Overworld.tsx`; their depth is the tile's world bottom plus that offset.
All pieces of a tall object must share one world depth: a two-tile lamp uses 16
on its upper tile and 0 on its base; a three-tile tree uses 32, 16, and 0.
`Scene` merges these cached tile draws with actors sorted by their sprite bottom.
At equal depth the map prop draws first, placing the actor in front. This prevents
the actor appearing between parts of one tree or lamp. The rule applies across
visual tile layers and does not depend on the `Level2` name. Tileset property
changes take effect when the map reloads. Collision shapes and movement do not
use this rendering metadata.

Controls: WASD/arrows move, F1 toggles physics/grid visualization, F2 switches
grid/free movement, left click/tap sets an A* destination in grid mode, and
plus/minus adjust camera zoom. Grid is the default mode.

## Build and run

Run commands from the `Miau2D/` repository root so relative asset paths resolve.
Prerequisites: a C++20 compiler, CMake, Ninja for the supplied presets, and raylib
discoverable by CMake. `CMakeLists.txt` declares CMake 3.20, but the preset file
uses schema version 6, which requires **CMake 3.25 or newer**.

```sh
cmake --preset debug
cmake --build --preset debug
./build/debug/bin/Hesan2D
```

For release, use `release` instead of `debug` in both commands and the binary
path. On Windows the executable has an `.exe` suffix. The normal PowerShell
helper is:

```powershell
.\scripts\build.ps1 -Config debug -Run
```

- raylib is found through `find_package(raylib REQUIRED)`; it is not downloaded
  by this project. Set `RAYLIB_ROOT` or configure with
  `cmake --preset debug -DHESAN_RAYLIB_ROOT=/path/to/raylib/prefix` when needed.
  The build also detects `C:/msys64/mingw64` on Windows.
- tmxlite is fetched from `fallahn/tmxlite`, pinned to `v1.4.5`, and built as a
  static library. First configuration needs Git/network access or an available
  local FetchContent source. Do not silently change the pinned dependency.
- If Ninja is unavailable, a separate build directory can use an installed
  generator, for example `cmake -S . -B build/local -G "Unix Makefiles"`, then
  `cmake --build build/local`. Do not mix generators in one build directory.
- CMake gathers `.cpp` and `.hpp` files recursively under `src/` with
  `CONFIGURE_DEPENDS`. Keep test programs with their own `main()`
  outside `src/` and add a separate test target when introducing tests.
- Compiler warnings are `/W4 /permissive-` on MSVC and
  `-Wall -Wextra -Wpedantic -Wshadow` elsewhere. Address warnings in changed code.
- `scripts/buildAndRun.ps1` has a trailing unconditional debug executable launch
  after restoring the previous working directory. Prefer `build.ps1 -Run` until
  that script is deliberately corrected.
- Keep generated files in ignored build directories. Assets are not copied to
  the binary directory; launch from the repository root.

## Existing physics contracts

- Coordinates increase rightward in X and downward in Y. Positions/sizes are in
  world pixels; velocities are units/second and displacements are units/frame.
- `AABB::position` is its top-left corner, and `size` is its full extent. Current
  code assumes valid nonnegative sizes; it does not validate every input.
- `AABB::Intersects` uses strict overlap: boxes sharing only an edge/corner do
  not intersect. `Contains` includes the boundary. Preserve these distinctions.
- `ResolveCollision(staticBox, outDisplacement)` computes a correction for the
  calling box. It chooses the smaller overlap axis, choosing Y on ties. Its
  overlap-width calculation is not a general minimum-separation solution for
  full containment; do not copy that shortcut into circle containment handling.
- `PhysicsWorld` stores static AABBs and circles. Both `CheckCollision` overloads
  check both static shape collections; bounds are enforced separately by
  movement and navigation walkability. `Query(AABB)` and `Query(Circle)` return
  only their matching static shape type. Queries scan the collections linearly.
- `GetCircleContact(a, b, outContact)` returns a normal that moves its first
  circle out of the second and a penetration depth. Tangency is not collision;
  nonpositive or nonfinite radii are rejected. Coincident centers use (1, 0) as
  a deterministic normal. The mixed `GetCircleAABBContact` variant moves the
  circle out of the box; `GetAABBCircleContact` moves the box out of the circle.
  `PhysicsWorld::GetCircleContact` checks circles, then AABBs, preserving
  insertion order within each collection.
- `ResolveOverlap` tries up to four passes. `MoveAndSlide` depenetrates, resolves
  AABB movement on the X then Y axes and handles circle contacts with bounded
  substeps, clamps to world bounds, and returns total actual displacement,
  including corrections. It does **not** call `SweptAABB`; that helper remains
  separate from movement.
- AABB raycasting uses `origin + direction * t`. Its `maxDistance`, `time`, and
  `distance` fields currently use this parameter `t`; it is physical distance
  only for a unit direction. `SweptAABB` passes full displacement with a limit
  of 1, making `time` a fraction of the movement. Preserve this relationship.
- `PhysicsWorld::Raycast` currently leaves `outHit` unchanged on a miss and
  excludes a hit exactly at `maxDistance` when selecting the closest hit.
  Treat changes to either behavior as explicit, tested fixes.
- The player's collider is `position + colliderOffset`, not its sprite bounds.
  Defaults are a 16 x 16 collider, offset (4, 8), and a 24 x 24 sprite source.
- Actor movement keeps that AABB; AABB and circle queries now
  collide with static AABBs and circles. Grid movement rejects occupied
  destination cells; continuous movement slides against both static shape types.
- Free movement normalizes input and sends `velocity * dt` to `MoveAndSlide`.
  Grid movement checks destination occupancy and interpolates one cardinal step
  over 0.16 seconds; it does not resolve collisions throughout that interpolation.
  Grid cells align the collider center to the cell center, then derive sprite
  position from collider size and offset.

## Tiled map constraints

`GetColliders()` imports object layers other than `CircleCollision` as AABBs, plus
tile layers whose names contain `collision`, `solid`, or `obstacle` case-insensitively.
Nonzero collision tiles are merged into horizontal spans per row. Object shapes in those
layers are reduced to `obj.getAABB()`; polygon shapes and tileset-local collision
objects are not imported. `GetCircleColliders()` separately reads equal-sided
ellipse objects from the `CircleCollision` object layer. Rendering skips the
exact tile-layer name `Collision`. `Draw()` draws cached background tiles;
`DrawDepthTilesBefore()` and `DrawRemainingDepthTiles()` draw the raised props
between actors. Collision discovery and rendering filters are not equivalent.

The renderer targets finite orthogonal tile maps; it warns on other orientations
but still loads them. Horizontal/vertical tile flips are handled; diagonal flips
and nested group traversal are not implemented. Keep `.tmx`/`.tsx` image paths
valid when moving assets. `Scene::Load()` reports map load failure to `main.cpp`.

## Circle collision

Circle-vs-circle and mixed circle/AABB overlap, contact data, map import, world
queries, movement response, and debug outlines are implemented. Circle and
AABB movement against circles use bounded substeps and sliding. `Actor` currently
uses an AABB collider; F1 draws its collider green. Map circles are outlined
cyan and turn red if they overlap each other or a static AABB. Very large frame
displacements can still tunnel because substeps are capped.

Use the existing small `Circle` struct with `Vector2 center` and `float radius`
and free collision functions in `src/Physics/Circle.hpp` and `.cpp`. Reuse
`AABB`; do not create a second rectangle representation. Circle collision rejects
zero, negative, and nonfinite radii and handles coincident centers without
normalizing a zero-length vector.

For map-authored colliders, add ellipse objects on a dedicated object layer such
as `CircleCollision`, using Tiled or editing the TMX directly. Keep circles
square; reject non-square ellipses or handle them as ellipses in a separate
feature. Leave the existing tile `Collision` layer for rectangular terrain.
Tiled stores an ellipse object's top-left position and size, so its circle
center is position plus half its size and its radius is half its width.

The `CircleCollision` layer is read by `GetCircleColliders()` and excluded from
the AABB loader. An ellipse in another layer is still reduced to its AABB and
will not participate in circle queries. Keep the circle layer name and object
shape contract explicit when changing this importer.

The current `Overworld01.tmx` map has three 12 x 12 circle objects centered on
the visible stone tiles at grid cells (22, 1), (23, 4), and (28, 9). Preserve
those placements when editing the example map unless the stone layout changes.

### Contact convention

Circle overlap uses strict penetration to match existing AABB intersection
behavior. Exact tangency is a miss; a separately requested touching/contact
query can have an inclusive contract. Name tolerances and use them consistently
rather than changing `<` to `<=` in isolated paths.

If resolution is needed, use plain contact data containing at least a normal
and penetration depth. Define the normal to point in the direction that moves
the **first argument A out of the second argument B**. Then the correction for
moving A against static B is `normal * penetration`. Reset outputs on a miss.
Reversing argument order reverses the normal for the same contact; ambiguous
coincident/tied cases need a documented deterministic fallback. Keep overlap
contact data separate from the existing ray/sweep `RaycastHit` time contract.

### Circle versus circle

- Compute `delta = a.center - b.center`, `r = a.radius + b.radius`, and squared
  distance `d2 = delta.x * delta.x + delta.y * delta.y`.
- For valid positive-radius circles, strict overlap is `d2 < r * r`. Use squared
  distances for detection and a square root only when contact depth is needed.
- For noncoincident centers, `normal = delta / sqrt(d2)` and
  `penetration = r - sqrt(d2)` using the convention above.
- Coincident centers require a stable fallback normal such as (1, 0), with
  penetration `r`. Do not produce NaNs. Cover unequal radii and containment.

### Circle versus AABB

- `CheckCircleAABBCollision` clamps the circle center to the AABB's min/max to
  obtain the closest point `q`.
  Set `delta = circle.center - q`. For valid positive-radius circles, detection
  is `dot(delta, delta) < radius * radius`, covering faces and rounded corners.
- When the center is outside the box and distance is nonzero, the correction
  direction for the **circle** is `delta / distance`, with depth
  `radius - distance`. Negate that normal if the AABB is argument A.
- When the center lies inside or on the box, `q == center`. Choose the nearest
  face and its outward normal. The depth for moving the circle out is
  **distance to that face + radius**, not radius minus distance. Resolve equal
  face distances left, right, top, then bottom; negate the normal when moving
  the box instead.
- `GetCircleAABBContact` returns the normal for moving the circle out of the
  box. `GetAABBCircleContact` negates that normal for moving the box out of the
  circle. Do not approximate a circle with its bounding box as the final test.

### Integration boundaries

- Circle world storage and circle/AABB storage remain separate. Keep these
  shape queries explicit; a polymorphic collider hierarchy is unnecessary.
- `PhysicsWorld::CheckCollision` and movement response support mixed shapes.
  `Query` remains shape-specific and does not return a mixed-shape collection.
- `PhysicsWorld::MoveAndSlide(Circle, ...)` handles static circles and AABBs
  with bounded substeps and sliding. Keep it separate from the AABB solver; the
  axis-by-axis box solver is not automatically correct around circle/box
  corners.
- `PhysicsWorld::ResolveOverlap(Circle)` and the AABB overload return the
  correction to move a shape out of static geometry. `MoveAndSlide` applies an
  initial correction before processing displacement.
- The current circle solver caps substeps, so unusually large displacements can
  still tunnel. If stronger continuous collision guarantees are requested,
  design a circle sweep/time-of-impact approach; `SweptAABB` is not a circle
  sweep.
- Circle world bounds must account for the radius. Check bounds can contain the
  shape before clamping; the current AABB clamp also assumes a fitting shape.
- If importing Tiled circles later, inspect the object shape explicitly and
  preserve the existing horizontal tile-span merging for rectangular terrain.
- Extend the F1 debug view with raylib shape outlines and, where useful, contact
  normals/correction vectors drawn in world space.

## Navigation, animation, and actor composition

- `GridPathfinder::SetGrid` configures world bounds and arbitrary positive cell
  width/height. `WorldToCell` uses the cell's top-left origin; `CellToWorld`
  returns its center. The A* search is deterministic, cardinal-only, uses
  Manhattan cost, and returns a path excluding the start and including the goal.
- `IsWalkable(cell, actorSize)` tests a centered AABB footprint against world
  bounds and both static collider shapes. It works independently of A*.
- If an `Actor` follows that path in grid mode, configure `Movement::SetGrid`
  with the same origin and cell size as the pathfinder. A pathfinding resolution
  can differ from map tile size, but movement step size must match it.
- `SpriteAnimation` stores named source-rectangle clips and playback timing; it
  does not own the texture. `Actor` selects `idle_down`, `walk_down`, and
  corresponding left/up/right clips based on actual movement and facing.
- `assets/characters/Character.png` uses 24 x 24 frames. The shared animation
  setup in `Scene.cpp` uses idle poses from row 0 at columns 0 (down), 6 (left),
  4 (up), and 2 (right). Walk frames come from row 1: columns 0-1 (down),
  12-13 (left), 16-17 (up), and 8-9 (right). Keep these source directions
  aligned with world movement: negative X is left, positive X is right, negative
  Y is up, and positive Y is down.
- `PlayerInput` turns keyboard state into movement vectors. `TouchInput` converts
  mouse/tap positions through the letterboxed viewport and camera. Touch path
  following currently operates in grid mode. `Player` composes both input
  controllers with an `Actor` and a `GridPathFollower`.
- `GridPathFollower` sends cardinal A* steps to the shared `Actor`.
  `WanderingNPC` derives from `Actor` and composes a seeded `WanderingBrain`; the
  brain picks walkable destinations and requests routes instead of implementing
  its own movement. NPCs are not currently inserted as dynamic colliders, so
  actors do not block one another.

## Verification and reusable knowledge

Keep regression checks local in the ignored `/tests` directory, as requested by
the user. Do not make the main project build depend on those ignored files or
assume they exist in a fresh checkout. The local depth checks capture draw calls
and use raylib CPU image compositing to check whole props, both sides of the
depth boundary, equal-depth ties, multiple actors, adjacent props, flat terrain,
hidden layers, collision preservation, and reloads. This verifies ordering and
compositing without a window; it does not verify the GPU driver. Do not claim a
visual playthrough from compilation or process startup alone.

Future geometry checks should cover:

- Separated, tangent, slightly overlapping, and deeply overlapping shapes.
- Circle pairs with different radii, coincident centers, and full containment.
- Circle contact with each box face and corner, plus centers inside/on the box.
- Negative coordinates, invalid/zero radii, degenerate boxes, and the chosen
  tolerance policy.
- Boolean symmetry and contact-normal direction; correction should remove
  penetration within tolerance and all outputs must remain finite.
- Existing AABB behavior, movement near walls/corners, and world-bound handling
  when those integration paths change.

Build the affected target and manually check the demo when runtime behavior
changes: F1 collider alignment, both F2 movement modes, buffered grid turns,
sliding, map bounds, resizing, and zoom. Report exact commands and results, and
state when dependencies, display access, or network access prevented a check.
Documentation-only edits need factual/path review, not new runtime tests.

Keep future work scoped to the requested feature. Record verified new APIs,
coordinate/contact conventions, build changes, and test commands here as they
land. If a workflow becomes a reusable AI skill, base it on these verified
commands and source paths, state its prerequisites and validation steps, and
keep planned features clearly separate from implemented behavior.
