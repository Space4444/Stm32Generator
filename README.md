# Pulse gererator

This is 3-channel pulse generator made on stm32 with STM32CubeMX and Keil µVision. It generates square pulses with configurable frequency,
duty cycle and phase shift between channels. Frequency ranges from less than 1 Hz up to 36 MHz.

It provides automatic synchronization between channels.
If you change the period of the 1-st channel, then periods of other channels will automatically change to the nearest values that divide period of the 1-st channel.
But if you don't need synchronization, you can just change frequency of 2-nd or 3-rd channel and it will not affect other channels.

Also it memorizes it's configuration and loads it on restart.

This version is built on the board stm32f103c8t6. It has no display and output signals are configured by 3 buttons, rotary encoder and 3 LEDs.

<img width="960" height="1280" alt="image" src="https://github.com/user-attachments/assets/ab546179-64d7-48fe-b77c-7bf93c429e72" />
