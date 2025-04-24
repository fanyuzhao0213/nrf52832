#ifndef MY_FDS_H  // 如果没有定义 MY_HEADER_H
#define MY_FDS_H  // 定义它，避免重复包含

#include "main.h"


typedef struct
{
    uint8_t  id_ble_code[17];               /*扫码获取的sensor-ble_code*/
	uint8_t  password[17];               	/*配置的连接密码*/
	uint8_t  password_config_flag;       	/*扫码获取的sensor-ble_code*/
} configuration_t;

enum
{
    WRITE_ID_CONFIG_INFO = 1,           /*写入洗衣机id配置信息*/
};

extern configuration_t m_dummy_cfg;



extern void my_flash_init(void);
extern void fds_once_storage(uint8_t flag);



#endif




