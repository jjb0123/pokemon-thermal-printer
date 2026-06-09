#pragma once
#include <stdint.h>
#include "../pokemon/pokemon.h"

void printer_init(void);
void printer_print_text(const char *text);
/* Draw each character of the art lines using the 5x7 font. */
void printer_print_ascii_art(const char *const *lines, uint16_t nlines);
/* Print a card: border, name/type/HP, art, and both moves. */
void printer_print_card(const PokemonCard *card);
void printer_feed(uint8_t lines);
