#include "..\app_cfg.h"

#ifndef _XMODEM_APP_CFG_
#define _XMODEM_APP_CFG_

//config file//////////////
#define XMODEM_START_DLY 3



//DEFINE//////////////////
#define CRC_MODE  'C'   
#define SUM_MODE  0x15
#define SOH       0x01
#define STX       0x02
#define EOT       0x04
#define ACK       0x06
#define NAK       0x15
#define CAN       0x18
#define EOF       0x1a

//xmodem
/********************************************************************************
  BYTE1   *      BYTE2      *     BYTE3       *     BYTE4~131    *  BYTE132~133 *
  Header  *  Packet Number  * ~Packet Number  *   Packet Data    *       CRC    *
*********************************************************************************/

//xmodem-1K
/**********************************************************************************
  BYTE1   *      BYTE2      *     BYTE3       *    BYTE4~1027    *  BYTE1028~1029 *
  Header  *  Packet Number  * ~Packet Number  *   Packet Data    *       CRC      *
***********************************************************************************/

#define XMODEM_READ_BYTE(__BYTE) serial_in(__BYTE)
extern uint8_t serial_in(uint8_t *pchDat);

#define XMODEM_WRITE_BYTE(__BYTE) serial_out(__BYTE)
extern uint8_t serial_out(uint8_t chDat);

#endif
