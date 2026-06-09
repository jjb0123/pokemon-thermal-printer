#include "speaker.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include <stdint.h>
#include <stddef.h>

#define SPEAKER_PIN  NRF_GPIO_PIN_MAP(0, 0)

// Improved tone generation with better timing accuracy
static void tone(uint32_t freq_hz, uint32_t duration_ms) {
    if (freq_hz == 0 || duration_ms == 0) {
        nrf_gpio_pin_clear(SPEAKER_PIN);
        if (duration_ms > 0) {
            nrf_delay_ms(duration_ms);
        }
        return;
    }
    
    // Calculate period in microseconds
    uint32_t period_us = 1000000u / freq_hz;
    uint32_t half_period = period_us / 2;
    uint32_t cycles = (freq_hz * duration_ms) / 1000u;
    
    for (uint32_t i = 0; i < cycles; i++) {
        nrf_gpio_pin_set(SPEAKER_PIN);
        nrf_delay_us(half_period);
        nrf_gpio_pin_clear(SPEAKER_PIN);
        nrf_delay_us(half_period);
    }
}

// Note frequencies (Hz)
#define REST  0
#define C4    261
#define D4    294
#define E4    330
#define F4    349
#define G4    392
#define A4    440
#define Bb4   466
#define B4    494
#define C5    523
#define D5    587
#define Eb5   622
#define E5    659
#define F5    698
#define G5    784
#define A5    880

// Timing values (milliseconds at ~120 BPM)
#define S    125   // Sixteenth note
#define E    250   // Eighth note
#define Q    500   // Quarter note
#define DQ   750   // Dotted quarter
#define H    1000  // Half note
#define DH   1500  // Dotted half

typedef struct { 
    uint32_t freq; 
    uint32_t dur; 
} Note;

// Littleroot Town Theme (Pokémon Ruby/Sapphire)
static const Note THEME[] = {
    // Intro phrase
    {REST, E},
    {G4, Q}, {D5, Q}, {E5, Q}, {D5, Q},
    {C5, Q}, {A4, Q}, {B4, Q}, {A4, Q},
    
    // Rising phrase
    {G4, Q}, {D5, Q}, {E5, Q}, {F5, Q},
    {E5, Q}, {D5, Q}, {C5, Q}, {A4, Q},
    
    // Descending phrase
    {G4, Q}, {A4, Q}, {B4, Q}, {D5, Q},
    {C5, Q}, {B4, Q}, {A4, Q}, {G4, Q},
    
    // Climax
    {A4, Q}, {B4, Q}, {C5, Q}, {D5, Q},
    {E5, Q}, {D5, Q}, {C5, Q}, {A4, Q},
    
    // Final rest
    {REST, H}
};

void speaker_init(void) {
    nrf_gpio_cfg_output(SPEAKER_PIN);
    nrf_gpio_pin_clear(SPEAKER_PIN);
}

void speaker_play_littleroot_town(void) {
    size_t num_notes = sizeof(THEME) / sizeof(THEME[0]);
    
    for (size_t i = 0; i < num_notes; i++) {
        // Play the note
        tone(THEME[i].freq, THEME[i].dur);
        
        // Small gap between notes for articulation
        nrf_gpio_pin_clear(SPEAKER_PIN);
        nrf_delay_ms(15);
    }
}

//play the theme with repetition (as in the game)
void speaker_play_littleroot_loop(void) {
    while (1) {
        speaker_play_littleroot_town();
        nrf_delay_ms(500);  // Pause between repetitions
    }
}

// Optional: Test function to verify speaker works
void speaker_test(void) {
    const uint32_t test_notes[] = {C4, D4, E4, F4, G4, A4, B4, C5};
    
    for (int i = 0; i < 8; i++) {
        tone(test_notes[i], 300);
        nrf_delay_ms(50);
    }
    
    // Play a descending scale
    for (int i = 6; i >= 0; i--) {
        tone(test_notes[i], 300);
        nrf_delay_ms(50);
    }
}