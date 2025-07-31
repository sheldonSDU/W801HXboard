/***************************************************************************** 
* 
* File Name : main.c
* 
* Description: main 
* 
* Copyright (c) 2014 Winner Micro Electronic Design Co., Ltd. 
* All rights reserved. 
* 
* Author : dave
* 
* Date : 2014-6-14
*****************************************************************************/ 
#include "wm_include.h"

void UserMain(void)
{
	printf("\n user task \n");

#if DEMO_CONSOLE
	CreateDemoTask();  // 调用wm_demo_console.h和.c内的函数
#endif
//用户自己的task
}

