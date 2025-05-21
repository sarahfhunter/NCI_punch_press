/*Updated on 05/09/2025
Author: Sarah Hunter, Heidi Hunter and Steele Mason
Purpose: INTERRUPT SERVICE ROUTINES. These are entered instantaneously if the digital interrupt pin they are attached to is triggered. 
         See setup() for which pins are established as interrupts.
*/

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
void StopISR() { 
  // if any of the motor off buttons are pressed, we enter this function and turn off motorOn flag
  motorOn = false; //this flag then updates the motor state in loop() 
}

void LightCurtainRoutine() {
  //if light curtain was triggered, we enter this function
  
  CLUTCH.State(false); //disengage the clutch
  
  //reset all flags after light curtain is done being triggered, so we have to restart SS or Continuous mode if they were interrupted
  TurnOffCont();
  TurnOffSS();
}