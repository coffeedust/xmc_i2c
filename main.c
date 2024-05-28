#include "DAVE.h"                 //Declarations from DAVE Code Generation (includes SFR declaration)
#include <stdio.h>
#include <stdbool.h>

int _write(int file, char *p, int len) {
	UART_Transmit(&UART_0, *p, len);
	return len;
}

uint32_t timerI2c = 0;
volatile uint8_t I2cTxCompletion = 0;
volatile uint8_t I2cRxCompletion = 0;
volatile bool bI2cNack = false;
volatile bool bI2cTimeout = false;

void callbackI2cTimeout(void *args) {
	bI2cTimeout = true;

	I2C_MASTER_AbortTransmit(&I2C_MASTER_0);
	I2C_MASTER_AbortReceive(&I2C_MASTER_0);
	I2C_MASTER_Init(&I2C_MASTER_0);
}

void enableCallbackI2cTimeout(bool enable) {
	if(enable) {
		SYSTIMER_StartTimer(timerI2c);
	}
	else {
		SYSTIMER_StopTimer(timerI2c);
	}
}

void callbackI2cTx(void) {
	I2cTxCompletion = 1;
}

void callbackI2cRx(void) {
	I2cRxCompletion = 1;
}

void I2C_NACK_IRQ_Handler(void) {
	bI2cNack = 1;
	I2C_MASTER_AbortTransmit(&I2C_MASTER_0);
	I2C_MASTER_SendStop(&I2C_MASTER_0);
	I2C_MASTER_Init(&I2C_MASTER_0);
}

void I2C_ARB_Lost_IRQ_Handler(void) {
	bI2cNack = 1;
}

void I2C_ERR_IRQ_Handler(void) {
	bI2cNack = 1;
}

bool writeI2c(uint8_t address, uint8_t *data, uint8_t length) {
	enableCallbackI2cTimeout(true);
	bI2cNack = 0;
	bI2cTimeout = 0;
	I2C_MASTER_Transmit(&I2C_MASTER_0, true, address, data, length, true);
	while((I2cTxCompletion == 0) && (bI2cTimeout == 0));
	I2cTxCompletion = 0;
	enableCallbackI2cTimeout(false);
	if(bI2cNack || bI2cTimeout) {
		bI2cNack = 0;
		bI2cTimeout = 0;
		return false;
	}
	else {
		return true;
	}
}

bool readI2c(uint8_t address, uint8_t *data, uint8_t length) {
	enableCallbackI2cTimeout(true);
	bI2cNack = 0;
	bI2cTimeout = 0;
	I2C_MASTER_Receive(&I2C_MASTER_0, true, address, data, length, true, true);
	while((I2cRxCompletion == 0) && (bI2cTimeout == 0));
	I2cRxCompletion = 0;
	enableCallbackI2cTimeout(false);
	if(bI2cNack || bI2cTimeout) {
		bI2cNack = 0;
		bI2cTimeout = 0;
		return false;
	}
	else {
		return true;
	}
}

int main(void)
{
  DAVE_STATUS_t status;

  status = DAVE_Init();           /* Initialization of DAVE APPs  */

  if (status != DAVE_STATUS_SUCCESS)
  {
    /* Placeholder for error handler code. The while loop below can be replaced with an user error handler. */
    XMC_DEBUG("DAVE APPs initialization failed\n");

    while(1U)
    {

    }
  }

  timerI2c = SYSTIMER_CreateTimer(10000, SYSTIMER_MODE_PERIODIC, callbackI2cTimeout, NULL);

  /* Placeholder for user application code. The while loop below can be replaced with user application code. */
  while(1U)
  {

  }
}
