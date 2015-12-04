/**************************************************
 Infrared MIDI Barrel Organ (Pianola infravermelha)
 Fernando S. Pacheco
 2015-11-20
 
 ==============
 Code Based on: 
 2-12-2011
 Spark Fun Electronics 2011
 Nathan Seidle
 
 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 ===============
 
 This code controls the VS1053 in what is called Real Time MIDI mode. To get the VS1053 into RT MIDI mode, power up
 the VS1053 with GPIO0 tied low, GPIO1 tied high.
 
 I (Nathan) use the NewSoftSerial library to send out the MIDI serial at 31250bps. This allows me to print regular messages
 for debugging to the terminal window. This helped me out a ton.
 
 5V : VS1053 VCC
 GND : VS1053 GND
 D3 (SoftSerial TX) : VS1053 RX
 D4 : VS1053 RESET
 D5: GPIO1
 
***************************************/

#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3); //Soft TX on 3, we don't use RX in this code

// When infrared sensor "reads" Black - turn note ON.
// White (paper) - turn OFF
#define ON 0
#define OFF 1

byte note = 0; //The MIDI note value to be played
byte resetMIDI = 4; //Tied to VS1053 Reset line
byte ledPin = 13; //MIDI traffic indicator
byte pinGPIO1 = 5; //HIGH will activate MIDI

byte i;
byte numNotes = 7; //number of notes
byte instrument = 18; //18=rock organ
byte pinNote[7]  = { 6,  7,  8,  9, 10,  11, 12};
char noteName[7] = {'C','D','E','F','G','A', 'B'};
byte noteMIDI[7] = { 48, 50, 52, 53, 55, 57, 59 };
byte noteON[7]={OFF, OFF, OFF, OFF, OFF, OFF, OFF};

void setup() {
  Serial.begin(57600);

  //Setup soft serial for MIDI control
  mySerial.begin(31250);
  
  for (i=0; i<numNotes; ++i) {
     pinMode(pinNote[i], INPUT);
  }

  pinMode(pinGPIO1, OUTPUT);
  digitalWrite(pinGPIO1,HIGH);
  
  //Reset the VS1053
  pinMode(resetMIDI, OUTPUT);
  digitalWrite(resetMIDI, LOW);
  delay(100);
  digitalWrite(resetMIDI, HIGH);
  delay(100);
}

void loop() {

  talkMIDI(0xB0, 0x07, 120); //0xB0 is channel message, set channel volume to near max (127)

  Serial.println("Basic Instruments");
  talkMIDI(0xB0, 0, 0x00); //Default bank GM1
  
  Serial.print(" Instrument: ");
  Serial.println(instrument, DEC);

  talkMIDI(0xC0, instrument, 0); //Set instrument number. 0xC0 is a 1 data byte command

  for (i=0; i<numNotes; ++i) {
    if (digitalRead(pinNote[i])==ON) {
       Serial.print(noteName[i]);
       if (noteON[i]==OFF) {
         noteOn(0, noteMIDI[i], 120);
         noteON[i]=ON;
         delay(5);
       }

    }
    else {
      noteON[i]=OFF;
      //Turn off the note with a given off/release velocity
      noteOff(0, noteMIDI[i], 60);
    }
  }  
}

//Send a MIDI note-on message.  Like pressing a piano key
//channel ranges from 0-15
void noteOn(byte channel, byte note, byte attack_velocity) {
  talkMIDI( (0x90 | channel), note, attack_velocity);
}

//Send a MIDI note-off message.  Like releasing a piano key
void noteOff(byte channel, byte note, byte release_velocity) {
  talkMIDI( (0x80 | channel), note, release_velocity);
}

//Plays a MIDI note. Doesn't check to see that cmd is greater than 127, or that data values are less than 127
void talkMIDI(byte cmd, byte data1, byte data2) {
  digitalWrite(ledPin, HIGH);
  mySerial.write(cmd);
  mySerial.write(data1);

  //Some commands only have one data byte. All cmds less than 0xBn have 2 data bytes 
  //(sort of: http://253.ccarh.org/handout/midiprotocol/)
  if( (cmd & 0xF0) <= 0xB0)
    mySerial.write(data2);

  digitalWrite(ledPin, LOW);
}

