/*  connections for the Teensy 3.5
      euclidean sequencer usbMIDI

FOR THIS PROJECT WIRING - ONLY USE THE 5V TOLERANT PINS, 
ie, DIGITAL ONLY, NOT the ANALOG PINS

For Euclidean Eurorack version:

connected to - pin ---- pin - connected to
------------------------------------------
all enc /etc - Gnd -----VIN - all enc /etc (T3.5 is 5v tolerant)
Save fr MIDI ---0 ----- A-GND -
rot1 red    ----1 ------3.3v -
rot1 green    --2 --------23 - button4
rot1 blue    ---3 --------22 - button3
rot2 red     ---4 --------21 - button2
rot2 green   ---5 --------20 - button1
rot2 blue  -----6 --------19 - Screen SCL i2c
rot3 red  ------7 --------18 - Screen SDA i2c
rot3 green   ---8 --------17 - enc  1R
rot3 blue  - ---9 --------16 - enc 1L
neopix rings  -10 -------15 - enc 1SW
output Gate1 - 11 -------14 - enc 3R
output inv1--- 12 -------13 - enc 3L
             - 3.3v ----GND -
output Gate2 - 24 -----DAC1 - (analogue only)
output Inv2 -- 25 -----DAC0 - (analogue only)
output Gate3  -26   -----39 - enc 3sw
output Inv3 ---27 -------38 - enc 2L
output Gate4 --28 -------37 - enc 2R
output Inv4 -- 29 -------36 - enc 2SW
rot4 red   --- 30 -------35 - enc 4R
rot4 green --- 31 -------34 - enc 4L
rot4 blue   -- 32 -------33 - enc 4SW

pin 49 underneath =  clock out
<<<<<<<<<<<<<<<<<<<<<<<<<<<

//---------pins note------------//
//save teensy pins 18 and 19 for i2c ssd1306 OLED

//------------neopixel note---------------------//
//note, as neopixel rings are wired in anticlockwise fashion, need to reverse the 
//call to leds to move the pixel along in
// a clockwise fashion. Therefore starting led = 15 and not zero
// therefore arrange rings to have pixel 15 at top.
//The euclid funciton gives a set of sixteen bits for on or off for each step
//this needs to be read in reverse, 


//---------------clock divider values ---------------------// 
/* a "beat" in terms of tempo is a quarter note 
* a quarter note is always 1 beat when talking about tempo, regardless of the time signature.
* there are 24 MIDI Clocks in every quarter note
* therefore a divider of 24 would give the correct beats tempo per minute
* 12 is twice as fast, so a beat plays each 1/8th note
* 48 plays each 1/2 note
* 24 pulses per 1/4 note = 24*4 = 96 pulses for a note
*/
