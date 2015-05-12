#include ".\app_cfg.h"

typedef enum {
    PACKET_CPL = 0,
    PACKET_CHAR_CPL,
    PACKET_DATA_CPL,
    PACKET_CHECK_CPL,
    PACKET_TIMEOUT,
    PACKET_NOCHAR,
    PACKET_ERROR_HEAD,
    PACKET_ERROR_PNUM,
    PACKET_ERROR_NPNUM,
    PACKET_ERROR_CHECK,
    PACKET_ERROR_CHAR_TIMEOUT,
    PACKET_ON_GOING,
}xmodem_packet_t;

#define XMODEM_PRINT_STRING_FSM_RESET() do{s_emState = XMODEM_PRINT_STRING_START;}while(0)
static fsm_rt_t xmodem_print_string(uint8_t *pchStr)
{
    static enum{
        XMODEM_PRINT_STRING_START = 0,
        XMODEM_PRINT_STRING_CHECK,
        XMODEM_PRINT_STRING_SEND,
    }s_emState = XMODEM_PRINT_STRING_START;
    static uint8_t *s_pchStr;

    switch(s_emState) {
        case XMODEM_PRINT_STRING_START:
            s_pchStr = pchStr;
            s_emState = XMODEM_PRINT_STRING_CHECK;
            break;
        case XMODEM_PRINT_STRING_CHECK:
            if('\0' != *s_pchStr) {
                s_emState = XMODEM_PRINT_STRING_SEND;
            }else {
                XMODEM_PRINT_STRING_FSM_RESET();
                return fsm_rt_cpl;
            }
            break;
        case XMODEM_PRINT_STRING_SEND:
            if(XMODEM_WRITE_BYTE(*s_pchStr)) {
                s_pchStr++;
                s_emState = XMODEM_PRINT_STRING_CHECK;
            }
        break;
    }

    return fsm_rt_on_going;
}

#define XMODEM_RUN_DELAY_FSM_RESET() do{s_emState = XMODEM_RUN_DELAY_FSM_START;}while(0)
static fsm_rt_t xmodem_run_delay()
{
    static enum{
        XMODEM_RUN_DELAY_FSM_START = 0,
        XMODEM_RUN_DELAY_FSM_STRING,
        XMODEM_RUN_DELAY_FSM_CHECK,
        XMODEM_RUN_DELAY_FSM_SEND,
        XMODEM_RUN_DELAY_FSM_DLY,
        XMODEM_RUN_DELAY_FSM_BB,
        XMODEM_RUN_DELAY_FSM_RN,
    }s_emState = XMODEM_RUN_DELAY_FSM_START;
    static uint8_t s_chTimeCnt;
    static uint16_t s_hwCnt;
    
    switch(s_emState) {
        case XMODEM_RUN_DELAY_FSM_START:
            s_hwCnt = 0;
            s_chTimeCnt = XMODEM_START_DLY;
            s_emState = XMODEM_RUN_DELAY_FSM_STRING;
            break;
        case XMODEM_RUN_DELAY_FSM_STRING:
            if(fsm_rt_cpl == xmodem_print_string((uint8_t *)"xmodem task start run......")) {
                s_emState = XMODEM_RUN_DELAY_FSM_CHECK;
            }
            break;
        case XMODEM_RUN_DELAY_FSM_CHECK:
            if(0xff != s_chTimeCnt) {
                s_emState = XMODEM_RUN_DELAY_FSM_SEND;
            }else {
                s_emState = XMODEM_RUN_DELAY_FSM_RN;
            }
            break;
        case XMODEM_RUN_DELAY_FSM_SEND:
            if(XMODEM_WRITE_BYTE(s_chTimeCnt+'0')) {
                s_emState = XMODEM_RUN_DELAY_FSM_BB;
            }
            break;
        case XMODEM_RUN_DELAY_FSM_BB:
            if(fsm_rt_cpl == xmodem_print_string((uint8_t *)"\b")) {
                s_emState = XMODEM_RUN_DELAY_FSM_DLY;
            }
            break;
        case XMODEM_RUN_DELAY_FSM_DLY:
            if(++s_hwCnt >= 40000) {
                s_hwCnt = 0;
                s_chTimeCnt--;
                s_emState = XMODEM_RUN_DELAY_FSM_CHECK;
            }
            break;
        case XMODEM_RUN_DELAY_FSM_RN:
            if(fsm_rt_cpl == xmodem_print_string((uint8_t *)"\r\n")) {
                XMODEM_RUN_DELAY_FSM_RESET();
                return fsm_rt_cpl;
            }
            break;
    }
    
    return fsm_rt_on_going;
}

#define TIME_TOKEN  60000
#define TIME_CHAR   30000
#define XMODEM_READ_BYTE_WITH_TIMEOUT_RESET() do{s_emState = XMODEM_READ_BYTE_WITH_TIMEOUT_START;}while(0)
static xmodem_packet_t xmodem_read_byte_with_timeout(uint8_t *pchByte,uint16_t hwTimeOut)
{
    static enum {
        XMODEM_READ_BYTE_WITH_TIMEOUT_START = 0,
        XMODEM_READ_BYTE_WITH_TIMEOUT_READ,
    }s_emState = XMODEM_READ_BYTE_WITH_TIMEOUT_START;
    static uint16_t s_hwTimeCnt;
    
    switch(s_emState) {
        case XMODEM_READ_BYTE_WITH_TIMEOUT_START:
            s_hwTimeCnt = 0;
            s_emState = XMODEM_READ_BYTE_WITH_TIMEOUT_READ;
            //break;
        case XMODEM_READ_BYTE_WITH_TIMEOUT_READ:
            if(XMODEM_READ_BYTE(pchByte)) {
                XMODEM_READ_BYTE_WITH_TIMEOUT_RESET();
                return PACKET_CHAR_CPL;
            }
            
            if(++s_hwTimeCnt > hwTimeOut) {
                XMODEM_READ_BYTE_WITH_TIMEOUT_RESET();
                return PACKET_ERROR_CHAR_TIMEOUT;
            }
            break;
    }
    
    return PACKET_ON_GOING;
}

#define XMODEM_PACKET_DATA_FSM_RESET() do{s_emState = XMODEM_PACKET_DATA_FSM_START;}while(0)
static xmodem_packet_t xmodem_packet_data(uint8_t *pchBuff,uint16_t hwDataSize)
{
    static enum{
        XMODEM_PACKET_DATA_FSM_START = 0,
        XMODEM_PACKET_DATA_FSM_DATA,
    }s_emState = XMODEM_PACKET_DATA_FSM_START;
    static uint16_t s_hwBuffCnt;
	xmodem_packet_t tRet;
	uint8_t chByte;
    
    switch(s_emState) {
        case XMODEM_PACKET_DATA_FSM_START:
            s_emState = XMODEM_PACKET_DATA_FSM_START;
            s_hwBuffCnt = 0;
            //break;
        case XMODEM_PACKET_DATA_FSM_DATA:
            tRet = (xmodem_packet_t)xmodem_read_byte_with_timeout(&chByte,TIME_CHAR);
            if(PACKET_CHAR_CPL == tRet) {
                pchBuff[s_hwBuffCnt] = chByte;
                if(++s_hwBuffCnt >= hwDataSize) {
                    XMODEM_PACKET_DATA_FSM_RESET();
                    return PACKET_DATA_CPL;
                }else {
                    //s_emState = ;
                }
            }else if(PACKET_ERROR_CHAR_TIMEOUT == tRet) {
                XMODEM_PACKET_DATA_FSM_RESET();
                return PACKET_ERROR_CHAR_TIMEOUT;
            }
            break;
    }
    
    return PACKET_ON_GOING;
}

#define XMODEM_PACKET_CHECK_FSM_RESET() do{s_emState = XMODEM_PACKET_CHECK_FSM_START;}while(0)
static xmodem_packet_t xmodem_packet_check(uint8_t *pchBuff,uint8_t chToken)
{
    static enum{
        XMODEM_PACKET_CHECK_FSM_START = 0,
        XMODEM_PACKET_CHECK_FSM_HIGHBYTE,
        XMODEM_PACKET_CHECK_FSM_LOWBYTE,
    }s_emState = XMODEM_PACKET_CHECK_FSM_START;
	xmodem_packet_t tRet;
	uint8_t chByte;
    
    switch(s_emState) {
        case XMODEM_PACKET_CHECK_FSM_START:
            s_emState = XMODEM_PACKET_CHECK_FSM_HIGHBYTE;
            //break;
        case XMODEM_PACKET_CHECK_FSM_HIGHBYTE:
            tRet = (xmodem_packet_t)xmodem_read_byte_with_timeout(&chByte,TIME_CHAR);
            if(PACKET_CHAR_CPL == tRet) {
                pchBuff[0] = chByte;
                if(SUM_MODE == chToken) {
                    XMODEM_PACKET_CHECK_FSM_RESET();
                    return PACKET_CHECK_CPL;
                }else {
                    s_emState = XMODEM_PACKET_CHECK_FSM_LOWBYTE;
                }
            }else if(PACKET_ERROR_CHAR_TIMEOUT == tRet) {
                XMODEM_PACKET_CHECK_FSM_RESET();
                return PACKET_ERROR_CHAR_TIMEOUT;
            }
            break;
        case XMODEM_PACKET_CHECK_FSM_LOWBYTE:
            tRet = (xmodem_packet_t)xmodem_read_byte_with_timeout(&chByte,TIME_CHAR);
            if(PACKET_CHAR_CPL == tRet) {
                pchBuff[1] = chByte;
                XMODEM_PACKET_CHECK_FSM_RESET();
                return PACKET_CHECK_CPL;
            }else if(PACKET_ERROR_CHAR_TIMEOUT == tRet) {
                XMODEM_PACKET_CHECK_FSM_RESET();
                return PACKET_ERROR_CHAR_TIMEOUT;
            }
            break;
    }
    
    return PACKET_ON_GOING;
}

#define XMODEM_PACKET_FSM_RESET() do{s_emState = XMODEM_PACKET_START;}while(0)
static xmodem_packet_t xmodem_packet(uint8_t *pchBuff,uint8_t chToken)
{
    static enum {
        XMODEM_PACKET_START = 0,
        XMODEM_PACKET_HEAD,
        XMODEM_PACKET_PNUM,
        XMODEM_PACKET_NPNUM,
        XMODEM_PACKET_DATA,
        XMODEM_PACKET_CHECK,
    }s_emState = XMODEM_PACKET_START;
    xmodem_packet_t tRet;
    uint8_t chByte;
    static uint16_t s_hwDataSize;//xmodem xmodem-1k
    
    switch(s_emState) {
        case XMODEM_PACKET_START:
            s_emState = XMODEM_PACKET_HEAD;
            //break;
        case XMODEM_PACKET_HEAD:
            tRet = (xmodem_packet_t)xmodem_read_byte_with_timeout(&chByte,TIME_TOKEN);
            if(PACKET_CHAR_CPL == tRet) {
                if(EOT == chByte) {
                    XMODEM_PACKET_FSM_RESET();
                    return PACKET_CPL;
                }else if(SOH == chByte) {
                    s_hwDataSize = 128;
                } else if(STX == chByte) {
                    s_hwDataSize = 1024;
                } else {
                    pchBuff[0] = chByte;
                    s_emState = XMODEM_PACKET_PNUM;
                }
            }else if(PACKET_ERROR_CHAR_TIMEOUT == tRet) {
                XMODEM_PACKET_FSM_RESET();
                return PACKET_NOCHAR;
            }
            break;
        case XMODEM_PACKET_PNUM:
            tRet = (xmodem_packet_t)xmodem_read_byte_with_timeout(&chByte,TIME_CHAR);
            if(PACKET_CHAR_CPL == tRet) {
                pchBuff[1] = chByte;
                s_emState = XMODEM_PACKET_NPNUM;
            }else if(PACKET_ERROR_CHAR_TIMEOUT == tRet) {
                XMODEM_PACKET_FSM_RESET();
                return PACKET_ERROR_CHAR_TIMEOUT;
            }
            break;
        case XMODEM_PACKET_NPNUM:
            tRet = (xmodem_packet_t)xmodem_read_byte_with_timeout(&chByte,TIME_CHAR);
            if(PACKET_CHAR_CPL == tRet) {
                pchBuff[2] = chByte;
                s_emState = XMODEM_PACKET_DATA;
            }else if(PACKET_ERROR_CHAR_TIMEOUT == tRet) {
                XMODEM_PACKET_FSM_RESET();
                return PACKET_ERROR_CHAR_TIMEOUT;
            }
            break;
        case XMODEM_PACKET_DATA:
            tRet = (xmodem_packet_t)xmodem_packet_data(pchBuff[3],s_hwDataSize);
            if(PACKET_DATA_CPL == tRet) {
                s_emState = XMODEM_PACKET_CHECK;
            }else if(PACKET_ERROR_CHAR_TIMEOUT == tRet) {
                XMODEM_PACKET_FSM_RESET();
                return PACKET_ERROR_CHAR_TIMEOUT;
            }
            break;
        case XMODEM_PACKET_CHECK:
            tRet = (xmodem_packet_t)xmodem_packet_check(pchBuff[3+s_hwDataSize],chToken);
            if(PACKET_CHECK_CPL == tRet) {
                XMODEM_PACKET_FSM_RESET();
                return PACKET_CPL;
            }
            break;
    }
    
    return PACKET_ON_GOING;
}

#define XMODEM_FRAME_FSM_RESET() do{s_emState = XMODEM_FRAME_FSM_START;}while(0)
static xmodem_packet_t xmodem_frame()
{
    static enum{
        XMODEM_FRAME_FSM_START = 0,
        XMODEM_FRAME_FSM_TOKEN,
        XMODEM_FRAME_FSM_PACKET,
        XMODEM_FRAME_FSM_PARSE,
    }s_emState = XMODEM_FRAME_FSM_START;
    static uint8_t s_chPacketBuff[50];//1029
    static uint8_t s_chToken;
    xmodem_packet_t tRet;
    
    switch(s_emState) {
        case XMODEM_FRAME_FSM_START:
            s_emState = XMODEM_FRAME_FSM_TOKEN;
            s_chToken = CRC_MODE;
            break;
        case XMODEM_FRAME_FSM_TOKEN:
            if(XMODEM_WRITE_BYTE(s_chToken)) {
                s_emState = XMODEM_FRAME_FSM_PACKET;
            }
            break;
        case XMODEM_FRAME_FSM_PACKET:
            tRet = (xmodem_packet_t)xmodem_packet(s_chPacketBuff,s_chToken);
            if(PACKET_CPL == tRet) {
                s_emState = XMODEM_FRAME_FSM_PARSE;
            }
            break;
        case XMODEM_FRAME_FSM_PARSE:
            return PACKET_CPL;
            break;
    }
    
    return PACKET_ON_GOING;
}

#define XMODEM_TASK_FSM_RESET() do{s_emState = XMODEM_TASK_FSM_START;}while(0)
fsm_rt_t xmodem_task()
{
    static enum{
        XMODEM_TASK_FSM_START = 0,
        XMODEM_TASK_FSM_DLY,
        XMODEM_TASK_FSM_FRAME,
    }s_emState = XMODEM_TASK_FSM_START;

    switch(s_emState) {
        case XMODEM_TASK_FSM_START:
            s_emState = XMODEM_TASK_FSM_DLY;
            break;
        case XMODEM_TASK_FSM_DLY:
            if(fsm_rt_cpl == xmodem_run_delay()) {
                s_emState = XMODEM_TASK_FSM_FRAME;
            }
            break;
        case XMODEM_TASK_FSM_FRAME:
            if(PACKET_CPL == xmodem_frame()) {
                XMODEM_TASK_FSM_RESET();
            }
            break;
    }

    return fsm_rt_on_going;
}
