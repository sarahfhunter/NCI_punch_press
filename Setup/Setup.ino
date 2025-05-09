/*Updated on 05/09/2025
Author: Sarah Hunter, Heidi Hunter and Steele Mason
Purpose: Setup
*/

/******************************READ ME***********************************/
/*
Basic Info About the Press:
- the punch press operates via a motor that runs a large flywheel.
- When the clutch is engaged, it follows a cyclical path up and down.
- the press has two air stops that need to be on in order to run the motor.
- the press uses a GemCo CAM to keep track of where the press is throughout it's cycle

The GemCo CAM states:
* degrees specified below are approximations
- there are four GemCo CAMs currently setup to detect the following positions (also shown in a diagram on the press):
  1. TDC (top dead center). When the press is between approx. -5°/+5° of the true top dead center, this GemCo CAM reads true
  2. Down stroke. When the press is between approx 5°-170° (just after TDC and just before bottom dead center)
  3. Indexer trigger. This is from approximately 195°-280° (idk) and indicates that the indexer is clear to move one stop
  4. TDC Stop. Triggered when the press is at approximately -15° from TDC. Clutch is disengaged at this point but momentum carries it to TDC ish

The Safety Features:
- these features are checked via the clearcore code:
  - two air valves are checked, if one isn't on, motor turns off (baked into hardware in this press, not code)
  - three motor stop buttons: one red button on the panel, and one button on the front of the press. When either pressed, motor turns off.
- these features are built into the circuitry:
  - E-stop
  - chain break switch (coming soon!)
  - ground fault sensor (coming soon!)

The Modes:
- There are 3 modes the press can operate in. To choose a mode, turn the selector key on the panel.

  1. Single Stroke (left position)
    - allows INCH mode when the press is at the top dead center (TDC) or downstroke, and completes the cycle when nearly at the bottom
  2. Inch (default, center position)
    - engages clutch when both palm buttons are pressed, disengages otherwise.
  3. Continuous (right position)
    - must start at TDC (top dead center)
    - must be armed by pressing CONTINUOUS_ARM_BUTTON on panel
    - once armed and at the TDC, pressing palm buttons will start it going continuously
    - stop continuous cycle by hitting the TOP_STOP_BUTTON on the front of the press. It will stop once it reaches TDC.

  Additional modes:
  - Enable light curtains
      - this is enabled during Setup(), not possible to update once clearcore is up and running. Need to e-stop to change it
  - Enable use of a servo indexer
  - Bumper stop enable
*/


#include "ClearCore.h"  //include clearcore library
#include <CcioPin.h>    //include clearcore library extender board
#define CcioPort ConnectorCOM1  //for extender board

// Define baud rate
#define baudRate 9600
#define TRIGGER_PULSE_TIME 25 // should this be 20?
#define SERVO_PPR 6400.0 //the number of pulses per revolution for the servo (change here)!
#define GEAR_REDUCER 5 //5 to 1
#define INDEXER_REDUCER 12 // 12 to 1 OR 6 to 1. Update if needed and it will update in the formula

/*************************************** DEFINE PINS ********************************************/
// MAIN BOARD
#define MOTOR_FW_CONTACTOR ConnectorIO0 // contactor to turn on motor
#define CLUTCH ConnectorIO1 // engage clutch
#define MOTOR_REV_CONTACTOR ConnectorIO2 // contactor for the motor that sends it in reverse
#define MOTOR_ON_BUTTON ConnectorIO3 
#define PALM_BUTTON_1 ConnectorDI7 //on press *interrupt
#define PALM_BUTTON_2 ConnectorDI8 //on press *interrupt
#define PALM_BUTTON_3 ConnectorA9 //on panel *interrupt
#define PALM_BUTTON_4 ConnectorA10 //on panel *interrupt
#define MOTOR_OFF_BUTTON ConnectorA11 //both the button on the press and the panel wired together *interrupt
#define LIGHT_CURTAIN ConnectorA12 // *interrupt
#define SERVO ConnectorM0 //servo motor

// EXPANSION BOARD A (INPUTS)
#define INDEXER_MODE_ENABLE CCIOA0
#define LIGHT_CURTAIN_ENABLE digitalRead(CCIOA1)
#define BUMPER_STOP_ENABLE CCIOA2
#define INDEXER_2_POS digitalRead(CCIOA3)
#define INDEXER_5_POS digitalRead(CCIOA4)
#define INDEXER_8_POS digitalRead(CCIOA5)
#define INDEXER_10_POS digitalRead(CCIOA6)
#define INDEXER_12_POS digitalRead(CCIOA7)

// EXPANSION BOARD B (INPUTS)
#define SS_MODE CCIOB0 //left
#define CONT_MODE CCIOB1 //right
#define RAM_FW CCIOB2
#define RAM_REV CCIOB3
#define MOTOR_FW CCIOB4
#define MOTOR_REV CCIOB5
#define INDEXER_FW CCIOB6
#define INDEXER_REV CCIOB7

// EXPANSION BOARD C (INPUTS)
#define CYCLE_INDEXER CCIOC0
#define ARM_CONTINUOUS_BUTTON CCIOC1
#define TOP_STOP_BUTTON CCIOC2 //panel and press wired together
#define AIR_1 CCIOC3 // check if air is flowing 
#define AIR_2 CCIOC4
#define TDC digitalRead(CCIOC5) // GemCo for TDC
#define DOWNSTROKE digitalRead(CCIOC6) // GemCo for Downstroke
#define TDC_STOP digitalRead(CCIOC7) // GemCo for TDC Stop

// EXPANSION BOARD D (EMPTY)
#define CLEAR_PATH digitalRead(CCIOD0) // GemCo for indexer 
#define PALM_BUTTONS_ON_PANEL !digitalRead(CCIOD1) //choose whether to use palm buttons on press or panel

// EXPANSION BOARD E (OUTPUTS)
#define MOTOR_ON_LIGHT CCIOE0
#define MOTOR_OFF_LIGHT CCIOE1
#define TDC_LIGHT CCIOE2
#define DOWNSTROKE_LIGHT CCIOE3
#define TDC_STOP_LIGHT CCIOE4 //rename from GEMCO_STOP_...
#define SS_LIGHT CCIOE5
#define ARM_CONTINUOUS_LIGHT CCIOE6
#define LIGHT_CURTAIN_ENABLED_LIGHT CCIOE7

// EXPANSION BOARD F (OUTPUTS)
#define CLEAR_PATH_LIGHT CCIOF0
#define COUNTER CCIOF1 // incremement counter
#define INDEXER_MODE_ENABLE_LIGHT CCIOF2

/******************************INITIALIZE BOOLS + SOME FUNCTION PROTOTYPES*********************************/
bool useButtonsOnPanel = true; //default is to use the palm buttons on the panel, not press

// Initialize button state and press time - volatile must be used for Arduino Interrupts
volatile bool button1Pressed = false;
volatile bool button2Pressed = false;
volatile bool button3Pressed = false;
volatile bool button4Pressed = false;

volatile uint32_t button1PressTime = 0;
volatile uint32_t button2PressTime = 0;
volatile uint32_t button3PressTime = 0;
volatile uint32_t button4PressTime = 0;

// For manual debounce
volatile long lastButton1PressTime = 0; 
volatile long lastButton2PressTime = 0; 
volatile long lastButton3PressTime = 0;
volatile long lastButton4PressTime = 0;
unsigned long debounceDelay = 50;  // 50ms debounce period

// Other bools for Single Stroke
bool ssStartedTDC = false;
bool motorOn = false; // bool for turning on motor
bool continuousModeArmed = false;  // Tracks whether continuous mode is armed
bool stopAtTop = false; // Tracks if the top stop button is pressed
bool enabledViaIndexer = false;

bool cycleBegun = false;
bool enableSS = true;
int numStrokes = 0; //number of times press has struck down (indexer gemco trigger)



void setup() {

  Serial.begin(baudRate);
  
  /**************************DEFINE INPUTS AND OUTPUTS******************************/
  /*-----------------------------main board----------------------------*/
  MOTOR_FW_CONTACTOR.Mode(Connector::OUTPUT_DIGITAL);
  CLUTCH.Mode(Connector::OUTPUT_DIGITAL);
  // OIL_PUMP.Mode(Connector::OUTPUT_DIGITAL);
  MOTOR_REV_CONTACTOR.Mode(Connector::OUTPUT_DIGITAL);
  MOTOR_ON_BUTTON.Mode(Connector::INPUT_DIGITAL);
  PALM_BUTTON_1.Mode(Connector::INPUT_DIGITAL);
  PALM_BUTTON_2.Mode(Connector::INPUT_DIGITAL);
  PALM_BUTTON_3.Mode(Connector::INPUT_DIGITAL);
  PALM_BUTTON_4.Mode(Connector::INPUT_DIGITAL);
  MOTOR_OFF_BUTTON.Mode(Connector::INPUT_DIGITAL);
  LIGHT_CURTAIN.Mode(Connector::INPUT_DIGITAL);

  /*--------------------------- extender boards A-F --------------------------*/
  CcioPort.Mode(Connector::CCIO); //For CCIIO-8 (extender boards)
  CcioPort.PortOpen();            //For CCIIO-8 (extender boards)

  // Set all pins on CCIOA (Extender Board 1) to be inputs
  for (int pin = CLEARCORE_PIN_CCIOA0; pin <= CLEARCORE_PIN_CCIOA7; pin++) {
      pinMode(pin, INPUT);
  }

  // Set all pins on CCIOB (Extender Board 2) to be INPUTS
  for (int pin = CLEARCORE_PIN_CCIOB0; pin <= CLEARCORE_PIN_CCIOB7; pin++) {
      pinMode(pin, INPUT);
  }

  // Set all pins on CCIOC (Extender Board 3) to be INPUTS
  for (int pin = CLEARCORE_PIN_CCIOC0; pin <= CLEARCORE_PIN_CCIOC7; pin++) {
      pinMode(pin, INPUT);
  }

  // Set all pins on CCIOD (Extender Board 4) to be INPUTS? empty rn
  for (int pin = CLEARCORE_PIN_CCIOD0; pin <= CLEARCORE_PIN_CCIOD7; pin++) {
      pinMode(pin, INPUT);
  }

  // Set all pins on CCIOB (Extender Board 5) to be Outputs
  for (int pin = CLEARCORE_PIN_CCIOE0; pin <= CLEARCORE_PIN_CCIOE7; pin++) {
      pinMode(pin, OUTPUT);
  }

  // Set all pins on CCIOB (Extender Board 6) to be Outputs
  for (int pin = CLEARCORE_PIN_CCIOF0; pin <= CLEARCORE_PIN_CCIOF7; pin++) {
      pinMode(pin, OUTPUT);
  }
  // if you need more extender boards, simply copy the code above and change it to C or D, etc

  Serial.println("done initializing in setup");


  /***************************** SETUP SERVO MOTOR ******************************************/ 
  MotorMgr.MotorModeSet(MotorManager::MOTOR_ALL, Connector::CPM_MODE_STEP_AND_DIR);
  SERVO.HlfbMode(MotorDriver::HLFB_MODE_HAS_BIPOLAR_PWM);
  SERVO.HlfbCarrier(MotorDriver::HLFB_CARRIER_482_HZ);
  SERVO.VelMax(INT32_MAX); // set max velocity
  SERVO.AccelMax(INT32_MAX); // setup max accel
  
  // wait five seconds for a port to open before starting motor stuff?
  uint32_t timeout = 5000;
  uint32_t startTime = millis();
  while(!Serial && millis() - startTime < timeout) {
    continue;
  }

  SERVO.EnableRequest(true);
  Serial.println("Motor Enabled");

  //waits for HLFB to assert (waits for homing to complete if applicable)
  Serial.println("Waiting for HLFB...");
  timeout = 5000; // 5 second timeout
  startTime = millis();
  while (SERVO.HlfbState() != MotorDriver::HLFB_ASSERTED) {
      if (millis() - startTime > timeout) {
          Serial.println("Timeout waiting for HLFB. Motor may not be connected.");
          break;
      }
  }
  Serial.println("Motor Ready");
  
  /************************************ATTACH INTERRUPTS**************************************/
  /*How to Use Interrupts: 
  - Each input that you want to attach an interrupt to needs to have a few things: 
      - declare it as an input (preferrably on *main clearcore board*, not extender). Ex: MOTOR_OFF_BUTTON.Mode(Connector::INPUT_DIGITAL);
      - specify an interrupt service routine (ISR) function that defines what will happen when the interrupt is triggered. Ex: StopISR()
      - attach Interrupt. see: https://www.arduino.cc/reference/cs/language/functions/external-interrupts/attachinterrupt/
        - Ex: attachInterrupt(digitalPinToInterrupt(7), StopISR, RISING); 
          - arguments:
            - digitalPinToInterrupt(<insert last digit of port #>). Ex: for port DI7, write digitalPinToInterrupt(7)
            - name of interrupt service routine function (StopISR)
            - type of interrupt: FALLING (triggers when HIGH -> LOW), RISING (triggers when LOW -> HIGH), CHANGE (triggers with any change), LOW (whenever it is low), HIGH (whenever it is high)
            - **typically** RISING or FALLING are better than just HIGH or LOW because then it only triggers if it previously was in the opposite state (which is often the goal)
      - NB: you cannot attach interrupts to expansion boards (I don't think)
  */ 
  attachInterrupt(digitalPinToInterrupt(7), button1ISR, RISING); // PALM_BUTTON_1 1 interrupt
  attachInterrupt(digitalPinToInterrupt(8), button2ISR, RISING);  // PALM_BUTTON_2 interrupt
  attachInterrupt(digitalPinToInterrupt(9), button3ISR, RISING);  // PALM_BUTTON_3 interrupt
  attachInterrupt(digitalPinToInterrupt(10), button4ISR, RISING);  // PALM_BUTTON_4 interrupt
  attachInterrupt(digitalPinToInterrupt(11), StopISR, RISING); //MOTOR_OFF_BUTTON interrupt
  if (LIGHT_CURTAIN_ENABLE) {
    attachInterrupt(digitalPinToInterrupt(12), LightCurtainRoutine, LOW); //LIGHT CURTAIN, LOW bc NC, and not falling bc it needs to trigger ANYTIME something is detected, not just the first time 
  }

  Serial.println("done attaching interrupts");

  /***********************************initialize stuff**********************************/
  // Initializes all lights to start off 
  digitalWrite(MOTOR_ON_LIGHT, false);
  digitalWrite(MOTOR_OFF_LIGHT, true);
  digitalWrite(TDC_LIGHT, false);
  digitalWrite(DOWNSTROKE_LIGHT, false);

  digitalWrite(TDC_STOP_LIGHT, false);
  digitalWrite(SS_LIGHT, false);
  digitalWrite(ARM_CONTINUOUS_LIGHT, false);
  digitalWrite(LIGHT_CURTAIN_ENABLED_LIGHT, false);

  //initialize motor and clutch to off/disengaged
  MOTOR_FW_CONTACTOR.State(false);
  MOTOR_REV_CONTACTOR.State(false);
  CLUTCH.State(false); 

  // Initialize local mode flags to be false
  TurnOffSS();
  TurnOffCont();
}

