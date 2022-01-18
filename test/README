# TTGO boiler temperture display
This small project I wrote is used to display the boiler temperture on the [TTGO](https://www.aliexpress.com/item/33050667207.html) module.
I used a [wemos d1 mini](https://www.aliexpress.com/item/32651747570.html) flashed with [Tasmota](https://tasmota.github.io/) and two [DS1820](https://www.aliexpress.com/item/4000550061662.html) to measure the tempertue and publish it to my MQTT broker running on [Home Assistant](https://www.home-assistant.io/). 
I used this [pipe](https://www.aliexpress.com/item/32951219389.html) to insert the upper sensor to the boiler. The bottom sensor was inserted with the thermostat. You will need to also buy a 3/4 splitter.
You can use [this tutorial](https://diyprojects.io/temperature-measurement-ds18b20-arduino-code-compatible-esp8266-esp32-publication-domoticz-http/ for the connections and schematicks.

The code here is used to read the temperature from the MQTT and display it on the screen. the temperature color changes above 50 and above 70 degrees.