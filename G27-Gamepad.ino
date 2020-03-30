#include <BleGamepad.h> 
#include <Rotary.h>
#include <EasyButton.h>

BleGamepad bleGamepad;

//Tasker: handle Bluetooth an Rotary's on different Cores 
TaskHandle_t TaskBT;
TaskHandle_t TaskBTN;

//Global Variables Thread
int nSendBtn = 0;

//KnobLeft-Pins
const int pKnobLeftCLK = 25;
const int pKnobLeftDT = 05;
const int pKnobLeftSW = 12;   //Change this PIN to 11 or 13 since it's a bootstrapping-pin, see: https://github.com/espressif/esp-idf/tree/master/examples/storage/sd_card#note-about-gpio12-esp32-only
//KnobRight-Pins
const int pKnobRightCLK = 19;
const int pKnobRightDT = 27;
const int pKnobRightSW = 02;

//Define Rotary Object
Rotary KnobLeft = Rotary(pKnobLeftCLK,pKnobLeftDT);
Rotary KnobRight =  Rotary(pKnobRightCLK,pKnobRightDT);
EasyButton ButtonLeft(pKnobLeftSW);
EasyButton ButtonRight(pKnobRightSW);


void fKnobLeft_Pressed(void) {
  Serial.println("KnobLeft: clicked");
  nSendBtn = (BUTTON_1);
}

void fKnobLeft_CClock(void) {
  Serial.println("KnobLeft: rotating right");
  nSendBtn = (BUTTON_2);
}

void fKnobLeft_Clock(void) {
  Serial.println("KnobLeft: rotating left");
  nSendBtn = (BUTTON_3);
}

void fKnobRight_Pressed(void) {
  Serial.println("KnobRight: clicked");
  nSendBtn = (BUTTON_4);
}

void fKnobRight_CClock(void) {
  Serial.println("KnobRight: rotating right");
  nSendBtn = (BUTTON_5);
}

void fKnobRight_Clock(void) {
  Serial.println("KnobRight: rotating left");
  nSendBtn = (BUTTON_6);
}

void ButtonLeftISR()
{
  //When button is being used through external interrupts, parameter INTERRUPT must be passed to read() function
  ButtonLeft.read(INTERRUPT);
}
void ButtonRightISR()
{
  //When button is being used through external interrupts, parameter INTERRUPT must be passed to read() function
  ButtonRight.read(INTERRUPT);
}

void setup() {
  Serial.begin(115200);
  
  //#### GamePad Bluetooth
  Serial.println("Starting BLE work!");
  bleGamepad.begin();
  //!!!! GamePad Bluetooth

  //#### Setup Buttons
  ButtonLeft.begin();
  ButtonRight.begin();
  ButtonLeft.onPressed(fKnobLeft_Pressed);
  ButtonRight.onPressed(fKnobRight_Pressed);
  if (ButtonLeft.supportsInterrupt())
  {
    ButtonLeft.enableInterrupt(ButtonLeftISR);
    Serial.println("Button will be used through interrupts");
  }
  if (ButtonRight.supportsInterrupt())
  {
    ButtonRight.enableInterrupt(ButtonRightISR);
    Serial.println("Button will be used through interrupts");
  }
  //!!!! Setup Buttons

  
  //#### Set Rotary to CPU 1
  xTaskCreatePinnedToCore(
                loopRotary,   /* Task function. */
                "BTNSend",     /* name of task. */
                10000,       /* Stack size of task */
                NULL,        /* parameter of the task */
                1,           /* priority of the task */
                &TaskBTN,      /* Task handle to keep track of created task */
                1);          /* pin task to core 0 */ 
  
  //#### Set Bluetooth to CPU 0 
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
  //Proccess Rotray
  while (true) {
    unsigned char KLres = KnobLeft.process();
    if (KLres == DIR_CW) {
      fKnobLeft_Clock();
    } else if (KLres == DIR_CCW) {
      fKnobLeft_CClock();
    }
    unsigned char KRres = KnobRight.process();
    if (KRres == DIR_CW) {
      fKnobRight_Clock();
    } else if (KRres == DIR_CCW) {
      fKnobRight_CClock();
    }
  }
}

void loop() {

}
