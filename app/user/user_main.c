/******************************************************************************
 * PV` FileName: user_main.c
 *******************************************************************************/

#include "user_config.h"
#include "bios.h"
#include "sdk/add_func.h"
#include "hw/esp8266.h"
#include "user_interface.h"
#include "tcp_srv_conn.h"
#include "flash_eep.h"
#include "wifi.h"
#include "hw/spi_register.h"
#include "sdk/rom2ram.h"
#include "web_iohw.h"
#include "tcp2uart.h"
#include "webfs.h"
#include "sdk/libmain.h"
#include "driver/i2c_eeprom.h"

#ifdef USE_WEB
#include "web_srv.h"
#endif

#ifdef UDP_TEST_PORT
#include "udp_test_port.h"
#endif

#ifdef USE_NETBIOS
#include "netbios.h"
#endif

#ifdef USE_SNTP
#include "sntp.h"
#endif

#ifdef USE_WDRV
#include "driver/wdrv.h"
#endif

//#ifdef USE_MODBUS
//#include "modbustcp.h"
#ifdef USE_RS485DRV
#include "driver/rs485drv.h"
#include "mdbtab.h"
#endif
//#endif

#include "power_meter.h"

#ifdef USE_WEB
extern void web_fini(const uint8 * fname);
static const uint8 sysinifname[] ICACHE_RODATA_ATTR = "protect/init.ini";
#endif

void ICACHE_FLASH_ATTR init_done_cb(void)
{
#if DEBUGSOO > 0
	os_printf("\nSDK Init - Ok\nCurrent 'heap' size: %d bytes\n", system_get_free_heap_size());
	os_printf("Flash ID: %08x, size: %u\n", spi_flash_get_id(), spi_flash_real_size());
    os_printf("Current config size: %d bytes\n", current_cfg_length());

	struct ets_store_wifi_hdr whd;
	spi_flash_read(((flashchip->chip_size/flashchip->sector_size)-1)*flashchip->sector_size, &whd, sizeof(whd));
	os_printf("Last sectors rewrite count: %u\n\n", whd.wr_cnt);
#endif
	//
	power_meter_init(3); // init timer/tasks
	ets_set_idle_cb(user_idle, NULL); // do not use sleep mode!
	//
#ifdef USE_WEB
	web_fini(sysinifname);
#endif
	switch(system_get_rst_info()->reason) {
	case REASON_SOFT_RESTART:
	case REASON_DEEP_SLEEP_AWAKE:
		break;
	default:
		New_WiFi_config(WIFI_MASK_ALL);
		break;
	}
#ifdef USE_RS485DRV
	rs485_drv_start();
	init_mdbtab();
#endif
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/
void ICACHE_FLASH_ATTR user_init(void) {
	sys_read_cfg();
	if(!syscfg.cfg.b.debug_print_enable) system_set_os_print(0);
//	GPIO0_MUX = VAL_MUX_GPIO0_SDK_DEF;
//	GPIO4_MUX = VAL_MUX_GPIO4_SDK_DEF;
//	GPIO5_MUX = VAL_MUX_GPIO5_SDK_DEF;
//	GPIO12_MUX = VAL_MUX_GPIO12_SDK_DEF;
//	GPIO13_MUX = VAL_MUX_GPIO13_SDK_DEF;
//	GPIO14_MUX = VAL_MUX_GPIO14_SDK_DEF;
//	GPIO15_MUX = VAL_MUX_GPIO15_SDK_DEF;
	// vad7
	//power_meter_init();
	//
	uart_init();
	system_timer_reinit();
#if (DEBUGSOO > 0 && defined(USE_WEB))
	os_printf("\nSimple WEB version: " WEB_SVERSION "\nOpenLoaderSDK v1.2\n");
#endif
	//if(syscfg.cfg.b.pin_clear_cfg_enable) test_pin_clr_wifi_config(); // сброс настроек, если замкнут пин RX
	set_cpu_clk(); // select cpu frequency 80 or 160 MHz
#ifdef USE_GDBSTUB
extern void gdbstub_init(void);
	gdbstub_init();
#endif
#if DEBUGSOO > 0
	if(eraminfo.size > 0) os_printf("Found free IRAM: base: %p, size: %d bytes\n", eraminfo.base,  eraminfo.size);
	os_printf("System memory:\n");
    system_print_meminfo();
    os_printf("Start 'heap' size: %d bytes\n", system_get_free_heap_size());
#endif
#if DEBUGSOO > 0
	os_printf("Set CPU CLK: %u MHz\n", ets_get_cpu_frequency());
#endif
	Setup_WiFi();
#ifdef USE_WDRV
    init_wdrv();
#endif

    WEBFSInit(); // файловая система

	system_deep_sleep_set_option(0);
	system_init_done_cb(init_done_cb);
}
