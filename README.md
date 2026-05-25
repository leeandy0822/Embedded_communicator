# Embedded Linux network connection module
<img width="1033" height="534" alt="image" src="https://github.com/user-attachments/assets/be0dd4cb-a2e4-4258-8998-f39b6641392b" />

## How to use
- WiringPi (Allow LCD and GPIO function)
```bash
$ sudo apt-get install wiringpi
```
- Clone the project
```bash
$ make
$ ./test <Server IP> <PORT>
```
![Uploading image.png…]()

## Circuit 
- LCD 
    - Follow this [link](https://www.circuitbasics.com/raspberry-pi-lcd-set-up-and-programming-in-c-with-wiringpi/) with 4-bit mode

- Buttom
    - Connect WiringPi Pin 8, 9, 30, 16 to 0V
    - If the signal is not robust, please connect resistor

