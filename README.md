# DigiSlate+

The project is in an early developement state.

DigiSlate+ is inspired from the [DigiSlate](https://www.youtube.com/watch?v=TnaWQZtmPek) and the DIY timecode generator from [mitkunz](https://github.com/mitkunz/diy_timecode_generator). The aim for the DigiSlate+ is not only to be able to read and display timecode, but also to generate it itself. To make this possible, the hardware is extended by a real time clock with high running accuracy. The RTC module DS3231 with temperature-compensated quartz offers the possibility.

The DigiSlate+ has two operating states. If an external time code is fed in and the slate is switched on, it starts in read mode. The time code is read, shown on the LED display and the time in the RTC is updated. The DS3231 sends an interrupt to the controller at the beginning of each second. The offset of frames to the time code fed in is stored in the EEPROM memory in order to provide a frame-accurate time in free-running mode.

As soon as the external time code is disconnected or the slate is switched on without a time code at the input, it changes to free-running mode. It now runs automatically with the time from the RTC and the saved frame offset. The accuracy of the RTC module allows a deviation of 1 frame in 8 hours.

A socond row of 7-segment LEDs display the scene and take numbers.