#ifndef UART_H  // ���û�ж��� MY_HEADER_H
#define UART_H  // �������������ظ�����

#include "main.h"

void uart_config(void);

void uart_event_handle(app_uart_evt_t * p_event);

void uart_reconfig(void);
#endif

