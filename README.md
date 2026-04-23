# ACPR_Hitboxes
A hitbox mod for Guilty Gear XX Accent Core Plus R via [GearLoader](http://github.com/YouKnow232/GearLoader) <br>
A partial port of [GGXXACPR_Overlay](http://github.com/YouKnow232/ggxxacpr_overlay)

## Hitboxes

|Thing        | Name          | Notes                                          |
| ---         | ---           | ---                                            |
|Red          | Hitbox        |                                                |
|Green        | Hurtbox       |                                                |
|Blue         | Pushbox       | Determines movement collision                  |
|Purple Cross | Pivot         | Actual character position, their origin point. |
|Orange       | Range Check   | Clean Hit box pictured. These boxes operate on the origin point instead of other boxes. |
|Purple       | Pushbox check | Command grab range pictured. These are range checks that operate on pushboxes. These usually only have a horizontal component with the exception of air throws. For air throws, the bottom of the pushbox needs to be in the air throw box. The box is extended to the height of the pushbox for the sake of visualization. |
<img width="452" height="403" alt="sidewinder" src="./docs/sidewinder.png" />
<img width="502" height="319" alt="sidewinder" src="./docs/potemkin_buster.png" /> <br>
Yup

## Settings

<img width="640" height="480" alt="sidewinder" src="./docs/Settings1.png" /> <br>
<img width="640" height="480" alt="sidewinder" src="./docs/Settings2.png" /> <br>

* Combine Boxes groups hitboxes and hurtboxes into one shape.
* Throw Boxes can be set to ALWAYS to always show universal throw ranges. If a character does a command grab it will overwrite the universal throw box and display the command grab range instead for its active frames.
* Widescreen Clip hides the boxes behind the widescreen side bars. If set to OFF boxes will draw over them and you can see boxes past the left and right ends of the screen.
* For the individual box toggles, Clean Hit boxes are distinguished from other misc range check boxes. Throw boxes are also separated from other misc pushbox checks.

These settings will save whenever the game would normally save its data.

## Known Issues

When Combine Boxes is set to ON, border size may be inconsistent on certain projectiles.
