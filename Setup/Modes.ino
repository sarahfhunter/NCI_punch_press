/*Updated on 05/09/2025
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

  if (digitalRead(MOTOR_FW)) { // only allow SS mode in FW, not reverse
  
    digitalWrite(SS_LIGHT, true); //indicate that SS mode is selected

    if (!enableSS && !CheckButtonPress()) {
      enableSS = true;
    }

    // start SS by checking if at TDC, if palm buttons pressed, and if we haven't already started SS
    if (TDC && CheckButtonPress() && !ssStartedTDC && enableSS) {
        ssStartedTDC = true;
    }

    if (ssStartedTDC) {   // if we started at top dead center, then allow SS functionality
      if (TDC) {
        Perform_INCH();
      }
      else if(DOWNSTROKE) {
        Perform_INCH();
      }
      else if (TDC_STOP) { 
        CLUTCH.State(false);  //disengage clutch
        ssStartedTDC = false;  // Reset for the next cycle
        enableSS = false;
      }
      else {
        // if after the downstroke and before the stop, just run!
        CLUTCH.State(true);
      }
    }
    else {
      //do nothing. Wasn't started TDC
    }  

  }
}

// CONTINUOUS CLUTCH CONTROL CYCLE

void Perform_CONTINUOUS() {

  //TODO: Don't require the arm continuous button?
    if (digitalRead(MOTOR_FW)) {

        //enable continuous mode enable based on indexer mode
        if (digitalRead(INDEXER_MODE_ENABLE)) {
          continuousModeArmed = true;
          digitalWrite(ARM_CONTINUOUS_LIGHT, true);
          enabledViaIndexer = true;
        }
        else if (!digitalRead(INDEXER_MODE_ENABLE) && enabledViaIndexer) {
          continuousModeArmed = false;
          digitalWrite(ARM_CONTINUOUS_LIGHT, false);
        }

        // If the ARM_CONTINUOUS_BUTTON is pressed, allow continuous mode and turn on indicator light
        if (digitalRead(ARM_CONTINUOUS_BUTTON)) { //|| digitalRead(INDEXER_MODE_ENABLE)) {  // Check to see if Arm Continuous Button was pressed
            continuousModeArmed = true;             // Arm the continuous mode                                                   
            digitalWrite(ARM_CONTINUOUS_LIGHT, true); // Turn arm continuous light ON
        }

        // If the press is at TDC, the buttons are pushed, and continuous mode armed, RUN!
        if (TDC && continuousModeArmed && CheckButtonPress()) {  
            CLUTCH.State(true); // run
        }
        
        // Check for the Top Stop Button press or maxing out cycles (if indexer mode enabled)
        if (digitalRead(TOP_STOP_BUTTON) || (numStrokes >= GetTotalStops())) {
            stopAtTop = true;
        }
        
        // If the top stop button was pressed and we are at either the first or second gemco stop, disengage and reset flags
        if (stopAtTop && TDC_STOP) {
            CLUTCH.State(false); // Clutch Disengaged
            Serial.println("Clutch should disengage");
            TurnOffCont(); // reset local flags
        } 
    }
}
