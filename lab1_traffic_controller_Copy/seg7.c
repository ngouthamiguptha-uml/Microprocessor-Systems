#include "seg7.h"
#include <Arduino.h>

// Bit mapping you are effectively using (after your real wiring):
// bit7=A, bit6=B, bit5=C, bit4=D, bit3=E, bit2=F, bit1=G, bit0=DP
// Active-HIGH segments (1 = ON)

static const uint8_t segLUT[10] =
{
    0b11111100, // 0 = A B C D E F
    0b01100000, // 1 = B C        (fixed)
    0b11011010, // 2 = A B D E G
    0b11110010, // 3 = A B C D G  (fixed)
    0b01100110, // 4 = B C F G
    0b10110110, // 5 = A F D C G  (still OK with mapping)
    0b10111110, // 6 = A F D E C G
    0b11100000, // 7 = A B C      (fixed)
    0b11111110, // 8 = A B C D E F G
    0b11110110  // 9 = A B C D F G
};

static const uint8_t digPins[4] = { PIN_DIGIT_0, PIN_DIGIT_1, PIN_DIGIT_2, PIN_DIGIT_3 };
static int     displayValue = 0;
static uint8_t currentDigit = 2;

static void sr_send(uint8_t value) {
    digitalWrite(PIN_SR_LATCH, LOW);
    shiftOut(PIN_SR_DATA, PIN_SR_CLK, LSBFIRST, value); // LSBFIRST: bit0→Q0(A)
    digitalWrite(PIN_SR_LATCH, HIGH);
}

void seg7Init(void) {
    pinMode(PIN_SR_DATA,  OUTPUT);
    pinMode(PIN_SR_CLK,   OUTPUT);
    pinMode(PIN_SR_LATCH, OUTPUT);

    for (int i = 0; i < 4; i++) {
        pinMode(digPins[i], OUTPUT);
        digitalWrite(digPins[i], HIGH); // all digits OFF
    }

    sr_send(0x00); // all segments OFF
}

void seg7SetValue(int value) {
    if (value < 0)  value = 0;
    if (value > 99) value = 99;
    displayValue = value;
}

void seg7Refresh(void)
{
    uint8_t digits[4];

    digits[0] = 0;
    digits[1] = 0;
    digits[2] = (displayValue / 10) % 10;
    digits[3] = displayValue % 10;

    // all digits OFF
    for (int i = 0; i < 4; i++)
        digitalWrite(digPins[i], HIGH);

    // push segments for current digit
    sr_send(segLUT[digits[currentDigit]]);

    // enable digit (your wiring: LOW enables)
    digitalWrite(digPins[currentDigit], LOW);

    delayMicroseconds(1500);

    currentDigit++;
    if (currentDigit >= 4) currentDigit = 2;
}


/*
#include "seg7.h"
#include <Arduino.h>

// ======================================================
// ENABLE DIAGNOSTIC MODE
// set to 1 to test segments, 0 for normal operation
// ======================================================
#define SEG7_DIAGNOSTIC_MODE 1


// 5641A3 Common Anode
// shiftOut uses LSBFIRST
// bit0 → Q0
// bit1 → Q1
// ...
// bit7 → Q7

static const uint8_t segLUT[10] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111  // 9
};

static const uint8_t digPins[4] =
{
    PIN_DIGIT_0,
    PIN_DIGIT_1,
    PIN_DIGIT_2,
    PIN_DIGIT_3
};

static int displayValue = 0;
static uint8_t currentDigit = 2;


// ======================================================
// Shift register send
// ======================================================
static void sr_send(uint8_t value)
{
    digitalWrite(PIN_SR_LATCH, LOW);
    shiftOut(PIN_SR_DATA, PIN_SR_CLK, LSBFIRST, value);
    digitalWrite(PIN_SR_LATCH, HIGH);
}


// ======================================================
// Diagnostic function
// Lights ONE segment at a time every 2 seconds
// ======================================================
void seg7Diagnostic(void)
{
    static uint8_t bit = 0;

    // turn all digits OFF
    for (int i = 0; i < 4; i++)
        digitalWrite(digPins[i], HIGH);

    // ACTIVE-HIGH segments: 1 = ON, 0 = OFF
    uint8_t value = (uint8_t)(1u << bit);

    sr_send(value);

    // enable one digit for visibility (keep your existing polarity)
    digitalWrite(digPins[3], LOW);

    Serial.print("Testing bit: ");
    Serial.println(bit);

    delay(10000); // 10 seconds

    bit++;
    if (bit >= 8) bit = 0;
}
// ======================================================
// Init
// ======================================================
void seg7Init(void)
{
    pinMode(PIN_SR_DATA,  OUTPUT);
    pinMode(PIN_SR_CLK,   OUTPUT);
    pinMode(PIN_SR_LATCH, OUTPUT);

    for (int i = 0; i < 4; i++)
    {
        pinMode(digPins[i], OUTPUT);
        digitalWrite(digPins[i], HIGH);
    }

    //sr_send(0xFF); // all OFF (common anode)
    sr_send(0x00);
}


// ======================================================
// Set value
// ======================================================
void seg7SetValue(int value)
{
    if (value < 0) value = 0;
    if (value > 99) value = 99;

    displayValue = value;
}


// ======================================================
// Refresh display
// ======================================================
void seg7Refresh(void)
{

#if SEG7_DIAGNOSTIC_MODE

    seg7Diagnostic();
    return;

#else

    uint8_t digits[4];

    digits[0] = 0;
    digits[1] = 0;
    digits[2] = (displayValue / 10) % 10;
    digits[3] = displayValue % 10;

    // turn all OFF
    for (int i = 0; i < 4; i++)
        digitalWrite(digPins[i], HIGH);

    sr_send(segLUT[digits[currentDigit]]);

    digitalWrite(digPins[currentDigit], LOW);

    delayMicroseconds(1500);

    currentDigit++;
    if (currentDigit >= 4)
        currentDigit = 2;

#endif
}*/
