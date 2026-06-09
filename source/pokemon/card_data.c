#include "card_data.h"
#include "pokemon_art.h"

/* Pikachu art is kept here; the rest live in pokemon_art.h */
#define PIKACHU_ART_ROWS 39
static const char *const pikachu_art[PIKACHU_ART_ROWS] = {
    "quu..___",
    " $$$b  `---.___",
    "  \"$$b        `--.                          ___.---uuudP",
    "   `$$b           `.__.------.___     ___.---'      $$$$\"              .",
    "     \"$b          -'            `.-'            $$$\"              .'|",
    "       \".                                       d$\"             _.'  |",
    "         `.   /                              ...\"             .'     |",
    "           `./                           ..::-'            _.'       |",
    "            /                         .:::-'            .-'         .'",
    "           :                          ::''\\          _.'            |",
    "          .' .-.             .-.           `.      .'               |",
    "          : /'$$|           .@\"$\\           `.   .'              _.-'",
    "         .'|$u$$|          |$$,$$|           |  <            _.-'",
    "         | `:$$:'          :$$$$$:           `.  `.       .-'",
    "         :                  `\"--'             |    `-.     \\",
    "        :##.       ==             .###.       `.      `.    `\\",
    "        |##:                      :###:        |        >     >",
    "        |#'     `...'`..'          `###'        x:      /     /",
    "         \\                                   xXX|     /    ./",
    "          \\                                xXXX'|    /   ./",
    "          /`-.                                  `.  /   /",
    "         :    `-  ...........,                   | /  .'",
    "         |         ``:::::::'       .            |<    `.",
    "         |             ```          |           x| \\ `.:```.",
    "         |                         .'    /'   xXX|  `:` M`M':.",
    "         |    |                    ;    /:' xXXX'|  -'MMMMM:'",
    "         `.  .'                   :    /:'       |-'MMMM.-'",
    "          |  |                   .'   /'        .'MMM.-'",
    "          `'`'                   :  ,'          |MMM<",
    "            |                     `'            |tbap\\",
    "             \\                                  :MM.-'",
    "              \\                 |              .''",
    "               \\.               `.            /",
    "                /     .:::::::.. :           /",
    "               |     .:::::::::::`         /",
    "               |   .:::------------\\       /",
    "              /   .''               >::'  /",
    "              `',:                 :    .'",
    "                                   `::'",
};

/* catch_rate is 1-255: higher is easier. Mew and the final-stage Pokemon
   are deliberately low. */
static const PokemonCard cards[] = {
    { "Bulbasaur",  PTYPE_GRASS,    art_bulbasaur,  ART_BULBASAUR_ROWS,   90,
      { "Vine Whip", 35 },    { "Tackle", 20 },        120 },
    { "Charmander", PTYPE_FIRE,     art_charmander, ART_CHARMANDER_ROWS,  80,
      { "Ember", 40 },        { "Scratch", 18 },       120 },
    { "Charizard",  PTYPE_FIRE,     art_charizard,  ART_CHARIZARD_ROWS,  160,
      { "Flamethrower", 90 }, { "Wing Attack", 45 },    30 },
    { "Growlithe",  PTYPE_FIRE,     art_growlithe,  ART_GROWLITHE_ROWS,   85,
      { "Bite", 30 },         { "Ember", 35 },         110 },
    { "Squirtle",   PTYPE_WATER,    art_squirtle,   ART_SQUIRTLE_ROWS,    95,
      { "Water Gun", 35 },    { "Bubble", 20 },        120 },
    { "Blastoise",  PTYPE_WATER,    art_blastoise,  ART_BLASTOISE_ROWS,  170,
      { "Hydro Pump", 95 },   { "Bite", 40 },           30 },
    { "Psyduck",    PTYPE_WATER,    art_psyduck,    ART_PSYDUCK_ROWS,     80,
      { "Confusion", 30 },    { "Scratch", 18 },       115 },
    { "Pikachu",    PTYPE_ELECTRIC, pikachu_art,    PIKACHU_ART_ROWS,     70,
      { "Thunderbolt", 55 },  { "Quick Attack", 25 },   90 },
    { "Raichu",     PTYPE_ELECTRIC, art_raichu,     ART_RAICHU_ROWS,     120,
      { "Thunder", 80 },      { "Mega Punch", 40 },     60 },
    { "Voltorb",    PTYPE_ELECTRIC, art_voltorb,    ART_VOLTORB_ROWS,     70,
      { "Spark", 25 },        { "Self-Destruct", 100 },100 },
    { "Eevee",      PTYPE_NORMAL,   art_eevee,      ART_EEVEE_ROWS,       85,
      { "Tackle", 22 },       { "Swift", 30 },         110 },
    { "Snorlax",    PTYPE_NORMAL,   art_snorlax,    ART_SNORLAX_ROWS,    200,
      { "Body Slam", 70 },    { "Rest", 0 },            25 },
    { "Jigglypuff", PTYPE_NORMAL,   art_jigglypuff, ART_JIGGLYPUFF_ROWS, 100,
      { "Sing", 0 },          { "Pound", 18 },         130 },
    { "Meowth",     PTYPE_NORMAL,   art_meowth,     ART_MEOWTH_ROWS,      75,
      { "Pay Day", 30 },      { "Scratch", 20 },       120 },
    { "Clefairy",   PTYPE_NORMAL,   art_clefairy,   ART_CLEFAIRY_ROWS,    90,
      { "Metronome", 50 },    { "Pound", 18 },         100 },
    { "Gengar",     PTYPE_GHOST,    art_gengar,     ART_GENGAR_ROWS,     130,
      { "Shadow Ball", 75 },  { "Lick", 25 },           45 },
    { "Dewgong",    PTYPE_ICE,      art_dewgong,    ART_DEWGONG_ROWS,    140,
      { "Ice Beam", 70 },     { "Aurora Beam", 50 },    60 },
    { "Abra",       PTYPE_PSYCHIC,  art_abra,       ART_ABRA_ROWS,        60,
      { "Teleport", 0 },      { "Confusion", 30 },     100 },
    { "Alakazam",   PTYPE_PSYCHIC,  art_alakazam,   ART_ALAKAZAM_ROWS,   135,
      { "Psychic", 95 },      { "Recover", 0 },         40 },
    { "Mew",        PTYPE_PSYCHIC,  art_mew,        ART_MEW_ROWS,        150,
      { "Psychic", 90 },      { "Transform", 0 },       10 },
};

uint8_t card_data_count(void) {
    return (uint8_t)(sizeof(cards) / sizeof(cards[0]));
}

const PokemonCard *card_data_get(uint8_t i) {
    if (i >= card_data_count()) i = 0;
    return &cards[i];
}
