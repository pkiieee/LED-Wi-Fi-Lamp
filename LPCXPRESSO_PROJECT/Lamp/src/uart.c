#include "lamp.h"
/*****************************************************************************
 *   uart.c:  UART API file for NXP LPC13xx Family Microprocessors
 *
 *   Copyright(C) 2008, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2008.08.21  ver 1.00    Preliminary version, first Release
 *
******************************************************************************/

// CodeRed - change for CMSIS 1.3
#define SystemFrequency SystemCoreClock

volatile uint32_t UARTStatus;
volatile uint8_t  UARTTxEmpty = 1;
volatile uint8_t  UARTBuffer[20];
volatile uint32_t UARTIndex = 0;
volatile uint8_t temp;
volatile uint8_t red = 0;
volatile uint8_t blue = 0;
volatile uint8_t green = 0;
volatile uint8_t white = 0;

/*****************************************************************************
** Function name:		UART_IRQHandler
**
** Descriptions:		UART interrupt handler
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void UART_IRQHandler(void)
{
  uint8_t IIRValue, LSRValue;
  uint8_t Dummy = Dummy;

  IIRValue = LPC_UART->IIR;
    
  IIRValue >>= 1;			/* skip pending bit in IIR */
  IIRValue &= 0x07;			/* check bit 1~3, interrupt identification */
  if (IIRValue == IIR_RLS)		/* Receive Line Status */
  {
    LSRValue = LPC_UART->LSR;
    /* Receive Line Status */
    if (LSRValue & (LSR_OE | LSR_PE | LSR_FE | LSR_RXFE | LSR_BI))
    {
      /* There are errors or break interrupt */
      /* Read LSR will clear the interrupt */
      UARTStatus = LSRValue;
      Dummy = LPC_UART->RBR;	/* Dummy read on RX to clear 
								interrupt, then bail out */
      return;
    }
    if (LSRValue & LSR_RDR)	/* Receive Data Ready */			
    {
      /* If no error on RLS, normal ready, save into the data buffer. */
      /* Note: read RBR will clear the interrupt */
    	temp =  LPC_UART->RBR;
    }
  }
  else if (IIRValue == IIR_RDA)	/* Receive Data Available */
  {
    /* Receive Data Available */
	temp =  LPC_UART->RBR;
  }
  else if (IIRValue == IIR_CTI)	/* Character timeout indicator */
  {
    /* Character Time-out indicator */
    UARTStatus |= 0x100;		/* Bit 9 as the CTI error */
  }
  else if (IIRValue == IIR_THRE)	/* THRE, transmit holding register empty */
  {
    /* THRE interrupt */
    LSRValue = LPC_UART->LSR;		/* Check status in the LSR to see if
								valid data in U0THR or not */
    if (LSRValue & LSR_THRE)
    {
      UARTTxEmpty = 1;
    }
    else
    {
      UARTTxEmpty = 0;
    }
  }
  UARTBuffer[UARTIndex++] = temp;
  // parse incoming UART data from ESP8266
  if(temp == 'R') {
	  red = UARTIndex-1;
  }
  else if(temp == 'G') {
	 green = UARTIndex-1;
  }
  else if(temp == 'B') {
	  blue = UARTIndex-1;
  }
  else if(temp == 'W') {
	  white = UARTIndex-1;
  }
  else if(temp == 'E') {
	  char redStr[3];
	  char greenStr[3];
	  char blueStr[3];
	  char whiteStr[3];
	  uint8_t i = 0;
	  uint8_t j = 0;
	  uint8_t k = 0;
	  for(i=0;i<3;i++) {
		  redStr[i] = '\0';
		  greenStr[i] = '\0';
		  blueStr[i] = '\0';
		  whiteStr[i] = '\0';
	  }
	  for(i=0;i<UARTIndex;i++) {
		  if(i == red) {
			k = 0;
			for(j=0;j<red;j++) {
				redStr[k++] = UARTBuffer[j];
			}
			uint8_t redSet = atoi(redStr);
			setRed(redSet);
		  }
		  else if(i == green) {
			  k = 0;
			  for(j=red+1;j<green;j++) {
				  greenStr[k++] = UARTBuffer[j];
			  }
			  uint8_t greenSet = atoi(greenStr);
			  setGreen(greenSet);
		  }
		  else if(i == blue) {
			  k = 0;
			  for(j=green+1;j<blue;j++) {
				  blueStr[k++] = UARTBuffer[j];
			  }
			  uint8_t blueSet = atoi(blueStr);
			  setBlue(blueSet);
		  }
		  else if(i == white) {
			  k = 0;
			  for(j=blue+1;j<white;j++) {
				  whiteStr[k++] = UARTBuffer[j];
			  }
			  uint8_t whiteSet = atoi(whiteStr);
			  setWhite(whiteSet);
		  }
	  }
	  UARTIndex = 0;
	  red = 0;
	  blue = 0;
	  green = 0;
	  white = 0;
  }



  return;
}


/*****************************************************************************
** Function name:		UARTInit
**
** Descriptions:		Initialize UART0 port, setup pin select,
**				clock, parity, stop bits, FIFO, etc.
**
** parameters:			UART baudrate
** Returned value:		None
** 
*****************************************************************************/
void UARTInit(uint32_t baudrate)
{
  uint32_t Fdiv;
  uint32_t regVal;

  UARTTxEmpty = 1;
  UARTIndex = 0;
  
  NVIC_DisableIRQ(UART_IRQn);

  LPC_IOCON->PIO1_6 &= ~0x07;    /*  UART I/O config */
  LPC_IOCON->PIO1_6 |= 0x01;     /* UART RXD */
  LPC_IOCON->PIO1_7 &= ~0x07;	
  LPC_IOCON->PIO1_7 |= 0x01;     /* UART TXD */
  /* Enable UART clock */
  LPC_SYSCON->SYSAHBCLKCTRL |= (1<<12);
  LPC_SYSCON->UARTCLKDIV = 0x1;     /* divided by 1 */

  LPC_UART->LCR = 0x83;             /* 8 bits, no Parity, 1 Stop bit */
  regVal = LPC_SYSCON->UARTCLKDIV;
  Fdiv = (((SystemFrequency/LPC_SYSCON->SYSAHBCLKDIV)/regVal)/16)/baudrate ;	/*baud rate */

  LPC_UART->DLM = Fdiv / 256;							
  LPC_UART->DLL = Fdiv % 256;
  LPC_UART->LCR = 0x03;		/* DLAB = 0 */
  LPC_UART->FCR = 0x07;		/* Enable and reset TX and RX FIFO. */

  /* Read to clear the line status. */
  regVal = LPC_UART->LSR;

  /* Ensure a clean start, no data in either TX or RX FIFO. */

  while (( LPC_UART->LSR & (LSR_THRE|LSR_TEMT)) != (LSR_THRE|LSR_TEMT) );
  while ( LPC_UART->LSR & LSR_RDR )
  {
	regVal = LPC_UART->RBR;	/* Dump data from RX FIFO */
  }
 
  /* Enable the UART Interrupt */
  NVIC_EnableIRQ(UART_IRQn);

#if TX_INTERRUPT
  LPC_UART->IER = IER_RBR | IER_THRE | IER_RLS;	/* Enable UART interrupt */
#else
  LPC_UART->IER = IER_RBR | IER_RLS;	/* Enable UART interrupt */
#endif
  return;
}

/*****************************************************************************
** Function name:		UARTSend
**
** Descriptions:		Send a block of data to the UART 0 port based
**				on the data length
**
** parameters:		buffer pointer, and data length
** Returned value:	None
** 
*****************************************************************************/
void UARTSend(uint8_t *BufferPtr, uint32_t Length)
{
  
  while ( Length != 0 )
  {
	  /* THRE status, contain valid data */
#if !TX_INTERRUPT
	  while ( !(LPC_UART->LSR & LSR_THRE) );
	  LPC_UART->THR = *BufferPtr;
#else
	  /* Below flag is set inside the interrupt handler when THRE occurs. */
      while ( !(UARTTxEmpty & 0x01) );
	  LPC_UART->THR = *BufferPtr;
      UARTTxEmpty = 0;	/* not empty in the THR until it shifts out */
#endif
      BufferPtr++;
      Length--;
  }
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
