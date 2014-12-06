SDL Sand
-----------------
The aim of this program is to simulate different particles (such as sand, water, oil or fire) interacting with each other.
Freedom is given to the player to experiment with the different materials in a sandbox environment.

Keymap
-----------------
SELECT: Exit game
START: Clear screen
Joystick: Move cursor
D-PAD left/right: change brush type
D-PAD up/down: increase/decrease pen size
A: paint w/ selected brush type
B: limit cursor speed to 1px/frame
Y: set the brush type to eraser
X: turn on/off emitters (applies to screen top only)
L/R: decrease/increase emitters density (applies to screen top only)

Usage
----------------
Select a brush type from the panel on the bottom part of the screen and draw while pressing
the A button. Use the B button to perform more accurate cursor movements. The particles falling from the top of the screen can be disabled at any moment by pressing the X button, or decreased/increased with L/R shoulder buttons.

Brush types (from left to right):

Group 1 (particles):
* water
* sand
* salt
* oil
* fire
* acid
* dirt

Group 2 (emitters):
* water spout
* sand spout
* salt spout
* oil spout

Group 3 (solids):
* wall
* torch
* stove
* plant
* ice
* iron wall
* void

Group 4:
* eraser

The authors
----------------
Thomas René Sidor (Studying computer science at the university of Copenhagen, Denmark) (Personal homepage: http://www.mcbyte.dk)
Kristian Jensen (Studying computer science at Roskilde University, Denmark)
Artur Rojek - GCW Zero port

Acknowledgements
----------------
CCmdLine - command line parser by Chris Losinger (http://www.codeproject.com/cpp/ccmdline.asp)
SDL - Simple DirectMedia Library (http://www.libsdl.org)

Origins
-----------------
The SDL Sand game (The Falling SDL-Sand Game) is a C++ implementation of the original 'World of
Sand' (and later 'Hell of Sand') game implemented in JAVA. SDL Sand uses the SDL (Simple DirectMedia
Library - http://www.libsdl.org/) library for screen output. Therefore the implementation can
possibly run on every platform supported by SDL.

The aim is to create a faster implementation of the game and possibly extend its features.

Links
-----------------
Project link: http://sourceforge.net/projects/sdlsand
The originial World of Sand from DOFI-BLOG (http://ishi.blog2.fc2.com/blog-entry-158.html)
The later version - Hell of Sand from DOFI-BLOG (http://ishi.blog2.fc2.com/blog-entry-164.html)
A Falling Sand Game forum (http://www.fallingsandgame.com/)
Falling Sand Game hosted (http://chir.ag/stuff/sand/)

License
-----------------
SDL Sand is released under GPLv2 license. See included LICENSE file for details.
