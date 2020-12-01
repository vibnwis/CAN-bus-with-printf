# CAN-Normal-demo-with-printf
This program does two main actions
1. to demostrate how to printf through suart3 or suart6 of STM32F407 to external terminal. Incoming CaN frames are captured through ISR, and passing up to a circular buffer for upper layer protocol to read from it.
2. to demonstrate how to configure STM32F407 CAN to transmit and receive CAN from other CAN device. Currently, the bit rate is 250kbits.
