\##File ownership



\###Gouthami (FSM / timing / integration):



controller.c/h

timer1\_1hz.c/h

traffic\_controller.ino



\###Quy (drivers / wiring / assembly):



keypad.c/h

seg7.c/h

hal.\* + .S files



\###Shared file by both:

pins\_mega.h



Function names to be followed:



hal.h (Quy's file):



void hal\_init(void);



// NS direction

void hal\_ns\_red(int on);

void hal\_ns\_yellow(int on);

void hal\_ns\_green(int on);



// EW direction

void hal\_ew\_red(int on);

void hal\_ew\_yellow(int on);

void hal\_ew\_green(int on);



void hal\_buzzer\_on(void);

void hal\_buzzer\_off(void);



keypad.h (Quy's file):



typedef enum {

&nbsp; KEY\_NONE, KEY\_STAR, KEY\_HASH,

&nbsp; KEY\_A, KEY\_B,

&nbsp; KEY\_0, KEY\_1, KEY\_2, KEY\_3, KEY\_4, KEY\_5, KEY\_6, KEY\_7, KEY\_8, KEY\_9

} KeyEvent;



KeyEvent keypad\_poll(void);





seg7.h (Quy's file):



void seg7\_init(void);

void seg7\_display\_number(int value);   // 0..9999



controller.h (Gouthami's file):



void controller\_init(void);

void controller\_on\_1hz\_tick(void);

void controller\_on\_key(KeyEvent k);

int  controller\_get\_seconds\_remaining(void);



