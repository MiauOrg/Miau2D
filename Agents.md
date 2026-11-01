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

## Project notes

- Use Conventional Commits for new commits.
- `GAMES.md` describes the game entry boundary and viewport setup. `tiled/README.md`
  describes the map draw layers and depth sorting.
- Raised prop tiles can set the numeric `sortOffsetY` tileset property. Its value
  is added to the tile's bottom edge to choose the depth used when sorting with
  actor feet. Give every tile in one tall prop the same world-space depth. The
  rendering property does not change collision shapes.
- The 24 x 24 `Character.png` idle frames are row 0, columns 0, 6, 4, and 2
  for down, left, up, and right respectively. Walking uses row 1, columns 0-1,
  12-13, 16-17, and 8-9 for those same directions.
- Keep regression checks local in the ignored `/tests` directory. Do not make
  the main build depend on ignored files or claim a live visual playthrough from
  a successful build or process startup alone.
