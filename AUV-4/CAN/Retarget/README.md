# How to use printf for stm32

printf does not work out of the box for stm32. This tutorial teaches you how to enable printf via UART for debugging.

## Step 0

Make sure you have generated stm32 project using STM32CUBEMX and selected the UART peripheral intended to be used for debugging (that later can be connected to FTDI to COM port).
If you are using a nucleo board, usually UART2 will be wired to on-board St-link that you can use directly.

Also make sure the UART is configured to 115200 baud rate (or any baud rate you agreed to).


## Step 1

Copy retarget.c into Src folder and retarget.h into Inc folder

## Step 2

Disable syscalls.c in the Src folder. This is done by right click -> properties -> C/C++ Build -> Tick Exclude resource from build

## Step 3

Now just include retarget.h into main.c and add this line in your source code and you are ready to use printf functions and see the message through any serial monitor.
'''bash
RetargetInit(&huart1);
'''
