# Pokemon Thermal Printer

A micro:bit v2 reads temperature, humidity, and light, spawns a wild Pokemon
whose type is weighted by the environment, and prints a card for it on a
DFR0503 thermal printer when you catch it with button A.

### Type from the environment
`pokemon_type_from_env()` scores all eight types with smooth linear weights
over normalized temperature, humidity, and light, then samples one. Fire likes
hot/dry/bright, Water likes cool/humid, Ghost likes darkness, and so on. Each
encounter then picks a random card of that type.

### Catch rate
Each card has a `catch_rate` from 1 to 255 (higher is easier). Pressing A rolls
a byte against it; on success the card prints, otherwise the Pokemon flees. The
1-255 catch-rate idea mirrors the mechanic from the mainline Pokemon games.
- Catch rate / capture mechanic background:
  https://bulbapedia.bulbagarden.net/wiki/Catch_rate

### Printing the art
The Pokemon drawings are multi-character ASCII art. A single character can't be
one printer pixel (the picture would turn into a blob), so each character is
drawn as its glyph from a 5x7 font. `printer_print_ascii_art()` rasterizes each
text line into an 8-row band and sends it with the ESC/POS raster command
`GS v 0`.

## Sources and citations

### ASCII art
- **Pikachu** (the detailed full-body drawing):
  https://asciiart.cc/view/11474v
- **The other 19 Pokemon** come from the "151 Pokemon ASCII Art" collection by
  Matheus Faria:
  https://gist.github.com/MatheusFaria/4cbb8b6dbe33fd5605cf8b8f7130ba6d
  That gist credits its own sources as:
  - http://www.fiikus.net/?pokedex
  - http://www.world-of-nintendo.com/pictures/text/
  - generated with http://www.text-image.com

### Font
- **5x7 glyph font** used to render the ASCII art is the classic fixed-width
  GLCD font from Adafruit-GFX (`glcdfont.c`):
  https://github.com/adafruit/Adafruit-GFX-Library/blob/master/glcdfont.c

### Printer / ESC-POS
- `GS v 0` raster bit-image and `ESC d` / `ESC @` commands are standard ESC/POS:
  https://reference.epson-biz.com/modules/ref_escpos/index.php

### Sensors
- AS7341 spectral sensor (light), SMUX setup and `GS v 0`-style register map:
  https://look.ams-osram.com/m/24266a3e584de4db/original/AS7341-DS000504.pdf
- SHTC3 temperature/humidity sensor datasheet (Sensirion).

## Building

```
make            # the game
make flash      # build + flash over the debugger
```

Test programs for bringing up the printer live in the repo root and build with
`make TEST_PRINTER=1`, `make TEST_BUTTON=1`, or `make TEST_BAUD=1`.

## Credits

ASCII art is the work of its original (mostly anonymous) artists, collected from
the sources listed above. Pokemon and all names are trademarks of Nintendo /
Game Freak / The Pokemon Company; this is a non-commercial hobby project.
This 

## Citations & AI Usage

We use nu-microbit-base (https://github.com/nu-ce346/nu-microbit-base) as the entire build-and-runtime foundation, essentially everything except our own `source/` code, which is why our Makefile can be so small: it only declares the chip, SDK version, and source files, then includes the repo's `boards/microbit_v2/Board.mk` and `nrf52x-base/make/AppMakefile.mk`, which supply the compiler flags, the bundled Nordic nRF5 SDK 16.0.0 drivers (`nrfx_uarte`, `nrf_twi_mngr`, `nrf_gpio`, `nrf_delay`, and the `printf`-over-serial retarget), the nRF52833 startup and "blank" linker script, and the `make flash` (OpenOCD) command, while compilation itself is done by the separately-installed ARM GNU Toolchain (`arm-none-eabi-gcc`) that produces the `.elf`/`.hex` in `_build/`; we used Claude Code to adapt this build setup from the previous labs, and our own work is the application layer in `source/` (the game logic, sensor wrappers, printer/font/art rendering, and speaker), with the ASCII art and 5x7 font cited in the sources above.


