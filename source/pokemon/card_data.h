#pragma once
#include "pokemon.h"

/* The card database now contains only Pokemon that have printable art. */
uint8_t            card_data_count(void);
const PokemonCard *card_data_get(uint8_t i);
