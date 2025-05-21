/*Updated on 05/09/2025
Author: Sarah Hunter, Heidi Hunter and Steele Mason
Purpose: Extra functions
*/

int CheckButtonPress() { // FUNCTION to check if Palm Buttons have been pressed within X amount of time of each other
  long pressTimeDif = button2PressTime - button1PressTime; //on press
  long panelTimeDif = button4PressTime - button3PressTime; //on panel

  if (!PALM_BUTTONS_ON_PANEL && button1Pressed && button2Pressed && abs(pressTimeDif) < 250) {
    return 1;  // If buttons on press were pressed within X time then return a true
  }   
  else if (PALM_BUTTONS_ON_PANEL && button3Pressed && button4Pressed && abs(panelTimeDif) < 250) {
    return 1;   // If buttons on panel were pressed within X time then return true
  }
  else {
    return 0; //if neither set of palm buttons pressed in proper timing, return false
  }
}

void TurnOffSS() { //turns off all SS related flags/lights
  ssStartedTDC = false;
  digitalWrite(SS_LIGHT, false);
}

void TurnOffCont() { //turns off all Continuous related flags/lights
  digitalWrite(ARM_CONTINUOUS_LIGHT, false); // turn the arm continous light OFF 
  continuousModeArmed = false;      // disarm the continuous mode
  stopAtTop = false;   // reset for next cycle
  numStrokes = 0; // reset
  enabledViaIndexer = false; //reset 
}

void UpdateCounter() {
  //everytime the gemCo at TDC_STOP is high, increment the counter once
  if (TDC_STOP) {
    digitalWrite(COUNTER, HIGH);
  }
  else {
    digitalWrite(COUNTER, LOW);
  }
}

bool CheckAir() {
  //TODO: if the air valves are hooked up to clearcore inputs, uncomment this function! 
  //Right now, this check is hardwired into electrical circuit
  
  //Check if both of the air valves are ON
  // if (digitalRead(AIR_1) && digitalRead(AIR_2)) {
  //   return true;
  // }
  // else {
  //   return false;
  // }
  return true;
}

