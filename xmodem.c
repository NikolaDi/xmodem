#include ".\app_cfg.h"

fsm_rt_t xmodem_task()
{
	static enum{
		XMODEM_TASK_START = 0,
		
	}s_emState = XMODEM_TASK_START;
	
	switch(s_emState) {
		case XMODEM_TASK_START:
		
			break;
	}
	
}
