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