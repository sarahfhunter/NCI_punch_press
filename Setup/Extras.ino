/*Updated on 05/08/2025
Author: Sarah Hunter, Heidi Hunter and Steele Mason
Purpose: Extra functions
*/

int CheckButtonPress() { // FUNCTION to check if Palm Buttons have been pressed within X amount of time of each other
  long pressTimeDif = button2PressTime - button1PressTime;
  long panelTimeDif = button4PressTime - button3PressTime;
  //TODO: add check if (useButtonsOnPanel) 
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

void TurnOffSS() { //turns off all SS related flags/lights
  ssStartedTDC = false;
  digitalWrite(SS_LIGHT, false);
}

void TurnOffCont() { //turns off all Continuous related flags/lights
  digitalWrite(ARM_CONTINUOUS_LIGHT, false); // turn the arm continous light OFF 
  continuousModeArmed = false;      // disarm the continuous mode
  stopAtTop = false;   // reset for next cycle
}

void UpdateCounter() {
  if (TDC_STOP) {
    digitalWrite(COUNTER, HIGH);
  }
  else {
    digitalWrite(COUNTER, LOW);
  }
}

bool CheckAir() {
  //Check if both of the air valves are ON
  // if (digitalRead(AIR_1) && digitalRead(AIR_2)) {
  //   return true;
  // }
  // else {
  //   return false;
  // }
  return true;
}


// void UpdateLight(int light, bool state) { //TODO: get rid of this later lol
//   if (state) {
//     digitalWrite(light, true);
//     // if (state == TDC) {digitalWrite(COUNTER, HIGH);} //TODO move elsewhere i think
//   }
//   else {
//     digitalWrite(light, false);
//     // if (state == TDC) {digitalWrite(COUNTER, LOW);}
//   }
// }

// void DispenseOil() {
//   /*GOAL: 
//     we want to dispense oil/lubricate at a given frequency
//     anything else?
//   */

//   //need to have a (global?) timer variable that is started and stopped. when? whenever green button is hit.
//   //we want to lubricate every x minutes.
//   // if that timer is mod x == 0, then lubricate 

//   //choosing the timer method will be the most advanced
//     //either can use millis() - start_time % milli_freq == 0?
//     // OR there may be clearcore specific timer functions we can use.
  
// }

