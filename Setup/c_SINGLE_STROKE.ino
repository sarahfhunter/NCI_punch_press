// SINGLE STROKE CLUTCH CONTROL CYCLE

void Perform_SINGLE_STROKE() {

  if (digitalRead(MOTOR_FW)) { // only allow SS mode in FW, not reverse
  
    digitalWrite(SS_LIGHT, true); //indicate that SS mode is selected

    // start SS by checking if at TDC, if palm buttons pressed, and if we haven't already started SS
    if (TDC && CheckButtonPress() && !ssStartedTDC) {
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
      }
      else {
        // if none of the gemco positions are being read, just run!
        CLUTCH.State(true);
      }
    }
    else {
      //do nothing. Wasn't started TDC
    }  

  }
}
