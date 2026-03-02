; ========= traffic_io.S (ATmega2560 / Arduino Mega 2560) =========
; Mixed C + AVR assembly for GPIO init + LED/Buzzer ON/OFF.
;
; Pin mapping used (from pins_mega.h):
;   NS_RED     = D22 = PA0
;   NS_YELLOW  = D24 = PA2
;   NS_GREEN   = D26 = PA4
;   EW_RED     = D28 = PA6
;   EW_YELLOW  = D30 = PC7
;   EW_GREEN   = D32 = PC5
;   BUZZER     = D34 = PC3
;
; Important:
; - Use .S (capital S) so the preprocessor runs and <avr/io.h> works.
; - GCC AVR calling convention for 8-bit args:
;     arg0 -> r24, arg1 -> r22
; - Return values (if any) are in r24 (not used here).

#define __SFR_OFFSET 0x00
#include <avr/io.h>

.global traffic_gpio_init_asm
.global traffic_set_output_asm

.section .text

; void traffic_gpio_init_asm(void);
traffic_gpio_init_asm:
    ; Configure Port A bits as outputs (D22..D29)
    sbi DDRA, 0      ; PA0 -> D22 (NS_RED)
    sbi DDRA, 2      ; PA2 -> D24 (NS_YELLOW)
    sbi DDRA, 4      ; PA4 -> D26 (NS_GREEN)
    sbi DDRA, 6      ; PA6 -> D28 (EW_RED)

    ; Configure Port C bits as outputs (D30..D37)
    sbi DDRC, 7      ; PC7 -> D30 (EW_YELLOW)
    sbi DDRC, 5      ; PC5 -> D32 (EW_GREEN)
    sbi DDRC, 3      ; PC3 -> D34 (BUZZER)
    ret

; void traffic_set_output_asm(uint8_t id, uint8_t on);
; id in r24, on in r22
traffic_set_output_asm:
    ; if on == 0 -> go to OFF path for selected id
    tst r22
    breq .Loff

    ; -------- ON paths --------
    cpi r24, 0
    breq .Lns_red_on
    cpi r24, 1
    breq .Lns_yel_on
    cpi r24, 2
    breq .Lns_grn_on
    cpi r24, 3
    breq .Lew_red_on
    cpi r24, 4
    breq .Lew_yel_on
    cpi r24, 5
    breq .Lew_grn_on
    cpi r24, 6
    breq .Lbuz_on
    ret

.Lns_red_on:
    sbi PORTA, 0
    ret
.Lns_yel_on:
    sbi PORTA, 2
    ret
.Lns_grn_on:
    sbi PORTA, 4
    ret
.Lew_red_on:
    sbi PORTA, 6
    ret
.Lew_yel_on:
    sbi PORTC, 7
    ret
.Lew_grn_on:
    sbi PORTC, 5
    ret
.Lbuz_on:
    sbi PORTC, 3
    ret

    ; -------- OFF paths --------
.Loff:
    cpi r24, 0
    breq .Lns_red_off
    cpi r24, 1
    breq .Lns_yel_off
    cpi r24, 2
    breq .Lns_grn_off
    cpi r24, 3
    breq .Lew_red_off
    cpi r24, 4
    breq .Lew_yel_off
    cpi r24, 5
    breq .Lew_grn_off
    cpi r24, 6
    breq .Lbuz_off
    ret

.Lns_red_off:
    cbi PORTA, 0
    ret
.Lns_yel_off:
    cbi PORTA, 2
    ret
.Lns_grn_off:
    cbi PORTA, 4
    ret
.Lew_red_off:
    cbi PORTA, 6
    ret
.Lew_yel_off:
    cbi PORTC, 7
    ret
.Lew_grn_off:
    cbi PORTC, 5
    ret
.Lbuz_off:
    cbi PORTC, 3
    ret
