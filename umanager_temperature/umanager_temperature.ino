/*

Based on the origianl Arduino code for AOTF control and interface with uManager

https://github.com/micro-manager/micro-manager/tree/master/DeviceAdapters/Arduino/AOTFcontroller

To connect to Micro-Manager, add Arduino-Hub device and create Group with single
read-only property, Arduino-Input-AnalogInput0
To update temperature, type any number into the property in the main MicroManager window

*/

// Temp sensor code:
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;

   unsigned int version_ = 2;

   // pin on which to receive the trigger (2 and 3 can be used with interrupts, although this code does not use interrupts)
   //int inPin_ = 2;
   // to read out the state of inPin_ faster, use
   int inPinBit_ = 1 ;//<< inPin_;  // bit mask

   // pin connected to DIN of TLV5618
   int dataPin = 3;
   // pin connected to SCLK of TLV5618
   int clockPin = 4;
   // pin connected to CS of TLV5618
   int latchPin = 5;

   const int SEQUENCELENGTH = 12;  // this should be good enough for everybody;)
   byte triggerPattern_[SEQUENCELENGTH] = {0,0,0,0,0,0,0,0,0,0,0,0};
   unsigned int triggerDelay_[SEQUENCELENGTH] = {0,0,0,0,0,0,0,0,0,0,0,0};
   int patternLength_ = 0;
   byte repeatPattern_ = 0;
   volatile long triggerNr_; // total # of triggers in this run (0-based)
   volatile long sequenceNr_; // # of trigger in sequence (0-based)
   int skipTriggers_ = 0;  // # of triggers to skip before starting to generate patterns
   byte currentPattern_ = 0;
   const unsigned long timeOut_ = 1000;
   bool blanking_ = false;
   bool blankOnHigh_ = false;
   bool triggerMode_ = false;
   boolean triggerState_ = false;



 void setup() {
  // temp sensor
  sensors.begin();
  sensors.getAddress(insideThermometer, 0);
  sensors.setResolution(insideThermometer, 11); // set 11 bit temperature sensor
  sensors.requestTemperaturesByIndex(0);

   // Higher speeds do not appear to be reliable
   Serial.begin(57600);

   //pinMode(inPin_, INPUT);
   pinMode (dataPin, OUTPUT);
   pinMode (clockPin, OUTPUT);
   pinMode (latchPin, OUTPUT);
   pinMode(8, OUTPUT);
   pinMode(9, OUTPUT);
   pinMode(10, OUTPUT);
   pinMode(11, OUTPUT);
   pinMode(12, OUTPUT);
   pinMode(13, OUTPUT);

   // Set analogue pins as input:
   DDRC = DDRC & B11000000;
   // Turn on build-in pull-up resistors
   PORTC = PORTC | B00111111;

   digitalWrite(latchPin, HIGH);
 }

 void loop() {

   if (Serial.available() > 0) {
     int inByte = Serial.read();
     switch (inByte) {

       // Set digital output
       case 1 :
          if (waitForSerial(timeOut_)) {
            currentPattern_ = Serial.read();
            // Do not set bits 6 and 7 (not sure if this is needed..)
            currentPattern_ = currentPattern_ & B00111111;
            if (!blanking_)
              PORTB = currentPattern_;
            Serial.write( byte(1));
          }
          break;

       // Get digital output
       case 2:
          Serial.write( byte(2));
          Serial.write( PORTB);
          break;

       // Set Analogue output (TODO: save for 'Get Analogue output')
       case 3:
         if (waitForSerial(timeOut_)) {
           int channel = Serial.read();
           if (waitForSerial(timeOut_)) {
              byte msb = Serial.read();
              msb &= B00001111;
              if (waitForSerial(timeOut_)) {
                byte lsb = Serial.read();
                analogueOut(channel, msb, lsb);
                Serial.write( byte(3));
                Serial.write( channel);
                Serial.write(msb);
                Serial.write(lsb);
              }
           }
         }
         break;

       // Sets the specified digital pattern
       case 5:
          if (waitForSerial(timeOut_)) {
            int patternNumber = Serial.read();
            if ( (patternNumber >= 0) && (patternNumber < SEQUENCELENGTH) ) {
              if (waitForSerial(timeOut_)) {
                triggerPattern_[patternNumber] = Serial.read();
                triggerPattern_[patternNumber] = triggerPattern_[patternNumber] & B00111111;
                Serial.write( byte(5));
                Serial.write( patternNumber);
                Serial.write( triggerPattern_[patternNumber]);
                break;
              }
            }
          }
          Serial.write( "n:");//Serial.print("n:");
          break;

       // Sets the number of digital patterns that will be used
       case 6:
         if (waitForSerial(timeOut_)) {
           int pL = Serial.read();
           if ( (pL >= 0) && (pL <= 12) ) {
             patternLength_ = pL;
             Serial.write( byte(6));
             Serial.write( patternLength_);
           }
         }
         break;

       // Skip triggers
       case 7:
         if (waitForSerial(timeOut_)) {
           skipTriggers_ = Serial.read();
           Serial.write( byte(7));
           Serial.write( skipTriggers_);
         }
         break;

       //  starts trigger mode
       case 8:
         if (patternLength_ > 0) {
           sequenceNr_ = 0;
           triggerNr_ = -skipTriggers_;
           triggerState_ = false; //digitalRead(inPin_) == HIGH;
           PORTB = B00000000;
           Serial.write( byte(8));
           triggerMode_ = true;
         }
         break;

         // return result from last triggermode
       case 9:
          triggerMode_ = false;
          PORTB = B00000000;
          Serial.write( byte(9));
          Serial.write( triggerNr_);
          break;

       // Sets time interval for timed trigger mode
       // Tricky part is that we are getting an unsigned int as two bytes
       case 10:
          if (waitForSerial(timeOut_)) {
            int patternNumber = Serial.read();
            if ( (patternNumber >= 0) && (patternNumber < SEQUENCELENGTH) ) {
              if (waitForSerial(timeOut_)) {
                unsigned int highByte = 0;
                unsigned int lowByte = 0;
                highByte = Serial.read();
                if (waitForSerial(timeOut_))
                  lowByte = Serial.read();
                highByte = highByte << 8;
                triggerDelay_[patternNumber] = highByte | lowByte;
                Serial.write( byte(10));
                Serial.write(patternNumber);
                break;
              }
            }
          }
          break;

       // Sets the number of times the patterns is repeated in timed trigger mode
       case 11:
         if (waitForSerial(timeOut_)) {
           repeatPattern_ = Serial.read();
           Serial.write( byte(11));
           Serial.write( repeatPattern_);
         }
         break;

       //  starts timed trigger mode
       case 12:
         if (patternLength_ > 0) {
           PORTB = B00000000;
           Serial.write( byte(12));
           for (byte i = 0; i < repeatPattern_ && (Serial.available() == 0); i++) {
             for (int j = 0; j < patternLength_ && (Serial.available() == 0); j++) {
               PORTB = triggerPattern_[j];
               delay(triggerDelay_[j]);
             }
           }
           PORTB = B00000000;
         }
         break;

       // Blanks output based on TTL input
       case 20:
         blanking_ = true;
         Serial.write( byte(20));
         break;

       // Stops blanking mode
       case 21:
         blanking_ = false;
         Serial.write( byte(21));
         break;

       // Sets 'polarity' of input TTL for blanking mode
       case 22:
         if (waitForSerial(timeOut_)) {
           int mode = Serial.read();
           if (mode==0)
             blankOnHigh_= true;
           else
             blankOnHigh_= false;
         }
         Serial.write( byte(22));
         break;

       // Gives identification of the device
       case 30:
         Serial.println("MM-Ard");
         break;

       // Returns version string
       case 31:
         Serial.println(version_);
         break;

       case 40:
         Serial.write( byte(40));
         Serial.write( PINC);
         break;

       case 41:

         if (waitForSerial(timeOut_)) {
           int pin = Serial.read();
           sensors.requestTemperaturesByIndex(0);
           int val = round(sensors.getTempC(insideThermometer)*10);
           if (pin >= 0 && pin <=5) {
              //int val = random(666); analogRead(pin);


              Serial.write( byte(41));
              Serial.write( pin);
              Serial.write( highByte(val));
              Serial.write( lowByte(val));

           }

         }

         break;

       case 42:
         if (waitForSerial(timeOut_)) {
           int pin = Serial.read();
           if (waitForSerial(timeOut_)) {
             int state = Serial.read();
             Serial.write( byte(42));
             Serial.write( pin);
             if (state == 0) {
                digitalWrite(14+pin, LOW);
                Serial.write( byte(0));
             }
             if (state == 1) {
                digitalWrite(14+pin, HIGH);
                Serial.write( byte(1));
             }
           }
         }
         break;

       }
    }

    // In trigger mode, we will blank even if blanking is not on..
    if (triggerMode_) {
      boolean tmp = PIND & inPinBit_;
      if (tmp != triggerState_) {
        if (blankOnHigh_ && tmp ) {
          PORTB = 0;
        }
        else if (!blankOnHigh_ && !tmp ) {
          PORTB = 0;
        }
        else {
          if (triggerNr_ >=0) {
            PORTB = triggerPattern_[sequenceNr_];
            sequenceNr_++;
            if (sequenceNr_ >= patternLength_)
              sequenceNr_ = 0;
          }
          triggerNr_++;
        }

        triggerState_ = tmp;
      }
    } else if (blanking_) {
      if (blankOnHigh_) {
        if (! (PIND & inPinBit_))
          PORTB = currentPattern_;
        else
          PORTB = 0;
      }  else {
        if (! (PIND & inPinBit_))
          PORTB = 0;
        else
          PORTB = currentPattern_;
      }
    }
}


bool waitForSerial(unsigned long timeOut)
{
    unsigned long startTime = millis();
    while (Serial.available() == 0 && (millis() - startTime < timeOut) ) {}
    if (Serial.available() > 0)
       return true;
    return false;
 }

// Sets analogue output in the TLV5618
// channel is either 0 ('A') or 1 ('B')
// value should be between 0 and 4095 (12 bit max)
// pins should be connected as described above
void analogueOut(int channel, byte msb, byte lsb)
{
  digitalWrite(latchPin, LOW);
  msb &= B00001111;
  if (channel == 0)
     msb |= B10000000;
  // Note that in all other cases, the data will be written to DAC B and BUFFER
  shiftOut(dataPin, clockPin, MSBFIRST, msb);
  shiftOut(dataPin, clockPin, MSBFIRST, lsb);
  // The TLV5618 needs one more toggle of the clockPin:
  digitalWrite(clockPin, HIGH);
  digitalWrite(clockPin, LOW);
  digitalWrite(latchPin, HIGH);
}
