#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "../sensors/sensors.h"

#define POKEMON_MAX_SPAWN 3

typedef enum {
    PTYPE_FIRE = 0,
    PTYPE_WATER,
    PTYPE_GRASS,
    PTYPE_ICE,
    PTYPE_GHOST,
    PTYPE_ELECTRIC,
    PTYPE_NORMAL,
    PTYPE_PSYCHIC,
    PTYPE_COUNT
} PokemonType;

typedef struct {
    const char *name;
    uint16_t    damage;
} PokemonMove;

typedef struct {
    const char        *name;
    PokemonType        type;
    /* ASCII art rows ('#'/glyph chars), rendered via the printer's font. */
    const char *const *art;
    uint16_t           art_rows;
    uint16_t           hp;
    PokemonMove        move1;
    PokemonMove        move2;
    uint8_t            catch_rate;  /* 1-255, higher = easier to catch */
} PokemonCard;

const char       *pokemon_type_name(PokemonType type);
void              pokemon_init_rng(uint32_t seed);
PokemonType       pokemon_type_from_env(SensorReading reading);
void              pokemon_select_group(SensorReading reading,
                                       const PokemonCard **out,
                                       uint8_t *count);
bool              pokemon_should_spawn(SensorReading reading);
/* Roll against the card's catch_rate. true = caught, false = it got away. */
bool              pokemon_attempt_catch(const PokemonCard *card);
