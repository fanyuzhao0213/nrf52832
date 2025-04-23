#ifndef UART_H  // 如果没有定义 MY_HEADER_H
#define UART_H  // 定义它，避免重复包含

#include "main.h"

void uart_config(void);

void uart_event_handle(app_uart_evt_t * p_event);

void uart_reconfig(void);
#endif

