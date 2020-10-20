#include <Bounce2.h>

//need to use include guard for euclid.h
#ifndef euclid_h
#define euclid_h //dont forget endif at bottom of tab 

// need six pins for each rotary

int rotaryPins[4][6]={ // left rot, right rot, red pin, green pin, blue pin, switch pin
{17,16,4,5,6,15},         //first rotary
{37,38,1,2,3,36},         //second rotary
{14,13,7,8,9,39},         //third rotary
{35,34,30,31,32,33},      //fourth rotary
};


int euclidParams[4][3] = {  //(int totalsteps, int beats or hits, int offset) 
  {16,1,0},
  {16,1,0},
  {16,1,0},
  {16,1,0}
};
unsigned int euclid(int totalSteps, int beats, int offset1);
int euclidOut[4] = {0,0,0,0};  //used to store the output of the euclidean function

//for blink without delay for avoiding while() in sw process (rotary button push)
unsigned long previousMillis = 0;
unsigned long pushInterval = 300;
unsigned long currentMillis;


// ROT-ENC CONSTANTS
#define  ROT_MAX 16
#define  ROT_MIN 1
//#define  ROT_d   1
#define  ROT_center 1
#define ROT_start 16

#define OFFSET_MIN 0
#define OFFSET_MAX 16
#define OFFSET_START 0

#define DIV_MIN 0
#define DIV_MAX 5
#define DIV_START 2  //=24 = 1/4 note

#define BPM_MIN 40
#define BPM_MAX 200
#define BPM_START 120

#define SLOT_MIN 0
#define SLOT_MAX 32
#define SLOT_START 1

#define SCALE_MIN 0 //C to B scales
#define SCALE_MAX 60//24
#define SCALE_START 24//12

#define MODE_MIN 0 //major / minor / pent / one_note
#define MODE_MAX 3
#define MODE_START 0

#define GATE_MIN 0 //note duration
#define GATE_MAX 16
#define GATE_START 0

int ROT_d[15] ={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
int ROT_STAY[15] ={3,3,3,3,3,3,3,3,3,3,3,3,3,3,3};
int ROT_LEFT[15] ={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
int ROT_RIGHT[15] ={2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};

int val1a[5] = {0,0,0,0,0};
int val1b[5] = {0,0,0,0,0};
int oldVal1[5] ={-1,-1,-1,-1,-1};
int valRot1[5] ={ROT_start,ROT_start,ROT_start,ROT_start,ROT_start};
int dirRot1[5] ={3,3,3,3,3};

int val2a[5] = {0,0,0,0,0};
int val2b[5] = {0,0,0,0,0};
int oldVal2[5] ={-1,-1,-1,-1,-1};
int valRot2[5] ={ROT_center,ROT_center,ROT_center,ROT_center,ROT_center};
int dirRot2[5] ={3,3,3,3,3};

int val3a[5] = {0,0,0,0,0};
int val3b[5] = {0,0,0,0,0};
int oldVal3[5] ={-1,-1,-1,-1,-1};
int valRot3[5] ={OFFSET_START,OFFSET_START,OFFSET_START,OFFSET_START,OFFSET_START};
int dirRot3[5] ={3,3,3,3,3};

int val4a[5] = {0,0,0,0,0};
int val4b[5] = {0,0,0,0,0};
int oldVal4[5] ={-1,-1,-1,-1,-1};
int valRot4[5] ={DIV_START,DIV_START,DIV_START,DIV_START,DIV_START};
int dirRot4[5] ={3,3,3,3,3};

int val5a[6] = {0,0,0,0,0,0};
int val5b[6] = {0,0,0,0,0,0};
int oldVal5[6] ={-1,-1,-1,-1,-1,-1};
int valRot5[6] ={BPM_START,BPM_START,BPM_START,BPM_START,BPM_START,BPM_START};
int dirRot5[6] ={3,3,3,3,3,3};

int val6a[5] = {0,0,0,0,0};
int val6b[5] = {0,0,0,0,0};
int oldVal6[5] ={-1,-1,-1,-1,-1};
int valRot6[5] ={0,0,0,0,0};
int dirRot6[5] ={3,3,3,3,3};

int val7a[5] = {0,0,0,0,0};  //valrot7[0] = scale 7[1] = mode start
int val7b[5] = {0,0,0,0,0};
int oldVal7[5] ={-1,-1,-1,-1,-1};
int valRot7[5] ={SCALE_START,MODE_START,0,0,0};
int dirRot7[5] ={3,3,3,3,3};

int val8a[5] = {0,0,0,0,0};
int val8b[5] = {0,0,0,0,0};
int oldVal8[5] ={-1,-1,-1,-1,-1};
int valRot8[5] ={0,0,0,0,0};
int dirRot8[5] ={3,3,3,3,3};

int val9a[5] = {0,0,0,0,0};//for gates
int val9b[5] = {0,0,0,0,0};
int oldVal9[5] ={-1,-1,-1,-1,-1};
int valRot9[5] ={0,0,0,0,0}; //this is now the number of steps, not a millis calc
int dirRot9[5] ={3,3,3,3,3};

int inverta[5] ={0,0,0,0,0};
int invertb[5] = {0,0,0,0,0};
int oldInvert[5] = {-1,-1,-1,-1};
int valInvert[5] = {0,0,0,0,0};
int dirInvert[5] = {3,3,3,3,3};
//#define INV_MIN 0 
//#define INV_MAX 1
//#define INV_START 0 

int mode[10] = {1,1,1,1,1,1,1,1,1,1}; //four switches, each can be 0,1,2, or 3
int oldMode[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1};

String extIntClock[2] = {"Internal","External"};

int dbTime = 5;

// Instantiate a Bounce object
Bounce debouncer1 = Bounce(); 
Bounce debouncer2 = Bounce(); 
Bounce debouncer3 = Bounce(); 
Bounce debouncer4 = Bounce(); 
Bounce debouncer5 = Bounce(); 
Bounce debouncer6 = Bounce(); 
Bounce debouncer7 = Bounce(); 
Bounce debouncer8 = Bounce(); 
//----------------------------used in setup, just here to clean the main page up-------------//
//------for rotary encoders------------//     
void setEncoders(){
   pinMode(rotaryPins[0][0],INPUT);
   pinMode(rotaryPins[0][1],INPUT);
   pinMode(rotaryPins[0][5],INPUT);
   digitalWrite(rotaryPins[0][0],HIGH);
   digitalWrite(rotaryPins[0][1],HIGH);
   pinMode(rotaryPins[1][0],INPUT);
   pinMode(rotaryPins[1][1],INPUT);
   pinMode(rotaryPins[1][5],INPUT);
   digitalWrite(rotaryPins[1][0],HIGH);
   digitalWrite(rotaryPins[1][1],HIGH); 

   pinMode(rotaryPins[2][0],INPUT);
   pinMode(rotaryPins[2][1],INPUT);
   pinMode(rotaryPins[2][5],INPUT);
   digitalWrite(rotaryPins[2][0],HIGH);
   digitalWrite(rotaryPins[2][1],HIGH);
   pinMode(rotaryPins[3][0],INPUT);
   pinMode(rotaryPins[3][1],INPUT);
   pinMode(rotaryPins[3][5],INPUT);
   digitalWrite(rotaryPins[3][0],HIGH);
   digitalWrite(rotaryPins[3][1],HIGH); 

}



//---------------method to scroll the rotary encoder colours on start-------------//
void startRot() {
for(int x=0; x<4;x++){
 //RED
          analogWrite(rotaryPins[x][2],0);
          analogWrite(rotaryPins[x][3],255);
          analogWrite(rotaryPins[x][4],255);
delay(200); //GREEN
          analogWrite(rotaryPins[x][2],255);
          analogWrite(rotaryPins[x][3],0);
          analogWrite(rotaryPins[x][4],255);
delay(200);  //BLUE
          analogWrite(rotaryPins[x][2],225);
          analogWrite(rotaryPins[x][3],225);
          analogWrite(rotaryPins[x][4],0);
delay(200);  //BLUE

  }
  for(int startGate=0;startGate<4;startGate++){ //----------end the start process with light blue
  mode[startGate] = 6;}
}

void ledProcess(int x){

      switch(mode[x]){
         case 0: //GREEN
          analogWrite(rotaryPins[x][2],255);
          analogWrite(rotaryPins[x][3],0);
          analogWrite(rotaryPins[x][4],255);
          break;
        case 1: //BLUE
          analogWrite(rotaryPins[x][2],255);
          analogWrite(rotaryPins[x][3],255);
          analogWrite(rotaryPins[x][4],0);
          break;         
          case 2: //RED
          analogWrite(rotaryPins[x][2],0);
          analogWrite(rotaryPins[x][3],255);
          analogWrite(rotaryPins[x][4],255);
          break;
        case 3: //YELLOW
          analogWrite(rotaryPins[x][2],0);
          analogWrite(rotaryPins[x][3],0);
          analogWrite(rotaryPins[x][4],255);
          break;
        case 4: //PURPLE
          analogWrite(rotaryPins[x][2],0);
          analogWrite(rotaryPins[x][3],255);
          analogWrite(rotaryPins[x][4],0);
          break;
        case 5: //WHITE
          analogWrite(rotaryPins[x][2],0);
          analogWrite(rotaryPins[x][3],0);
          analogWrite(rotaryPins[x][4],0);
          break;   
        case 6: //LIGHT BLUE
          analogWrite(rotaryPins[x][2],225);
          analogWrite(rotaryPins[x][3],0);
          analogWrite(rotaryPins[x][4],0);          
      }
}


//--------for steps--- if encoder mode ==1 ------//
void rotEncProcess(int x)
{
  val1a[x] = digitalRead( rotaryPins[x][0] );
  val1b[x] = digitalRead( rotaryPins[x][1]);
//  delay(1);

  if( val1a[x] == HIGH && val1b[x] == HIGH ){
    if( dirRot1[x] == ROT_LEFT[x] ){
        valRot1[x]+=ROT_d[x];
        if(valRot1[x]>ROT_MAX){valRot1[x]=ROT_MAX;}
    }else if( dirRot1[x] == ROT_RIGHT[x] ){
        valRot1[x] -=ROT_d[x];
        if(valRot1[x]<ROT_MIN){valRot1[x]=ROT_MIN;}
    }
    dirRot1[x] = ROT_STAY[x];
  } else {
    if( val1a[x] == LOW ){
      dirRot1[x] = ROT_LEFT[x];
    }
    if( val1b[x] == HIGH ){
      dirRot1[x] = ROT_RIGHT[x];
    }
  }
}
//----------------------for beats/hits-----------//
void rotEncProcess2(int x)
{
  val2a[x] = digitalRead( rotaryPins[x][0] );
  val2b[x] = digitalRead( rotaryPins[x][1] );
 // delay(1);

// Rotary Encoder 2
  if( val2a[x] == HIGH && val2b[x] == HIGH ){
    if( dirRot2[x] == ROT_LEFT[x] ){
        valRot2[x]+=ROT_d[x];
        if(valRot2[x]>ROT_MAX){valRot2[x]=ROT_MAX;}
    }else if( dirRot2[x] == ROT_RIGHT[x] ){
        valRot2[x] -=ROT_d[x];
        if(valRot2[x]<ROT_MIN){valRot2[x]=ROT_MIN;}
    }
    dirRot2[x] = ROT_STAY[x];
  } else {
    if( val2a[x] == LOW ){
      dirRot2[x] = ROT_LEFT[x];
    }
    if( val2b[x] == HIGH ){
      dirRot2[x] = ROT_RIGHT[x];
    }
  }
}

//----------------------for offset-----------//
void rotEncProcess3(int x) //for offset
{
  val3a[x]= digitalRead( rotaryPins[x][0] );
  val3b[x] = digitalRead( rotaryPins[x][1] );
 // delay(1);

// Rotary Encoder 3
  if( val3a[x] == HIGH && val3b[x] == HIGH ){
    if( dirRot3[x] == ROT_LEFT[x] ){
        valRot3[x]+=ROT_d[x];
        if(valRot3[x]>OFFSET_MAX){valRot3[x]=OFFSET_MAX;}
    }else if( dirRot3[x] == ROT_RIGHT[x] ){
        valRot3[x] -=ROT_d[x];
        if(valRot3[x]<OFFSET_MIN){valRot3[x]=OFFSET_MIN;}
    }
    dirRot3[x] = ROT_STAY[x];
  } else {
    if( val3a[x] == LOW ){
      dirRot3[x] = ROT_LEFT[x];
    }
    if( val3b[x] == HIGH ){
      dirRot3[x] = ROT_RIGHT[x];
    }
  }
}


//----------------------for setting the clock DIV-----------//
void rotEncProcess4(int x) //for offset
{
  val4a[x]= digitalRead( rotaryPins[x][0] );
  val4b[x] = digitalRead( rotaryPins[x][1] );
  //delay(1);

// Rotary Encoder 4
  if( val4a[x] == HIGH && val4b[x] == HIGH ){
    if( dirRot4[x] == ROT_LEFT[x] ){
        valRot4[x]+=ROT_d[x];
        if(valRot4[x]>DIV_MAX){valRot4[x]=DIV_MAX;}
    }else if( dirRot4[x] == ROT_RIGHT[x] ){
        valRot4[x] -=ROT_d[x];
        if(valRot4[x]<DIV_MIN){valRot4[x]=DIV_MIN;}
    }
    dirRot4[x] = ROT_STAY[x];
  } else {
    if( val4a[x] == LOW ){
      dirRot4[x] = ROT_LEFT[x];
    }
    if( val4b[x] == HIGH ){
      dirRot4[x] = ROT_RIGHT[x];
    }
  }
}

//---------------------------process for gates
void rotEncProcessGate(int x)
{
  val9a[x]= digitalRead( rotaryPins[x][0] );
  val9b[x] = digitalRead( rotaryPins[x][1] );
  //delay(1);

// Rotary Encoders
  if( val9a[x] == HIGH && val9b[x] == HIGH ){
    if( dirRot9[x] == ROT_LEFT[x] ){
        valRot9[x]+=ROT_d[x];
        if(valRot9[x]>GATE_MAX){valRot9[x]=GATE_MAX;}
    }else if( dirRot9[x] == ROT_RIGHT[x] ){
        valRot9[x] -=ROT_d[x];
        if(valRot9[x]<GATE_MIN){valRot9[x]=GATE_MIN;}
    }
    dirRot9[x] = ROT_STAY[x];
  } else {
    if( val9a[x] == LOW ){
      dirRot9[x] = ROT_LEFT[x];
    }
    if( val9b[x] == HIGH ){
      dirRot9[x] = ROT_RIGHT[x];
    }
  }
}
//----------------------for setting the clock DIV-----------//
void rotEncProcess5() 
{
  val5a[5]= digitalRead( rotaryPins[0][0] );
  val5b[5] = digitalRead( rotaryPins[0][1] );
  //delay(1);

// Rotary Encoder 4 -- stored in slot5
  if( val5a[5] == HIGH && val5b[5] == HIGH ){
    if( dirRot5[5] == ROT_LEFT[5] ){
        valRot5[5]+=ROT_d[5];
        if(valRot5[5]>BPM_MAX){valRot5[5]=BPM_MAX;}
    }else if( dirRot5[5] == ROT_RIGHT[5] ){
        valRot5[5] -=ROT_d[5];
        if(valRot5[5]<BPM_MIN){valRot5[5]=BPM_MIN;}
    }
    dirRot5[5] = ROT_STAY[5];
  } else {
    if( val5a[5] == LOW ){
      dirRot5[5] = ROT_LEFT[5];
    }
    if( val5b[5] == HIGH ){
      dirRot5[5] = ROT_RIGHT[5];
    }
  }
}

//----------------------for setting the memory slot-----------//
void rotEncProcessMem() 
{
  val6a[0]= digitalRead( rotaryPins[0][0] );
  val6b[0] = digitalRead( rotaryPins[0][1] );
  //delay(1);

// Rotary Encoder 1---stored in slot6
  if( val6a[0] == HIGH && val6b[0] == HIGH ){
    if( dirRot6[0] == ROT_LEFT[6] ){
        valRot6[0]+=ROT_d[6];
        if(valRot6[0]>SLOT_MAX){valRot6[0]=SLOT_MAX;}
    }else if( dirRot6[0] == ROT_RIGHT[6] ){
        valRot6[0] -=ROT_d[6];
        if(valRot6[0]<SLOT_MIN){valRot6[0]=SLOT_MIN;}
    }
    dirRot6[0] = ROT_STAY[6];
  } else {
    if( val6a[0] == LOW ){
      dirRot6[0] = ROT_LEFT[6];
    }
    if( val6b[0] == HIGH ){
      dirRot6[0] = ROT_RIGHT[6];
    }
  }
}

//------------------rotary process for scales
/*
void rotEncProcessScales() //for scales - val7 = scale, val8 = mode
{
  val7a[0]= digitalRead( rotaryPins[1][0] );//for scales = rotary 1
  val7b[0] = digitalRead( rotaryPins[1][1] );
  //delay(1);

// Rotary Encoder 1
  if( val7a[0] == HIGH && val7b[0] == HIGH ){
    if( dirRot7[0] == ROT_LEFT[7] ){
        valRot7[0]+=ROT_d[7];
        if(valRot7[0]>SCALE_MAX){valRot7[0]=SCALE_MAX;}
    }else if( dirRot7[0] == ROT_RIGHT[7] ){
        valRot7[0] -=ROT_d[7];
        if(valRot7[0]<SCALE_MIN){valRot7[0]=SCALE_MIN;}
    }
    dirRot7[0] = ROT_STAY[7];
  } else {
    if( val7a[0] == LOW ){
      dirRot7[0] = ROT_LEFT[7];
    }
    if( val7b[0] == HIGH ){
      dirRot7[0] = ROT_RIGHT[7];
    }
  }
}
//--------for rotary process for mode - major, minor, pent
void rotEncProcessModes() //for scales - val7 = scale, val8 = mode
{
  val7a[1]= digitalRead( rotaryPins[2][0] );//for scales = rotary 1
  val7b[1] = digitalRead( rotaryPins[2][1] );
  //delay(1);

// Rotary Encoder 1
  if( val7a[1] == HIGH && val7b[1] == HIGH ){
    if( dirRot7[1] == ROT_LEFT[8] ){
        valRot7[1]+=ROT_d[8];
        if(valRot7[1]>MODE_MAX){valRot7[1]=MODE_MAX;}
    }else if( dirRot7[1] == ROT_RIGHT[8] ){
        valRot7[1] -=ROT_d[8];
        if(valRot7[1]<MODE_MIN){valRot7[1]=MODE_MIN;}
    }
    dirRot7[1] = ROT_STAY[8];
  } else {
    if( val7a[1] == LOW ){
      dirRot7[1] = ROT_LEFT[8];
    }
    if( val7b[1] == HIGH ){
      dirRot7[1] = ROT_RIGHT[8];
    }
  }
}
*/
//----------------process for invert send notes off rotary4
/*void rotEncProcessInvert(int x){
  inverta[x]= digitalRead( rotaryPins[x][0] );//for invert, all rotary channels
  invertb[x] = digitalRead( rotaryPins[x][1] );
  //delay(1);

  if( inverta[x] == HIGH && invertb[x] == HIGH ){
    if( dirInvert[x] == ROT_LEFT[9] ){
        valInvert[x]+=ROT_d[9];
        if(valInvert[x]>INV_MAX){valInvert[x]=INV_MAX;}
    }else if( dirInvert[x] == ROT_RIGHT[9] ){
        valInvert[x] -=ROT_d[9];
        if(valInvert[x]<INV_MIN){valInvert[x]=INV_MIN;}
    }
    dirInvert[x] = ROT_STAY[9];
  } else {
    if( inverta[x] == LOW ){
      dirInvert[x] = ROT_LEFT[9];
    }
    if( invertb[x] == HIGH ){
      dirInvert[x] = ROT_RIGHT[9];
    }
  }

}*/

//------------switch process----------//
void swProcess(int x){      
  if( digitalRead(rotaryPins[x][5]) ){
       currentMillis = millis();
         if(currentMillis - previousMillis > pushInterval) {
          // save the last time
            previousMillis = currentMillis;   
      mode[x]++;
      mode[x] %=7;
     }
  }
}

#endif //guard for euclid,h
