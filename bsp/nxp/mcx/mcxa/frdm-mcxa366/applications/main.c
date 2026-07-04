/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-10-24     Magicoe      first version
 * 2020-01-10     Kevin/Karl   Add PS demo
 * 2020-09-21     supperthomas fix the main.c
 * 2025-08-18     Alex Yang    Add P1_7 button with LED blink control
 *
 */

#include <rtdevice.h>
#include "drv_pin.h"
#include <rtthread.h>
#define LED_PIN        ((3*32)+18)  /* Original LED pin */
#define BUTTON_PIN     ((1*32)+7)   /* P1_7 button pin */

static rt_bool_t led_state = RT_FALSE;        /* Current LED state */

#define SAMPLE_UART1_NAME    "uart3"                /* 串口设备名称 */
static rt_device_t    serial_uart3;                /* 串口设备句柄 */
static struct rt_semaphore rx_sem;      /* 用于接收消息的信号量 */
struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;  /* 初始化配置参数 */

/* 接收数据回调函数 */
static rt_err_t uart_input(rt_device_t dev,rt_size_t size)//接收数据回调（消息队列处理）函数
{
    rt_sem_release(&rx_sem);//串口接收数据产生中断，调用该函数发送接收信号量
    return RT_EOK;
}

/* 串口处理线程函数 */
static void serial_thread_entry(void* parameter)
{
    char ch;
		rt_kprintf("serial task run\n");
    /* 接收到信号量后处理串口数据 */
    while(1)
    {
        while(rt_device_read(serial_uart3,-1,&ch,1)!=1)//阻塞等待接收信号量，等到信号量后读取1个字节的数据
            rt_sem_take(&rx_sem,RT_WAITING_FOREVER);
            rt_device_write(serial_uart3,0,&ch,1);//读取到的数据通过串口错位输出 
    }
}

/* 串口主函数调用 */
static int uart_int_sample(void)
{
    rt_err_t ret=RT_EOK;
    char uart_name[RT_NAME_MAX];
    char str[] = "RT-Thread MCX A366 uart3 Start!\n";
    rt_strncpy(uart_name,SAMPLE_UART1_NAME,RT_NAME_MAX);
    serial_uart3=rt_device_find(uart_name);//查找串口设备
    if(!serial_uart3)
    {
        rt_kprintf("find %s failed!\n",uart_name);
        return RT_ERROR;
    }
    rt_kprintf("start mq\n");
    rt_device_open(serial_uart3,RT_DEVICE_FLAG_INT_RX);//以轮询方式打开串口
    rt_sem_init(&rx_sem,"rx_sem",0,RT_IPC_FLAG_FIFO);//初始化信号量
    rt_device_set_rx_indicate(serial_uart3,uart_input);//设置接收回调（消息队列处理）函数
    rt_device_write(serial_uart3,0,str,(sizeof(str)-1));//发送字符串
    rt_thread_t thread=rt_thread_create("serial",serial_thread_entry,RT_NULL,1024,25,10);//创建串口处理线程
    if(thread!=RT_NULL)
    {
        rt_kprintf("start thread\n");
        rt_thread_startup(thread);
    }
    else
        ret=RT_ERROR;
    return ret;//返回结果
}


/* Button interrupt callback function */
void button_irq_callback(void *args)
{
    rt_kprintf("SW2 pressed\n");
}

int main(void)
{
#if defined(__CC_ARM)
    rt_kprintf("using armcc, version: %d\n", __ARMCC_VERSION);
#elif defined(__clang__)
    rt_kprintf("using armclang, version: %d\n", __ARMCC_VERSION);
#elif defined(__ICCARM__)
    rt_kprintf("using iccarm, version: %d\n", __VER__);
#elif defined(__GNUC__)
    rt_kprintf("using gcc, version: %d.%d\n", __GNUC__, __GNUC_MINOR__);
#endif

    rt_kprintf("FRDM-MCXA366\r\n");

    /* Configure LED pin as output */
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(LED_PIN, PIN_LOW);

    /* Configure button pin as input with pull-up */
    rt_pin_mode(BUTTON_PIN, PIN_MODE_INPUT_PULLUP);
	
		uart_int_sample();

    /* Attach interrupt to button pin */
    rt_pin_attach_irq(BUTTON_PIN, PIN_IRQ_MODE_FALLING, button_irq_callback, RT_NULL);
    rt_pin_irq_enable(BUTTON_PIN, PIN_IRQ_ENABLE);

    while (1)
    {
        /* Toggle LED state */
        led_state = !led_state;

        rt_pin_write(LED_PIN, led_state ? PIN_HIGH : PIN_LOW);

        rt_thread_mdelay(500);
    }
}

void cmd_reboot(void)
{
    rt_hw_cpu_reset();
}
MSH_CMD_EXPORT_ALIAS(cmd_reboot, reboot, Reboot);
