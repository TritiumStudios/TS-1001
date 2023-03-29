# TS-1001

## Programming

To program the TS-1001, first connect the programming pins to a USB to Serial converter as shown below.
```
Note: Do not connect 3.3V to the programming pin if the power cable is connected!

  GND   DTR   TX   RX  RTS  3.3V
 ________________________________
|                            _   |
|   O    O    O    O    O   |O|  |
|____________________________*___|

```

Next, connect the USB to Serial Converter to your PC, open the Arduino IDE, and load `TS-1001.ino`.

Install the following boards.

- [ESP32 Arduino](https://randomnerdtutorials.com/installing-esp32-arduino-ide-2-0/)

Install the following libraries. 

- [Adafruit Neopixel](https://github.com/adafruit/Adafruit_NeoPixel)

Choose the ESP32 Dev Module as your board and choose the port of your USB to Serial converter.

Now you're ready to compile and flash! 🙌
