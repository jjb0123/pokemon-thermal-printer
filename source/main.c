#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf_delay.h"
#include "nrf_gpio.h"

#include "sensors/sensors.h"
#include "pokemon/pokemon.h"
#include "speaker/speaker.h"
#include "printer/printer.h"

/* Buttons: P0.14 (A) and P0.23 (B), active low with internal pull-up */
#define BUTTON_A NRF_GPIO_PIN_MAP(0, 14)
#define BUTTON_B NRF_GPIO_PIN_MAP(0, 23)

typedef enum {
    BUTTON_NONE = 0,
    BUTTON_PRESS_A,
    BUTTON_PRESS_B,
} ButtonPress;

/* Debounce a single button; returns true once it is confirmed pressed
 * and then released. */
static bool button_pressed(uint32_t pin) {
    if (nrf_gpio_pin_read(pin) == 0) {
        nrf_delay_ms(20);
        if (nrf_gpio_pin_read(pin) == 0) {
            while (nrf_gpio_pin_read(pin) == 0) {}
            return true;
        }
    }
    return false;
}

/* Poll both buttons every 50 ms for up to ms; returns which was pressed. */
static ButtonPress button_wait(uint32_t ms) {
    for (uint32_t i = 0; i < ms / 50; i++) {
        if (button_pressed(BUTTON_A)) return BUTTON_PRESS_A;
        if (button_pressed(BUTTON_B)) return BUTTON_PRESS_B;
        nrf_delay_ms(50);
    }
    return BUTTON_NONE;
}

static const PokemonCard *spawned[POKEMON_MAX_SPAWN];
static uint8_t            spawned_count = 0;

int main(void) {
    printf("=== Pokemon Thermal Printer ===\n");

    nrf_gpio_cfg_input(BUTTON_A, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(BUTTON_B, NRF_GPIO_PIN_PULLUP);
    speaker_init();
    sensors_init();
    printer_init();

    SensorReading boot = sensors_read();
    uint32_t seed = *(uint32_t *)&boot.temperature_c
                  ^ (uint32_t)(boot.humidity_pct * 100.0f)
                  ^ ((uint32_t)boot.light_level << 16)
                  ^ 0xDEADBEEFu;
    pokemon_init_rng(seed);

    printf("Ready! Press A to catch, B to skip.\n\n");

    while (true) {
        SensorReading env = sensors_read();
        printf("Temp: %.1fC / %.1fF   Hum: %.1f%%   Light: %u\n",
               env.temperature_c,
               env.temperature_c * 1.8f + 32.0f,
               env.humidity_pct,
               (unsigned)env.light_level);

        if (spawned_count == 0 && pokemon_should_spawn(env)) {
            pokemon_select_group(env, spawned, &spawned_count);
            const PokemonCard *c = spawned[0];
            printf("A wild %s appeared! (%s, HP %u, catch %u/255)\n",
                   c->name, pokemon_type_name(c->type), c->hp, c->catch_rate);
            printf("Press A to throw a Pokeball, B to skip!\n");
        } else if (spawned_count > 0) {
            printf("%s is still here, press A to catch or B to skip!\n",
                   spawned[0]->name);
        }

        ButtonPress press = button_wait(7000);
        if (spawned_count > 0) {
            const PokemonCard *c = spawned[0];
            if (press == BUTTON_PRESS_A) {
                printf("You threw a Pokeball at %s...\n", c->name);
                if (pokemon_attempt_catch(c)) {
                    printf("Gotcha! %s was caught!\n", c->name);
                    printer_print_card(c);
                    speaker_play_littleroot_town();
                } else {
                    printf("Oh no! %s broke free and got away!\n", c->name);
                }
                spawned_count = 0;
            } else if (press == BUTTON_PRESS_B) {
                printf("You let %s go on its way.\n", c->name);
                spawned_count = 0;
            }
        }

        printf("\n");
    }
}
