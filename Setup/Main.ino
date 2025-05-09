/*Updated on 05/08/2025
Author: Sarah Hunter, Heidi Hunter and Steele Mason
Purpose:
*/

void loop() {
  // Serial.println("main loop");

  UpdateIndexer(); //update indexer position 

  /******************* UPDATE BUTTON VALUES, all are read-only variables ********************/

  //PALM BUTTONS - this is what sets them back to false eventually
  button1Pressed = PALM_BUTTON_1.State();
  button2Pressed = PALM_BUTTON_2.State();
  button3Pressed = PALM_BUTTON_3.State();
  button4Pressed = PALM_BUTTON_4.State();

  /************************************ UPDATE LIGHTS & COUNTER ********************************************/
  digitalWrite(TDC_LIGHT, TDC);                //update light based on GemCo1 (TDC) state
  digitalWrite(DOWNSTROKE_LIGHT, DOWNSTROKE); //update light based on GemCo2 (downstroke) state
  digitalWrite(TDC_STOP_LIGHT, TDC_STOP); //update light based on GemCo3 (TDC stop) state
  digitalWrite(CLEAR_PATH_LIGHT, CLEAR_PATH); // GemCo for indexer being able to rotate safely
  digitalWrite(LIGHT_CURTAIN_ENABLED_LIGHT, LIGHT_CURTAIN_ENABLE);
  UpdateCounter();

  /************************************** UPDATE MOTOR STATE *************************************/
  // If MOTOR_ON_BUTTON button is pressed, and both air valves are on: turn on motorOn flag (which later turns on motor)
  if (MOTOR_ON_BUTTON.State() && CheckAir()) {
    motorOn = true;
  }
  
  if (digitalRead(MOTOR_REV)) {
    //let the flywheel slow down, user will have to visually see when flywheel stops and can hit green button again.
    motorOn = false;
  }

  //if at ANY POINT the air goes off, shut off motor
  if (!CheckAir()) {
    motorOn = false;
  }

  // if MOTOR_ON_BUTTON was previously pressed, then turn on the motor and associated lights
  if (motorOn) {
    digitalWrite(MOTOR_ON_LIGHT, true); //Contactor is Engaged (big green light on)
    digitalWrite(MOTOR_OFF_LIGHT, false);  // big red light is off
    // if (digitalRead(MOTOR_FW)) {
      // MOTOR_REV_CONTACTOR.State(false); // turn OFF motor in reverse
      MOTOR_FW_CONTACTOR.State(true); // turn on motor in forward
      // Serial.println("Motor FW");
      
    // }
    // else {
      // MOTOR_FW_CONTACTOR.State(false); // turn off fw motor!!!
      // MOTOR_REV_CONTACTOR.State(true); // turn on motor in reverse
      // Serial.println("Motor REV");
    // }
    
  }
  else {
    digitalWrite(MOTOR_ON_LIGHT, false); //Contactor is disengaged (big green light off)
    digitalWrite(MOTOR_OFF_LIGHT, true); //big red light on
    MOTOR_FW_CONTACTOR.State(false); // turn off motor
    // MOTOR_REV_CONTACTOR.State(false); // turn OFF motor in reverse
    TurnOffSS(); //reset flags for other modes
    TurnOffCont(); //reset flags for other modes 
    // Serial.println("Motor Off");
  }

  /**************************************** MAIN STATE MACHINE ******************************************/
  if (motorOn) {    // if stop button has NOT been pressed and the air is on, then enter into the modes
    //Check which mode is selected
    if (digitalRead(SS_MODE)) {
      TurnOffCont(); //turn off other mode's flags/lights
      Perform_SINGLE_STROKE ();
    }
    else if (digitalRead(CONT_MODE)) {
      TurnOffSS(); //turn off other mode's flags/lights
      Perform_CONTINUOUS ();
    }
    else {
      TurnOffSS(); //turn off other mode's flags/lights
      TurnOffCont(); //turn off other mode's flags/lights
      Perform_INCH();
    }
  }
  else {
    //motor not on!
    CLUTCH.State(false);   // clutch disengaged for sure 
  }
}

