#include "IRremote.h"
#include "IRremoteInt.h"

void setup(void);
void storeCode(struct decode_results *results);
void sendCode(int repeat);
void loop(void);
void sony_poweron(void);
void sony_volup(void);
void sony_voldown(void);
void sony_chup(void);
void sony_chdown(void);
