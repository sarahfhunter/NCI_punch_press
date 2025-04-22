/*Created on 8/17/2024
Author: Heidi Hunter and Steele Mason
Purpose:
*/
// Function prototype declarations so it compiles
void Perform_INCH();
void Perform_SINGLE_STROKE();
void Perform_CONTINUOUS();

void loop() {

  /**************************************UPDATE GEMCO AND BUTTON VALUES, all are read-only variables********************************************/
  //GEMCO POSITIONS 
  TDC = digitalRead(GemCo1);
  downStroke = digitalRead(GemCo2);
  TDC_Stop1 = digitalRead(GemCo3);
  TDC_Stop2 = digitalRead(GemCo4); 

  //PALM BUTTONS
  button1Pressed = PALM_BUTTON_1.State();
  button2Pressed = PALM_BUTTON_2.State();

  /****************************************UPDATE LIGHTS*********************************************************************/
  UpdateGemCoLight(TDC_LIGHT, TDC);                //update TDC light based on GemCo1 state
  UpdateGemCoLight(DOWNSTROKE_LIGHT, downStroke); //update downstroke light based on GemCo2 state
  UpdateGemCoLight(GEMCO_STOP_1_LIGHT, TDC_Stop1); //update TDC_Stop1 light based on GemCo3 state
  UpdateGemCoLight(GEMCO_STOP_2_LIGHT, TDC_Stop2); //update TDC_Stop2 light based on GemCo4 state


  /**************************************UPDATE MOTOR STATE*************************************/
  // If MOTOR_ON_BUTTON button is pressed and both air valves are on, turn on motorOn flag (which later turns on motor)
  if (MOTOR_ON_BUTTON.State() && AIR_1.State() && AIR_2.State()) {
    motorOn = true;
  }

  //if at ANY POINT the air goes off, shut off motor
  if (!AIR_1.State() || !AIR_2.State()) {
    motorOn = false;
  }

  // if MOTOR_ON_BUTTON was previously pressed, then turn on the motor and associated lights
  if (motorOn) {
    MOTOR_ON_LIGHT.State(true); //Contactor is Engaged (big green light on)
    MOTOR_OFF_LIGHT.State(false);  // big red light is off
    MOTOR.State(true); // turn on motor
  }
  else {
    MOTOR_ON_LIGHT.State(false); //Contactor is disengaged (big green light off)
    MOTOR_OFF_LIGHT.State(true); //big red light on
    MOTOR.State(false); // turn off motor
    TurnOffSS(); //reset flags for other modes
    TurnOffCont(); //reset flags for other modes 
    //Serial.println("motor ON flag turn off");
  }

  /*************************************************Main State Machine*******************************************************/
  if (motorOn) {    // if stop button has NOT been pressed and the air is on, then enter into the modes
    //Check which mode is selected
    if (SS_MODE.State()) {
      TurnOffCont(); //turn off other mode's flags/lights
      Perform_SINGLE_STROKE ();
    }
    else if (CONT_MODE.State()) {
      TurnOffSS(); //turn off other mode's flags/lights
      Perform_CONTINUOUS ();
    }
    else {
      TurnOffSS(); //turn off other mode's flags/lights
      TurnOffCont(); //turn off other mode's flags/lights
      Perform_INCH();
    }
  }
  else {
    //motor not on!
    CLUTCH.State(false);   // clutch disengaged for sure 
  }
}

/*********************************** INTERRUPT SERVICE ROUTINES ************************************/
void button1ISR() { //PALM_BUTTON_1 ISR
        long currentTime = Milliseconds();  // Get the current time

    // If enough time has passed since the last press, it's a valid press (debouncing)
    if (currentTime - lastButton1PressTime > debounceDelay) {
        button1Pressed = true;
        button1PressTime = currentTime;  // Record time of valid button press
    }

    // Update last button press time
    lastButton1PressTime = currentTime;
}

void button2ISR() { //PALM_BUTTON_2 ISR
      long currentTime = Milliseconds();  // Get the current time

    // If enough time has passed since the last press, it's a valid press (debouncing)
    if (currentTime - lastButton2PressTime > debounceDelay) {
        button2Pressed = true;
        button2PressTime = currentTime;  // Record time of valid button press
    }

    // Update last button press time
    lastButton2PressTime = currentTime;
}

// Interrupt Service Rountine for both stops
void StopISR() { //MOTOR_OFF_BUTTON and MOTOR_OFF_BUTTON_2 ISR
  motorOn = false; //this flag then updates the motor state in loop() 
  Serial.println("stop was pressed");
}

/***************************************** EXTRA FUNCTIONS **********************************************************/

int CheckButtonPress() { // FUNCTION to check if Palm Buttons have been pressed within X amount of time of each other
  long timeDif = button1PressTime - button2PressTime;

  if (button1Pressed && button2Pressed && abs(timeDif) < 250) {
      return 1;  // If buttons were pressed within X time then return a 1
    }   
    else {
      return 0;   // If1 buttons were pressed within X time then return a 0
    }
}

void TurnOffSS() { //turns off all SS related flags/lights
  ssStartedTDC = false;
  digitalWrite(SS_LIGHT, false);
}

void TurnOffCont() { //turns off all Continuous related flags/lights
  digitalWrite(ARM_CONTINUOUS_LIGHT, false); // turn the arm continous light OFF 
  continuousModeArmed = false;      // disarm the continuous mode
  TopStopButtonPressed = false;   // reset for next cycle
}

void UpdateGemCoLight(int light, bool state) {
  if (state) {
    digitalWrite(light, true);
    if (state == TDC) {digitalWrite(COUNTER, HIGH);}
  }
  else {
    digitalWrite(light, false);
    if (state == TDC) {digitalWrite(COUNTER, LOW);}
  }
}

void DispenseOil() {
  /*GOAL: 
    we want to dispense oil/lubricate at a given frequency
    anything else?
  */

  //need to have a (global?) timer variable that is started and stopped. when? whenever green button is hit.
  //we want to lubricate every x minutes.
  // if that timer is mod x == 0, then lubricate 

  //choosing the timer method will be the most advanced
    //either can use millis() - start_time % milli_freq == 0?
    // OR there may be clearcore specific timer functions we can use.
  
}


