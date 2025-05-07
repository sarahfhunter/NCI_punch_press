// CONTINUOUS CLUTCH CONTROL CYCLE

void Perform_CONTINUOUS() {
    if (digitalRead(MOTOR_FW)) {
        // If the ARM_CONTINUOUS_BUTTON is pressed, allow continuous mode and turn on indicator light
        if (digitalRead(ARM_CONTINUOUS_BUTTON)) {  // Check to see if Arm Continuous Button was pressed
            continuousModeArmed = true;             // Arm the continuous mode                                                   
            digitalWrite(ARM_CONTINUOUS_LIGHT, true); // Turn arm continuous light ON
        }

        // If the press is at TDC, the buttons are pushed, and continuous mode armed, RUN!
        if (TDC && continuousModeArmed && CheckButtonPress()) {  
            CLUTCH.State(true); // run
        }

        // Check for the Top Stop Button press
        if (digitalRead(TOP_STOP_BUTTON) == 0) { //top stop is normally closed
            TopStopButtonPressed = true;
        }
        
        // If the top stop button was pressed and we are at either the first or second gemco stop, disengage and reset flags
        if (TopStopButtonPressed && TDC_STOP) {
            CLUTCH.State(false); // Clutch Disengaged
            TurnOffCont(); // reset local flags
        } 
    }
}
