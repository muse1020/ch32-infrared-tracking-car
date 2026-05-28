#ifndef __HEADFILE_H
#define __HEADFILE_H

#include "stdio.h"
#include "stdint.h"
#include "string.h"

// ======================== CH32V307 SDK ========================
#include "ch32v30x.h"
#include "ch32v30x_dvp.h"
#include "ch32v30x_exti.h"
#include "ch32v30x_gpio.h"
#include "ch32v30x_rcc.h"
#include "ch32v30x_tim.h"
#include "ch32v30x_usart.h"

// ======================== Seekfree Common ========================
#include "zf_common_clock.h"
#include "zf_common_debug.h"
#include "zf_common_function.h"
#include "zf_common_interrupt.h"
#include "zf_common_fifo.h"
#include "zf_common_typedef.h"

// ======================== Seekfree Driver ========================
#include "zf_driver_delay.h"
#include "zf_driver_dvp.h"
#include "zf_driver_exti.h"
#include "zf_driver_gpio.h"
#include "zf_driver_pwm.h"
#include "zf_driver_soft_iic.h"
#include "zf_driver_timer.h"
#include "zf_driver_uart.h"

// ======================== Seekfree Device ========================
#include "zf_device_camera.h"
#include "zf_device_dl1a.h"
#include "zf_device_dm1xa.h"
#include "zf_device_gnss.h"
#include "zf_device_mt9v03x_dvp.h"
#include "zf_device_scc8660_dvp.h"
#include "zf_device_type.h"
#include "zf_device_wireless_uart.h"

// ======================== User Code ========================
#include "infrared_tracking.h"

#endif
