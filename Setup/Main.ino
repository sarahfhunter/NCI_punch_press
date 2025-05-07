/*Created on 8/17/2024
Author: Heidi Hunter and Steele Mason
Purpose:
*/
// Function prototype declarations so it compiles
// void Perform_INCH();
// void Perform_SINGLE_STROKE();
// void Perform_CONTINUOUS();

void loop() {
  // Serial.println("main loop");
  // digitalWrite(MOTOR_ON_LIGHT, true);

  UpdateIndexer(); //update indexer position

  if (digitalRead(CYCLE_INDEXER)) { //if the cycle indexer button is pressed, it should move all the way around
    MoveDistance(GetTotalStops());
  }

  /******************* UPDATE GEMCO AND BUTTON VALUES, all are read-only variables ********************/
  //GEMCO POSITIONS 
  // TDC = digitalRead(TDC);
  // downStroke = digitalRead(DOWNSTROKE);
  // TDC_STOP = digitalRead(GemCo3);
  // TDC_Stop2 = digitalRead(GemCo4); 

  //PALM BUTTONS - TODO test with this commented out
  // button1Pressed = PALM_BUTTON_1.State();
  // button2Pressed = PALM_BUTTON_2.State();

  /************************************ UPDATE LIGHTS & COUNTER ********************************************/
  UpdateLight(TDC_LIGHT, TDC);                //update light based on GemCo1 (TDC) state
  UpdateLight(DOWNSTROKE_LIGHT, DOWNSTROKE); //update light based on GemCo2 (downstroke) state
  UpdateLight(TDC_STOP_LIGHT, TDC_STOP); //update light based on GemCo3 (TDC stop) state
  // UpdateGemCoLight(CLEAR, ); // GemCo for indexer being able to rotate safely
  UpdateLight(LIGHT_CURTAIN_ENABLED_LIGHT, digitalRead(LIGHT_CURTAIN_ENABLE));
  UpdateCounter();

  /************************************** UPDATE MOTOR STATE *************************************/
  // If MOTOR_ON_BUTTON button is pressed, both air valves are on, and Forward is selected: turn on motorOn flag (which later turns on motor)
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
      // digitalWrite(MOTOR_REV_CONTACTOR, false); // turn OFF motor in reverse
      MOTOR_FW_CONTACTOR.State(true); // turn on motor in forward
      // Serial.println("Motor FW");
      
    // }
    // else {
      MOTOR_FW_CONTACTOR.State(false); // turn off fw motor!!!
      // digitalWrite(MOTOR_REV_CONTACTOR, true); // turn on motor in reverse
      // Serial.println("Motor REV");
    // }
    
  }
  else {
    digitalWrite(MOTOR_ON_LIGHT, false); //Contactor is disengaged (big green light off)
    digitalWrite(MOTOR_OFF_LIGHT, true); //big red light on
    MOTOR_FW_CONTACTOR.State(false); // turn off motor
    // digitalWrite(MOTOR_REV_CONTACTOR, false); // turn OFF motor in reverse
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
    // Serial.println("Clutch DISengaged");
  }
}

