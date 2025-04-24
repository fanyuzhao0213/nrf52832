#ifndef MY_FDS_H  // ���û�ж��� MY_HEADER_H
#define MY_FDS_H  // �������������ظ�����

#include "main.h"


typedef struct
{
    uint8_t  id_ble_code[17];               /*ɨ���ȡ��sensor-ble_code*/
	uint8_t  password[17];               	/*���õ���������*/
	uint8_t  password_config_flag;       	/*ɨ���ȡ��sensor-ble_code*/
} configuration_t;

enum
{
    WRITE_ID_CONFIG_INFO = 1,           /*д��ϴ�»�id������Ϣ*/
};

extern configuration_t m_dummy_cfg;



extern void my_flash_init(void);
extern void fds_once_storage(uint8_t flag);



#endif




