# Tiled layers (read me)

Quick guide to how map layers work in Hesan2D. The engine looks at layer **names** and **properties** to decide what draws behind or in front of the player.

## The layers

| Layer | What goes here | Draw order |
|---|---|---|
| `Ground` | Grass, base floor | Always under everything |
| `Level2` | Flat decor: paths, dirt, rugs, decals | Under the player |
| `Props` | Things you walk in front of *and* behind: walls, lamps, rocks, tree trunks, bush bases | Y-sorted with the player |
| `Overhead` | Tree tops, canopies, roofs, anything that should always cover the player | Drawn after the player |
| `Collision` | Invisible walls | Never drawn |

## How to turn on y-sorting

A layer only y-sorts if you tell it to. Add a custom boolean property on the layer:

```
ysort = true
```

Or just put `props` or `ysort` in the layer name.

That's the "enable this" part. Without it, the layer draws flat, in Tiled order, and the player will always be on top of it.

## How to draw over the player

Add this property to the layer:

```
above = true
```

Or name the layer `Overhead` / `Above`.

This layer is drawn after every character, so it always covers them.

## How the sorting works

- Player sorts by their **feet**.
- Tiles sort by the **bottom edge** of the tile.
- Bigger Y = closer to the camera = drawn later = in front.

So a player standing north of a prop is behind it, and a player standing south of it is in front.

## Trees and other tall props

A tree is more than one tile tall. Don't put the whole tree on one layer or the player will render in front of the canopy.

Split it:

- Canopy / top half -> `Overhead`
- Trunk / base -> `Props`
- Grass or path under it -> `Level2`

Now walking behind the tree hides you behind the leaves, and walking in front puts you over the trunk. This is how RPG Maker does it with its "star" tiles.

## Don't put flat stuff on `Props`

Paths, dirt, rugs, shadows, floor decals -> `Level2`.

If you y-sort a flat tile, it will draw on top of the player whenever the player is north of it. That looks broken.

## Adding a new prop

1. Can the player walk behind it?
   - Yes -> `Props`
   - No, it should always cover the player -> `Overhead`
2. Is it taller than one tile? Split it: top half to `Overhead`, base to `Props`.
3. Make sure the layer has `ysort = true` or `above = true`.
4. Flat floor stuff stays on `Level2`.
5. Blocking tiles stay on `Collision`.

## What the current map uses

`Overworld01.tmx` follows this setup:

- `Ground` - grass
- `Level2` - the orange dirt patch
- `Props` - house, garden wall and gate, tree trunks, bush bases, lamp poles, mailbox body, boulders, rocks
- `Overhead` - pine canopies, bush top rows, lamp heads, mailbox lid, boulder tops
- `Collision` - collision rectangles
