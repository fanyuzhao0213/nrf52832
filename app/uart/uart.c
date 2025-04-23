#include "uart.h"

//�����¼��ص����������ڳ�ʼ��ʱע�ᣬ�ú������ж��¼����Ͳ����д���
//�����յ����ݳ��ȴﵽ�趨�����ֵ���߽��յ����з�������Ϊһ�����ݽ�����ɣ�֮�󽫽��յ����ݷ��͸�����
void uart_event_handle(app_uart_evt_t * p_event)
{
    static uint8_t data_array[BLE_UARTS_MAX_DATA_LEN];
    static uint8_t index = 0;
    uint32_t       err_code;
    //�ж��¼�����
    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY://���ڽ����¼�
            UNUSED_VARIABLE(app_uart_get(&data_array[index]));
            index++;
            //���մ������ݣ������յ����ݳ��ȴﵽm_ble_uarts_max_data_len���߽��յ����з�����Ϊһ�����ݽ������
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
        //ͨѶ�����¼������������
        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;
        //FIFO�����¼������������
        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}
//��������
void uart_config(void)
{
	uint32_t err_code;
	
	//���崮��ͨѶ�������ýṹ�岢��ʼ��
	const app_uart_comm_params_t comm_params =
	{
		RX_PIN_NUMBER,//����uart��������
		TX_PIN_NUMBER,//����uart��������
		RTS_PIN_NUMBER,//����uart RTS���ţ����عرպ���Ȼ������RTS��CTS���ţ����������������ԣ������������������ţ����������Կ���ΪIOʹ��
		CTS_PIN_NUMBER,//����uart CTS����
		APP_UART_FLOW_CONTROL_DISABLED,//�ر�uartӲ������
		false,//��ֹ��ż����
		NRF_UART_BAUDRATE_115200//uart����������Ϊ115200bps
	};
	//��ʼ�����ڣ�ע�ᴮ���¼��ص�����
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
	if(uart_enabled == false)//��ʼ������
	{
		uart_config();
		uart_enabled = true;
	}
	else
	{
		app_uart_close();//����ʼ������
		uart_enabled = false;
	}
}




