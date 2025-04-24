#include "my_fds.h"

#define CONFIG_FILE     (0xF100)
#define CONFIG_REC_KEY  (0x1010)

/* Flag to check fds initialization. */
static bool volatile m_fds_initialized_falg = false; 	//fds初始化完成标志位
static bool volatile m_fds_update_flag =false; 			 //fds更新完成标志位
static bool volatile m_fds_write_flag  =false;  		 //fds写完成标志位
static bool volatile m_fds_delete_flag =false; 		   //fds删除 完成标志 
static bool volatile  m_fds_gc_flag = false;   			 //fds更新事件标志

fds_stat_t stat = {0};
fds_record_desc_t desc = {0};
fds_find_token_t  tok  = {0};


configuration_t m_dummy_cfg = {0};

/* A record containing dummy configuration data. */
static fds_record_t const m_dummy_record =
{
    .file_id           = CONFIG_FILE,
    .key               = CONFIG_REC_KEY,
    .data.p_data       = &m_dummy_cfg,
    /* The length of a record is always expressed in 4-byte units (words). */
    .data.length_words = (sizeof(m_dummy_cfg) + 3) / sizeof(uint32_t),
};


/**@brief   Sleep until an event is received. */
static void power_manage(void)
{
#ifdef SOFTDEVICE_PRESENT
    (void) sd_app_evt_wait();
#else
    __WFE();
#endif
}

/**
* @brief  等待fds初始化完成
* @note   即等待fds初始化完成事件触发
* @param  None
* @retval None
*/
static void wait_for_fds_ready(void)
{
    while (!m_fds_initialized_falg)
    {
        power_manage();
    }
}
/**
* @brief  等待fds更新完成
* @note   即等待fds更新完成事件触发
* @param  None
* @retval None
*/
static void wait_for_fds_update(void)
{
    while (!m_fds_update_flag)
    {
        power_manage();
    }
}

/**
* @brief  等待fds写完成
* @note   即等待FDS写完成事件触发
* @param  None
* @retval None
*/
static void wait_for_fds_write(void)
{
    while (!m_fds_write_flag)
    {
        power_manage();  //这里会卡死 写会失败
    }
}

/**
* @brief  等待FDS删除完成
* @note   即等待FDS删除回收事件触发
* @param  None
* @retval None
*/
static void wait_for_fds_delete(void)
{
	while (!m_fds_delete_flag)
    {
        power_manage();
    }
}


/**
* @brief  等待碎片回收完成
* @note   即等待碎片回收事件触发
* @param  None
* @retval None
*/
static void wait_for_fds_gc(void)
{
	while (!m_fds_gc_flag)
    {
        power_manage();
    }
}

/**
* @brief  FDS碎片整理
* @note   使用fds_gc函数进行碎片整理
*         并且等待碎片整理完成事件
* @param  None
* @retval None
*/
void fds_gc_data(void)
{
	ret_code_t err_code;
	m_fds_gc_flag = false;
	err_code = fds_gc();
	APP_ERROR_CHECK(err_code);
	wait_for_fds_gc();
	
}

void read_systerm_config_from_fds(void)
{
    ret_code_t err_code;

    /* Wait for fds to initialize. */
    wait_for_fds_ready();

    memset(&tok, 0x00, sizeof(fds_find_token_t));
    memset(&desc, 0x00, sizeof(fds_record_desc_t));
    memset(&stat, 0x00, sizeof(fds_stat_t));

    err_code = fds_stat(&stat);
    APP_ERROR_CHECK(err_code);

    err_code = fds_record_find(CONFIG_FILE, CONFIG_REC_KEY, &desc, &tok);
    if (err_code == NRF_SUCCESS)
    {
                /* A config file is in flash. Let's update it. */
        fds_flash_record_t config = {0};

        /* Open the record and read its contents. */

        err_code = fds_record_open(&desc, &config);
        APP_ERROR_CHECK(err_code);
        /* Copy the configuration from flash into m_dummy_cfg. */

        memcpy(&m_dummy_cfg, config.p_data, sizeof(configuration_t));
        NRF_LOG_INFO("[FLASH] m_dummy_cfg.password_config_flag : %d", m_dummy_cfg.password_config_flag);
        NRF_LOG_INFO("[FLASH] m_dummy_cfg.id_ble_code : %s", m_dummy_cfg.id_ble_code);
        NRF_LOG_INFO("[FLASH] m_dummy_cfg.password : %s", m_dummy_cfg.password);
        /* Close the record when done reading. */

        err_code = fds_record_close(&desc);
        APP_ERROR_CHECK(err_code);
        /* Write the updated record to flash. */
    }
    else
    {
        /* System config not found; write a new one. */
        //NRF_LOG_INFO("Writing config file...");
        NRF_LOG_INFO("[FLASH] Writing1 config file...");
        err_code = fds_record_write(&desc, &m_dummy_record);
        APP_ERROR_CHECK(err_code);
    }
}


void my_fds_once_write(configuration_t *data)
{
	ret_code_t err_code;

	memset(&tok, 0x00, sizeof(fds_find_token_t));
	memset(&desc, 0x00, sizeof(fds_record_desc_t));
	memset(&stat, 0x00, sizeof(fds_stat_t));

	err_code = fds_stat(&stat);
	APP_ERROR_CHECK(err_code);

	err_code = fds_record_find(CONFIG_FILE, CONFIG_REC_KEY, &desc, &tok);
	if (err_code == NRF_SUCCESS)
	{
		/* Write the updated record to flash. */
		m_fds_update_flag =false;
		err_code = fds_record_update(&desc, &m_dummy_record);
		if(err_code == NRF_SUCCESS)
		{
			wait_for_fds_update();
			NRF_LOG_INFO("---------FDS OK-------------");
		}
		else
		{
			NRF_LOG_INFO("--------FDS ERR = %d----------",err_code);
			fds_gc_data();
			err_code = fds_record_update(&desc,&m_dummy_record);	
			if(err_code == NRF_SUCCESS)
			{
				wait_for_fds_update();
				NRF_LOG_INFO("--------FDS OK---------");
			}
			else
			{
				NRF_LOG_INFO("--------FDS ERR = %d-------",err_code);
			}
		}
	}
	else
	{
		/* System config not found; write a new one. */
		//写入记录
		NRF_LOG_INFO("fds_record_write ................");
		m_fds_write_flag = false;
		err_code = fds_record_write(&desc,&m_dummy_record);
		
		if(err_code == NRF_SUCCESS)
		{
			wait_for_fds_write();
		}
		else
		{
			NRF_LOG_INFO("FDS WRITE ERR");
		}
	}
}


static void fds_evt_handler(fds_evt_t const * p_evt)
{

switch (p_evt->id)
{
	case FDS_EVT_INIT:			//FSD 初始化
		if (p_evt->result == NRF_SUCCESS)
		{
			m_fds_initialized_falg = true;
		}
		break;

	case FDS_EVT_WRITE:
		{
			if (p_evt->result == NRF_SUCCESS)
			{
				m_fds_write_flag = true;
				NRF_LOG_INFO("Record1 ID:\t0x%04x",  p_evt->write.record_id);
				NRF_LOG_INFO("File1 ID:\t0x%04x",    p_evt->write.file_id);
				NRF_LOG_INFO("Record1 key:\t0x%04x", p_evt->write.record_key);
			}
		}
		break;

	case FDS_EVT_DEL_RECORD:
		{
			if (p_evt->result == NRF_SUCCESS)
			{
				NRF_LOG_INFO("Record2 ID:\t0x%04x",  p_evt->del.record_id);
				NRF_LOG_INFO("File2 ID:\t0x%04x",    p_evt->del.file_id);
				NRF_LOG_INFO("Record2 key:\t0x%04x", p_evt->del.record_key);
			}
		}
		break;
	case FDS_EVT_UPDATE:    //更新
		 if(p_evt->result == NRF_SUCCESS)
		 {
				m_fds_update_flag = true;
		 }				
		break;
	case FDS_EVT_GC:
		if(p_evt->result == NRF_SUCCESS)
		{
			m_fds_gc_flag = true;
		}
		break;
	default:
		break;
	}
}

/**@brief   Wait for fds to initialize. */
void my_flash_init(void)
{
    ret_code_t err_code;
    (void) fds_register(fds_evt_handler);

    err_code = fds_init();
    APP_ERROR_CHECK(err_code);
	wait_for_fds_ready(); 								 //等待初始化完成
	read_systerm_config_from_fds();
}

void fds_once_storage(uint8_t flag)
{
    if(flag == WRITE_ID_CONFIG_INFO)                    /*写入洗衣机ID属性*/
    {
        m_dummy_cfg.password_config_flag = 1;
		memcpy(m_dummy_cfg.id_ble_code,sensor_ble_code,16);
		memcpy(m_dummy_cfg.password, sensor_password, 16);
//		memcpy(my_read_data.id_ble_code,sensor_ble_code,18);
    }
    my_fds_once_write(&m_dummy_cfg);
}
