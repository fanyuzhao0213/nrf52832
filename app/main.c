/****************************************Copyright (c)************************************************              
**--------------File Info-----------------------------------------------------------------------------
** File name         : main.c
** Last modified Date: 2020-9-25        
** Last Version      :		   
** Descriptions      : 使用的SDK版本-SDK_17.0.2
**						
**----------------------------------------------------------------------------------------------------
** Descriptions      : 串口透传长包传输（最大传输长度244个字节），本例中使用的设备名称是中文设备名称：串口透传
**                   ：增加了对通知是否使能的判断
**                   ：为防止在未连接、通知未使能的情况下串口数据干扰程序，只有通知使能后才初始化串口接收串口数据，通知关闭后，关闭串口
**---------------------------------------------------------------------------------------------------*/

#include "main.h"
#include "uart.h"

BLE_UARTS_DEF(m_uarts, NRF_SDH_BLE_TOTAL_LINK_COUNT);    //定义名称为m_uarts的串口透传服务实例
NRF_BLE_GATT_DEF(m_gatt);                                //定义名称为m_gatt的GATT模块实例
NRF_BLE_QWR_DEF(m_qwr);                                  //定义一个名称为m_qwr的排队写入实例
BLE_ADVERTISING_DEF(m_advertising);                      //定义名称为m_advertising的广播模块实例

//定义串口透传服务UUID列表
ble_uuid_t m_adv_uuids[]=                                          
{
    {BLE_UUID_UARTS_SERVICE, UARTS_SERVICE_UUID_TYPE}
};

//发送的最大数据长度
uint16_t   m_ble_uarts_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;            
bool uart_enabled = false;
//该变量用于保存连接句柄，初始值设置为无连接
uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID; 

//设备名称数组  中文名称：串口透传
const char device_name[12] = {0xE4,0xB8,0xB2,0xE5,0x8F,0xA3,0xE9,0x80,0x8F,0xE4,0xBC,0xA0};
	
//GAP参数初始化，该函数配置需要的GAP参数，包括设备名称，外观特征、首选连接参数
static void gap_params_init(void)
{
    ret_code_t              err_code;
	  //定义连接参数结构体变量
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;
    //设置GAP的安全模式
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    //设置GAP设备名称，使用英文设备名称
//    err_code = sd_ble_gap_device_name_set(&sec_mode,
//                                              (const uint8_t *)DEVICE_NAME,
//                                              strlen(DEVICE_NAME));
	
	//设置GAP设备名称，这里使用了中文设备名称
    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)device_name,
                                          sizeof(device_name));
																					
    //检查函数返回的错误代码
		APP_ERROR_CHECK(err_code);
																				
    //设置首选连接参数，设置前先清零gap_conn_params
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;//最小连接间隔
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;//最小连接间隔
    gap_conn_params.slave_latency     = SLAVE_LATENCY;    //从机延迟
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT; //监督超时
    //调用协议栈API sd_ble_gap_ppcp_set配置GAP参数
    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
																					
}
//GATT事件处理函数，该函数中处理MTU交换事件
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    //如果是MTU交换事件
	  if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        //设置串口透传服务的有效数据长度（MTU-opcode-handle）
			  m_ble_uarts_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_INFO("Data len is set to 0x%X(%d)", m_ble_uarts_max_data_len, m_ble_uarts_max_data_len);
    }
    NRF_LOG_DEBUG("ATT MTU exchange completed. central 0x%x peripheral 0x%x",
                  p_gatt->att_mtu_desired_central,
                  p_gatt->att_mtu_desired_periph);
}
//初始化GATT程序模块
static void gatt_init(void)
{
	ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);    //初始化GATT程序模块
    APP_ERROR_CHECK(err_code);		//检查函数返回的错误代码
	err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);	  //设置ATT MTU的大小,这里设置的值为247
    APP_ERROR_CHECK(err_code);		//检查函数返回的错误代码
}

//排队写入事件处理函数，用于处理排队写入模块的错误
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    //检查错误代码
	APP_ERROR_HANDLER(nrf_error);
}

/* 
* 从机模式 发送单包数据  
* data in m_send_data
* data len is m_send_len
*/
void my_ble_send(uint8_t* data, uint16_t len, uint16_t conn_handle)
{
		uint32_t err_code;
		do
		{
				err_code = ble_uarts_data_send(&m_uarts, data, &len, conn_handle);
				if ((err_code != NRF_ERROR_INVALID_STATE) &&
						(err_code != NRF_ERROR_RESOURCES) &&
						(err_code != NRF_ERROR_NOT_FOUND))
				{
						APP_ERROR_CHECK(err_code);
				}
		} while (err_code == NRF_ERROR_RESOURCES);
		NRF_LOG_INFO("my ble send return : %d\r\n",err_code);
//		NRF_LOG_INFO("my ble send return : %d",err_code);
}

//串口透传事件回调函数，串口透出服务初始化时注册
static void uarts_data_handler(ble_uarts_evt_t * p_evt)
{
	//判断事件类型:接收到新数据事件
    if (p_evt->type == BLE_UARTS_EVT_RX_DATA)
    {
        uint32_t err_code;
        //串口打印出接收的数据
        for (uint32_t i = 0; i < p_evt->params.rx_data.length; i++)
        {
//			NRF_LOG_HEXDUMP_INFO(p_evt->params.rx_data.p_data,p_evt->params.rx_data.length);
            do
            {
                err_code = app_uart_put(p_evt->params.rx_data.p_data[i]);
                if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY))
                {
                    NRF_LOG_ERROR("Failed receiving NUS message. Error 0x%x. ", err_code);
                    APP_ERROR_CHECK(err_code);
                }
            } while (err_code == NRF_ERROR_BUSY);
        }
        if (p_evt->params.rx_data.p_data[p_evt->params.rx_data.length - 1] == '\r')
        {
            while (app_uart_put('\n') == NRF_ERROR_BUSY);
        }
    }
		//判断事件类型:发送就绪事件，该事件在后面的试验会用到，当前我们在该事件中翻转指示灯D4的状态，指示该事件的产生
    if (p_evt->type == BLE_UARTS_EVT_TX_RDY)
    {
		nrf_gpio_pin_toggle(LED_4);
	}
}
//服务初始化，包含初始化排队写入模块和初始化应用程序使用的服务
static void services_init(void)
{
    ret_code_t         err_code;
	//定义串口透传初始化结构体
	ble_uarts_init_t     uarts_init;
	//定义排队写入初始化结构体变量
    nrf_ble_qwr_init_t qwr_init = {0};

    //排队写入事件处理函数
    qwr_init.error_handler = nrf_qwr_error_handler;
    //初始化排队写入模块
    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
		//检查函数返回值
    APP_ERROR_CHECK(err_code);
    
		
	/*------------------以下代码初始化串口透传服务-------------*/
	//清零串口透传服务初始化结构体
	memset(&uarts_init, 0, sizeof(uarts_init));
	//设置串口透传事件回调函数
    uarts_init.data_handler = uarts_data_handler;
    //初始化串口透传服务
    err_code = ble_uarts_init(&m_uarts, &uarts_init);
    APP_ERROR_CHECK(err_code);
	/*------------------初始化串口透传服务-END-----------------*/
}

//连接参数协商模块事件处理函数
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    ret_code_t err_code;
    //判断事件类型，根据事件类型执行动作
	  //连接参数协商失败，断开当前连接
    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
		//连接参数协商成功
		if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_SUCCEEDED)
    {
       //功能代码;
    }
}

//连接参数协商模块错误处理事件，参数nrf_error包含了错误代码，通过nrf_error可以分析错误信息
static void conn_params_error_handler(uint32_t nrf_error)
{
    //检查错误代码
	  APP_ERROR_HANDLER(nrf_error);
}


//连接参数协商模块初始化
static void conn_params_init(void)
{
    ret_code_t             err_code;
	  //定义连接参数协商模块初始化结构体
    ble_conn_params_init_t cp_init;
    //配置之前先清零
    memset(&cp_init, 0, sizeof(cp_init));
    //设置为NULL，从主机获取连接参数
    cp_init.p_conn_params                  = NULL;
	  //连接或启动通知到首次发起连接参数更新请求之间的时间设置为5秒
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
	  //每次调用sd_ble_gap_conn_param_update()函数发起连接参数更新请求的之间的间隔时间设置为：30秒
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
	  //放弃连接参数协商前尝试连接参数协商的最大次数设置为：3次
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
	  //连接参数更新从连接事件开始计时
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
	  //连接参数更新失败不断开连接
    cp_init.disconnect_on_fail             = false;
	  //注册连接参数更新事件句柄
    cp_init.evt_handler                    = on_conn_params_evt;
	  //注册连接参数更新错误事件句柄
    cp_init.error_handler                  = conn_params_error_handler;
    //调用库函数（以连接参数更新初始化结构体为输入参数）初始化连接参数协商模块
    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

//广播事件处理函数
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t err_code;
    //判断广播事件类型
    switch (ble_adv_evt)
    {
        //快速广播启动事件：快速广播启动后会产生该事件
		case BLE_ADV_EVT_FAST:
            NRF_LOG_INFO("Fast advertising.");
			//设置广播指示灯为正在广播（D1指示灯闪烁）
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;
        //广播IDLE事件：广播超时后会产生该事件
        case BLE_ADV_EVT_IDLE:
			//设置广播指示灯为广播停止（D1指示灯熄灭）
            err_code = bsp_indication_set(BSP_INDICATE_IDLE);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;
    }
}
//广播初始化
static void advertising_init(void)
{
    ret_code_t             err_code;
	  //定义广播初始化配置结构体变量
    ble_advertising_init_t init;
    //配置之前先清零
    memset(&init, 0, sizeof(init));
    //设备名称类型：全称
    init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;
	  //是否包含外观：包含
    init.advdata.include_appearance      = false;
	  //Flag:一般可发现模式，不支持BR/EDR
    init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
	  //UUID放到扫描响应里面
	  init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids  = m_adv_uuids;
	
    //设置广播模式为快速广播
    init.config.ble_adv_fast_enabled  = true;
	  //设置广播间隔和广播持续时间
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;
    //广播事件回调函数
    init.evt_handler = on_adv_evt;
    //初始化广播
    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);
    //设置广播配置标记。APP_BLE_CONN_CFG_TAG是用于跟踪广播配置的标记，这是为未来预留的一个参数，在将来的SoftDevice版本中，
		//可以使用sd_ble_gap_adv_set_configure()配置新的广播配置
		//当前SoftDevice版本（S132 V7.0.1版本）支持的最大广播集数量为1，因此APP_BLE_CONN_CFG_TAG只能写1。
    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

//BLE事件处理函数
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code = NRF_SUCCESS;
    //判断BLE事件类型，根据事件类型执行相应操作
    switch (p_ble_evt->header.evt_id)
    {
        //断开连接事件
		case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle = BLE_CONN_HANDLE_INVALID;    
			//打印提示信息
			NRF_LOG_INFO("Disconnected.");
            break;
				
        //连接事件
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected.");
			//设置指示灯状态为连接状态，即指示灯D1常亮
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
			//保存连接句柄
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
			NRF_LOG_INFO("BLE_GAP_EVT_CONNECTED m_conn_handle : %d",m_conn_handle);
			//将连接句柄分配给排队写入实例，分配后排队写入实例和该连接关联，这样，当有多个连接的时候，通过关联不同的排队写入实例，很方便单独处理各个连接
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
            break;
				
        //PHY更新事件
        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
						//响应PHY更新规程
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;
		//安全参数请求事件
		case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            //不支持配对
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
				 
		//系统属性访问正在等待中
		case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            //系统属性没有存储，更新系统属性
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;
        //GATT客户端超时事件
        case BLE_GATTC_EVT_TIMEOUT:
            NRF_LOG_DEBUG("GATT Client Timeout.");
				    //断开当前连接
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;
				
        //GATT服务器超时事件
        case BLE_GATTS_EVT_TIMEOUT:
            NRF_LOG_DEBUG("GATT Server Timeout.");
				    //断开当前连接
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;
    }
}

//初始化BLE协议栈
static void ble_stack_init(void)
{
    ret_code_t err_code;
    //请求使能SoftDevice，该函数中会根据sdk_config.h文件中低频时钟的设置来配置低频时钟
    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);
    
    //定义保存应用程序RAM起始地址的变量
    uint32_t ram_start = 0;
	  //使用sdk_config.h文件的默认参数配置协议栈，获取应用程序RAM起始地址，保存到变量ram_start
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    //使能BLE协议栈
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    //注册BLE事件回调函数
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}
//初始化电源管理模块
static void power_management_init(void)
{
    ret_code_t err_code;
	  //初始化电源管理
    err_code = nrf_pwr_mgmt_init();
	  //检查函数返回的错误代码
    APP_ERROR_CHECK(err_code);
}

//初始化指示灯
static void leds_init(void)
{
    ret_code_t err_code;
    //初始化BSP指示灯
    err_code = bsp_init(BSP_INIT_LEDS, NULL);
    APP_ERROR_CHECK(err_code);

}
//初始化APP定时器模块
static void timers_init(void)
{
    //初始化APP定时器模块
    ret_code_t err_code = app_timer_init();
	  //检查返回值
    APP_ERROR_CHECK(err_code);

    //加入创建用户定时任务的代码，创建用户定时任务。 

}
static void log_init(void)
{
    //初始化log程序模块
	  ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
    //设置log输出终端（根据sdk_config.h中的配置设置输出终端为UART或者RTT）
    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

//空闲状态处理函数。如果没有挂起的日志操作，则睡眠直到下一个事件发生后唤醒系统
static void idle_state_handle(void)
{
    //处理挂起的log
	  if (NRF_LOG_PROCESS() == false)
    {
        //运行电源管理，该函数需要放到主循环里面执行
			  nrf_pwr_mgmt_run();
    }
}
//启动广播，该函数所用的模式必须和广播初始化中设置的广播模式一样
static void advertising_start(void)
{
   //使用广播初始化中设置的广播模式启动广播
	 ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
	 //检查函数返回的错误代码
   APP_ERROR_CHECK(err_code);
}

//主函数
int main(void)
{
	//初始化log程序模块
	log_init();
	uart_config();	//初始化串口
	timers_init();	//初始化APP定时器
	leds_init();	//出使唤按键和指示灯
	power_management_init();	//初始化电源管理
	ble_stack_init();	//初始化协议栈
	gap_params_init();	//配置GAP参数
	gatt_init();	//初始化GATT
	services_init();	//初始化服务
	advertising_init();		//初始化广播
	conn_params_init();	//连接参数协商初始化
	
	NRF_LOG_INFO("BLE Template example started.");  
	NRF_LOG_INFO("PROJECT_SW_VERSION : %s",PROJECT_SW_VERSION);  
	
	advertising_start();//启动广播
	//主循环
	while(true)
	{
		//处理挂起的LOG和运行电源管理
		idle_state_handle();
	}
}
