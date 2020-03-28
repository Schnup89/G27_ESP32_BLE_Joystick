#include <BleGamepad.h> 
#include "KY040rotary.h"

BleGamepad bleGamepad;

//Tasker: handle Bluetooth an Rotary's on different Cores 
TaskHandle_t TaskBT;
TaskHandle_t TaskBTN;

//Global Variables Thread
int nSendBtn = 0;
int nLastBtn = 0;   //Debounce, see function BTCMD


//KnobLeft-Pins
const int pKnobLeftCLK = 25;
const int pKnobLeftDT = 05;
const int pKnobLeftSW = 12;
//KnobRight-Pins
const int pKnobRightCLK = 19;
const int pKnobRightDT = 27;
const int pKnobRightSW = 02;

//Define Rotary Object
KY040 KnobLeft(pKnobLeftCLK,pKnobLeftDT,pKnobLeftSW);
KY040 KnobRight(pKnobRightCLK,pKnobRightDT,pKnobRightSW);


//Set Command and Debounce Rotarys
void BTCmd(int btn) {
  //Dont debounce Buttons
  if (btn == BUTTON_1 || btn == BUTTON_4) {
    nSendBtn = btn; 
  }else{
    //Debounce Rotarys
    if (nLastBtn == btn) {
      nSendBtn = btn;
    }
    nLastBtn = btn;
  }
}

void fKnobLeft_Pressed(void) {
  Serial.println("KnobLeft: clicked");
  BTCmd(BUTTON_1);
}

void fKnobLeft_CClock(void) {
  Serial.println("KnobLeft: rotating left");
  BTCmd(BUTTON_2);
}

void fKnobLeft_Clock(void) {
  Serial.println("KnobLeft: rotating right");
  BTCmd(BUTTON_3);
}

void fKnobRight_Pressed(void) {
  Serial.println("KnobRight: clicked");
  BTCmd(BUTTON_4);
}

void fKnobRight_CClock(void) {
  Serial.println("KnobRight: rotating left");
  BTCmd(BUTTON_5);
}

void fKnobRight_Clock(void) {
  Serial.println("KnobRight: rotating right");
  BTCmd(BUTTON_6);
}


void setup() {
  Serial.begin(115200);
  
  //#### GamePad Bluetooth
  Serial.println("Starting BLE work!");
  bleGamepad.begin();
  //!!!! GamePad Bluetooth

  //#### Set Rotary to CPU 1
  xTaskCreatePinnedToCore(
                loopRotary,   /* Task function. */
                "BTNSend",     /* name of task. */
                10000,       /* Stack size of task */
                NULL,        /* parameter of the task */
                1,           /* priority of the task */
                &TaskBTN,      /* Task handle to keep track of created task */
                1);          /* pin task to core 0 */ 
  
  //#### Set Bluetooth to CPU 1  
  xTaskCreatePinnedToCore(
                procBTCMD,   /* Task function. */
                "BTSend",     /* name of task. */
                10000,       /* Stack size of task */
                NULL,        /* parameter of the task */
                1,           /* priority of the task */
                &TaskBT,      /* Task handle to keep track of created task */
                0);          /* pin task to core 0 */   
}

//Send Bluetooth Button on Change of nSendBtn 
void procBTCMD(void * pvParameters ) {
  while (1) {
    if (nSendBtn != 0) {
      Serial.println("Send");
      int btn = nSendBtn;
      if(bleGamepad.isConnected()) {
        bleGamepad.press(btn);
        delay(150);
        bleGamepad.release(btn);
      }
      nSendBtn = 0;
    }
   delay(1);    // <- Watchdog needs this
  }
}

//Send Bluetooth Button on Change of nSendBtn 
void loopRotary(void * pvParameters ) {
  //Initialize the Rotarys
  Serial.println("Init Rotarys!");
  if (!KnobLeft.Begin() ) {
    Serial.println("unable to init rotate button");
    while (1);
  }
  if (!KnobRight.Begin() ) {
    Serial.println("unable to init rotate button");
    while (1);
  }
  //Set Functions on Event
  KnobLeft.OnButtonClicked(fKnobLeft_Pressed);
  KnobLeft.OnButtonLeft(fKnobLeft_CClock);
  KnobLeft.OnButtonRight(fKnobLeft_Clock);
  KnobRight.OnButtonClicked(fKnobRight_Pressed);
  KnobRight.OnButtonLeft(fKnobRight_CClock);
  KnobRight.OnButtonRight(fKnobRight_Clock);
  Serial.println("KY-040 rotary encoder OK");
  
  //Proccess Rotray
  while (true) {
    KnobLeft.Process( millis() );
    KnobRight.Process( millis() );
  }
}


void loop() {

}
