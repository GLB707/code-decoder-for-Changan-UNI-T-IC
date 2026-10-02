#include <HardwareCAN.h>
//#include "changes.h"
/*
   Example of use of the HardwareCAN library
   This application sends two times one frame of data and then blinkes 3 times. Then repeats after 2 seconds.
   It also produces data that are sent periodically using another two frames.

   Please read the file changes.h to see the changes to be performed to the core in order to use this
*/
int input[4];
int data[10];
int id[2];
int output[4];
byte msgD0 ; // variable to be used in the example.
boolean start2;
boolean f = false;
// Instanciation of CAN interface
HardwareCAN canBus(CAN1_BASE);
CanMsg msg ;
CanMsg *r_msg;
CAN_STATUS Stat ;

void CANSetup(int m)
{
  CAN_STATUS Stat ;

  // Initialize CAN module
  canBus.map(CAN_GPIO_PB8_PB9);       // This setting is already wired in the Olimexino-STM32 board
  Stat = canBus.begin(CAN_SPEED_500, CAN_MODE_NORMAL);    // Other speeds go from 125 kbps to 1000 kbps. CAN allows even more choices.

  //canBus.filter(0, m << 21, 0xFFFFFFFF);
  canBus.filter(0, m << 21, m << 21);
  canBus.set_irq_mode();

  Stat = canBus.status();
  if (Stat != CAN_OK)
    /* Your own error processing here */ ;   // Initialization failed
}



CAN_TX_MBX CANsend(CanMsg *pmsg)
{
  CAN_TX_MBX mbx;

  do
  {
    mbx = canBus.send(pmsg) ;
#ifdef USE_MULTITASK
    vTaskDelay( 1 ) ;
#endif
  }
  while (mbx == CAN_TX_NO_MBX) ;
  return mbx ;
}


void SendCANmessage(long id = 0x001, byte dlength = 8, byte d0 = 0x00, byte d1 = 0x00, byte d2 = 0x00, byte d3 = 0x00, byte d4 = 0x00, byte d5 = 0x00, byte d6 = 0x00, byte d7 = 0x00)
{

  msg.IDE = CAN_ID_STD;
  msg.RTR = CAN_RTR_DATA;
  msg.ID = id ;
  msg.DLC = dlength;


  msg.Data[0] = d0 ;
  msg.Data[1] = d1 ;
  msg.Data[2] = d2 ;
  msg.Data[3] = d3 ;
  msg.Data[4] = d4 ;
  msg.Data[5] = d5 ;
  msg.Data[6] = d6 ;
  msg.Data[7] = d7 ;
  CANsend(&msg) ;      // Send this frame

}

void su (int n1, int n2, int n3, int n4) {

  input[0] = n1;
  input[1] = n2;
  input[2] = n3;
  input[3] = n4;
  //1 байт///////////////////////////////////////////////
  int dop1bayt[10];
  dop1bayt[0] = input[0] / 16;
  dop1bayt[1] = input[0] - (dop1bayt[0] * 16);

  if (dop1bayt[1]<4 and dop1bayt[1] >= 0) {
    output[0] = 0xE9 - 0x10 * dop1bayt[0];
    output[0] = output[0] + dop1bayt[1];
    if (output[0] < 0) {
      output[0] = 0xFF - abs(output[0]) + 1;
    }
  }

  else if (dop1bayt[1]<8 and dop1bayt[1] >= 4) {
    output[0] = 0xE5 - 0x10 * dop1bayt[0];
    output[0] = output[0] + dop1bayt[1] - 4;
    if (output[0] < 0) {
      output[0] = 0xFF - abs(output[0]) + 1;
    }
  }

  else if (dop1bayt[1]<0xC and dop1bayt[1] >= 8) {
    output[0] = 0xF1 - 0x10 * dop1bayt[0];
    output[0] = output[0] + dop1bayt[1] - 8;
  }

  else if (dop1bayt[1] <= 0xF and dop1bayt[1] >= 0xC) {
    output[0] = 0xED - 0x10 * dop1bayt[0];
    if (output[0] < 0) {
      output[0] = 0xFF - abs(output[0]) + 1;
    }
    output[0] = output[0] + dop1bayt[1] - 0xC;

  }
  //////////////////////////////////////////////////////



  //4байт///////////////////////////////////////////////
  int dop4bayt[10];

  dop4bayt[0] = input[3] / 16;
  dop4bayt[1] = input[3] - (dop4bayt[0] * 16);

  if (dop4bayt[1]<8 and dop4bayt[1] >= 0) {
    if (dop4bayt[0] % 2 == 0) {

      output[3] = 0x10 * dop4bayt[0] + 0x20;
      output[3] = output[3] + 0xE - dop4bayt[1];
    }
    else {
      output[3] = 0x10 * dop4bayt[0];
      output[3] = output[3] + 0xE - dop4bayt[1];
    }

    if (output[3] < 0) {
      output[3] = 0xFF - abs(output[3]) + 1;
    }
    if (output[3] > 0xFF) {
      output[3] = output[3] - 0xFF - 1;
    }
  }

  else if (dop4bayt[1]<0xF and dop4bayt[1] > 7) {

    if (dop4bayt[0] % 2 == 0) {

      output[3] = 0x10 * dop4bayt[0] + 0x20;
      output[3] = output[3] + 0x16 - dop4bayt[1] + 8;
    }
    else {
      output[3] = 0x10 * dop4bayt[0];
      output[3] = output[3] + 0x16 - dop4bayt[1] + 8;
    }

    if (output[3] < 0) {
      output[3] = 0xFF - abs(output[3]) + 1;
    }
    if (output[3] > 0xFF) {
      output[3] = output[3] - 0xFF - 1;
    }
  }
  //////////////////////////////////////////////////////


  //влияние 1 байта на 4/////////////////////////////////////////////////

  if (dop1bayt[0] < 1) {
    switch (dop1bayt[1]) {
      case 1: output[3] = output[3] + 0x80; break;
      case 2: output[3] = output[3] + 0x80 - 0x40; break;
      case 3: output[3] = output[3] + 0x80 - 0x40 + 0x80; break;
      case 4: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0; break;
      case 5: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80; break;
      case 6: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40; break;
      case 7: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80; break;
      case 8: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10; break;
      case 9: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80; break;
      case 10: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40; break;
      case 11: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40 + 0x80; break;
      case 12: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40 + 0x80 - 0xA0; break;
      case 13: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80; break;
      case 14: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40; break;
      case 15: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 + 0x80; break;
    }
    while (output[3] < 0) {
      output[3] = 0xFF - abs(output[3]) + 1;
    }
    while (output[3] > 0xFF) {
      output[3] = output[3] - 0xFF - 1;
    }

  }

  else {

    for (int i = 0; i < dop1bayt[0]; i++) {
      output[3] = output[3] + 208;

      while (output[3] < 0) {
        output[3] = 0xFF - abs(output[3]) + 1;
      }
      while (output[3] > 0xFF) {
        output[3] = output[3] - 0xFF - 1;
      }
    }

    switch (dop1bayt[1]) {
      case 1: output[3] = output[3] + 0x80; break;
      case 2: output[3] = output[3] + 0x80 - 0x40; break;
      case 3: output[3] = output[3] + 0x80 - 0x40 + 0x80; break;
      case 4: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0; break;
      case 5: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80; break;
      case 6: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40; break;
      case 7: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80; break;
      case 8: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10; break;
      case 9: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80; break;
      case 10: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40; break;
      case 11: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40 + 0x80; break;
      case 12: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40 + 0x80 - 0xA0; break;
      case 13: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80; break;
      case 14: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40; break;
      case 15: output[3] = output[3] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 + 0x80; break;
    }
    while (output[3] < 0) {
      output[3] = 0xFF - abs(output[3]) + 1;
    }
    while (output[3] > 0xFF) {
      output[3] = output[3] - 0xFF - 1;
    }


    switch (dop1bayt[0]) {
      case 1: output[3] = output[3] - 200; break;
      case 2: output[3] = output[3] - 0xC8 + 0x24; break;
      case 3: output[3] = output[3] - 0xC8 + 0x24 - 0xC8; break;
      case 4: output[3] = output[3] - 0xC8 + 0x24 - 0xC8 + 0x2A; break;
      case 5: output[3] = output[3] - 0xC8 + 0x24 - 0xC8 + 0x2A - 0xC8; break;
      case 6: output[3] = output[3] - 0xC8 + 0x24 - 0xC8 + 0x2A - 0xC8 + 0x24; break;
      case 7: output[3] = output[3] - 0xC8 + 0x24 - 0xC8 + 0x2A - 0xC8 + 0x24 - 0xC8; break;
      case 8: output[3] = output[3] - 0xC8 + 0x24 - 0xC8 + 0x2A - 0xC8 + 0x24 - 0xC8 + 0x2D; break;
      case 9: output[3] = output[3] - 0xC8 + 0x24 - 0xC8 + 0x2A - 0xC8 + 0x24 - 0xC8 + 0x2D - 0xC8; break;
      case 10: output[3] = output[3] - 0xC8 + 0x24 - 0xC8 + 0x2A - 0xC8 + 0x24 - 0xC8 + 0x2D - 0xC8 + 0x24; break;
      case 11: output[3] = output[3] - 0xC8 + 0x24 - 0xC8 + 0x2A - 0xC8 + 0x24 - 0xC8 + 0x2D - 0xC8 + 0x24 - 0xC8; break;
      case 12: output[3] = output[3] - 0xC8 + 0x24 - 0xC8 + 0x2A - 0xC8 + 0x24 - 0xC8 + 0x2D - 0xC8 + 0x24 - 0xC8 + 0x2A; break;
      case 13: output[3] = output[3] - 0xC8 + 0x24 - 0xC8 + 0x2A - 0xC8 + 0x24 - 0xC8 + 0x2D - 0xC8 + 0x24 - 0xC8 + 0x2A - 0xC8; break;
      case 14: output[3] = output[3] - 0xC8 + 0x24 - 0xC8 + 0x2A - 0xC8 + 0x24 - 0xC8 + 0x2D - 0xC8 + 0x24 - 0xC8 + 0x2A - 0xC8 + 0x24; break;
      case 15: output[3] = output[3] - 0xC8 + 0x24 - 0xC8 + 0x2A - 0xC8 + 0x24 - 0xC8 + 0x2D - 0xC8 + 0x24 - 0xC8 + 0x2A - 0xC8 + 0x24 - 0xC8; break;
    }
    while (output[3] < 0) {
      output[3] = 0xFF - abs(output[3]) + 1;
    }
    while (output[3] > 0xFF) {
      output[3] = output[3] - 0xFF - 1;
    }


  }


  ///////////////////////////////////////////////////////////////////////









  ///////////////////////////  ВЛИЯНИЕ 4 БАЙТА НА 1 ////////////////////////////////////////////

  if (dop4bayt[0] < 1) {
    switch (dop4bayt[1]) {
      case 1: output[0] = output[0] - 0x80; break;
      case 2: output[0] = output[0] - 0x80 + 0x40; break;
      case 3: output[0] = output[0] - 0x80 + 0x40 - 0x80; break;
      case 4: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0; break;
      case 5: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80; break;
      case 6: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40; break;
      case 7: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80; break;
      case 8: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0; break;
      case 9: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80; break;
      case 10: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40; break;
      case 11: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40 - 0x80; break;
      case 12: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40 - 0x80 + 0xA0; break;
      case 13: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80; break;
      case 14: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40; break;
      case 15: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80; break;
    }
    while (output[0] < 0) {
      output[0] = 0xFF - abs(output[0]) + 1;
    }
    while (output[0] > 0xFF) {
      output[0] = output[0] - 0xFF - 1;
    }
  }

  else {

    for (int i = 0; i < dop4bayt[0]; i++) {
      output[0] = output[0] - 240;

      while (output[0] < 0) {
        output[0] = 0xFF - abs(output[0]) + 1;
      }
      while (output[0] > 0xFF) {
        output[0] = output[0] - 0xFF - 1;
      }
    }

    switch (dop4bayt[1]) {
      case 1: output[0] = output[0] - 0x80; break;
      case 2: output[0] = output[0] - 0x80 + 0x40; break;
      case 3: output[0] = output[0] - 0x80 + 0x40 - 0x80; break;
      case 4: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0; break;
      case 5: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80; break;
      case 6: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40; break;
      case 7: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80; break;
      case 8: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0; break;
      case 9: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80; break;
      case 10: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40; break;
      case 11: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40 - 0x80; break;
      case 12: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40 - 0x80 + 0xA0; break;
      case 13: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80; break;
      case 14: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40; break;
      case 15: output[0] = output[0] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80; break;
    }
    while (output[0] < 0) {
      output[0] = 0xFF - abs(output[0]) + 1;
    }
    while (output[0] > 0xFF) {
      output[0] = output[0] - 0xFF - 1;
    }


    switch (dop4bayt[0]) {
      case 1: output[0] = output[0] - 0x8; break;
      case 2: output[0] = output[0] - 0x8 + 0xE4; break;
      case 3: output[0] = output[0] - 0x8 + 0xE4 - 0x8; break;
      case 4: output[0] = output[0] - 0x8 + 0xE4 - 0x8 + 0xEE; break;
      case 5: output[0] = output[0] - 0x8 + 0xE4 - 0x8 + 0xEE - 0x8; break;
      case 6: output[0] = output[0] - 0x8 + 0xE4 - 0x8 + 0xEE - 0x8 + 0xE4; break;
      case 7: output[0] = output[0] - 0x8 + 0xE4 - 0x8 + 0xEE - 0x8 + 0xE4 - 0x8; break;
      case 8: output[0] = output[0] - 0x8 + 0xE4 - 0x8 + 0xEE - 0x8 + 0xE4 - 0x8 + 0xEB; break;
      case 9: output[0] = output[0] - 0x8 + 0xE4 - 0x8 + 0xEE - 0x8 + 0xE4 - 0x8 + 0xEB - 0x8; break;
      case 10: output[0] = output[0] - 0x8 + 0xE4 - 0x8 + 0xEE - 0x8 + 0xE4 - 0x8 + 0xEB - 0x8 + 0xE4; break;
      case 11: output[0] = output[0] - 0x8 + 0xE4 - 0x8 + 0xEE - 0x8 + 0xE4 - 0x8 + 0xEB - 0x8 + 0xE4 - 0x8; break;
      case 12: output[0] = output[0] - 0x8 + 0xE4 - 0x8 + 0xEE - 0x8 + 0xE4 - 0x8 + 0xEB - 0x8 + 0xE4 - 0x8 + 0xEE; break;
      case 13: output[0] = output[0] - 0x8 + 0xE4 - 0x8 + 0xEE - 0x8 + 0xE4 - 0x8 + 0xEB - 0x8 + 0xE4 - 0x8 + 0xEE - 0x8; break;
      case 14: output[0] = output[0] - 0x8 + 0xE4 - 0x8 + 0xEE - 0x8 + 0xE4 - 0x8 + 0xEB - 0x8 + 0xE4 - 0x8 + 0xEE - 0x8 + 0xE4; break;
      case 15: output[0] = output[0] - 0x8 + 0xE4 - 0x8 + 0xEE - 0x8 + 0xE4 - 0x8 + 0xEB - 0x8 + 0xE4 - 0x8 + 0xEE - 0x8 + 0xE4 - 0x8; break;
    }
    if (output[0] < 0) {
      output[0] = 0xFF - abs(output[0]) + 1;
    }
    while (output[0] > 0xFF) {
      output[0] = output[0] - 0xFF - 1;


    }
  }
  //////////////////////////////////////////////////////////////////////////////////////////////

  ////////////////////////////////// 2 БАЙТ //////////////////////////////////////////////////////
  int dop2bayt[10];

  dop2bayt[0] = input[1] / 16;
  dop2bayt[1] = input[1] - (dop2bayt[0] * 16);

  if (dop2bayt[1]<0x07 and dop2bayt[1] >= 0) {
    output[1] = 0xEE - 0x10 * dop2bayt[0];
    output[1] = output[1] - dop2bayt[1];

    while (output[1] < 0) {
      output[1] = 0xFF - abs(output[1]) + 1;
    }
    while (output[1] > 0xFF) {
      output[1] = output[1] - 0xFF - 1;
    }
  }

  else if (dop2bayt[1] <= 0x0F and dop2bayt[1] >= 0x08) {
    output[1] = 0xF6 - 0x10 * dop2bayt[0];
    output[1] = output[1] - dop2bayt[1] + 8;

    while (output[1] < 0) {
      output[1] = 0xFF - abs(output[1]) + 1;
    }
    while (output[1] > 0xFF) {
      output[1] = output[1] - 0xFF - 1;
    }
  }

  if (dop2bayt[1] == 0x07)
  {
    output[1] = 0xE8 - 0x10 * dop2bayt[0];

    while (output[1] < 0) {
      output[1] = 0xFF - abs(output[1]) + 1;
    }
    while (output[1] > 0xFF) {
      output[1] = output[1] - 0xFF - 1;
    }
  }
  /////////////////////////////////////////////////////////////////////////////////////////////


  //////////// 3 бАЙТ ///////////////////////////////////////////////////////////////////////

  int dop3bayt[10];

  dop3bayt[0] = input[2] / 16;
  dop3bayt[1] = input[2] - (dop3bayt[0] * 16);
  if (dop3bayt[1] == 0 or dop3bayt[1] == 1) {
    if (dop3bayt[0] % 2 == 0) {

      output[2] = 0x10 * dop3bayt[0] + 0x20;
      output[2] = output[2] + 0xA - dop3bayt[1];
    }
    else {
      output[2] = 0x10 * dop3bayt[0];
      output[2] = output[2] + 0xA - dop3bayt[1];
    }

    while (output[2] < 0) {
      output[2] = 0xFF - abs(output[2]) + 1;
    }
    while (output[2] > 0xFF) {
      output[2] = output[2] - 0xFF - 1;
    }
  }



  else if (dop3bayt[1] == 2 or dop3bayt[1] == 3) {
    if (dop3bayt[0] % 2 == 0) {

      output[2] = 0x10 * dop3bayt[0] + 0x20;
      output[2] = output[2] + 0xC - dop3bayt[1] + 2;
    }
    else {
      output[2] = 0x10 * dop3bayt[0];
      output[2] = output[2] + 0xC - dop3bayt[1] + 2;
    }

    while (output[2] < 0) {
      output[2] = 0xFF - abs(output[2]) + 1;
    }
    while (output[2] > 0xFF) {
      output[2] = output[2] - 0xFF - 1;
    }
  }



  else if (dop3bayt[1] == 4 or dop3bayt[1] == 5) {
    if (dop3bayt[0] % 2 == 0) {

      output[2] = 0x10 * dop3bayt[0] + 0x20;
      output[2] = output[2] + 0x6 - dop3bayt[1] + 4;
    }
    else {
      output[2] = 0x10 * dop3bayt[0];
      output[2] = output[2] + 0x6 - dop3bayt[1] + 4;
    }

    while (output[2] < 0) {
      output[2] = 0xFF - abs(output[2]) + 1;
    }
    while (output[2] > 0xFF) {
      output[2] = output[2] - 0xFF - 1;
    }
  }

  else if (dop3bayt[1] == 6 or dop3bayt[1] == 7) {
    if (dop3bayt[0] % 2 == 0) {

      output[2] = 0x10 * dop3bayt[0] + 0x20;
      output[2] = output[2] + 0x8 - dop3bayt[1] + 6;
    }
    else {
      output[2] = 0x10 * dop3bayt[0];
      output[2] = output[2] + 0x8 - dop3bayt[1] + 6;
    }

    while (output[2] < 0) {
      output[2] = 0xFF - abs(output[2]) + 1;
    }
    while (output[2] > 0xFF) {
      output[2] = output[2] - 0xFF - 1;
    }
  }

  else if (dop3bayt[1] == 8 or dop3bayt[1] == 9) {
    if (dop3bayt[0] % 2 == 0) {

      output[2] = 0x10 * dop3bayt[0] + 0x20;
      output[2] = output[2] + 0x12 - dop3bayt[1] + 8;
    }
    else {
      output[2] = 0x10 * dop3bayt[0];
      output[2] = output[2] + 0x12 - dop3bayt[1] + 8;
    }

    while (output[2] < 0) {
      output[2] = 0xFF - abs(output[2]) + 1;
    }
    while (output[2] > 0xFF) {
      output[2] = output[2] - 0xFF - 1;
    }
  }


  else if (dop3bayt[1] == 0xA or dop3bayt[1] == 0xB) {
    if (dop3bayt[0] % 2 == 0) {

      output[2] = 0x10 * dop3bayt[0] + 0x20;
      output[2] = output[2] + 0x14 - dop3bayt[1] + 0xA;
    }
    else {
      output[2] = 0x10 * dop3bayt[0];
      output[2] = output[2] + 0x14 - dop3bayt[1] + 0xA;
    }

    while (output[2] < 0) {
      output[2] = 0xFF - abs(output[2]) + 1;
    }
    while (output[2] > 0xFF) {
      output[2] = output[2] - 0xFF - 1;
    }
  }

  else if (dop3bayt[1] == 0xC or dop3bayt[1] == 0xD) {
    if (dop3bayt[0] % 2 == 0) {

      output[2] = 0x10 * dop3bayt[0] + 0x20;
      output[2] = output[2] + 0xE - dop3bayt[1] + 0xC;
    }
    else {
      output[2] = 0x10 * dop3bayt[0];
      output[2] = output[2] + 0xE - dop3bayt[1] + 0xC;
    }

    while (output[2] < 0) {
      output[2] = 0xFF - abs(output[2]) + 1;
    }
    while (output[2] > 0xFF) {
      output[2] = output[2] - 0xFF - 1;
    }
  }


  else if (dop3bayt[1] == 0xE or dop3bayt[1] == 0xF) {
    if (dop3bayt[0] % 2 == 0) {

      output[2] = 0x10 * dop3bayt[0] + 0x20;
      output[2] = output[2] + 0x10 - dop3bayt[1] + 0xE;
    }
    else {
      output[2] = 0x10 * dop3bayt[0];
      output[2] = output[2] + 0x10 - dop3bayt[1] + 0xE;
    }

    while (output[2] < 0) {
      output[2] = 0xFF - abs(output[2]) + 1;
    }
    while (output[2] > 0xFF) {
      output[2] = output[2] - 0xFF - 1;
    }
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////




  //////// ВЛИЯНИЕ 2 БАЙТА НА 3 ////////////////////////////////////////////////////////

  if (dop2bayt[0] < 1) {
    switch (dop2bayt[1]) {
      case 1: output[2] = output[2] + 0x80; break;
      case 2: output[2] = output[2] + 0x80 - 0x40; break;
      case 3: output[2] = output[2] + 0x80 - 0x40 + 0x80; break;
      case 4: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0; break;
      case 5: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80; break;
      case 6: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40; break;
      case 7: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80; break;
      case 8: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10; break;
      case 9: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80; break;
      case 10: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40; break;
      case 11: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40 + 0x80; break;
      case 12: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40 + 0x80 - 0xA0; break;
      case 13: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80; break;
      case 14: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40; break;
      case 15: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 + 0x80; break;
    }

    while (output[2] < 0) {
      output[2] = 0xFF - abs(output[2]) + 1;
    }
    while (output[2] > 0xFF) {
      output[2] = output[2] - 0xFF - 1;
    }
  }

  else {

    for (int i = 0; i < dop2bayt[0]; i++) {
      output[2] = output[2] + 208;

      while (output[2] < 0) {
        output[2] = 0xFF - abs(output[2]) + 1;
      }
      while (output[2] > 0xFF) {
        output[2] = output[2] - 0xFF - 1;
      }
    }

    switch (dop2bayt[1]) {
      case 1: output[2] = output[2] + 0x80; break;
      case 2: output[2] = output[2] + 0x80 - 0x40; break;
      case 3: output[2] = output[2] + 0x80 - 0x40 + 0x80; break;
      case 4: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0; break;
      case 5: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80; break;
      case 6: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40; break;
      case 7: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80; break;
      case 8: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10; break;
      case 9: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80; break;
      case 10: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40; break;
      case 11: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40 + 0x80; break;
      case 12: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40 + 0x80 - 0xA0; break;
      case 13: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80; break;
      case 14: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40; break;
      case 15: output[2] = output[2] + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 - 0x80 + 0x10 + 0x80 - 0x40 + 0x80 - 0xA0 + 0x80 - 0x40 + 0x80; break;

    }

    while (output[2] < 0) {
      output[2] = 0xFF - abs(output[2]) + 1;
    }
    while (output[2] > 0xFF) {
      output[2] = output[2] - 0xFF - 1;
    }

    switch (dop2bayt[0]) {
      case 1: output[2] = output[2] - 200; break;
      case 2: output[2] = output[2] - 0xC8 + 0x24; break;
      case 3: output[2] = output[2] - 0xC8 + 0x24 - 0xC8; break;
      case 4: output[2] = output[2] - 0xC8 + 0x24 - 0xC8 + 0x2E; break;
      case 5: output[2] = output[2] - 0xC8 + 0x24 - 0xC8 + 0x2E - 0xC8; break;
      case 6: output[2] = output[2] - 0xC8 + 0x24 - 0xC8 + 0x2E - 0xC8 + 0x24; break;
      case 7: output[2] = output[2] - 0xC8 + 0x24 - 0xC8 + 0x2E - 0xC8 + 0x24 - 0xC8; break;
      case 8: output[2] = output[2] - 0xC8 + 0x24 - 0xC8 + 0x2E - 0xC8 + 0x24 - 0xC8 + 0x29; break;
      case 9: output[2] = output[2] - 0xC8 + 0x24 - 0xC8 + 0x2E - 0xC8 + 0x24 - 0xC8 + 0x29 - 0xC8; break;
      case 10: output[2] = output[2] - 0xC8 + 0x24 - 0xC8 + 0x2E - 0xC8 + 0x24 - 0xC8 + 0x29 - 0xC8 + 0x24; break;
      case 11: output[2] = output[2] - 0xC8 + 0x24 - 0xC8 + 0x2E - 0xC8 + 0x24 - 0xC8 + 0x29 - 0xC8 + 0x24 - 0xC8; break;
      case 12: output[2] = output[2] - 0xC8 + 0x24 - 0xC8 + 0x2E - 0xC8 + 0x24 - 0xC8 + 0x29 - 0xC8 + 0x24 - 0xC8 + 0x2A; break;
      case 13: output[2] = output[2] - 0xC8 + 0x24 - 0xC8 + 0x2E - 0xC8 + 0x24 - 0xC8 + 0x29 - 0xC8 + 0x24 - 0xC8 + 0x2A - 0xC8; break;
      case 14: output[2] = output[2] - 0xC8 + 0x24 - 0xC8 + 0x2E - 0xC8 + 0x24 - 0xC8 + 0x29 - 0xC8 + 0x24 - 0xC8 + 0x2A - 0xC8 + 0x24; break;
      case 15: output[2] = output[2] - 0xC8 + 0x84 - 0xC8 + 0x2E - 0xC8 + 0x24 - 0xC8 + 0x29 - 0xC8 + 0x24 - 0xC8 + 0x2A - 0xC8 + 0x24 - 0xC8; break;
    }

    while (output[2] < 0) {
      output[2] = 0xFF - abs(output[2]) + 1;
    }
    while (output[2] > 0xFF) {
      output[2] = output[2] - 0xFF - 1;
    }

  }

  //////////////////////////////////////////////////////////////////////////////////////


  //////////////// ВЛИЯНИЕ 3 БАЙТА НА 2 ///////////////////////////////////////////////////
  if (dop3bayt[0] < 1) {
    switch (dop3bayt[1]) {
      case 1: output[1] = output[1] - 0x80; break;
      case 2: output[1] = output[1] - 0x80 + 0x40; break;
      case 3: output[1] = output[1] - 0x80 + 0x40 - 0x80; break;
      case 4: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0; break;
      case 5: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80; break;
      case 6: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40; break;
      case 7: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80; break;
      case 8: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0; break;
      case 9: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80; break;
      case 10: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40; break;
      case 11: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40 - 0x80; break;
      case 12: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40 - 0x80 + 0xA0; break;
      case 13: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80; break;
      case 14: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40; break;
      case 15: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80; break;
    }
    while (output[1] < 0) {
      output[1] = 0xFF - abs(output[1]) + 1;
    }
    while (output[1] > 0xFF) {
      output[1] = output[1] - 0xFF - 1;
    }
  }

  else {

    for (int i = 0; i < dop3bayt[0]; i++) {
      output[1] = output[1] - 240;

      while (output[1] < 0) {
        output[1] = 0xFF - abs(output[1]) + 1;
      }
      while (output[1] > 0xFF) {
        output[1] = output[1] - 0xFF - 1;
      }
    }

    switch (dop3bayt[1]) {
      case 1: output[1] = output[1] - 0x80; break;
      case 2: output[1] = output[1] - 0x80 + 0x40; break;
      case 3: output[1] = output[1] - 0x80 + 0x40 - 0x80; break;
      case 4: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0; break;
      case 5: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80; break;
      case 6: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40; break;
      case 7: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80; break;
      case 8: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0; break;
      case 9: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80; break;
      case 10: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40; break;
      case 11: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40 - 0x80; break;
      case 12: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40 - 0x80 + 0xA0; break;
      case 13: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80; break;
      case 14: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40; break;
      case 15: output[1] = output[1] - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80 + 0xD0 - 0x80 + 0x40 - 0x80 + 0xA0 - 0x80 + 0x40 - 0x80; break;
    }
    while (output[1] < 0) {
      output[1] = 0xFF - abs(output[1]) + 1;
    }
    while (output[1] > 0xFF) {
      output[1] = output[1] - 0xFF - 1;
    }


    switch (dop3bayt[0]) {
      case 1: output[1] = output[1] - 0x8; break;
      case 2: output[1] = output[1] - 0x8 + 0xE4; break;
      case 3: output[1] = output[1] - 0x8 + 0xE4 - 0x8; break;
      case 4: output[1] = output[1] - 0x8 + 0xE4 - 0x8 + 0xEB; break;
      case 5: output[1] = output[1] - 0x8 + 0xE4 - 0x8 + 0xEB - 0x8; break;
      case 6: output[1] = output[1] - 0x8 + 0xE4 - 0x8 + 0xEB - 0x8 + 0xE4; break;
      case 7: output[1] = output[1] - 0x8 + 0xE4 - 0x8 + 0xEB - 0x8 + 0xE4 - 0x8; break;
      case 8: output[1] = output[1] - 0x8 + 0xE4 - 0x8 + 0xEB - 0x8 + 0xE4 - 0x8 + 0xED; break;
      case 9: output[1] = output[1] - 0x8 + 0xE4 - 0x8 + 0xEB - 0x8 + 0xE4 - 0x8 + 0xED - 0x8; break;
      case 10: output[1] = output[1] - 0x8 + 0xE4 - 0x8 + 0xEB - 0x8 + 0xE4 - 0x8 + 0xED - 0x8 + 0xE4; break;
      case 11: output[1] = output[1] - 0x8 + 0xE4 - 0x8 + 0xEB - 0x8 + 0xE4 - 0x8 + 0xED - 0x8 + 0xE4 - 0x8; break;
      case 12: output[1] = output[1] - 0x8 + 0xE4 - 0x8 + 0xEB - 0x8 + 0xE4 - 0x8 + 0xED - 0x8 + 0xE4 - 0x8 + 0xEA; break;
      case 13: output[1] = output[1] - 0x8 + 0xE4 - 0x8 + 0xEB - 0x8 + 0xE4 - 0x8 + 0xED - 0x8 + 0xE4 - 0x8 + 0xEA - 0x8; break;
      case 14: output[1] = output[1] - 0x8 + 0xE4 - 0x8 + 0xEB - 0x8 + 0xE4 - 0x8 + 0xED - 0x8 + 0xE4 - 0x8 + 0xEA - 0x8 + 0xE5; break;
      case 15: output[1] = output[1] - 0x8 + 0xE4 - 0x8 + 0xEB - 0x8 + 0xE4 - 0x8 + 0xED - 0x8 + 0xE4 - 0x8 + 0xEA - 0x8 + 0xE5 - 0x9; break;
    }
    if (output[1] < 0) {
      output[1] = 0xFF - abs(output[1]) + 1;
    }
    while (output[1] > 0xFF) {
      output[1] = output[1] - 0xFF - 1;


    }
  }



  /////////////////////////////////////////////////////////////////////////////////////////




  ////////////// ПРИМЕЧАНИЯ //////////////////////////////////////////////////////////////////

  if (input [1] == 0x3F) {
    output[1]++;
  }
  if (dop1bayt[1] == 7) {
    output[2]++;
  }
  /*if ((input [1]>=0x81 and input [2]>=0x80)==false){
    output[1]--;
    }*/

  if (input[1] == 0x81 or input[1] == 0x83 or input[1] == 0x85 or input[1] == 0x86 or input[1] == 0x89 or input[1] == 0x8B or input[1] == 0x8D ) {
    //output[1] = output[1] + 2;
  }
  else if (input[1] == 0x80 or input[1] == 0x82 or input[1] == 0x84 or input[1] == 0x87 or input[1] == 0x88 or input[1] == 0x8A or input[1] == 0x8C or input[1] == 0x8E or input[1] == 0x8F ) {
    // output[1] = output[1] + 2;
  }
  else if (input[1] == 0x91 or input[1] == 0x93 or input[1] == 0x95 or input[1] == 0x99 or input[1] == 0x9B or input[1] == 0x9D or input[1] == 0x9F ) {
    //output[1] = output[1] + 2;
  }
  else if (input[1] == 0x90 or input[1] == 0x92 or input[1] == 0x94 or input[1] == 0x96 or input[1] == 0x97 or input[1] == 0x98 or input[1] == 0x9A or input[1] == 0x9C or input[1] == 0x9E) {
    //output[1] = output[1] + 1;
  }
  else if (input[1] == 0xA1 or input[1] == 0xA2 or input[1] == 0xA3 or input[1] == 0xA5 or input[1] == 0xA6 or input[1] == 0xA9 or input[1] == 0xAB or input[1] == 0xAD or input[1] == 0xAE or input[1] == 0xAF ) {
    //output[1] = output[1] + 2;
  }
  else if (input[1] == 0xA0 or input[1] == 0xA4 or input[1] == 0xA7 or input[1] == 0xA8 or input[1] == 0xAA or input[1] == 0xAC) {
    //output[1] = output[1] + 1;
  }
  else if (input[1] == 0xB0 or input[1] == 0xB2 or input[1] == 0xB4 or input[1] == 0xB7 or input[1] == 0xB8 or input[1] == 0xBA or input[1] == 0xBC ) {
    //output[1] = output[1] - 1;
  }
  else if (input [1] == 0xC0  or input [1] == 0xC8 or input[1] == 0xCC or input[1] == 0xC7 ) {
    //output[1] = output[1] - 5;
    output[2] = output[2] + 4;
  }
  else if (input[1 ] == 0xC1 or input[1] == 0xC2 or input[1] == 0xC3 or input[1] == 0xC4 or input[1] == 0xC5 or input[1] == 0xC6 or input[1] == 0xC9 or input[1] == 0xCA or input[1] == 0xCB or input[1] == 0xCD or input[1] == 0xCe or input[1] == 0xCF ) {
    // output[1] = output[1] - 4;
    output[2] = output[2] + 4;
  }
  else if (input [1] == 0xD0  or input [1] == 0xD8 or input[1] == 0xDC or input[1] == 0xD7  or input[1] == 0xD4  ) {
    // output[1] = output[1] - 5;
    output[2] = output[2] + 4;
  }
  else if (input[1 ] == 0xD1 or input[1] == 0xD2 or input[1] == 0xD3 or input[1] == 0xD5 or input[1] == 0xD6 or input[1] == 0xD9 or input[1] == 0xDA or input[1] == 0xDB or input[1] == 0xDD or input[1] == 0xDe or input[1] == 0xDF ) {
    //output[1] = output[1] - 4;
    output[2] = output[2] + 4;
  }
  else if (input [1] == 0xE8  or input[1] == 0xE7) {
    // output[1] = output[1] - 5;
    output[2] = output[2] + 4;
  }
  else if (input [1] == 0xE0  or input[1 ] == 0xE1 or input[1] == 0xE2 or input[1] == 0xE3 or input[1] == 0xE4  or input[1] == 0xE5 or input[1] == 0xE6 or input[1] == 0xE9 or input[1] == 0xEA or input[1] == 0xEB or input[1] == 0xEC or input[1] == 0xED or input[1] == 0xEE or input[1] == 0xEF ) {
    //output[1] = output[1] - 4;
    output[2] = output[2] + 4;
  }

  if (input[0] + input[3] >= 0xF0 and dop1bayt[1] != 0 and dop1bayt[0] != 0xD) {
    output[2]++;
  }

  if (dop1bayt[1] == 0xD and input[3] >= 0x60) {
    output[2]++;
  }

  else if (input[3] == 0xDF) {
    output[2]++;
  }

 if (input[1]>=0x80 and dop3bayt[1] % 2 == 1){
  output[0]--;
}

else if (dop3bayt[1]==0xF){
 output[0]--;
}
else if (dop2bayt[1]==0xF){
 output[0]--;
}

  while (output[1] < 0) {
    output[1] = 0xFF - abs(output[1]) + 1;
  }
  while (output[1] > 0xFF) {
    output[1] = output[1] - 0xFF - 1;
  }
  while (output[2] < 0) {
    output[2] = 0xFF - abs(output[2]) + 1;
  }
  while (output[2] > 0xFF) {
    output[2] = output[2] - 0xFF - 1;
  }
}
void setup() {
  //delay(7000);
  CANSetup(0x700) ;
  pinMode(PC13, OUTPUT);
  start2 = false;

}
void start1() {
  CANSetup(0x700);
  // if (start2 == false) {
  start2 = true;
  canBus.free();
  SendCANmessage(0x701, 8, 0x02 , 0X10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 ) ;
  delay(15);
  canBus.free();
  SendCANmessage(0x701, 8, 0x02 , 0X10, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00 ) ;
  delay(15);
  canBus.free();
  SendCANmessage(0x701, 8, 0x02 , 0X27, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 ) ;

  delay(20);
  if ( ( r_msg = canBus.recv() ) != NULL )
  {

    id[0] = (r_msg->ID);
    data[0] = (r_msg->Data[0]);
    data[1] = (r_msg->Data[1]);
    data[2] = (r_msg->Data[2]);
    data[3] = (r_msg->Data[3]);
    data[4] = (r_msg->Data[4]);
    data[5] = (r_msg->Data[5]);
    data[6] = (r_msg->Data[6]);
    data[7] = (r_msg->Data[7]);
    canBus.free();
  }

  su(data[3], data[4], data[5], data[6]);

  delay (20);
  canBus.free();
  SendCANmessage(0x701, 8, 0x06 , 0X27, 0x02, output[0], output[1], output[2], output[3], 0x00 ) ;
  delay(30);
  //canBus.free();
  if ( ( r_msg = canBus.recv() ) != NULL )
  {

    id[0] = (r_msg->ID);
    data[0] = (r_msg->Data[0]);
    data[1] = (r_msg->Data[1]);
    data[2] = (r_msg->Data[2]);
    data[3] = (r_msg->Data[3]);
    data[4] = (r_msg->Data[4]);
    data[5] = (r_msg->Data[5]);
    data[6] = (r_msg->Data[6]);
    data[7] = (r_msg->Data[7]);

  }

  if (id[0] == 0x709 and data[0] == 2 and data[1] == 0x67 and data[2] == 0x02 and data[3] == 0 and data[4] == 0) {
    f = true;
  }
  else {
    delay(4000);
  }
  // }

}
void printl() {
  CANSetup(0x700) ;
  canBus.free();
  SendCANmessage(0x701, 8, 0x10 , 0X0E, 0x2F, 0xF0, 0x01, 0x03, 0x00, 0x00 ) ;
    delay(15);
    canBus.free();
    SendCANmessage(0x701, 8, 0x21 , 0X00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00 ) ;
    delay(15);
    canBus.free();
    SendCANmessage(0x701, 8, 0x22 , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ) ;
    delay(15);
   // canBus.free();
   // SendCANmessage(0x701, 8, 0x30 , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ) ;
   // delay(15);
  canBus.free();
  SendCANmessage(0x701, 8, 0x02 , 0X3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );
  delay(100);

}
void loop() {
  CANSetup(0x700) ;
  if ( f == false ) {
    start1();
  }

  printl();


}
