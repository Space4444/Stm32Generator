# Pulse generator

## Introduction

This is 3-channel pulse generator that generates square signals. It is made on stm32 with STM32CubeMX and Keil µVision.
This version is built on the board stm32f103c8t6 and ST-LINK V2 programmer was used to install the code.
Output signals are amplified by low-to-high voltage level shifter.

## [Demo video](https://youtu.be/u5IdKObk2uI)

## Features
  - Frequency range from less than 1 Hz up to 10 MHz
  - Configurable frequency, duty cycle and phase shift between channels
  - Mode with automatic synchronization between channels
  - Saving configuration and automatic loading it on restart

## Installation

At the time when I built this project, I used Keil µVision to compile and to load the program in the board.
But now whean I tried to replicate the process, I for some reason get error about device mismatch.
Then I figured out another approach:

### If you want to just load existing compiled project

1. Download and install [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html). The installer should also install drivers for ST-LINK.
2. Open the program and click "Open file". Then find and select file "Stm32Generator.hex" (You have to download it from the "/MDK-ARM/Stm32Generator" in this repo)
3. Click "Download". Then connection with ST-LINK should establish and the compiled file should load to the board, thats it.

### If you want to modify code

1. Download and install [Keil](https://www.keil.com/demo/eval/arm.htm).
2. Download this repo and open "Stm32Generator.uvprojx" with Keil.
3. In the Keil press Alt+F7 and "Options for target" window should pop up.
4. There in the "Target" tab find option "ARM Compiler" and choose default compiler.
5. Press F7 to build the project, then the file "Stm32Generator.hex" should update.
6. Repeat the process shoun above to load this file in the board.

## Usage

Output signals are configured using a rotary encoder and 6 buttons (see sircuit diagram).

By pressing on buttons B1, B2 and B3 you can turn on and off channels.
LEDs D1, D2 and D3 incicate whether the channels are on or off. (Button B1 and LED L1 correspond to channel 1, etc)

By pressing on button B6 you can select the channel to configure. Button B5 switches signal parameter that is currently configured
(frequency, duty cycle or phase shift). And button B4 changes the step of parameter change when rotating the encoder.

The configuration is saving every 30 seconds if any parameter was changed.

### Channel synchronization
If you change the period of the 1-st channel, then periods of other channels will automatically change to the nearest values that divide
or are divisible by the period of the 1-st channel.
But if you don't need synchronization, you can just change frequency of 2-nd or 3-rd channel and it will not affect other channels.

<img width="640" height="376" alt="image" src="https://github.com/user-attachments/assets/fcf75946-8241-4893-ab0c-d1cbfcb70374" />

<img width="282" height="376" alt="image" src="https://github.com/user-attachments/assets/ab546179-64d7-48fe-b77c-7bf93c429e72" />

## License

[MIT](LICENSE) © Space4444
