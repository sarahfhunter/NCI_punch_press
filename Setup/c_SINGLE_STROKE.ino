// SINGLE STROKE CLUTCH CONTROL CYCLE

void Perform_SINGLE_STROKE() {
  digitalWrite(SS_LIGHT, true); //indicate that SS mode is selected

  // start SS by checking if at TDC, if palm buttons pressed, and if we haven't already started SS
  if (TDC && CheckButtonPress() && !ssStartedTDC) {
      ssStartedTDC = true;
  }

  if (ssStartedTDC) {   // if we started at top dead center, then allow SS functionality
    if (TDC) {
      Perform_INCH();
    }
    else if(downStroke) {
      Perform_INCH();
    }
    else if ((TDC_Stop1 || TDC_Stop2)) { 
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
