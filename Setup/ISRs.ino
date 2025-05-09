/*Updated on 05/08/2025
Author: Sarah Hunter, Heidi Hunter and Steele Mason
Purpose: INTERRUPT SERVICE ROUTINES
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
void StopISR() { //MOTOR_OFF_BUTTON and MOTOR_OFF_BUTTON_2 ISR
  motorOn = false; //this flag then updates the motor state in loop() 
  // Serial.println("stop was pressed");
}

void LightCurtainRoutine() {
  //light curtain was triggered
  if (digitalRead(LIGHT_CURTAIN_ENABLE)) {
    //disengage the clutch
    CLUTCH.State(false);
    FlashLightCurtainLight();
    // Serial.println("light curtain flagged, clutch disengaged");
  }
  //else, do nothing
}