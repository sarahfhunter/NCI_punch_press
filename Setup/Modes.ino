/*Updated on 05/22/2025
Author: Sarah Hunter, Heidi Hunter and Steele Mason
Purpose: INCH, SINGLE STROKE, AND CONTINUOUS MODE
*/

// INCH/JOG CLUTCH CONTROL CYCLE
void Perform_INCH () {

  // If buttons were pressed within X amount of time of each other then engage Clutch
  if (CheckButtonPress() == 1) {
      CLUTCH.State(true);  //engage clutch
    }   
    else {
      CLUTCH.State(false);   //disengage clutch
    }

}

// SINGLE STROKE CLUTCH CONTROL CYCLE
void Perform_SINGLE_STROKE() {
  //TODO test this: don't allow the cycle to start again until the bumper stop switch is high

  if (digitalRead(MOTOR_FW)) { // only allow SS mode in FW, not reverse
  
    digitalWrite(SS_LIGHT, true); //indicate that SS mode is selected

    // if we are at TDC and the bumper stop is NOT high, then kick out of SS mode and restart
    if (TDC && !CheckBumperStop()) {
      CLUTCH.State(false);
      TurnOffSS(); //turn off SS mode
    }

    if (!enableSS && !CheckButtonPress() && CheckBumperStop()) { //reset enableSS if the palm buttons have been released, and checks for bumper stop if enabled
      enableSS = true;
    }

    // start SS by checking if at TDC, if palm buttons pressed,  if we haven't already started SS, and if ss is enabled
    if (TDC && CheckButtonPress() && !ssStartedTDC && enableSS) {
        ssStartedTDC = true;
    }

    // Perform SS
    if (ssStartedTDC) {   // if we started at top dead center, then allow SS
      if (TDC || DOWNSTROKE) {
        Perform_INCH();
      }
      else if (TDC_STOP) { 
        CLUTCH.State(false);  //disengage clutch
        ssStartedTDC = false;  // Reset for the next cycle
        enableSS = false; // Reset for next cycle.
      }
      else {
        // if not TDC, downstroke, or TDC_stop, just run!
        CLUTCH.State(true);
      }
    }
  }
}

// CONTINUOUS CLUTCH CONTROL CYCLE
void Perform_CONTINUOUS() {

    if (digitalRead(MOTOR_FW)) { // only allow Cont mode if motor is FW, not REV
        //enable continuous mode enable based on indexer mode
        if (digitalRead(INDEXER_MODE_ENABLE)) { //check to see if indexer mode is enabled
          // enable continuous mode because indexer mode is selected
          continuousModeArmed = true;
          digitalWrite(ARM_CONTINUOUS_LIGHT, true);
          enabledViaIndexer = true; // indicates that continuous mode was enabled because of the indexer mode
        }
        else if (!digitalRead(INDEXER_MODE_ENABLE) && enabledViaIndexer) {
          //disable continuous mode when indexer mode is exited and we had previously enabled it via indexer mode
          continuousModeArmed = false;
          digitalWrite(ARM_CONTINUOUS_LIGHT, false);
        }

        // enable continuous mode based on arm continuous button
        if (digitalRead(ARM_CONTINUOUS_BUTTON)) {  // Check to see if Arm Continuous Button was pressed
            continuousModeArmed = true;             // Arm the continuous mode                                                   
            digitalWrite(ARM_CONTINUOUS_LIGHT, true); // Turn arm continuous light ON
        }

        // If the press is at TDC, continuous mode armed, and the buttons are pushed, RUN!
        if (TDC && continuousModeArmed && CheckButtonPress()) {  
            CLUTCH.State(true); // run
        }

        // if the bumper stop is enabled, and if the press is at DOWNSTROKE and if bumper switch is NOT pressed, then disengage the clutch
        if (DOWNSTROKE && !CheckBumperStop()) { //TODO: not sure if DOWNSTROKE is the right check here, might be checking as early as +10 degrees
          CLUTCH.State(false);
          TurnOffCont(); // reset continuous flags 
        }

        // Check for the Top Stop Button press or maxing out cycles (if indexer mode enabled)
        if (digitalRead(TOP_STOP_BUTTON) || (numStrokes >= GetTotalStops())) {
            stopAtTop = true; //set this flag to true, will cause it to stop when it reaches TDC_Stop
        }
        
        // If we should stopAtTop and we are at TDC_STOp (GemCO), then disengage clutch and reset flags for next continuous mode instance
        if (stopAtTop && TDC_STOP) {
            CLUTCH.State(false); // Clutch Disengaged
            Serial.println("Clutch should disengage");
            TurnOffCont(); // reset local flags
        } 
    }
}
