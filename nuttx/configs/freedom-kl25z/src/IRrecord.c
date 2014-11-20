/*
 * IRrecord: record and play back IR signals as a minimal 
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * An IR LED must be connected to the output PWM pin 3.
 * A button must be connected to the input BUTTON_PIN; this is the
 * send button.
 * A visible LED can be connected to STATUS_PIN to provide status.
 *
 * The logic is:
 * If the button is pressed, send the IR code.
 * If an IR code is received, record it.
 *
 * Version 0.11 September, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include "kl_gpio.h"
#include "up_arch.h"
#include "chip/kl_pit.h"
#include "chip/kl_tpm.h"
#include "chip/kl_sim.h"

#include <stdio.h>
#include <arch/board/IRremote.h>
#include <arch/board/IRremoteInt.h>
#include <arch/board/IRrecord.h>

#include "freedom-kl25z.h"

int RECV_PIN = 11;
int BUTTON_PIN = 12;
int STATUS_PIN = 13;

#define HIGH 1
#define LOW  0

decode_results_t results;

const unsigned int rawcode[]={ 0x0000, 0x0067, 0x0000, 0x000d, 0x0060, 0x0018, 0x0030,
                               0x0018, 0x0018, 0x0018, 0x0030, 0x0018, 0x0018, 0x0018,
                               0x0030, 0x0018, 0x0018, 0x0018, 0x0018, 0x0018, 0x0030,
                               0x0018, 0x0018, 0x0018, 0x0018, 0x0018, 0x0018, 0x0018,
                               0x0018, 0x0000 };

void setup()
{
  int ret, i;
  uint32_t regval;

  printf("Executing IRrecord setup!\n");

  regval = getreg32(KL_SIM_SCGC6);
  regval |= SIM_SCGC6_TPM0 | SIM_SCGC6_TPM1 | SIM_SCGC6_TPM2;
  putreg32(regval, KL_SIM_SCGC6);

  /* Configure PIN used to receive IR */
  kl_configgpio(GPIO_IR_RECV);

  /* Configure PIN used to send IR */
  kl_configgpio(GPIO_IR_LED1);

#if 0
  /* Freeze Timer when enter Debug Mode */
  putreg32(0, PIT_MCR);

  /* Default start value to 50us */
  putreg32(1200, PIT_LDVAL0);

  /* Attached to PIT Interrupt */
  ret = irq_attach(KL_IRQ_PIT, periodic_read_ir);
  if (ret == OK)
    {
        up_enable_irq(KL_IRQ_PIT);
    }

  /* Enable Timer and Timer Interrupt */
  putreg32(PIT_TCTRL_TIE | PIT_TCTRL_TEN, PIT_TCTRL0);
#endif

  /* Sony PowerON just to test*/
    putreg32((1 << 20), KL_GPIOE_PDDR);
    for (i = 0; i < 3; i++) {
      sendPronto(rawcode); // Sony TV power code
      //delayMicroseconds(10000);
      usleep(50000);
    }

  //enableIRIn(); // Start the receiver
}

// Storage for the recorded code
int codeType = -1; // The type of code
unsigned long codeValue; // The code value if not raw
unsigned int rawCodes[RAWBUF]; // The durations if raw
int codeLen; // The length of the code
int toggle = 0; // The RC5/6 toggle state

// Stores the code for later playback
// Most of this code is just logging
void storeCode(decode_results_t *results)
{
  int i;
  codeType = results->decode_type;
  int count = results->rawlen;
  if (codeType == UNKNOWN) {
    printf("Received unknown code, saving as raw\n");
    codeLen = results->rawlen - 1;
    // To store raw codes:
    // Drop first value (gap)
    // Convert from ticks to microseconds
    // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
    for (i = 1; i <= codeLen; i++) {
      if (i % 2) {
        // Mark
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK - MARK_EXCESS;
        printf(" m");
      } 
      else {
        // Space
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK + MARK_EXCESS;
        printf(" s");
      }
      printf("%d", rawCodes[i - 1]);
    }
    printf("");
  }
  else {
    if (codeType == NEC) {
      printf("Received NEC: ");
      if (results->value == REPEAT) {
        // Don't record a NEC repeat value as that's useless.
        printf("repeat; ignoring.\n");
        return;
      }
    } 
    else if (codeType == SONY) {
      printf("Received SONY: ");
    } 
    else if (codeType == RC5) {
      printf("Received RC5: ");
    } 
    else if (codeType == RC6) {
      printf("Received RC6: ");
    } 
    else {
      printf("Unexpected codeType ");
      printf("%d", codeType);
      printf("");
    }
    printf(results->value);
    codeValue = results->value;
    codeLen = results->bits;
  }
}

void sendCode(int repeat) {
  if (codeType == NEC) {
    if (repeat) {
      sendNEC(REPEAT, codeLen);
      printf("Sent NEC repeat\n");
    } 
    else {
      sendNEC(codeValue, codeLen);
      printf("Sent NEC ");
      printf("%X", codeValue);
    }
  } 
  else if (codeType == SONY) {
    sendSony(codeValue, codeLen);
    printf("Sent Sony ");
    printf("%X", codeValue);
  } 
  else if (codeType == RC5 || codeType == RC6) {
    if (!repeat) {
      // Flip the toggle bit for a new button press
      toggle = 1 - toggle;
    }
    // Put the toggle bit into the code to send
    codeValue = codeValue & ~(1 << (codeLen - 1));
    codeValue = codeValue | (toggle << (codeLen - 1));
    if (codeType == RC5) {
      printf("Sent RC5 ");
      printf("%X", codeValue);
      sendRC5(codeValue, codeLen);
    } 
    else {
      sendRC6(codeValue, codeLen);
      printf("Sent RC6 ");
      printf("%X\n", codeValue);
    }
  } 
  else if (codeType == UNKNOWN /* i.e. raw */) {
    // Assume 38 KHz
    sendRaw(rawCodes, codeLen, 38000);
    printf("Sent raw\n");
  }
}

int lastButtonState;

void sony_poweron(void)
{
    int i = 0;
    for (i = 0; i < 3; i++) {
      sendSony(0xa90, 12); // Sony TV power code
      usleep(10000); //delayMicroseconds(10000);
    }
}

void sony_volup(void)
{
    int i = 0;
    for (i = 0; i < 3; i++) {
      sendSony(0x490, 12); // Sony TV power code
      usleep(10000); //delayMicroseconds(10000);
    }
}

void sony_voldown(void)
{
    int i = 0;
    for (i = 0; i < 3; i++) {
      sendSony(0xc90, 12); // Sony TV power code
      usleep(10000); //delayMicroseconds(10000);
    }
}

void sony_chup(void)
{
    int i = 0;
    for (i = 0; i < 3; i++) {
      sendSony(0x090, 12); // Sony TV power code
      usleep(10000); //delayMicroseconds(10000);
    }
}

void sony_chdown(void)
{
    int i = 0;
    for (i = 0; i < 3; i++) {
      sendSony(0x890, 12); // Sony TV power code
      usleep(10000); //delayMicroseconds(10000);
    }
}

void loop() {
  // If button pressed, send the code.
  int buttonState = kl_gpioread(BUTTON_PIN);
  if (lastButtonState == HIGH && buttonState == LOW) {
    printf("Released\n");
    enableIRIn(); // Re-enable receiver
  }

  if (buttonState) {
    printf("Pressed, sending\n");
    kl_gpiowrite(STATUS_PIN, HIGH);
    sendCode(lastButtonState == buttonState);
    kl_gpiowrite(STATUS_PIN, LOW);
    usleep(50000); // Wait a bit between retransmissions
  } 
  else if (decode(&results)) {
    kl_gpiowrite(STATUS_PIN, HIGH);
    storeCode(&results);
    resume(); // resume receiver
    kl_gpiowrite(STATUS_PIN, LOW);
  }
  lastButtonState = buttonState;
}
