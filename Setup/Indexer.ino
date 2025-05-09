/*Updated on 05/08/2025
Author: Sarah Hunter, Heidi Hunter and Steele Mason
Purpose: INDEXER
*/

void UpdateIndexer() {
  /*  Update the position of the indexer based on:
  *     - the position of the press (CLEAR_PATH)
  *     - the indexer jog FW/REV toggle
  *     - the cycle indexer button
  *
  *     If CLEAR_PATH is HI, move one stop (based on number of positions selected on servo)   
  *     If jog FW/ jog REV is selected, move a fraction of a stop
  *     If cycle indexer button is pressed, do a full rotation (move all of the stops)
  */
  if (digitalRead(INDEXER_MODE_ENABLE)) {
    //TODO: be aware of the stops it has been going one stop on start up if it was at TDC_STOP
    if (TDC_STOP && !cycleBegun) { //cycle indexer according to press position //TODO: update back to CLEAR_PATH
      //if we are past 190 degrees ish
      MoveDistance(1); //move one stop?
      cycleBegun = true;
    }
    else if (!TDC_STOP) {
      cycleBegun = false;
    }
    
    if (digitalRead(INDEXER_FW)) { //jog forward
      MoveDistance(GetTotalStops()/SERVO_PPR); //jog forward 60 pulses
    }
    else if (digitalRead(INDEXER_REV)) { //jog backward
      //reverse
      MoveDistance(-GetTotalStops()/SERVO_PPR); //jog backward 60 pulses
    }
    else if (digitalRead(CYCLE_INDEXER)) { //if the cycle indexer button is pressed, it should move one stop
      MoveDistance(1);
    }
    //if neither FW or REV, don't move
  }
  else {
    Serial.println("indexer mode disabled");
  }
}

/*------------------------------------------------------------------------------
 * MoveDistance
 *
 *    Command "distance" number of step pulses away from the current position
 *    Prints the move status to the USB serial port
 *    Returns when HLFB asserts (indicating the motor has reached the commanded
 *    position)
 *
 * Parameters:
 *    //int distance  - The distance, in step pulses, to move
 *    int numStops - the number of stops in 360 degrees it should take
 *
 * Returns: True/False depending on whether the move was successfully triggered.
 */
bool MoveDistance(float numStops) {
    // Check if an alert is currently preventing motion
    if (SERVO.StatusReg().bit.AlertsPresent) {
        Serial.println("Servo Motor status: 'In Alert'. Move Canceled.");
        return false;
    }
  
    //compute number of pulses needed based on selector switch
    int pulses = SERVO_PPR * GEAR_REDUCER * INDEXER_REDUCER * numStops / GetTotalStops();  
    // pulses = pulses per motor revolution (6400) * gear box ratio (5) * indexer ratio (12 or 6) * number of stops / total stops 

    Serial.print("Commanding ");
    Serial.print(pulses);
    Serial.println(" pulses");

    // Command the move of incremental distance
    SERVO.Move(pulses);

    // Add a short delay to allow HLFB to update
    delay(2);

    // Waits for HLFB to assert (signaling the move has successfully completed)
    Serial.println("Moving.. Waiting for HLFB");
    while (!SERVO.StepsComplete() || SERVO.HlfbState() != MotorDriver::HLFB_ASSERTED) {
        continue;
    }

    Serial.println("Move Done");
    return true;
}

int GetTotalStops() {
  //based on 5 selector switch, return number of stops
    if (INDEXER_2_POS) { return 2; }
    else if (INDEXER_5_POS) { return 5; }
    else if (INDEXER_8_POS) { return 8; }
    else if (INDEXER_10_POS) { return 10; }
    else if (INDEXER_12_POS) { return 12; }
    else { return 4; }
}


