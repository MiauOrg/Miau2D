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

A layer y-sorts if you add a custom boolean property on the layer:

```
ysort = true
```

Or just put `props` or `ysort` in the layer name.

Individual tiles with a numeric `sortOffsetY` tileset property also y-sort on
ordinary layers. Tiles without that property on ordinary layers draw flat, in
Tiled order. An `above` layer always draws above actors, including tiles with a
`sortOffsetY` property.

## How to draw over the player

Add this property to the layer:

```
above = true
```

Or name the layer `Overhead` / `Above`.

This layer is drawn after every character, so it always covers them.

## How the sorting works

- Player sorts by their **feet**.
- Tiles sort by the **bottom edge** of the tile plus their optional `sortOffsetY`.
- Bigger Y = closer to the camera = drawn later = in front.

So a player standing north of a prop is behind it, and a player standing south of it is in front.

## Trees and other tall props

A tree is more than one tile tall. To sort the whole tree at its base, put its
tiles on `Props` and give each tile a `sortOffsetY` in the tileset. The offset
is measured downward from that tile's bottom edge to the tree's base. For a
three-tile-high tree with 16-pixel tiles, use offsets 32, 16, and 0 from top to
bottom. Both sides of a wide tree use the same offset at each height.

The example tileset already supplies these offsets for trees, lamps, the house,
and other raised props. These are drawing settings only. Keep collision shapes
on `Collision` or `CircleCollision`, independently of the visual layers.

Use `Overhead` for a canopy or other tile that should *always* cover actors,
even when their feet are below it. Changes to tileset properties require a map
reload because the draw list is cached when the map loads.

## Don't put flat stuff on `Props`

Paths, dirt, rugs, shadows, floor decals -> `Level2`.

If you y-sort a flat tile, it will draw on top of the player whenever the player is north of it. That looks broken.

## Adding a new prop

1. Can the player walk behind it?
   - Yes -> `Props`
   - No, it should always cover the player -> `Overhead`
2. Is it taller than one tile? Place all parts on `Props` and set their
   `sortOffsetY` values to the distance from each tile bottom to the common base.
3. Make sure the layer has `ysort = true` or `above = true`.
4. Flat floor stuff stays on `Level2`.
5. Blocking tiles stay on `Collision`.

## What the current map uses

`Overworld01.tmx` follows this setup:

- `Ground` - grass
- `Level2` - the orange dirt patch and flat terrain
- `Props` - the house, trees, lamps, mailbox, boulders, rocks, and bush bases
- `Overhead` - bush top rows that always cover actors
- `Collision` - collision rectangles
