# Microcontroller Practices SIC 2026

Repository containing all practical exercises for the Microcontroller course at USTH.

## Structure

Organized into separate folders for each practical exercise:

* Practice 1
* Practice 2
* Practice 4
* Practice 5
* Practice 6
* Practice 7
* Practice 8


## Hardware and Software

### Hardware
* ATmega328P microcontroller — the actual chip: 8-bit AVR core, 32KB flash, timers, ADC, SPI/I2C/UART, programmed at the register level
* Arduino-compatible development board — the carrier board hosting the chip: USB-to-serial for programming, and header pinout access (a hardware convenience, not a software framework)

### Software
* PlatformIO as the build/upload toolchain
* Pure AVR-C, compiled directly against ATmega328P registers/datasheet — no Arduino framework or libraries, unless a specific practice notes otherwise
