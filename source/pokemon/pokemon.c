#include "pokemon.h"
#include "card_data.h"

static const char *const TYPE_NAMES[PTYPE_COUNT] = {
    "Fire", "Water", "Grass", "Ice", "Ghost", "Electric", "Normal", "Psychic",
};

const char *pokemon_type_name(PokemonType type) {
    return (type < PTYPE_COUNT) ? TYPE_NAMES[type] : "???";
}

static uint32_t rng_state = 12345;

static uint32_t rng_next(void) {
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state;
}

void pokemon_init_rng(uint32_t seed) {
    rng_state = seed ? seed : 12345;
}

static float type_weight(PokemonType t, float temp, float hum, float lux) {
    float T = (temp + 10.0f) / 50.0f;
    float H = hum  / 100.0f;
    float L = lux  / 255.0f;
    if (T < 0.0f) T = 0.0f; else if (T > 1.0f) T = 1.0f;
    if (H < 0.0f) H = 0.0f; else if (H > 1.0f) H = 1.0f;

    float d;
    switch (t) {
        case PTYPE_FIRE:
            return 0.3f + T * 2.5f + (1.0f - H) * 0.5f + L * 0.4f;

        case PTYPE_WATER:
            return 0.3f + (1.0f - T) * 1.5f + H * 1.5f;

        case PTYPE_GRASS:
            return 0.2f + L * 2.0f + H * 0.5f;

        case PTYPE_ICE:
            return 0.1f + (1.0f - T) * (1.0f - T) * 3.0f + H * 0.3f;

        case PTYPE_GHOST:
            return 0.3f + (1.0f - L) * 3.0f + H * 0.3f;

        case PTYPE_ELECTRIC:
            return 0.4f + L * 1.5f + T * (1.0f - T) * 2.0f;

        case PTYPE_NORMAL:
            return 0.8f;

        case PTYPE_PSYCHIC:
            d = L - 0.35f;
            if (d < 0.0f) d = -d;
            return 0.2f + (d < 0.4f ? (0.4f - d) * 3.5f : 0.0f);

        default:
            return 0.5f;
    }
}

PokemonType pokemon_type_from_env(SensorReading r) {
    float weights[PTYPE_COUNT];
    float total = 0.0f;

    for (int i = 0; i < PTYPE_COUNT; i++) {
        float w = type_weight((PokemonType)i,
                              r.temperature_c, r.humidity_pct,
                              (float)r.light_level);
        weights[i] = w < 0.0f ? 0.0f : w;
        total += weights[i];
    }

    float pick = (float)(rng_next() % 10000u) / 10000.0f * total;
    float cum  = 0.0f;
    for (int i = 0; i < PTYPE_COUNT; i++) {
        cum += weights[i];
        if (pick < cum) return (PokemonType)i;
    }
    return PTYPE_NORMAL;
}

void pokemon_select_group(SensorReading r,
                          const PokemonCard **out,
                          uint8_t *count) {
    /* One wild Pokemon per encounter. Pick a type from the environment,
       then a random card of that type. */
    PokemonType type = pokemon_type_from_env(r);

    const PokemonCard *matches[32];
    uint8_t m = 0;
    uint8_t n = card_data_count();
    for (uint8_t i = 0; i < n && m < 32; i++) {
        const PokemonCard *c = card_data_get(i);
        if (c->type == type) matches[m++] = c;
    }

    if (m == 0) {
        out[0] = card_data_get((uint8_t)(rng_next() % n));  /* fallback: any */
    } else {
        out[0] = matches[rng_next() % m];
    }
    *count = 1;
}

bool pokemon_attempt_catch(const PokemonCard *card) {
    if (!card) return false;
    return (uint8_t)(rng_next() & 0xFF) < card->catch_rate;
}

bool pokemon_should_spawn(SensorReading r) {
    (void)r;
    return true;  /* a new wild Pokemon is always available when idle */
}
