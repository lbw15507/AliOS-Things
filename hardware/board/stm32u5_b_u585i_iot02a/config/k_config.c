/**
  ******************************************************************************
  * @file    k_config.c
  * @author  MCU China FAE team
  * @version 1.0
  * @date    05/01/2019
  * @brief   aos porting layer
  ******************************************************************************
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
  *
  ******************************************************************************
  */

#include <k_api.h>
#include <assert.h>
#include <stdio.h>
#include <sys/time.h>

#if (RHINO_CONFIG_HW_COUNT > 0)
void soc_hw_timer_init(void)
{
}

hr_timer_t soc_hr_hw_cnt_get(void)
{
    return 0;
    //return *(volatile uint64_t *)0xc0000120;
}

lr_timer_t soc_lr_hw_cnt_get(void)
{
    return 0;
}
#endif /* RHINO_CONFIG_HW_COUNT */

#if (RHINO_CONFIG_MM_TLF > 0)

#if defined (__CC_ARM) /* Keil / armcc */

extern unsigned int Image$$RW_IRAM1$$ZI$$Limit;
extern size_t g_iram1_start;
extern size_t g_iram1_total_size;

k_mm_region_t g_mm_region[1];
int           g_region_num = 1;
void aos_heap_set()
{
    g_mm_region[0].start = (uint8_t*)&Image$$RW_IRAM1$$ZI$$Limit;
    g_mm_region[0].len   =
        (g_iram1_start + g_iram1_total_size - (size_t)&Image$$RW_IRAM1$$ZI$$Limit);
}

#elif defined (__ICCARM__)/* IAR */
int           g_region_num = 1;
k_mm_region_t g_mm_region[1];
void aos_heap_set()
{
    #pragma section="HEAP"
    uint8_t* heap_start= (uint8_t*)__section_begin("HEAP");
    uint8_t* heap_end=(uint8_t*)__section_end("HEAP");
    /* heap_start and heap_len is set by linkscript(*.ld) */
    g_mm_region[0].start = heap_start;
    g_mm_region[0].len = heap_end - heap_start;
}
#else /* GCC */
extern void         *_estack;
extern void         *__bss_end__;
/* __bss_end__ and _estack is set by linkscript(*.ld)
   heap and stack begins from __bss_end__ to _estack */
k_mm_region_t g_mm_region[1];
int           g_region_num = 1;
void aos_heap_set()
{
    g_mm_region[0].start = (uint8_t*)&__bss_end__;
    g_mm_region[0].len   =
        ((uint8_t*)&_estack - (uint8_t*)&__bss_end__) - RHINO_CONFIG_SYSTEM_STACK_SIZE;
}
#endif
#endif

#if (RHINO_CONFIG_TASK_STACK_CUR_CHECK > 0)
size_t soc_get_cur_sp()
{
    size_t sp = 0;
#if defined (__GNUC__)&&!defined(__CC_ARM)
    asm volatile(
        "mov %0,sp\n"
        :"=r"(sp));
#endif
    return sp;
}
static void soc_print_stack()
{
    void    *cur, *end;
    int      i=0;
    int     *p;

    end   = krhino_cur_task_get()->task_stack_base + krhino_cur_task_get()->stack_size;
    cur = (void *)soc_get_cur_sp();
    p = (int*)cur;
    while(p < (int*)end) {
        if(i%4==0) {
            printf("\r\n%08x:",(uint32_t)p);
        }
        printf("%08x ", *p);
        i++;
        p++;
    }
    printf("\r\n");
    return;
}
#endif

void soc_err_proc(kstat_t err)
{
    (void)err;
    printf("soc_err_proc %d \r\n", err);
    //krhino_backtrace_now();
    #if (RHINO_CONFIG_TASK_STACK_CUR_CHECK > 0)
    //soc_print_stack();
    #endif
    while(1);
}

krhino_err_proc_t g_err_proc = soc_err_proc;