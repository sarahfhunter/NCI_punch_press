/*Created on 8/17/2024
Author: Heidi Hunter and Steele Mason
Purpose:
*/
// Function prototype declarations so it compiles
void Perform_INCH();
void Perform_SINGLE_STROKE();
void Perform_CONTINUOUS();

void loop() {

  MoveIndexer(); //update indexer position

  /******************* UPDATE GEMCO AND BUTTON VALUES, all are read-only variables ********************/
  //GEMCO POSITIONS 
  // TDC = digitalRead(TDC);
  // downStroke = digitalRead(DOWNSTROKE);
  // TDC_STOP = digitalRead(GemCo3);
  // TDC_Stop2 = digitalRead(GemCo4); 

  //PALM BUTTONS - TODO test with this commented outs
  button1Pressed = PALM_BUTTON_1.State();
  button2Pressed = PALM_BUTTON_2.State();

  /************************************ UPDATE LIGHTS ********************************************/
  UpdateGemCoLight(TDC_LIGHT, TDC);                //update light based on GemCo1 (TDC) state
  UpdateGemCoLight(DOWNSTROKE_LIGHT, DOWNSTROKE); //update light based on GemCo2 (downstroke) state
  UpdateGemCoLight(TDC_STOP_LIGHT, TDC_STOP); //update light based on GemCo3 (TDC stop) state
  // UpdateGemCoLight(CLEAR, ); // GemCo for indexer being able to rotate safely

  /************************************** UPDATE MOTOR STATE *************************************/
  // If MOTOR_ON_BUTTON button is pressed and both air valves are on, turn on motorOn flag (which later turns on motor)
  if (MOTOR_ON_BUTTON.State() && CheckAir()) {
    motorOn = true;
  }

  //if at ANY POINT the air goes off, shut off motor
  if (!CheckAir()) {
    motorOn = false;
  }

  // if MOTOR_ON_BUTTON was previously pressed, then turn on the motor and associated lights
  if (motorOn) {
    digitalWrite(MOTOR_ON_LIGHT, true); //Contactor is Engaged (big green light on)
    digitalWrite(MOTOR_OFF_LIGHT, false);  // big red light is off
    MOTOR.State(true); // turn on motor
  }
  else {
    digitalWrite(MOTOR_ON_LIGHT, false); //Contactor is disengaged (big green light off)
    digitalWrite(MOTOR_OFF_LIGHT, true); //big red light on
    MOTOR.State(false); // turn off motor
    TurnOffSS(); //reset flags for other modes
    TurnOffCont(); //reset flags for other modes 
  }

  /**************************************** MAIN STATE MACHINE ******************************************/
  if (motorOn) {    // if stop button has NOT been pressed and the air is on, then enter into the modes
    //Check which mode is selected
    if (digitalRead(SS_MODE)) {
      TurnOffCont(); //turn off other mode's flags/lights
      Perform_SINGLE_STROKE ();
    }
    else if (digitalRead(CONT_MODE)) {
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

void button3ISR() { //PALM_BUTTON_2 ISR
      long currentTime = Milliseconds();  // Get the current time

    // If enough time has passed since the last press, it's a valid press (debouncing)
    if (currentTime - lastButton3PressTime > debounceDelay) {
        button3Pressed = true;
        button3PressTime = currentTime;  // Record time of valid button press
    }

    // Update last button press time
    lastButton3PressTime = currentTime;
}

void button4ISR() { //PALM_BUTTON_2 ISR
      long currentTime = Milliseconds();  // Get the current time

    // If enough time has passed since the last press, it's a valid press (debouncing)
    if (currentTime - lastButton4PressTime > debounceDelay) {
        button4Pressed = true;
        button4PressTime = currentTime;  // Record time of valid button press
    }

    // Update last button press time
    lastButton4PressTime = currentTime;
}

// Interrupt Service Rountine for both stops
void StopISR() { //MOTOR_OFF_BUTTON and MOTOR_OFF_BUTTON_2 ISR
  motorOn = false; //this flag then updates the motor state in loop() 
  // Serial.println("stop was pressed");
}

void LightCurtainRoutine() {
  //light curtain was triggered
  if (digitalRead(LIGHT_CURTAIN_ENABLE)) {
    //disengage the clutch
    CLUTCH.State(false); //TODO: how to make sure that it doesn't just get turned right back on? maybe set interrupt to HIGH not rising?
  }
  //else, do nothing
}

/*********************************** EXTRA FUNCTIONS ****************************************/
void MoveIndexer() {
  if (digitalRead(INDEXER_MODE_ENABLE)) {
    if (digitalRead(INDEXER_GEMCO)) { //cycle indexer according to press position
      //if we are past 190 degrees ish
      MoveDistance(1); //move one stop?
    }
    else if (digitalRead(INDEXER_FW)) { //jog forward
      MoveDistance(60); //TODO change to degrees
    }
    else if (digitalRead(INDEXER_REV)) { //jog backward
      //reverse
      MoveDistance(-60); //TODO change to degrees
    }
    //if neither FW or REV, don't move.
  }
}

int CheckButtonPress() { // FUNCTION to check if Palm Buttons have been pressed within X amount of time of each other
  long pressTimeDif = button2PressTime - button1PressTime;
  long panelTimeDif = button4PressTime - button3PressTime;

  if (button1Pressed && button2Pressed && abs(pressTimeDif) < 250) {
    return 1;  // If buttons were pressed within X time then return a true
  }   
  else if (button3Pressed && button4Pressed && abs(panelTimeDif) < 250) {
    return 1;   // If buttons were pressed within X time then return true
  }
  else {
    return 0; //if neither set of palm buttons pressed in proper timing, return false
  }
}

bool CheckAir() {
  //Check if both of the air valves are ON
  if (digitalRead(AIR_1) && digitalRead(AIR_2)) {
    return true;
  }
  else {
    return false;
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

/*------------------------------------------------------------------------------
 * MoveDistance
 *
 *    Command "distance" number of step pulses away from the current position
 *    Prints the move status to the USB serial port
 *    Returns when HLFB asserts (indicating the motor has reached the commanded
 *    position)
 *
 * Parameters:
 *    //int distance  - The distance, in step pulses, to move
 *    int numStops - the number of stops in 360 degrees it should take
 *
 * Returns: True/False depending on whether the move was successfully triggered.
 */
bool MoveDistance(int numStops) {
    // Check if an alert is currently preventing motion
    if (SERVO.StatusReg().bit.AlertsPresent) {
        Serial.println("Servo Motor status: 'In Alert'. Move Canceled.");
        return false;
    }
  
    //compute distance based on selector switch
    int totalStops = 0;
    if (INDEXER_2_POS) { totalStops = 2; }
    else if (INDEXER_5_POS) { totalStops = 5; }
    else if (INDEXER_8_POS) { totalStops = 8; }
    else if (INDEXER_10_POS) { totalStops = 10; }
    else { totalStops = 12; } //12 positions

    int distance = 6400 * GEAR_REDUCER * INDEXER_REDUCER * numStops / totalStops;  // pulses = pulses per motor revolution (6400) * gear box ratio (5) * indexer ratio (12 or 6) / # stops 

    Serial.print("Commanding ");
    Serial.print(distance);
    Serial.println(" pulses");

    // Command the move of incremental distance
    SERVO.Move(distance);

    // Add a short delay to allow HLFB to update
    delay(2);

    // Waits for HLFB to assert (signaling the move has successfully completed)
    Serial.println("Moving.. Waiting for HLFB");
    while (!SERVO.StepsComplete() || SERVO.HlfbState() != MotorDriver::HLFB_ASSERTED) {
        continue;
    }

    Serial.println("Move Done");
    return true;
}


