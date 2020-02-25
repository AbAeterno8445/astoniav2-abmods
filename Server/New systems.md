# Information on other new features

## Breaches
- Very much like the path of exile league, breaches are timed portals that may be activated when stepped on.

- When activated, it will start expanding the breach's dimension outwards (giving affected tiles a purple-ish hue) and awakening breachborn monsters on the edges of this circle.

- The breach has a limited distance to which it will expand, and a limited duration. You must kill as many of these monsters as you can before the breach starts closing.

- Breachborn monsters that leave their domain's influence immediately despawn.

Breach portals are on-step items that use driver 71. Their data usage is as follows:
- **data[0]:** the breach's time remaining in seconds.
- **data[1]:** total radius in tiles of the breach's influence.
- **data[2]:** travelled distance towards total radius (should start at 0).
- **data[3]:** 0 if opening, 1 if closing (should start at 0).

## Map device
- The map device item (driver 71) lets players access an interface where they select one of many base maps, and if they have charges for it, they may enter a new instance of it.

- Each map is associated to a tier level, which should indicate how strong the monsters within are.

- On successful use, the map device creates a portal at a pre-configured position that accesses the new instance. This portal has by default 6 charges, and 1 is consumed every time the player uses it. When the new portal's charges reach 0, the portal dies.

### Associated commands:
- **/setmapcharges \<base map id> \<amount>** - God only, sets the charges for the given map to the given amount.