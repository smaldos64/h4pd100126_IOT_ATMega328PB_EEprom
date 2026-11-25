/**
 * @file main.c
 * @author ltpe
 * @date 2026-11-05
 * @brief Main function
 */
#define F_CPU 16000000UL  // Antager 16 MHz krystal (ret hvis din kører med f.eks. 8 MHz)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#define DEFAULT_TIMEOUT_1000MS  1000u       /* 1000 ms = 1 sekund */
#define TIMEOUT_250MS           250u
#define MIN_TIMEOUT_MS          1u
#define MAX_TIMEOUT_MS          60000u      /* 60 sekunder maks */

#define EEPROM_MAGIC_16BIT      0x5A3Cu     // Marker for initialiseret 16-bit
#define GreenLedBit PORTB1  // Antag at den grønne LED er tilsluttet pin PB5

static uint16_t EEMEM ee_timeout_magic = EEPROM_MAGIC_16BIT;
static uint16_t EEMEM ee_timeout_ms = TIMEOUT_250MS;

volatile uint16_t active_timeout_ms = DEFAULT_TIMEOUT_1000MS;

static uint16_t eeprom_load_timeout_ms(void) 
{
    uint16_t magic = eeprom_read_word(&ee_timeout_magic);
    if (EEPROM_MAGIC_16BIT == magic) 
    {
        uint16_t stored = eeprom_read_word(&ee_timeout_ms);
        if (stored >= MIN_TIMEOUT_MS && stored <= MAX_TIMEOUT_MS) 
        {
            return stored;
        }
    }
    return DEFAULT_TIMEOUT_1000MS;
}

static void eeprom_save_timeout_ms(uint16_t timeout_ms) 
{
    uint16_t magic = eeprom_read_word(&ee_timeout_magic);
    uint16_t current = (EEPROM_MAGIC_16BIT == magic) ? eeprom_read_word(&ee_timeout_ms) : 0u;

    if ((EEPROM_MAGIC_16BIT == magic) && (current != timeout_ms))
    {
        //eeprom_update_word(&ee_timeout_ms, timeout_ms);
        eeprom_write_word(&ee_timeout_ms, timeout_ms);
        current = eeprom_read_word(&ee_timeout_ms);
    }
}

// Initialiseringsfunktion til Timer 1 med dynamisk millisekund-beregning
void Timer1_Init_ms(uint16_t timeout_ms) {
    // 1. Nulstil register for en sikkerheds skyld
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;

    // 2. Vælg CTC-mode (Clear Timer on Compare Match)
    // WGM13:0 = 0100 -> WGM12 sættes i TCCR1B
    TCCR1B |= (1 << WGM12);

    // 3. Vælg Prescaler = 256
    // CS12 = 1, CS11 = 0, CS10 = 0 (giver prescaler 256)
    TCCR1B |= (1 << CS12);

    // 4. Beregn OCR1A-værdien ud fra ønskede millisekunder
    // Formel: Ticks = (F_CPU / Prescaler / 1000) * timeout_ms
    // For 16 MHz og prescaler 256 er frekvensen ind i timeren: 16.000.000 / 256 = 62.500 Hz (62,5 ticks pr. ms)
    uint32_t ticks = ((F_CPU / 256UL) * timeout_ms) / 1000UL;

    if (ticks > 0) {
        ticks--; // Da sammenligningen starter fra 0
    }
    
    // Sørg for at værdien ikke overstiger 16-bit max (65535)
    if (ticks > 65535) {
        ticks = 65535; 
    }

    OCR1A = (uint16_t)ticks;

    // 5. Aktivér Compare Match A Interrupt for Timer 1
    TIMSK1 |= (1 << OCIE1A);
}

// ISR (Interrupt Service Routine) for Timer 1 Compare Match A
ISR(TIMER1_COMPA_vect) {
    // Indsæt den kode, der skal udføres ved hvert timeout.
    // F.eks. toggling af en LED eller flag-sætning til hovedprogrammet.
    PORTB ^= (1 << GreenLedBit);
}

int main(void) {
    DDRB |= (1 << GreenLedBit);  // Sæt PB1 som output for den grønne LED

    active_timeout_ms = eeprom_load_timeout_ms();
    // if (!timer1_apply_timeout_ms(active_timeout_ms)) {
    //     active_timeout_ms = DEFAULT_TIMEOUT_MS;
    //     timer1_apply_timeout_ms(active_timeout_ms);
    // }

    Timer1_Init_ms(active_timeout_ms);

    eeprom_save_timeout_ms(2500);

    // Aktivér globale interrupts
    sei();

    while (1) {
        // Hovedloop kan køre her uafhængigt
    }
}
