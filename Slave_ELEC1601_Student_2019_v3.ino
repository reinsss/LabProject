//-----------------------------------------------------------------------------------------------------------//
//                                                                                                           //
//  Slave_ELEC1601_Student_2019_v3                                                                           //
//  The Instructor version of this code is identical to this version EXCEPT it also sets PIN codes           //
//  20191008 Peter Jones                                                                                     //
//                                                                                                           //
//  Bi-directional passing of serial inputs via Bluetooth                                                    //
//  Note: the void loop() contents differ from "capitalise and return" code                                  //
//                                                                                                           //
//  This version was initially based on the 2011 Steve Chang code but has been substantially revised         //
//  and heavily documented throughout.                                                                       //
//                                                                                                           //
//  20190927 Ross Hutton                                                                                     //
//  Identified that opening the Arduino IDE Serial Monitor asserts a DTR signal which resets the Arduino,    //
//  causing it to re-execute the full connection setup routine. If this reset happens on the Slave system,   //
//  re-running the setup routine appears to drop the connection. The Master is unaware of this loss and      //
//  makes no attempt to re-connect. Code has been added to check if the Bluetooth connection remains         //
//  established and, if so, the setup process is bypassed.                                                   //
//                                                                                                           //
//-----------------------------------------------------------------------------------------------------------//

#include <SoftwareSerial.h>   //Software Serial Port
#include <Servo.h>;

#define RxD 7
#define TxD 6
#define ConnStatus A1
#define PIN_IRLEFT_IN 10
#define PIN_IRLEFT_OUT 9
#define PIN_IRRIGHT_IN 3
#define PIN_IRRIGHT_OUT 2

#define DEBUG_ENABLED  1

// ##################################################################################
// ### EDIT THE LINES BELOW TO MATCH YOUR SHIELD NUMBER AND CONNECTION PIN OPTION ###
// ##################################################################################

int shieldPairNumber = 12;

// CAUTION: If ConnStatusSupported = true you MUST NOT use pin A1 otherwise "random" reboots will occur
// CAUTION: If ConnStatusSupported = true you MUST set the PIO[1] switch to A1 (not NC)

boolean ConnStatusSupported = true;   // Set to "true" when digital connection status is available on Arduino pin

// #######################################################

// The following two string variable are used to simplify adaptation of code to different shield pairs

String slaveNameCmd = "\r\n+STNA=Slave";   // This is concatenated with shieldPairNumber later

SoftwareSerial blueToothSerial(RxD,TxD);

Servo servoLeft;
Servo servoRight;

int spdLeft;
int spdRight;
int irLeft;
int irRight;

char stateJoy;
char stateButton;

boolean flagAuto;

void setup()
{
    Serial.begin(9600);
    blueToothSerial.begin(38400);                    // Set Bluetooth module to default baud rate 38400
    
    pinMode(RxD, INPUT);
    pinMode(TxD, OUTPUT);
    pinMode(ConnStatus, INPUT);



    pinMode(PIN_IRLEFT_IN, INPUT);                         
    pinMode(PIN_IRLEFT_OUT, OUTPUT);      
    pinMode(PIN_IRRIGHT_IN, INPUT);                         
    pinMode(PIN_IRRIGHT_OUT, OUTPUT);

    

    //  Check whether Master and Slave are already connected by polling the ConnStatus pin (A1 on SeeedStudio v1 shield)
    //  This prevents running the full connection setup routine if not necessary.

    if(ConnStatusSupported) Serial.println("Checking Slave-Master connection status.");

    if(ConnStatusSupported && digitalRead(ConnStatus)==1)
    {
        Serial.println("Already connected to Master - remove USB cable if reboot of Master Bluetooth required.");
    }
    else
    {
        Serial.println("Not connected to Master.");
        
        setupBlueToothConnection();   // Set up the local (slave) Bluetooth module

        delay(1000);                  // Wait one second and flush the serial buffers
        Serial.flush();                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               
        blueToothSerial.flush();
    }
}


void loop()
{
    char recvChar;
    delay(1000);

    

      while(1)
    {   
        servoLeft.attach(13);
        servoRight.attach(12);
        //spdLeft = 1700;
        //spdRight = 1300;
        delay(50);
        stateButton =  blueToothSerial.read();
        stateJoy = blueToothSerial.read();
        Serial.println(stateButton);
        Serial.println(stateJoy);
        Serial.println("slave");

    if(blueToothSerial.available())   // Check if there's any data sent from the remote Bluetooth shield
    {
        //stateJoy = blueToothSerial.read();
        //Serial.print("slave");
    }

    if(Serial.available())            // Check if there's any data sent from the local serial terminal. You can add the other applications here.
    {   
        
        //stateButton = Serial.read();
        //Serial.print(stateJoy);
        //blueToothSerial.print(stateJoy);
    }
    
    if (stateButton == 't') {
        flagAuto = true;
        }

    // track
    if (!flagAuto) {
        // the car controlled by the joystick
        if (stateJoy == '0') {
        // turn left
        for (int i = 0; i < 200; i+=10) {
            servoLeft.writeMicroseconds(1500-i); 
            servoRight.writeMicroseconds(1500);                             
        }
        } else if (stateJoy == '1') {
        // turn right
        for (int i = 0; i < 200; i+= 10) {
            servoLeft.writeMicroseconds(1500);  
            servoRight.writeMicroseconds(1500-i);                            
        }
        } else if (stateJoy == '2') {
        // forward
        servoLeft.writeMicroseconds(1800); 
        servoRight.writeMicroseconds(1200);
        
        } else if (stateJoy == '3') {
            // back
            servoLeft.writeMicroseconds(1200); 
            servoRight.writeMicroseconds(1800);

        } else if (stateJoy == '4') {

          }
        servoLeft.detach();                           // Stop sending servo signal
        servoRight.detach();
        delay(100);
    }

    // maze
    if (flagAuto) {
        int irLeft = irDetect(9, 10, 38000);
        int irRight = irDetect(2, 3, 38000);

        if (irLeft == 0 && irRight == 1) { // RHS obj, turn right
            for (int i = 0; i < 50; i+= 10) {
                servoLeft.writeMicroseconds(1500);  
                servoRight.writeMicroseconds(1500-i);                            
            }
        } else if (irRight == 0 && irLent == 1) { // LHS obj, turn left
            for (int i = 0; i < 200; i+=10) {
                servoLeft.writeMicroseconds(1500-i); 
                servoRight.writeMicroseconds(1500);                             
            }
        } else if (irLeft == 1 && irRight == 1) { // on the way, go straight
            servoLeft.writeMicroseconds(1800); 
            servoRight.writeMicroseconds(1200);
            delay(100);
        } else if (irLeft == 0 && irRight == 0) { //deviate // go back
            servoLeft.writeMicroseconds(1200); 
            servoRight.writeMicroseconds(1800);
            delay(100);
        }

    }

    }
}
  

int irDetect(int irLedPin, int irReceiverPin, long frequency)
{
  tone(irLedPin, frequency, 8);              // IRLED 38 kHz for at least 1 ms
  delay(1);                                  // Wait 1 ms
  int ir = digitalRead(irReceiverPin);       // IR receiver -> ir variable
  delay(1);                                  // Down time before recheck
  return ir;                                 // Return 1 no detect, 0 detect
}  

void setupBlueToothConnection()
{
    Serial.println("Setting up the local (slave) Bluetooth module.");

    slaveNameCmd += shieldPairNumber;
    slaveNameCmd += "\r\n";

    blueToothSerial.print("\r\n+STWMOD=0\r\n");      // Set the Bluetooth to work in slave mode
    blueToothSerial.print(slaveNameCmd);             // Set the Bluetooth name using slaveNameCmd
    blueToothSerial.print("\r\n+STAUTO=0\r\n");      // Auto-connection should be forbidden here
    blueToothSerial.print("\r\n+STOAUT=1\r\n");      // Permit paired device to connect me
    
    //  print() sets up a transmit/outgoing buffer for the string which is then transmitted via interrupts one character at a time.
    //  This allows the program to keep running, with the transmitting happening in the background.
    //  Serial.flush() does not empty this buffer, instead it pauses the program until all Serial.print()ing is done.
    //  This is useful if there is critical timing mixed in with Serial.print()s.
    //  To clear an "incoming" serial buffer, use while(Serial.available()){Serial.read();}

    blueToothSerial.flush();
    delay(2000);                                     // This delay is required

    blueToothSerial.print("\r\n+INQ=1\r\n");         // Make the slave Bluetooth inquirable
    
    blueToothSerial.flush();
    delay(2000);                                     // This delay is required
    
    Serial.println("The slave bluetooth is inquirable!");
}
