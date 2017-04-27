# Arduino Digital Thermometer

A simple digital thermometer built for arduino.  This was my first ever arduino project and I had a lot of fun putting it together.

It reads temperature from the DS18B20 temperature probe and outputs it to a 5x7 LED dot matrix (LTP-747E).  If temperature rises or falls outside of a preset range (20-25 C), indicitor LEDs are lit up.

### Components Used
 * Arduino UNO
 * DS18B20 based temperature probe (https://www.sparkfun.com/products/11050)
 * 4.7k resistor
 * LTP-747E 5x7 LED dot matrix
 * breadboard + wires
 * 3x LED + 1k resistor (could use way less)


### Future Enhancements
 * Add hardware inputs (buttons/potentiometer) to allow adjustment of target range
 * Connect to a power tail to allow household-voltage appliances to be turned on/off (way more useful than LEDs!)

### *Wiring diagram coming soon!*
