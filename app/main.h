#ifndef MAIN_H  // ���û�ж��� MY_HEADER_H
#define MAIN_H  // �������������ظ�����

//���õ�C��ͷ�ļ�
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
//Log��Ҫ���õ�ͷ�ļ�
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
//APP��ʱ����Ҫ���õ�ͷ�ļ�
#include "app_timer.h"

#include "bsp_btn_ble.h"
//�㲥��Ҫ���õ�ͷ�ļ�
#include "ble_advdata.h"
#include "ble_advertising.h"
//��Դ������Ҫ���õ�ͷ�ļ�
#include "nrf_pwr_mgmt.h"
//SoftDevice handler configuration��Ҫ���õ�ͷ�ļ�
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
//����д��ģ����Ҫ���õ�ͷ�ļ�
#include "nrf_ble_qwr.h"
//GATT��Ҫ���õ�ͷ�ļ�
#include "nrf_ble_gatt.h"
//���Ӳ���Э����Ҫ���õ�ͷ�ļ�
#include "ble_conn_params.h"
//����͸����Ҫ���õ�ͷ�ļ�
#include "my_ble_uarts.h"

//DFU��Ҫ���õ�ͷ�ļ�
#include "nrf_dfu_ble_svci_bond_sharing.h"
#include "nrf_svci_async_function.h"
#include "nrf_svci_async_handler.h"
#include "nrf_power.h"
#include "ble_dfu.h"
#include "nrf_bootloader_info.h"

#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif
#include "app_uart.h"


#define PROJECT_SW_VERSION        		"V-Hw01.01.00-Fw01.00.03"
#define DEVICE_NAME                     "FYZ_DFU_BLE"                      // �豸�����ַ��� 
#define UARTS_SERVICE_UUID_TYPE         BLE_UUID_TYPE_VENDOR_BEGIN         // ����͸������UUID���ͣ������Զ���UUID
#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(100, UNIT_1_25_MS)   // ��С���Ӽ�� (0.1 ��) 
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(200, UNIT_1_25_MS)   // ������Ӽ�� (0.2 ��) 
#define SLAVE_LATENCY                   0                                  // �ӻ��ӳ� 
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)    // �ල��ʱ(4 ��) 
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)              // �����״ε���sd_ble_gap_conn_param_update()�����������Ӳ����ӳ�ʱ�䣨5�룩
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)             // ����ÿ�ε���sd_ble_gap_conn_param_update()�����������Ӳ����ļ��ʱ�䣨30�룩
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                  // ����������Ӳ���Э��ǰ�������Ӳ���Э�̵���������3�Σ�

#define APP_ADV_INTERVAL                320                                // �㲥��� (200ms)����λ0.625 ms 
#define APP_ADV_DURATION                0                                  // �㲥����ʱ�䣬��λ��10ms������Ϊ0��ʾ����ʱ 

#define APP_BLE_OBSERVER_PRIO           3               //Ӧ�ó���BLE�¼����������ȼ���Ӧ�ó������޸ĸ���ֵ
#define APP_BLE_CONN_CFG_TAG            1               //SoftDevice BLE���ñ�־

#define UART_TX_BUF_SIZE 256                            //���ڷ��ͻ����С���ֽ�����
#define UART_RX_BUF_SIZE 256                            //���ڽ��ջ����С���ֽ�����

//����stack dump�Ĵ�����룬��������ջ����ʱȷ����ջλ��
#define DEAD_BEEF                       0xDEADBEEF     
               
//���崮��͸������UUID�б�
extern ble_uuid_t m_adv_uuids[];
 
//���͵�������ݳ���
extern uint16_t  m_ble_uarts_max_data_len;            
extern bool 	uart_enabled;
//�ñ������ڱ������Ӿ������ʼֵ����Ϊ������
extern uint16_t m_conn_handle; 


extern void my_ble_send(uint8_t* data, uint16_t len, uint16_t conn_handle);
#endif // MY_HEADER_H

