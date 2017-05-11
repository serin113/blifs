# blifs
Simulate and generate 2D cellular automata in a console by using formatted files.

Sample files are in the `boards` & `rules` folders.

## Compiling
Run `./build.sh` on a Linux system, or in an *Ubuntu Subsystem for Windows 10* (developer mode should be enabled first).

## Command line options

### Required (with conditions):
* **`-b board-file`**: Load a board file (required, unless `-g` is passed which ignores it)
* **`-r rules-file`**: Load a rules file (required, unless `-i 0` is passed)
* **`-o file`**:       Output final result as a board file (required when `-g` is passed, optional otherwise)

### Optional:
#### Info display options:
* **`-c`**: Display only initial & final iteration, or info for `-g` (default)
* **`-v`**: Display verbose output
* **`-s`**: Don't display any output
#### Other options:
* **`-n integer`**: Number of times to run simulations (1 by default) (if 0 is passed, then the rules file isn't required)
* **`-d double`**: Delay in seconds between displaying each generation (0 by default) *(you probably want to use this with `-v` if you plan on actually viewing the simulation as it happens)*
* **`-g w:h:d`**: Generate a random board of size `w*h` with seed `s` (`w` & `h` are required, `d` is optional and is `2` by default)
* **`-i`**: Interactive mode, press any key to go to the next generation *(you probably want to use `-v` as well for this)*
#### Help options:
* **`-h b`**: Show info on board files
* **`-h r`**: Show info on rules files
* **`-h`**:Show the help options

## Sample Usage
Load the `boards/gol_glider` board, simulate it with the `rules/life` rules for 100 times, with a 1=second delay between each iteration, and output the resulting board in the 100th iteration to `board.out`:
```
./blifs -b boards/gol_glider -r rules/life -n 100 -v -d 1 -o board.out
```

Generate a random 20-wide 40-high board with the seed `12345`, and output the board to `random.out`:
```
./blifs -g 20:40:12345 -o random.out
```

## To-do
* RLE file compatibility
* ncurses port
* Adding cells from a board file to a larger board

## Board file format

A board file is a space-delimited plain text file with the format :
```
<w> <h>
<col> <row>
<col> <row>
...
<col> <row>
```

The `<w> <h>` pair defines the width & height of the simulation grid, respectively.

Each `<col> <row>` pair defines a **live cell** in the grid, and is **zero-indexed** (col is in `[0, w-1]`, row is in `[0, h-1]`).

***e.g.*** the configuration of a glider in a `10*10` grid (also in `boards/gol_glider`):
```
10 10
0 0
1 1
1 2
2 0
2 1
```

## Rules file format
A rule file is a space-delimited plain text file with the format:
```
<0> <1> <2> <3> <4> <5> <6> <7> <8>
<0> <1> <2> <3> <4> <5> <6> <7> <8>
```
Each of the numbers can either have 0 or 1 in the file. Each line has exactly 9 numbers.

The first line corresponds to which number of neighbors is needed for a live cell to live (1) or die (0).

The second line corresponds to which number of neighbors is needed for a dead cell to become live (1) or remain dead (0).

***e.g.*** the *Game of Life* rules configuration (also in `rules/life`):
```
0 0 1 1 0 0 0 0 0
0 0 0 1 0 0 0 0 0
```
## License (GPLv3)
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
