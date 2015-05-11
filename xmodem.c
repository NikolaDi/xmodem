#include ".\app_cfg.h"

typedef struct{
    uint8_t *pchString;
}xmodem_print_t;
xmodem_print_t xmodem_print;

// static fsm_rt_t xmodem_print_rn()
// {
    // static enum{
        // XMODEM_PRINT_RN_FSM_START = 0,
        // XMODEM_PRINT_RN_FSM_RRRRR,
        // XMODEM_PRINT_RN_FSM_NNNNN,
    // }s_emState = XMODEM_PRINT_RN_FSM_START;
    
    // switch(s_emState) {
        // case XMODEM_PRINT_RN_FSM_START:
            // s_emState = XMODEM_PRINT_RN_FSM_RRRRR;
            // break;
        // case XMODEM_PRINT_RN_FSM_RRRRR:
            // if(XMODEM_WRITE_BYTE('\r\n'))
            // break;
        // case XMODEM_PRINT_RN_FSM_NNNNN:
        
            // break;
    // }
// }

#define XMODE_PRINT_STRING_FSM_RESET() do{s_emState = XMODEM_PRINT_STRING_START;}while(0)
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
                XMODE_PRINT_STRING_FSM_RESET();
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
            
            break;
    }

    return fsm_rt_on_going;
}
