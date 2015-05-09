#include "..\app_cfg.h"

#ifndef _XMODEM_APP_CFG_
#define _XMODEM_APP_CFG_


#define XMODE_READ_BYTE(__BYTE) serial_in(__BYTE)
extern uint8_t serial_in(uint8_t *pchDat);

#define XMODEM_WRIT_BYTE(__BYTE) serial_out(__BYTE)
extern uint8_t serial_out(uint8_t chDat);

#endif
