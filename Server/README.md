# Extra server content

Remember to extract .dat.tar.xz back into a .dat folder.

## New macros/settings
- **gendefs.h:**
    - **TURNING_ENABLED:** 0 or 1, enables/disables character turning animations.
    - **ESCAPE_ENABLED:** 0 or 1, enable/disables escaping mechanic when fighting.

- **data.h:**
    - **INST_MAX:** maximum simultaneous amount of alive instances.
    - **INST_MAXBASES:** maximum amount of instance bases.
    - **INST_DURATION:** instance lifespan in ticks, after inactivity (no players within).

## God instance management commands
- **gotoinst \<id> \<x> \<y>** - Transports you to instance \<id> at \<x>, \<y>.

- **instnewbase \<name> \<filename> \<width> \<height>** - Create a new instance base with the given parameters.
    Will be saved to .dat/inst_basedata/\<filename>.dat.

- **instnew \<basename> \<nochars>** - Attempts to create an instance out of an existing base with name \<basename>.
    If \<nochars> is 1 or higher, the new instance won't load any characters (for easier map editing).

- **instsave** - Saves the map data of the instance you're currently in to its base.

- **instdelbase \<basename>** - Attempts to delete an instance base given its name.

- **insttemp \<temp #>** - Activates instanced character placement mode. Right clicking a tile will create a character of the given template #.
These characters don't react to anything and stay still. Right clicking on one of these characters' tiles while in this mode will remove them.
You can also look (alt+click) at one of these characters to remove them.

## New item drivers
- 70: instance transporter, data[0] = destination x, data[1] = destination y, data[2] = ID of instance base

## Misc
Lag scrolls can lead back to an instance if it's still alive.