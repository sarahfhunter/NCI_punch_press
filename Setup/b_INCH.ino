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