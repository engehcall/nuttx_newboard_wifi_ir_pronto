#include "kl_gpio.h"
#include "up_arch.h"
#include "chip/kl_tpm.h"

#include <stdint.h>
#include <nuttx/config.h>
#include <arch/board/IRremoteInt.h>

/*int MATCH(int measured, int desired) {return measured >= TICKS_LOW(desired) && measured <= TICKS_HIGH(desired);}
int MATCH_MARK(int measured_ticks, int desired_us) {return MATCH(measured_ticks, (desired_us + MARK_EXCESS));}
int MATCH_SPACE(int measured_ticks, int desired_us) {return MATCH(measured_ticks, (desired_us - MARK_EXCESS));}*/
void TIMER_CONFIG_KHZ(int freq)
{
        uint32_t regval;
        uint16_t pwmval = CORE_CLOCK / (freq * 2);

        putreg32(TPM_SC_CMOD_DIS, TPM1_SC);
        putreg32(0, TPM1_CNT);
        putreg32(pwmval, TPM1_MOD);
        putreg32(pwmval/3, TPM1_C0V);
        putreg32(TPM_CnSC_MSB | TPM_CnSC_ELSB, TPM1_C0SC);
        putreg32(TPM_SC_CPWMS, TPM1_SC);
        regval = getreg32(TPM1_SC);
        regval |= TPM_SC_PS_DIV1 | TPM_SC_CMOD_LPTPM_CLK;
        putreg32(regval, TPM1_SC);
}

void TIMER_CONFIG_NORMAL(void)
{
	uint32_t regval;

	/* stop timmer */
	regval = TPM_SC_CMOD_DIS;
	putreg32(regval, TPM1_SC);
}

