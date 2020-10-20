// Sept 2020 - 
//renamed as Euclidean_Sequencer_Eurorack_1020
//done - changing cv out to inverted gate out
//done - changing MIDI to midi clock in via DIN
//done - changing pins for the inv gate outs
//done - change pin allocation as rx tx1 on pins 0 and 1 to be used.
//done - change diplay lib to U8g2lib - reduced latency by partial screen update
//done - TODO invert, send on for off point and off on the hit pattern
//done - TODO - remove all notes, as CV clock and MIDI clock only in the Eurorack version, no notes.
//done - change note on length by using step number instead of millis


//-----updating screen to avoid latency--

/*
 * OLEDssd1306.h - container for setting out functions for the OLED using the 
 * U8g2lib library
 * 
 * https://github.com/olikraus/u8g2/wiki/u8g2reference
 * 
 * screen size is 128 X 64 pixels
 * = 16 tiles wide (128 / 8) and 8 tiles high (64 / 8)
 */
/*
 * list of modes
 * light Blue - note on time, single value per channel
 * Green - clock Divider, per channel
 * dark Blue - looped steps number, per channel 
 * Red - select hits on/off, per channel
 * Yellow - offset, per channel
 * Purple:
 * ch1 - memory slot
 * not used - ch2 - scale (C,D,E,etc)
 * not used - ch3 - scale (major, minor, pent, 1note)
 * not used - ch4 - BPM
 * White Ch1 - internal BPM
 */

///////////////

#include "RGB_Rot_Enc.h"
#include <MIDI.h>

//use include guard
#ifndef U8g2lib
#define U8g2lib  //dont forget endif at bottom of tab 


#include <WS2812Serial.h>//use nonblocking ws2812 serial lib to avoid blocking of ext clk MIDI input
#include "memory.h" //this brings in the sd card for storing patches
#include "euclid.h" // brings in the note / scale / arp values


//for DIN connection MIDI out
//MIDI_CREATE_DEFAULT_INSTANCE();
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

//-------add a micros() clock so that we dont need to ue interrupts or timer3

unsigned long newTime; 
unsigned long oldTime; 
unsigned long INTERVAL;//micros

//------------------------MIDI external Clock section----------------//
boolean BPMExternal = 1;
int extCount;
/*
 * http://little-scale.blogspot.com/2008/05/how-to-deal-with-midi-clock-signals-in.html
 */
byte midi_start = 0xfa;
byte midi_stop = 0xfc;
byte midi_clock = 248;//0xf8;
byte midi_continue = 0xfb;
int play_flag = 0;
byte data;

int midiDivCounter=0;
int midiDiv =1; //option for future change in resolution

//----------------------for on/off buttons--------------//
int buttonPin[4] ={20,21,22,23};
boolean currentState[4] ={LOW,LOW,LOW,LOW};
boolean lastState[4] ={LOW,LOW,LOW,LOW};
boolean switchState[4] ={LOW,LOW,LOW,LOW};

//------------------------for leds------------------------//
int positionLed [4] ={15,31,47,63};
const int numled = 64;
const int pin = 10;
byte drawingMemory[numled*3];         //  3 bytes per LED
DMAMEM byte displayMemory[numled*12]; // 12 bytes per LED

WS2812Serial leds(numled, displayMemory, drawingMemory, pin, WS2812_GRB);
//less intense brightness//
#define RED    0x050000
#define GREEN  0x000500
#define BLUE   0x000009
#define YELLOW 0x101400
#define PINK   0x120009
#define ORANGE 0x100400
#define WHITE2  0x00001 //so that it does not clash with oled WHITE
#define ALLOFF 0x000000
/*
#define RED    0xFF0000
#define GREEN  0x00FF00
#define BLUE   0x0000FF
#define YELLOW 0xFFFF00
#define PINK   0xFF1088
#define ORANGE 0xE05800
#define WHITE  0xFFFFFF
*/

//------------------------------for midi and beat timing-------------------------------//
String screenDIV[6] ={"1","2","4","8","16","32"};
int divRatio[6] = {96,48,24,12,6,3};//note multipliers
int channelDiv[5] = {2,2,2,2,2};//initialise each channel at 24 div


//---------Clock-------------//
byte commandByte;
byte noteByte;
byte velocityByte;
byte inClock;
int clockState =0;
int bpm =120;
const byte CLOCK = 0xF8; //=midi beat clock message (or 248 decimal) for usb

boolean masterClock = true; //true for using teensy usbmidi clock

volatile float midIntervall;// = (1000.0/(float(bpm)/60.0))/24.0;
unsigned long pastTime = micros();
unsigned long currentTime = micros();
int tickCount[4]= {0,0,0,0};//use to count steps for each rotary // was int (not unsigned)
int tickCounter;
 int stepNum=48;
byte incomingType;

int clockOut = 49;//3.5mm clock output pin jack for eurorack clock sync
int tock =0; //clockout timing with beat count

int noteFlags[4] ={0,0,0,0};

//for sending 3.5mm eurorack clock out from euclidean - using pin 33? try55. = a 3v3 trigger voltage.
unsigned long oldClock;
unsigned long currentClock;
unsigned long triggerLength = 50000; // microsecond trigger length for the high pulse. = 1 to 10 millis

int gatePins[4] ={11,24,26,28};
int gateInvPins[4] ={12,25,27,29};
//--gate pins

//-------------------for display--------------//

const uint8_t tileX[5] = {0,4,7,10,13}; //use five sections for x axis text areas
//first tile section is for headings
const uint8_t tileY[5] = {0,1,3,5,7}; //four text areas plus the zero for simple for loops

const uint8_t tile_area_width = 3;
const uint8_t tile_area_height = 1; // this will allow cour18 chars to fit into the area
int memScreenFlag =1;
int baseScreenFlag=1;
int bpmScreenFlag =1;

//-----------------display methods-------------------//

void displaySplash(){ //load basic screen parameters
  u8g2.clearBuffer();         // clear the internal memory
  u8g2.setFont(u8g2_font_helvR10_tr); // choose a suitable font
  u8g2.drawStr(25,40,"M0rk0_SQE"); // write something to the internal memory
  u8g2.sendBuffer();          // transfer internal memory to the display
  delay(1500);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_finderskeepers_tr);
  mainScreen();
  bpmScreenVal(120);
}

void mainScreen(){
  u8g2.clearBuffer();
  u8g2.drawStr(0,10, "Mem slot ="); // memoryScreenSlot value goes at drawUTF8(60,10, cstr); , DisplayArea(7, 0, 7, 2);
  u8g2.drawStr(0,25, "BPM ="); // bpm val goes at   u8g2.drawUTF8(40,24, cstr);, DisplayArea(5, 2, 6, 1);
  u8g2.drawStr(0,45, "butn 1 - save");
  u8g2.drawStr(0,60, "butn 2 - load");
  u8g2.drawStr(70,25, "Internal");
  u8g2.sendBuffer();
}



void memoryScreenSlot(int mem){  //text for memory slot number
  u8g2.clearBuffer(); 
  String str;
  char cstr[12];
  str = String(memoryBank[mem]);
  str.toCharArray(cstr,12);
  u8g2.drawUTF8(60,10, cstr); 
  u8g2.updateDisplayArea(7, 0, 7, 2);
}

//change these to use tiles, so that only update portion of screen
// u8g2.updateDisplayArea(tile_area_x_pos, tile_area_y_pos, tile_area_width, tile_area_height);

void memorySaved(int mem){
  u8g2.clearBuffer(); 
  u8g2.drawStr(60,10, "Saved....");
  u8g2.updateDisplayArea(7, 0, 7, 2);
  u8g2.clearBuffer(); 
}

void memoryRead(int mem){
  u8g2.clearBuffer(); 
  u8g2.drawStr(60,10, "Loaded....");
  u8g2.updateDisplayArea(7, 0, 7, 2);
  u8g2.clearBuffer(); 
}

void bpmScreen(){
 /* u8g2.clearBuffer();   
  String str;
  char cstr[12];
  str = String(extIntClock[BPMExternal]);
  str.toCharArray(cstr,12);
  u8g2.drawUTF8(60,24, cstr);
  u8g2.updateDisplayArea(7, 2, 7, 2);
  u8g2.clearBuffer(); 
  */
}

void bpmScreenVal(int val){
  u8g2.clearBuffer();   
  String str;
 char cstr[12];
  str = String(val);
  str.toCharArray(cstr,12);
  u8g2.drawUTF8(40,24, cstr);
  u8g2.updateDisplayArea(5, 2, 3, 1);
}


void writeOut(char *text, int Xs, int Ys){   //text, tile x and tile y from arrays
u8g2.clearBuffer();  
u8g2.drawUTF8(tileX[Xs]*8,(tileY[Ys]*8)+8,text);
  // now only update the selected area, the rest of the display content is not changed
u8g2.updateDisplayArea(tileX[Xs], tileY[Ys],tile_area_width, tile_area_height);
 }

 
//-----------------------------setup section------------------------setup-------------------//
void setup() {  
//------------setup the midi-------------//


   Serial1.begin(31250); //midi serial baud rate = 31250 - or native
 //  Serial.begin(115200);
  
  MIDI.begin(MIDI_CHANNEL_OMNI);

newTime = micros(); //start the bpm timer clock

  
//---------setup the leds----------------//
//for ws28 serial
  leds.begin();

//------------setup the display--------//
  u8g2.setBusClock(400000);
  u8g2.begin();
  u8g2.setFlipMode(1);
  
  displaySplash();

//----------------setup the encoders and check working--------//
  setEncoders();

//---------------for channel on/off push buttons----------------//
  pinMode(buttonPin[0], INPUT_PULLUP);
  pinMode(buttonPin[1], INPUT_PULLUP);
  pinMode(buttonPin[2], INPUT_PULLUP);
  pinMode(buttonPin[3], INPUT_PULLUP);

//for eurorack gate outputs 1-4
for (int x = 0;x<4;x++){
  pinMode(gatePins[x], OUTPUT);   
}
for (int x = 0;x<4;x++){
  pinMode(gateInvPins[x], OUTPUT);   
}

  
  pinMode(clockOut, OUTPUT); //clock out//for eurorack clock out


//********************check that SD card is functioning***************//
  if (!SD.begin(chipSelect)) {
//    Serial.println("initialization failed!");
    return;}
    delay(100);
//  Serial.println("SD OK!");
//*******************end of SD check******************************//
  
startRot();

}
//--------------end of setup-----------end of setup---------------end of setup-----//

void Sync() {
  midiDivCounter++;
  if (midiDivCounter >=midiDiv){

//more than one trigger per on note, need to use a divider or time
//check to see if the gate is being set via this call
//Serial.println("clock"); //DeBug
  MidiClock();
  midiDivCounter =0;  
  }
}

//-------------------- loop ---------------------------loop------------------loop----------//
void loop(){

//Serial.println(MIDI.read()); //DeBug

if(BPMExternal==1){
  INTERVAL = 60000/(bpm);
  if ((micros()-oldTime) > INTERVAL) {
    oldTime = micros();
    MidiClock();
  }
}

//-----------DIN MIDI---check to see if using external clock - do all the time-----------------//
if(Serial1.available() > 0) {
  BPMExternal=0;masterClock=false;
  data = Serial1.read();
//Serial.println(data); //DeBug

  if(data == midi_start) {
    play_flag = 1;
    }
  else if(data == midi_continue) {
    play_flag = 1;
    }
  else if(data == midi_stop) {
    play_flag = 0;
    }
  else if((data == 248/*midi_clock*/) /*&& (play_flag == 1)*/) {
  
    Sync();
    
    }
} 
   
//--------------------------------end of set external midi clock-------------------//


//--------check on off for four channels all the time--------//
checkOnOff();

//read all the rotaries all the time
//this does swprocess(), ledProcess() and rotEncProcess**(enc); 
encoderProcess();

//load all euclid info into one number for each rotary
 for (int seq=0;seq<4;seq++){
      euclidOut[seq] = euclid(euclidParams[seq][0],euclidParams[seq][1],euclidParams[seq][2]);
    }
   
//update the beat locations all the time = does Led.Show() all the time, 
//needed as need to update changes to beat locations. todo alt - do this only of beat locations change?

doLedBeats();

//-----send 3.5mm clock out------48 ticks divided by 8
if((tickCounter==0)||(tickCounter ==6)||(tickCounter==12)||(tickCounter ==18)||
(tickCounter==24)||(tickCounter==30)||(tickCounter==36)||(tickCounter==42)){
  digitalWrite(clockOut,HIGH);
  }
if((tickCounter ==1)||(tickCounter ==7)||(tickCounter==13)||(tickCounter==19)||
(tickCounter==25)||(tickCounter==31)||(tickCounter==37)||(tickCounter==43)){
  digitalWrite(clockOut,LOW);
  }
}
//------------------------- end of loop ------------end loop-----------end loop----------//


//----------for on/off buttons--------------//
void checkOnOff(){
  for (int onoff=0;onoff<4;onoff++){
  currentState[onoff] = digitalRead(buttonPin[onoff]);
  if (currentState[onoff] == LOW && lastState[onoff] == HIGH){//if button has just been pressed
    
    //toggle the state of the LED
    if (switchState[onoff] == LOW){
      switchState[onoff] = HIGH;
      
    } else {
      switchState[onoff] = LOW;  
          }
  }//end of if
  lastState[onoff] = currentState[onoff];
    }//end of for
}
//----------------end of on/off buttons-----------------//


//-------------------------midi clock action for internal and external clock----------------------------//
void  MidiClock(){  //do this every time the Timer3 interval timer is triggered

  if (masterClock == true){  //allows clock to be switched off  
     currentTime = micros();

     midIntervall = (1000/(bpm/60.0))/24.0;//set the midi clock using ref to bpm

     if ((currentTime - pastTime) > midIntervall*1000) {
      usbMIDI.sendRealTime(CLOCK);

//testing gate vs trigger
for(int x=0;x<4;x++){
  if(valRot9[x] == 0) { //valRot9 = gates number of steps to be on
    digitalWrite(gatePins[x],LOW);
    digitalWrite(gateInvPins[x], LOW); //line added for setting of inverting pins
    }
 }

        
    //---use per channel gate send-----//   
    sendVoice1();    
    sendVoice2();
    sendVoice3();
    sendVoice4();

    pastTime = currentTime;

    tickCounter++;
   
    if(tickCounter >=stepNum){
      tickCounter=0;
    }
   
     }//end if interval time hit
    
    }//------------end if masterclock-----below is if external clock--//

    else if (masterClock == false){

//remove double hit on single pulse
for(int x=0;x<4;x++){
  if(valRot9[x] == 0) { //valRot9 = gates number of steps to be on
    digitalWrite(gatePins[x],LOW);
    digitalWrite(gateInvPins[x], LOW); //line added for setting of inverting pins
    }
 }
    sendVoice1(); 
    sendVoice2();
    sendVoice3();
    sendVoice4();
     
    tickCounter++;
    if(tickCounter >=stepNum){tickCounter=0;}
    }//end of if not masterclock
}





//-------------------end of midiclock()-----------------------//


//------------------gate / voice send----- has option for sending midi note (not used)---//
void sendVoice1(){
  if (tickCounter%divRatio[channelDiv[0]] == 0) {
    tickCount[0]++;  
    //reset tickCount to zero if greater than step number
    if(tickCount[0]>euclidParams[0][0]-1){tickCount[0]=0;} 
  
      //switch note off if step location is past gate off location
      if(tickCount[0]>noteoffArray[0][0][1]+gates[0]){
        usbMIDI.sendNoteOff(noteoffArray[0][0][0],0,1);
        digitalWrite(gatePins[0],LOW);// Gate
        }
        
//------test new location
    for(int sender=0;sender<euclidParams[0][0]+1;sender++){
      if(tickCount[0] == sender-1 && bitRead(euclidOut[0],euclidParams[0][0]-sender)==1 && switchState[0] ==HIGH)
         {
           usbMIDI.sendNoteOn(allNotes[scale[scaleSelection]+incrementor[Modes[modeSelection]][0]],125,1);
             noteoffArray[0][0][1] =tickCount[0]; // step location
             noteoffArray[0][0][0] =allNotes[scale[scaleSelection]+incrementor[Modes[modeSelection]][0]]; //=note value
           digitalWrite(gatePins[0],HIGH);
           }
      if(tickCount[0] == sender-1 && bitRead(euclidOut[0],euclidParams[0][0]-sender)==0 && switchState[0] ==HIGH)
         {
          digitalWrite(gateInvPins[0],HIGH);
         }
      }//end of for sender

  }//end of if tickcounter
}

//-----------------
void sendVoice2(){
  if (tickCounter%divRatio[channelDiv[1]] == 0) {

    tickCount[1]++;    
  if(tickCount[1]>euclidParams[1][0]-1){tickCount[1]=0;} 
      //switch note off if step location is past gate off location
  if(tickCount[1]>noteoffArray[1][0][1]+gates[1]){
    usbMIDI.sendNoteOff(noteoffArray[1][0][0],0,2); 
    digitalWrite(gatePins[1],LOW); //gate2
    }
    
    for(int sender=0;sender<euclidParams[1][0]+1;sender++){
      if(tickCount[1] == sender-1 && bitRead(euclidOut[1],euclidParams[1][0]-sender)==1 && switchState[1] ==HIGH)
         {
          usbMIDI.sendNoteOn(allNotes[scale[scaleSelection]+incrementor[Modes[modeSelection]][1]],125,2);
             noteoffArray[1][0][1] =tickCount[1]; // step location
             noteoffArray[1][0][0] =allNotes[scale[scaleSelection]+incrementor[Modes[modeSelection]][1]]; //=note value
              digitalWrite(gatePins[1],HIGH);//gate2
         }
       if(tickCount[1] == sender-1 && bitRead(euclidOut[1],euclidParams[1][0]-sender)==0 && switchState[1] ==HIGH)
         {
          digitalWrite(gateInvPins[1],HIGH);
           }
    }//end of for
  }  
}
//------------------
void sendVoice3(){
   if (tickCounter%divRatio[channelDiv[2]] == 0) {

    tickCount[2]++;
  //reset tickcounter for rotary if over stepcount
  if(tickCount[2]>euclidParams[2][0]-1){tickCount[2]=0;}
      //switch note off if step location is past gate off location
  if(tickCount[2]>noteoffArray[2][0][1]+gates[2]){
    usbMIDI.sendNoteOff(noteoffArray[2][0][0],0,3); 
    digitalWrite(gatePins[2],LOW);//gate3
    }

    
    for(int sender=0;sender<euclidParams[2][0]+1;sender++){
      if(tickCount[2] == sender-1 && bitRead(euclidOut[2],euclidParams[2][0]-sender)==1 && switchState[2] ==HIGH)
         {
           usbMIDI.sendNoteOn(allNotes[scale[scaleSelection]+incrementor[Modes[modeSelection]][2]],125,3);
             noteoffArray[2][0][1] =tickCount[2]; // step location
             noteoffArray[2][0][0] =allNotes[scale[scaleSelection]+incrementor[Modes[modeSelection]][2]]; //=note value
           digitalWrite(gatePins[2],HIGH);//gate3
       //    digitalWrite(gateInvPins[2], LOW);
           }
      if(tickCount[2] == sender-1 && bitRead(euclidOut[2],euclidParams[2][0]-sender)==0 && switchState[2] ==HIGH)
         {
          digitalWrite(gateInvPins[2],HIGH);
         }

    }//end of for

  }   
}
//-----------------
void sendVoice4(){
   if (tickCounter%divRatio[channelDiv[3]] == 0) {

    tickCount[3]++;
  //reset tickcounter for rotary if over stepcount
  if(tickCount[3]>euclidParams[3][0]-1){tickCount[3]=0;}
      //switch note off if step location is past gate off location
  if(tickCount[3]>noteoffArray[3][0][1]+gates[3]){
    usbMIDI.sendNoteOff(noteoffArray[3][0][0],0,4); 
    digitalWrite(gatePins[3],LOW);//gate4
    }
    
    for(int sender=0;sender<euclidParams[3][0]+1;sender++){
      if(tickCount[3] == sender-1 && bitRead(euclidOut[3],euclidParams[3][0]-sender)==1 && switchState[3] ==HIGH)
         {

          usbMIDI.sendNoteOn(allNotes[scale[scaleSelection]+incrementor[Modes[modeSelection]][3]],125,4);
             noteoffArray[3][0][1] =tickCount[3]; // step location
             noteoffArray[3][0][0] =allNotes[scale[scaleSelection]+incrementor[Modes[modeSelection]][3]]; //=note value
           digitalWrite(gatePins[3],HIGH);//gate4
           }
      if(tickCount[3] == sender-1 && bitRead(euclidOut[3],euclidParams[3][0]-sender)==0 && switchState[3] ==HIGH)
         {
          digitalWrite(gateInvPins[3],HIGH);
         }
           
    }//end of for

  }  
}

//------------------------------------do the leds--------------------//
void doLedBeats(){
//---------set to blue only those steps active, so switch off all first
for(int all=0;all<64;all++){leds.setPixel(all, ALLOFF);}//use ws serial as non blocking

//----------set those leds that are step, to blueish white
for(int rings=0;rings<4;rings++){
    for(int onSteps=0;onSteps<euclidParams[rings][0];onSteps++){leds.setPixel(15-onSteps+(16*rings), WHITE2);}
}


//------------do the leds for the hits
for(int allRot=0;allRot<4;allRot++){
    for(int beats = 0; beats<16; beats++){
      if(bitRead(euclidOut[allRot],beats)==1)
          {leds.setPixel((15+(16*allRot))-(euclidParams[allRot][0]-beats-1), RED);} 
    }
}
//-----------do the leds for the step indicator
for(int xx=0;xx<4;xx++){
     if(switchState[xx]==1){ leds.setPixel (  15 -(tickCount[xx])    +(16*xx)  , BLUE);}
     else{leds.setPixel(  15 -(tickCount[xx])    +(16*xx)  , GREEN );}
      }
         
//----call to show leds only once   
        leds.show();
}
//---------------------end of doLedBeats-----------------//


//------------------------- Euclid calculation functions -------------------------------------------//
//--- the three funcitons below are taken directly from http://clsound.com/euclideansequenc.html ---//
//--- acknowledgment to Craig Lee ------------------------------------------------------------------//

//------------Function to right rotate n by d bits---------------------------------//
uint16_t rightRotate(int shift, uint16_t value, uint8_t pattern_length) {
  uint16_t mask = ((1 << pattern_length) - 1);
  value &= mask;
  return ((value >> shift) | (value << (pattern_length - shift))) & mask;
}

//----1---------Function to find the binary length of a number by counting bitwise-------//
int findlength(unsigned int bnry) {
  boolean lengthfound = false;
  int length = 1; // no number can have a length of zero - single 0 has a length of one, but no 1s for the sytem to count
  for (int q = 32; q >= 0; q--) {
    int r = bitRead(bnry, q);
    if (r == 1 && lengthfound == false) {
      length = q + 1;
      lengthfound = true;
    }
  }
  return length;
}

//-----2--------Function to concatenate two binary numbers bitwise----------------------//
unsigned int ConcatBin(unsigned int bina, unsigned int binb) {
  int binb_len = findlength(binb);
  unsigned int sum = (bina << binb_len);
  sum = sum | binb;
  return sum;
}

//------3-------------------Euclidean bit sorting funciton-------------------------------//
unsigned int euclid(int n, int k, int o) { // inputs: n=total, k=beats, o = offset
  int pauses = n - k;
  int pulses = k;
  int offset = o;
  int steps = n;
  int per_pulse = pauses / k;
  int remainder = pauses % pulses;
  unsigned int workbeat[n];
  unsigned int outbeat;
  uint16_t outbeat2;
  int workbeat_count = n;
  int a;
  int b;
  int trim_count;

  for (a = 0; a < n; a++) { // Populate workbeat with unsorted pulses and pauses
    if (a < pulses) {
      workbeat[a] = 1;
    }
    else {
      workbeat[a] = 0;
    }
  }

  if (per_pulse > 0 && remainder < 2) { // Handle easy cases where there is no or only one remainer
    for (a = 0; a < pulses; a++) {
      for (b = workbeat_count - 1; b > workbeat_count - per_pulse - 1; b--) {
        workbeat[a] = ConcatBin(workbeat[a], workbeat[b]);
      }
      workbeat_count = workbeat_count - per_pulse;
    }

    outbeat = 0; // Concatenate workbeat into outbeat - according to workbeat_count
    for (a = 0; a < workbeat_count; a++) {
      outbeat = ConcatBin(outbeat, workbeat[a]);
    }

    if (offset > 0) {
      outbeat2 = rightRotate(offset, outbeat, steps); // Add offset to the step pattern
    }
    else {
      outbeat2 = outbeat;
    }

    return outbeat2;
  }

  else {
    if (pulses == 0) {
      pulses = 1;  // Prevent crashes when k=0 and n goes from 0 to 1
    }
    int groupa = pulses;
    int groupb = pauses;
    int iteration = 0;
    if (groupb <= 1) {
    }

    while (groupb > 1) { //main recursive loop
      
      if (groupa > groupb) { // more Group A than Group B
        int a_remainder = groupa - groupb; // what will be left of groupa once groupB is interleaved
        trim_count = 0;
        for (a = 0; a < groupa - a_remainder; a++) { //count through the matching sets of A, ignoring remaindered
          workbeat[a] = ConcatBin(workbeat[a], workbeat[workbeat_count - 1 - a]);
          trim_count++;
        }
        workbeat_count = workbeat_count - trim_count;

        groupa = groupb;
        groupb = a_remainder;
      }

      else if (groupb > groupa) { // More Group B than Group A
        int b_remainder = groupb - groupa; // what will be left of group once group A is interleaved
        trim_count = 0;
        for (a = workbeat_count - 1; a >= groupa + b_remainder; a--) { //count from right back through the Bs
          workbeat[workbeat_count - a - 1] = ConcatBin(workbeat[workbeat_count - a - 1], workbeat[a]);

          trim_count++;
        }
        workbeat_count = workbeat_count - trim_count;
        groupb = b_remainder;
      }

      else if (groupa == groupb) { // groupa = groupb
        trim_count = 0;
        for (a = 0; a < groupa; a++) {
          workbeat[a] = ConcatBin(workbeat[a], workbeat[workbeat_count - 1 - a]);
          trim_count++;
        }
        workbeat_count = workbeat_count - trim_count;
        groupb = 0;
      }

      else {
        //Serial.println("ERROR");
      }
      iteration++;
    }

    outbeat = 0; // Concatenate workbeat into outbeat - according to workbeat_count
    for (a = 0; a < workbeat_count; a++) {
      outbeat = ConcatBin(outbeat, workbeat[a]);
    }

    if (offset > 0) {
      outbeat2 = rightRotate(offset, outbeat, steps); // Add offset to the step pattern
    }
    else {
      outbeat2 = outbeat;
    }

    return outbeat2;
  }
}
//------------------end euclidian math-------------------------------------//

//------------------------------------------encoder process------------//
 void encoderProcess(){

    for(int sw=0;sw<4;sw++){
    swProcess(sw); //check to see is rotary push button is pressed
      if(oldMode[sw] != mode[sw]){
      oldMode[sw] = mode[sw];
     // Serial.println(sw);
      ledProcess(sw); //set the rotary led colour to pushbutton mode
      }
    }

    
for(int enc=0;enc<4;enc++){
if(mode[enc]==1){ //1=step number
  rotEncProcess(enc); 
    if(oldVal1[enc] != valRot1[enc]){
      oldVal1[enc] = valRot1[enc];
     // Serial.println(enc);
      euclidParams[enc][0] = oldVal1[enc]; //puts the step number from oldVal1 into the euclid array
    }
} }

for(int enc=0;enc<4;enc++){
if(mode[enc]==2){ //2 = beats
  rotEncProcess2(enc); 
    if(oldVal2[enc] != valRot2[enc]){
      oldVal2[enc] = valRot2[enc];
    //  Serial.println(enc);
      euclidParams[enc][1] = oldVal2[enc]; //puts the beat number from oldVal1 into the euclid array
    }
}  } 

for(int enc=0;enc<4;enc++){
if(mode[enc]==3){ //3 = offset amount
  rotEncProcess3(enc); 
    if(oldVal3[enc] != valRot3[enc]){
      oldVal3[enc] = valRot3[enc];
      euclidParams[enc][2] = oldVal3[enc]; //puts the offset number from oldVal1 into the euclid array
        }
     } }

for(int enc=0;enc<4;enc++){
if(mode[enc]==0){ //0 = DIV number
  rotEncProcess4(enc); 
    if(oldVal4[enc] != valRot4[enc]){
      oldVal4[enc] = valRot4[enc];
      channelDiv[enc] = oldVal4[enc]; //puts the div number from oldVal1 into the euclid array
        }
     } }
      

//for(int enc=0;enc<4;enc++){
if(mode[3]==4){ //4 = BPM number for forth rotary 
  rotEncProcess5(); 
    if(oldVal5[5] != valRot5[5]){
      oldVal5[5] = valRot5[5];
      bpm = oldVal5[5]; //

        }
     }

//------------encoder 2 in purple mode = scale selector
if(mode[1]==4) {
//  rotEncProcessScales();
    if(oldVal7[0] != valRot7[0]){
      oldVal7[0] = valRot7[0];
      scaleSelection = oldVal7[0]; //

    }//end of if
}//-----------end of if enc2 in purple mode
//------------encoder 2 in purple mode = mode selector
if(mode[2]==4) {
//rotEncProcessModes();
    if(oldVal7[1] != valRot7[1]){
      oldVal7[1] = valRot7[1];
      modeSelection = oldVal7[1]; //

    }//end of if
}//-----------end of if enc2 in purple mode




if(mode[0]==4) //&& digitalRead(buttonPin[0]==HIGH))
{ //4 = memory action on first rotary 
//copy divs to mem slot
if (memScreenFlag ==1){

    memoryScreenSlot(0);
    memScreenFlag =0;
    bpmScreenFlag =1;
    }

for(int mem=0;mem<4;mem++){
  memChannelDIV[mem]=channelDiv[mem];
  ONOFF[mem]=switchState[mem];
}

   rotEncProcessMem(); //read the rotary pins and update
   
    if(oldVal6[0] != valRot6[0]){
      oldVal6[0] = valRot6[0];
      memSlot = oldVal6[0]; //

      memoryScreenSlot(memSlot);

     }

//-------------if writing to memory----------//
  static uint8_t lastBtnState1 = HIGH;
  uint8_t state1 = digitalRead(buttonPin[0]);
  if (state1 != lastBtnState1) {
    lastBtnState1 = state1;
    if (state1 == LOW) {
       writeMemory(memSlot);
       memorySaved(memSlot);
       
       }
    }
//-------------if reading from memory----------//
  static uint8_t lastBtnState2 = HIGH;
  uint8_t state2 = digitalRead(buttonPin[1]);
  if (state2 != lastBtnState2) {
    lastBtnState2 = state2;
    if (state2 == LOW) {
       readMemory(memSlot);
       memoryRead(memSlot);//update screen
       }
       //-----put div values and onoff values into current
      for(int mem=0;mem<4;mem++){
        channelDiv[mem]=memChannelDIV[mem];
        switchState[mem]=ONOFF[mem];
      } 
    }
  }//end of memory action on first rotary


if(mode[0]==5){ // = ?? = white rotary indicator
    memScreenFlag=1;
  

          //-------next thing to do
       if(bpmScreenFlag==1){
    //    bpmScreen();
        bpmScreenVal(bpm);
        bpmScreenFlag=0;
        baseScreenFlag=1;
       }
            rotEncProcess5(); 
            if(oldVal5[5] != valRot5[5]){
            oldVal5[5] = valRot5[5];
            bpm = oldVal5[5];
            bpmScreenVal(bpm);
            
       }
    }


//----------------process for gates
for(int enc=0;enc<4;enc++){
if(mode[enc]==6){ //6 = gate

if(mode[0]==6){
     if(baseScreenFlag==1){
      
     u8g2.clearBuffer(); 
       baseScreenFlag=0;
        }
}
  
  rotEncProcessGate(enc); 
    if(oldVal9[enc] != valRot9[enc]){
      oldVal9[enc] = valRot9[enc];
      gates[enc] = oldVal9[enc]; //puts the  number from oldVal9 into gate value for channel

        }
      } 
    }     



 }//end of encoderprocess()
//---------end encoder process----------//

//#endif //guard
#endif //guard
