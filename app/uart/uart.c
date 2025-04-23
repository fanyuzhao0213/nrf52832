#include "uart.h"

//串口事件回调函数，串口初始化时注册，该函数中判断事件类型并进行处理
//当接收的数据长度达到设定的最大值或者接收到换行符后，则认为一包数据接收完成，之后将接收的数据发送给主机
void uart_event_handle(app_uart_evt_t * p_event)
{
    static uint8_t data_array[BLE_UARTS_MAX_DATA_LEN];
    static uint8_t index = 0;
    uint32_t       err_code;
    //判断事件类型
    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY://串口接收事件
            UNUSED_VARIABLE(app_uart_get(&data_array[index]));
            index++;
            //接收串口数据，当接收的数据长度达到m_ble_uarts_max_data_len或者接收到换行符后认为一包数据接收完成
            if ((data_array[index - 1] == '\n') ||
                (data_array[index - 1] == '\r') ||
                (index >= m_ble_uarts_max_data_len))
            {
                if (index > 1)
                {
					NRF_LOG_INFO("m_conn_handle : %d,index:%d",m_conn_handle,index);
					my_ble_send(data_array,index,m_conn_handle);
                }
                index = 0;
            }
            break;
        //通讯错误事件，进入错误处理
        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;
        //FIFO错误事件，进入错误处理
        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}
//串口配置
void uart_config(void)
{
	uint32_t err_code;
	
	//定义串口通讯参数配置结构体并初始化
	const app_uart_comm_params_t comm_params =
	{
		RX_PIN_NUMBER,//定义uart接收引脚
		TX_PIN_NUMBER,//定义uart发送引脚
		RTS_PIN_NUMBER,//定义uart RTS引脚，流控关闭后虽然定义了RTS和CTS引脚，但是驱动程序会忽略，不会配置这两个引脚，两个引脚仍可作为IO使用
		CTS_PIN_NUMBER,//定义uart CTS引脚
		APP_UART_FLOW_CONTROL_DISABLED,//关闭uart硬件流控
		false,//禁止奇偶检验
		NRF_UART_BAUDRATE_115200//uart波特率设置为115200bps
	};
	//初始化串口，注册串口事件回调函数
	APP_UART_FIFO_INIT(&comm_params,
						 UART_RX_BUF_SIZE,
						 UART_TX_BUF_SIZE,
						 uart_event_handle,
						 APP_IRQ_PRIORITY_LOWEST,
						 err_code);

	APP_ERROR_CHECK(err_code);

}



void uart_reconfig(void)
{
	if(uart_enabled == false)//初始化串口
	{
		uart_config();
		uart_enabled = true;
	}
	else
	{
		app_uart_close();//反初始化串口
		uart_enabled = false;
	}
}




