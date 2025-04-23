#ifndef MAIN_H  // 如果没有定义 MY_HEADER_H
#define MAIN_H  // 定义它，避免重复包含

//引用的C库头文件
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
//Log需要引用的头文件
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
//APP定时器需要引用的头文件
#include "app_timer.h"

#include "bsp_btn_ble.h"
//广播需要引用的头文件
#include "ble_advdata.h"
#include "ble_advertising.h"
//电源管理需要引用的头文件
#include "nrf_pwr_mgmt.h"
//SoftDevice handler configuration需要引用的头文件
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
//排序写入模块需要引用的头文件
#include "nrf_ble_qwr.h"
//GATT需要引用的头文件
#include "nrf_ble_gatt.h"
//连接参数协商需要引用的头文件
#include "ble_conn_params.h"
//串口透传需要引用的头文件
#include "my_ble_uarts.h"

//DFU需要引用的头文件
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
#define DEVICE_NAME                     "FYZ_DFU_BLE"                      // 设备名称字符串 
#define UARTS_SERVICE_UUID_TYPE         BLE_UUID_TYPE_VENDOR_BEGIN         // 串口透传服务UUID类型：厂商自定义UUID
#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(100, UNIT_1_25_MS)   // 最小连接间隔 (0.1 秒) 
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(200, UNIT_1_25_MS)   // 最大连接间隔 (0.2 秒) 
#define SLAVE_LATENCY                   0                                  // 从机延迟 
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)    // 监督超时(4 秒) 
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)              // 定义首次调用sd_ble_gap_conn_param_update()函数更新连接参数延迟时间（5秒）
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)             // 定义每次调用sd_ble_gap_conn_param_update()函数更新连接参数的间隔时间（30秒）
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                  // 定义放弃连接参数协商前尝试连接参数协商的最大次数（3次）

#define APP_ADV_INTERVAL                320                                // 广播间隔 (200ms)，单位0.625 ms 
#define APP_ADV_DURATION                0                                  // 广播持续时间，单位：10ms。设置为0表示不超时 

#define APP_BLE_OBSERVER_PRIO           3               //应用程序BLE事件监视者优先级，应用程序不能修改该数值
#define APP_BLE_CONN_CFG_TAG            1               //SoftDevice BLE配置标志

#define UART_TX_BUF_SIZE 256                            //串口发送缓存大小（字节数）
#define UART_RX_BUF_SIZE 256                            //串口接收缓存大小（字节数）

//用于stack dump的错误代码，可以用于栈回退时确定堆栈位置
#define DEAD_BEEF                       0xDEADBEEF     
               
//定义串口透传服务UUID列表
extern ble_uuid_t m_adv_uuids[];
 
//发送的最大数据长度
extern uint16_t  m_ble_uarts_max_data_len;            
extern bool 	uart_enabled;
//该变量用于保存连接句柄，初始值设置为无连接
extern uint16_t m_conn_handle; 


extern void my_ble_send(uint8_t* data, uint16_t len, uint16_t conn_handle);
#endif // MY_HEADER_H

