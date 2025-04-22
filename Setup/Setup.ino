/* FINAL VERSION, Updated on 2/21/2025
Purpose: Blue Punch Press
Author: Heidi Hunter, Steele Mason and Sarah Hunter
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
  3. TDC Stop 1. Triggered when the press is at approximately -20° from TDC. This is the first indicator that we are getting close to TDC
  4. TDC Stop 2. Triggered when press is at approx -10° from TDC. This is the second, backup indicator that we are getting close to TDC

The Safety Features:
- these features are checked via the clearcore code:
  - two air valves are checked, if one isn't on, motor turns off
  - two motor stop buttons: one red button on the panel, and one button on the front of the press. When either pressed, motor turns off.
- these features are built into the circuitry:
  - E-stop
  - motor-on contactor switch (green button on front of panel)
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
*/

/*************************PIN INVENTORY*********************************/
/*
MAIN BOARD
IO0 - motor-on light (output)
IO1 - contactor for motor (output)
IO2 - clutch engage/disengage (output)
IO3 - red light (motor off)
IO4 - air1 (input)
IO5 - air2 (input)
DI6 - motor on button (input)
DI7 - motor off button (input) *interrupt
DI8 - palm button 1 (left, input) *interrupt
A9 - palm button 2 (right, input) *interrupt
A10 - Continuous mode (input)
A11 - Single Stroke (SS) mode (input)
A12 - extra stop button (motor off) close to palm buttons, NC *interrupt

EXTENDER BOARD A INPUTS
CCIOA0 - top stop button 
CCIOA1 - GemCo1
CCIOA2 - GemCo2
CCIOA3 - GemCo3 
CCIOA4 - GemCo4
CCIOA5 - arm continuous button (input) 
CCIOA6 - chain break??

EXTENDER BOARD B OUTPUTS //todo switch around to have gemco lights be 1-4?
CCIOB0 - green led (GemCo stop 2)
CCIOB1 - orange led (GemCo stop 1)
CCIOB2 - red led (downstroke)
CCIOB3 - blue led (TDC)
CCIOB4 - green led (SS mode) // will eventually be orange I believe
CCIOB5 - orange led (continuous mode armed)
CCIOB6 - counter
*/

#include "ClearCore.h"  //include clearcore library
#include <CcioPin.h>    //include clearcore library extender board
#define CcioPort ConnectorCOM1  //for extender board

//Define Button inputs
#define PALM_BUTTON_1 ConnectorDI8 
#define PALM_BUTTON_2 ConnectorA9
#define MOTOR_ON_BUTTON ConnectorDI6
#define MOTOR_OFF_BUTTON ConnectorDI7
#define AIR_1 ConnectorIO4 // check if air is flowing 
#define AIR_2 ConnectorIO5
#define MOTOR_OFF_BUTTON_2 ConnectorA12 // extra stop button by the palm buttons
#define TOP_STOP_BUTTON CCIOA0 
#define ARM_CONTINUOUS_BUTTON CCIOA5

//Define Light outputs
#define MOTOR_ON_LIGHT ConnectorIO0 // big green one
#define MOTOR_OFF_LIGHT ConnectorIO3 // big red one
#define SS_LIGHT CCIOB4 // green
#define ARM_CONTINUOUS_LIGHT CCIOB5 // ORANGE

// define gemco light outputs
#define TDC_LIGHT CCIOB3 // blue
#define DOWNSTROKE_LIGHT CCIOB2 // red
#define GEMCO_STOP_1_LIGHT CCIOB1 // orange
#define GEMCO_STOP_2_LIGHT CCIOB0 // green

//define other outputs
#define MOTOR ConnectorIO1 // contactor to turn on motor
#define CLUTCH ConnectorIO2 // engage clutch
#define COUNTER CCIOB6 // incremement counter

//Selector Switches
//INCH_MODE is default center position. No wires connected 
#define SS_MODE ConnectorA11 //left
#define CONT_MODE ConnectorA10 //right

// Gemco States (extender board A) (inputs)
#define GemCo1 CCIOA1
#define GemCo2 CCIOA2
#define GemCo3 CCIOA3
#define GemCo4 CCIOA4

// Define baud rate
#define baudRate 9600

/******************************INITIALIZE BOOLS + SOME FUNCTION PROTOTYPES*********************************/
// Initialize button state and press time - volatile must be used for Arduino Interrupts
volatile bool button1Pressed = false;
volatile bool button2Pressed = false;
volatile uint32_t button1PressTime = 0;
volatile uint32_t button2PressTime = 0;

// GemCo renaming
bool TDC = false;
bool downStroke = false;
bool TDC_Stop1 = false;
bool TDC_Stop2 = false;

// Other bools for Single Stroke
bool ssStartedTDC = false;
bool motorOn = false; // bool for turning on motor
bool continuousModeArmed = false;  // Tracks whether continuous mode is armed
bool TopStopButtonPressed = false; // Tracks if the top stop button is pressed

// Initialize Interrupt Service Routines
void button1ISR();
void button2ISR();
void StopISR();

volatile long lastButton1PressTime = 0; // For manual debounce
volatile long lastButton2PressTime = 0; // For manual debounce

unsigned long debounceDelay = 50;  // 50ms debounce period


void setup() {

  Serial.begin(baudRate);
  
  /**************************DEFINE INPUTS AND OUTPUTS******************************/
  /*-----------------------------main board----------------------------*/
  //inputs
  PALM_BUTTON_1.Mode(Connector::INPUT_DIGITAL);
  PALM_BUTTON_2.Mode(Connector::INPUT_DIGITAL);
  MOTOR_ON_BUTTON.Mode(Connector::INPUT_DIGITAL);
  MOTOR_OFF_BUTTON.Mode(Connector::INPUT_DIGITAL);
  MOTOR_OFF_BUTTON_2.Mode(Connector::INPUT_DIGITAL);
  SS_MODE.Mode(Connector::INPUT_DIGITAL);
  CONT_MODE.Mode(Connector::INPUT_DIGITAL);
  AIR_1.Mode(Connector::INPUT_DIGITAL);
  AIR_2.Mode(Connector::INPUT_DIGITAL);

  //outputs
  CLUTCH.Mode(Connector::OUTPUT_DIGITAL);
  MOTOR.Mode(Connector::OUTPUT_DIGITAL);
  MOTOR_ON_LIGHT.Mode(Connector::OUTPUT_DIGITAL);
  MOTOR_OFF_LIGHT.Mode(Connector::OUTPUT_DIGITAL);

  /*--------------------------- extender boards A, B --------------------------*/
  // Set all pins on CCIOA (Extender Board 1) to be inputs
  for (int pin = CLEARCORE_PIN_CCIOA0; pin <= CLEARCORE_PIN_CCIOA7; pin++) {
      pinMode(pin, INPUT);
  }

  // Set all pins on CCIOB (Extender Board 2) to be Outputs
  for (int pin = CLEARCORE_PIN_CCIOB0; pin <= CLEARCORE_PIN_CCIOB7; pin++) {
      pinMode(pin, OUTPUT);
  }

  // if you need more extender boards, simply copy the code above and change it to C or D, etc

  CcioPort.Mode(Connector::CCIO); //For CCIIO-8 (extender boards)
  CcioPort.PortOpen();            //For CCIIO-8 (extender boards)
  
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
  */ 
  attachInterrupt(digitalPinToInterrupt(8), button1ISR, RISING); // PALM_BUTTON_1 1 interrupt
  attachInterrupt(digitalPinToInterrupt(9), button2ISR, RISING);  // PALM_BUTTON_2 interrupt
  attachInterrupt(digitalPinToInterrupt(7), StopISR, RISING); //MOTOR_OFF_BUTTON interrupt
  attachInterrupt(digitalPinToInterrupt(12), StopISR, FALLING); // MOTOR_OFF_BUTTON_2 interrupt

  /***********************************initialize stuff**********************************/
  // Initializes all lights to start off 
  MOTOR_ON_LIGHT.State(false);
  MOTOR_OFF_LIGHT.State(false); 

  //initialize motor and clutch to off/disengaged
  MOTOR.State(false);
  CLUTCH.State(false); 

  // Initialize local mode flags to be false
  TurnOffSS();
  TurnOffCont();
}

