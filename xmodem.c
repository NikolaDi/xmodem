#include ".\app_cfg.h"

typedef struct{
	uint8_t *pchString;
}xmodem_print_t;
xmodem_print_t xmodem_print;

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
			s_pchStr = (uint8_t *)"xmode task start run...\r\n";
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
			if(XMODEM_WRIT_BYTE(*s_pchStr)) {
				s_pchStr++;
				s_emState = XMODEM_PRINT_STRING_CHECK;
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
		XMODEM_TASK_FSM_WAIT,
	}s_emState = XMODEM_TASK_FSM_START;
	
	switch(s_emState) {
		case XMODEM_TASK_FSM_START:
			if(fsm_rt_cpl == xmodem_print_string((uint8_t *)"xmode task start run...\r\n")) {
				//s_emState = XMODEM_TASK_FSM_WAIT;
			}
			break;
		case XMODEM_TASK_FSM_WAIT:
		
			break;
	}
	
	return fsm_rt_on_going;
}
