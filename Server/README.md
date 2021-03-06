# Extra server content

Remember to extract .dat.tar.xz back into a .dat folder.

## New features

- **Instances:** You can create instanced areas of a certain width/height. Concept very similar to how Path of Exile handles it, where a certain area can have multiple personal instances for each player.

- **Multi-colored text format:** You may have lines with multiple colors using the format '/|\<font code>|'.
eg. `"/|0|This is red. /|2|This is green. /|1960|This is purple."`. Gods can send messages formatted this way directly from the client. Check font macros in **macros.h** for all available fonts.

- **CGI map editor:** found as mapper.cgi, you can use it to edit rectangular areas of the map directly from the web. Changes are reflected live if the server is up, or get queued if it's offline. Make sure to include any new graphics you use into the cgi/assets folder, and set it as static in your webserver.

## New macros/settings
- **gendefs.h:**
    - **TURNING_ENABLED:** 0 or 1, enables/disables character turning animations.
    - **ESCAPE_ENABLED:** 0 or 1, enables/disables escaping mechanic when fighting.
    - **DIAGONAL_ESCAPE:** 0 or 1, enables/disables characters moving diagonally through each other.
    - **AUTOERASE_ENABLED:** 0 or 1, enables/disables deletion of inactive player accounts.

- **data.h:**
    - **INST_MAX:** maximum simultaneous amount of alive instances.
    - **INST_MAXBASES:** maximum amount of instance bases.
    - **INST_DURATION:** instance lifespan in ticks, after inactivity (no players within).

- **macros.h:**
    - Many font-related macros, explained within the file.

## New commands
- **/tpmode** - Toggles teleport mode for gods. In this mode, you directly teleport to your cursor instead of moving normally.

- **/xray** - Toggles x-ray vision for builders.

## God instance management commands
- **/gotoinst \<id> \<x> \<y>** - Transports you to instance \<id> at \<x>, \<y>.

- **/instnewbase \<name> \<filename> \<width> \<height>** - Create a new instance base with the given parameters.
    Will be saved to .dat/inst_basedata/\<filename>.dat.

- **/instnew \<basename> \<nochars>** - Attempts to create an instance out of an existing base with name \<basename>.
    If \<nochars> is 1 or higher, the new instance won't load any characters (for easier map editing).

- **/instsave** - Saves the map data of the instance you're currently in to its base.

- **/instlistbases** - Lists currently created and available instance bases.

- **/instdelbase \<basename>** - Attempts to delete an instance base given its name.

- **/insttemp \<temp #>** - Activates instanced character placement mode. Right clicking a tile will create a character of the given template #.
These characters don't react to anything and stay still. Right clicking on one of these characters' tiles while in this mode will remove them.
You can also look (alt+click) at one of these characters to remove them.

## New item drivers
- 70: instance transporter, data[0] = destination x, data[1] = destination y, data[2] = ID of instance base

## Misc
Lag scrolls can lead back to an instance if it's still alive.

This document has a few preview images/gifs showing some of the new features:
https://docs.google.com/document/d/1q26_0Y2CCTRTST_v3Pe-BQlaDwFhOGgYrkdJ3jFr-_k/edit?usp=sharing
