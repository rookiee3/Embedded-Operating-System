/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                          (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
*                                               EXAMPLE #2
*********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
*                                              定义常量
*********************************************************************************************************
*/

#define          TASK_STK_SIZE     512                /* Size of each task's stacks (# of WORDs)       */

#define          TASK_START_ID       0                /* Application tasks IDs                         */
#define          TASK_CLK_ID         1
#define          TASK_1_ID           2
#define          TASK_2_ID           3
#define          TASK_3_ID           4
#define          TASK_4_ID           5
#define          TASK_5_ID           6

#define          TASK_START_PRIO    10                /* Application tasks priorities                  */
#define          TASK_CLK_PRIO      11
#define          TASK_1_PRIO        12
#define          TASK_2_PRIO        13
#define          TASK_3_PRIO        14
#define          TASK_4_PRIO        15
#define          TASK_5_PRIO        16

/*
*********************************************************************************************************
*                                              定义变量
*********************************************************************************************************
*/

OS_STK        TaskStartStk[TASK_STK_SIZE];            /* Startup    task stack                         */
OS_STK        TaskClkStk[TASK_STK_SIZE];              /* Clock      task stack                         */
OS_STK        Task1Stk[TASK_STK_SIZE];                /* Task #1    task stack                         */
OS_STK        Task2Stk[TASK_STK_SIZE];                /* Task #2    task stack                         */
OS_STK        Task3Stk[TASK_STK_SIZE];                /* Task #3    task stack                         */
OS_STK        Task4Stk[TASK_STK_SIZE];                /* Task #4    task stack                         */
OS_STK        Task5Stk[TASK_STK_SIZE];                /* Task #5    task stack                         */

OS_EVENT     *AckMbox;                                /* Message mailboxes for Tasks #4 and #5         */
OS_EVENT     *TxMbox;

/*
*********************************************************************************************************
*                                              函数声明
*********************************************************************************************************
*/

        void  TaskStart(void *data);                  /* Function prototypes of tasks                  */
static  void  TaskStartCreateTasks(void);
static  void  TaskStartDispInit(void);
static  void  TaskStartDisp(void);
        void  TaskClk(void *data);
        void  Task1(void *data);
        void  Task2(void *data);
        void  Task3(void *data);
        void  Task4(void *data);
        void  Task5(void *data);

/*$PAGE*/
/*
*********************************************************************************************************
*                                              主函数
*********************************************************************************************************
*/

void main (void)
{
    OS_STK *ptos;
    OS_STK *pbos;
    INT32U  size;


    PC_DispClrScr(DISP_FGND_WHITE);                        /* Clear the screen                         */

    OSInit();                                              /* Initialize uC/OS-II                      */

    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */

    PC_ElapsedInit();                                      /* Initialized elapsed time measurement     */

    ptos        = &TaskStartStk[TASK_STK_SIZE - 1];        /* TaskStart() will use Floating-Point      */
    pbos        = &TaskStartStk[0];
    size        = TASK_STK_SIZE;
    OSTaskStkInit_FPE_x86(&ptos, &pbos, &size);            
    OSTaskCreateExt(TaskStart,
                   (void *)0,
                   ptos,
                   TASK_START_PRIO,
                   TASK_START_ID,
                   pbos,
                   size,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSStart();                                             /* Start multitasking                       */
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                               启动任务
*********************************************************************************************************
*/

void  TaskStart (void *pdata)
{
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
#endif
    INT16S     key;


    pdata = pdata;                                         /* Prevent compiler warning                 */

    TaskStartDispInit();                                   /* Setup the display                        */

    OS_ENTER_CRITICAL();                                   /* Install uC/OS-II's clock tick ISR        */
    PC_VectSet(0x08, OSTickISR);
    PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    OS_EXIT_CRITICAL();

    OSStatInit();                                          /* Initialize uC/OS-II's statistics         */

    AckMbox = OSMboxCreate((void *)0);                     /* Create 2 message mailboxes               */
    TxMbox  = OSMboxCreate((void *)0);

    TaskStartCreateTasks();                                /* Create all other tasks                   */

    for (;;) {
        TaskStartDisp();                                   /* Update the display                       */

        if (PC_GetKey(&key)) {                             /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                PC_DOSReturn();                            /* Yes, return to DOS                       */
            }
        }

        OSCtxSwCtr = 0;                                    /* Clear context switch counter             */
        OSTimeDly(OS_TICKS_PER_SEC);                       /* Wait one second                          */
    }
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                              初始化显示
*********************************************************************************************************
*/

static  void  TaskStartDispInit (void)
{
/*                                1111111111222222222233333333334444444444555555555566666666667777777777 */
/*                      01234567890123456789012345678901234567890123456789012345678901234567890123456789 */
    PC_DispStr( 0,  0, "                         uC/OS-II, The Real-Time Kernel                         ", DISP_FGND_WHITE + DISP_BGND_RED + DISP_BLINK);
    PC_DispStr( 0,  1, "                                Jean J. Labrosse                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  2, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  3, "                                    EXAMPLE #2                                  ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  4, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  5, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  6, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  7, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  8, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  9, "Task           Total Stack  Free Stack  Used Stack  ExecTime (uS)               ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 10, "-------------  -----------  ----------  ----------  -------------               ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 11, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 12, "TaskStart():                                                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 13, "TaskClk()  :                                                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 14, "Task1()    :                                                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 15, "Task2()    :                                                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 16, "Task3()    :                                                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 17, "Task4()    :                                                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 18, "Task5()    :                                                                    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 19, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 20, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 21, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 22, "#Tasks          :        CPU Usage:     %                                       ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 23, "#Task switch/sec:                                                               ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 24, "                            <-PRESS 'ESC' TO QUIT->                             ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY + DISP_BLINK);
/*                                1111111111222222222233333333334444444444555555555566666666667777777777 */
/*                      01234567890123456789012345678901234567890123456789012345678901234567890123456789 */
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                              刷新显示
*********************************************************************************************************
*/

static  void  TaskStartDisp (void)
{
    char   s[80];


    sprintf(s, "%5d", OSTaskCtr);                                  /* Display #tasks running               */
    PC_DispStr(18, 22, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    sprintf(s, "%3d", OSCPUUsage);                                 /* Display CPU usage in %               */
    PC_DispStr(36, 22, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    sprintf(s, "%5d", OSCtxSwCtr);                                 /* Display #context switches per second */
    PC_DispStr(18, 23, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    sprintf(s, "V%4.2f", (float)OSVersion() * 0.01);               /* Display uC/OS-II's version number    */
    PC_DispStr(75, 24, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    switch (_8087) {                                               /* Display whether FPU present          */
        case 0:
             PC_DispStr(71, 22, " NO  FPU ", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;

        case 1:
             PC_DispStr(71, 22, " 8087 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;

        case 2:
             PC_DispStr(71, 22, "80287 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;

        case 3:
             PC_DispStr(71, 22, "80387 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                              创建任务
*********************************************************************************************************
*/

static  void  TaskStartCreateTasks (void)
{
    OSTaskCreateExt(TaskClk,
                   (void *)0,
                   &TaskClkStk[TASK_STK_SIZE - 1],
                   TASK_CLK_PRIO,
                   TASK_CLK_ID,
                   &TaskClkStk[0],
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt(Task1,
                   (void *)0,
                   &Task1Stk[TASK_STK_SIZE - 1],
                   TASK_1_PRIO,
                   TASK_1_ID,
                   &Task1Stk[0],
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt(Task2,
                   (void *)0,
                   &Task2Stk[TASK_STK_SIZE - 1],
                   TASK_2_PRIO,
                   TASK_2_ID,
                   &Task2Stk[0],
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt(Task3,
                   (void *)0,
                   &Task3Stk[TASK_STK_SIZE - 1],
                   TASK_3_PRIO,
                   TASK_3_ID,
                   &Task3Stk[0],
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt(Task4,
                   (void *)0,
                   &Task4Stk[TASK_STK_SIZE-1],
                   TASK_4_PRIO,
                   TASK_4_ID,
                   &Task4Stk[0],
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSTaskCreateExt(Task5,
                   (void *)0,
                   &Task5Stk[TASK_STK_SIZE-1],
                   TASK_5_PRIO,
                   TASK_5_ID,
                   &Task5Stk[0],
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                               TASK #1
*
* Description: This task executes every 100 mS and measures the time it task to perform stack checking
*              for each of the 5 application tasks.  Also, this task displays the statistics related to
*              each task's stack usage.
*********************************************************************************************************
*/
//这段代码的主要作用是循环遍历一组任务，并获取每个任务的栈使用情况并显示在屏幕上。每次循环会延时一段时间（100毫秒），然后继续下一次循环。
void  Task1 (void *pdata)
{
    INT8U    err;
    OS_STK_DATA  data;//栈使用情况
    INT16U  time;//task1执行所需的时间
    INT8U   i;
    char  s[80];

    pdata = pdata;
    for(;;)
    {
       for(i = 0;i < 7;i ++)
       {
           PC_ElapsedStart();//开始计时
           err = OSTaskStkChk(TASK_START_PRIO+i,&data);//获取栈的使用情况
           time = PC_ElapsedStop();//停止计时并获取经过的时间
           //如果task1执行时没有错误
           if(err == OS_NO_ERR)
           {
              sprintf(s,"%4ld    %4ld    %4ld    %6d",data.OSFree + data.OSUsed,data.OSFree,data.OSUsed,time);
              //data.OSFree + data.OSUsed 是一个长整型的表达式，表示任务栈的总使用量。
              //data.OSFree 是长整型变量，表示任务栈的剩余空间。
              //data.OSUsed 是长整型变量，表示任务栈的已使用空间。
              //time 是一个整数变量，表示任务执行所需的时间。
              PC_DispStr(19,12+i,s,DISP_FGND_BLACK+DISP_BGND_LIGHT_GRAY);//屏幕上显示信息
           }
           OSTimeDlyHMSM(0,0,0,100);
       }
    }


}
/*$PAGE*/
/*
*********************************************************************************************************
*                                               TASK #2
*
* Description: This task displays a clockwise rotating wheel on the screen.
*********************************************************************************************************
*/
//显示顺时针旋转
void  Task2 (void *data)
{
    data = data;
    for(;;)
    {
        PC_DispChar(70,15,'|',DISP_FGND_YELLOW+DISP_BGND_BLUE);
        OSTimeDly(20);//延时20个时钟节拍 70和15表示在屏幕上显示的坐标
        PC_DispChar(70,15,'/',DISP_FGND_YELLOW+DISP_BGND_BLUE);
        OSTimeDly(20);
        PC_DispChar(70,15,'-',DISP_FGND_YELLOW+DISP_BGND_BLUE);
        OSTimeDly(20);
        PC_DispChar(70,15,'\\',DISP_FGND_YELLOW+DISP_BGND_BLUE);//“\\”代表一个反斜杠
        OSTimeDly(20);


    }

}
/*$PAGE*/
/*
*********************************************************************************************************
*                                               TASK #3
*
* Description: This task displays a counter-clockwise rotating wheel on the screen.
*
* Note(s)    : I allocated 500 bytes of storage on the stack to artificially 'eat' up stack space.
*********************************************************************************************************
*/

void  Task3 (void *data)
{
    char  dummy[500];
    INT16U i;
    data = data;
    for(i=0;i<499;i++)
    {
       dummy[i] = '?';
    }
    for(;;)
    {
        PC_DispChar(70,16,'|',DISP_FGND_YELLOW+DISP_BGND_BLUE);
        OSTimeDly(20);
        PC_DispChar(70,16,'\\',DISP_FGND_YELLOW+DISP_BGND_BLUE);
        OSTimeDly(20);
        PC_DispChar(70,16,'-',DISP_FGND_YELLOW+DISP_BGND_BLUE);
        OSTimeDly(20);
        PC_DispChar(70,16,'/',DISP_FGND_YELLOW+DISP_BGND_BLUE);
        OSTimeDly(20);
    }

}
/*$PAGE*/
/*
*********************************************************************************************************
*                                               TASK #4
*
* Description: This task sends a message to Task #5.  The message consist of a character that needs to
*              be displayed by Task #5.  This task then waits for an acknowledgement from Task #5
*              indicating that the message has been displayed.
*********************************************************************************************************
*/

void  Task4 (void *data)
{
    char txmsg;//字符变量 用于发送的信息
    INT8U err;//存储错误码

    data = data;
    txmsg = 'A';//初始化为A
    for(;;)
    {
        OSMboxPost(TxMbox,(void*)&txmsg);//向消息邮箱发送信息txmsg
        OSMboxPend(AckMbox,0,&err);// 阻塞等待 AckMbox 的消息到达
        txmsg++;
        if(txmsg == 'Z')
        {
            txmsg = 'A';
        }
    }

}
/*$PAGE*/
/*
*********************************************************************************************************
*                                               TASK #5
*
* Description: This task displays messages sent by Task #4.  When the message is displayed, Task #5
*              acknowledges Task #4.
*********************************************************************************************************
*/

void  Task5 (void *data)
{
     char *rxmsg;//字符指针 用于接收变量
     INT8U  err;//存储错误码

     data = data;
     for(;;)
     {
         rxmsg = (char*) OSMboxPend(TxMbox,0,&err);// 阻塞等待 TxMbox 的消息到达，并将其赋值给 rxmsg
         PC_DispChar(70,18,*rxmsg,DISP_FGND_YELLOW+DISP_BGND_BLUE);//屏幕上显示信息
         OSTimeDlyHMSM(0,0,1,0);//延时一秒
         OSMboxPost(AckMbox,(void*)1);//向ackmbox发送信息表示已经收到
     }

}
/*$PAGE*/
/*
*********************************************************************************************************
*                                               CLOCK TASK
*********************************************************************************************************
*/

void  TaskClk (void *data)
{
    char s[40];


    data = data;
    for (;;) {
        PC_GetDateTime(s);
        PC_DispStr(60, 23, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);
        OSTimeDly(OS_TICKS_PER_SEC);
    }
}

