# Pulse generator

## Introduction

This is 3-channel pulse generator that generates square signals. It is made on stm32 with STM32CubeMX and Keil µVision.

## Features
  - Frequency range from less than 1 Hz up to 10 MHz
  - Configurable frequency, duty cycle and phase shift between channels
  - Mode with automatic synchronization between channels
  - Saving configuration and automatic loading it on restart

## Overview

If you change the period of the 1-st channel, then periods of other channels will automatically change to the nearest values that divide
or are divisible by the period of the 1-st channel.
But if you don't need synchronization, you can just change frequency of 2-nd or 3-rd channel and it will not affect other channels.

Also it memorizes it's configuration and loads it on restart.

This version is built on the board stm32f103c8t6 and I used the ST-LINK programmer to install the code.
It has no display and output signals are configured by 6 buttons, rotary encoder and 3 LEDs.

<img width="640" height="376" alt="image" src="https://github.com/user-attachments/assets/e4d9183a-3f88-47c0-95f7-8313c68afeb9" />

<img width="282" height="376" alt="image" src="https://github.com/user-attachments/assets/ab546179-64d7-48fe-b77c-7bf93c429e72" />

## License

[MIT](LICENSE) © Space4444
