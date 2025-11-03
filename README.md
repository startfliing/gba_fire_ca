# gba_fire_ca
A few of my own cellular automata designed for the Gameboy Advance to simulate types of fire: Forest Fires, Flames, and Fireworks

[My Youtube Channel](https://www.youtube.com/@StartFliing)

## GBA Resources
Here are some resources I used/am using to learn about the GBA
- [devkitPro](https://devkitpro.org/)
- [Tonc](https://gbadev.net/tonc/foreword.html)
- [gbadev](https://gbadev.net/)

## Foreword
THIS IS NOT THE BEST C++ CODE EVER. IT IS FAR FROM PERFECT. THIS IS INTENDED AS A PROOF OF CONCEPT, RATHER THAN A FINAL PRODUCT. PLEASE TAKE THIS INTO CONSIDERATION!

I love feedback, criticisms, suggestions, comments, concerns, PRs, issues, and things of that nature.

I really reccomend using the `no$gba` emulator specifically for it's debugging tools. Extremely helpful for seeing tilesets and maps in the VRAM viewer while a game is running. It can be daunting at first, but I encourage you to explore some of it's other functionalities and tools as well.

I also have used a Windows machine for the development of this project. There might be differences for building this project on a Mac or Linux that I am not familiar with.

### Makefile

Important environment variables to set.

- DEVKITARM — path to devkitARM installation
- DEVKITPRO — path to devkitPro

In addition to `make` and `make clean`, I've added two custom options for `make`;

- `make pad` will pad the gba file to the nearest 4kb which might help if flashing the file to a GBA cart
- `make full` will run `make clean`, `make`, and then `make pad` in a row for a "full build" of the projects 

## Conway's Game of Life
If you aren't familiar with Conway's Game of Life, it is a cellular automaton that has a very specific ruleset to create simulations. Each generation, every cell is checked and the rules are applied to create the next generation. Simulations don't have an end point, and will continue indefinitely
- 2 cell states, alive or dead
- Living cell has 2 or 3 neighbors, it continues living
- Living cell has < 2 or > 3, it will die
- Dead cell has exactly 3 neighbors, it will become alive
- Checks the 8 neighboring cells

The sections below are three variations I created for different types of fire, and I will list some of their characteristics to show how they differ from Conway's Game of Life.

## Firework
- 32 cell states, indicating intensity. 0 is black, 31 is white
- 0 intensity cells have a chance to change to 31 intensity cells to initiate a "firework"
- Checks 4 neighbors in cardinal directions
- Uses "highest-intensity" ruleset to determine the state of cells
- Slight randomness to make decay more realistic
- Slight variation in size

## Flame
- 8 states, indicating intensity, from black to red to orange to yellow to white.
- Checks the three neighbors below each cell
- two variations of rulesets: "average-intesity" with some random variation and "highest-intensity" with some random variation
- Manually defined bottom row to initiate, and propagate randomness in the flame

## Forest Fire
- 3 states, green is living trees, red is burning trees, black is burnt trees
- checks three neighbors in randomly defined direction
- Initial state selects one cell to be burning
- Green cells have a chance to turn red if there are red cells in indicated direction
- Red cells have a chance to extinguish to black cells
- Black cells do not propagate fire and will remain black for the entirety of the simulation