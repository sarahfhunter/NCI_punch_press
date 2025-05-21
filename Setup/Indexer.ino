/*Updated on 05/09/2025
Author: Sarah Hunter, Heidi Hunter and Steele Mason
Purpose: All code related to controlling the INDEXER is here.
*/

bool newCycle = true;
bool cycleEnabled = true;

  /*  Update the position of the indexer based on:
  *     - the position of the press (CLEAR_PATH)
  *     - the indexer jog FW/REV toggle
  *     - the cycle indexer button
  *
  *     If CLEAR_PATH is HI, move one stop (based on number of positions selected on servo)   
  *     If jog FW/ jog REV is selected, move a fraction of a stop
  *     If cycle indexer button is pressed, do a full rotation (move all of the stops)
  */
void UpdateIndexer() {

  if (digitalRead(INDEXER_MODE_ENABLE)) {
    //ERROR FIXED!!!! When we move the indexer based on CLEAR_PATH, it prevents the TDC_STOP light from being detected in the continuous mode function??
      //this was caused by blocking code (i don't totally know why). Jog and cycle are still blocking code, but moving it based on gemco is now non-blocking.
      
    if (CLEAR_PATH && newCycle) { // If the gemco corresponding to indexer being clear to move is triggered, then start moving!
        MoveDistance(1, false); //move one stop, non-blocking
        numStrokes++; //increment how many stops the indexer has moved
        newCycle = false; // stop indexer from calling MoveDistance() until it has finished moving
    }
    else if (!CLEAR_PATH && !newCycle){ //once press has moved and indexer has moved, reset
      newCycle = true; //reset for next cycle
      Serial.println("Resetting newCycle from CLEAR_PATH");
    }
    
    if (digitalRead(INDEXER_FW)) { //jog forward
      MoveDistance(GetTotalStops()/SERVO_PPR, true); //jog forward 60 pulses, blocking
    }
    else if (digitalRead(INDEXER_REV)) { //jog backward
      MoveDistance(-GetTotalStops()/SERVO_PPR, true); //jog backward 60 pulses, blocking
    }
    else if (digitalRead(CYCLE_INDEXER) && cycleEnabled) { //if the cycle indexer button is pressed, it should move one stop
      MoveDistance(1, true); // move forward one stop, blocking
    }
    else {
      //if neither FW or REV, or cycling, don't move
    }
  }
  else {
    // Indexer mode disabled
  }
}

/*------------------------------------------------------------------------------
 * MoveDistance
 *
 *    Command "numStops" number of stops away from the current position
 *
 * Parameters:
 *    int numStops - the number of stops in 360 degrees it should take
 *    bool block - if true, waits for servo to complete it's command. Change this bool input with caution!!!!
 *
 * Returns: True/False depending on whether the move was successfully triggered.
 */
bool MoveDistance(float numStops, bool block) {
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

    if (block) {
      // Add a short delay to allow HLFB to update
      delay(2);

      // Waits for HLFB to assert (signaling the move has successfully completed)
      Serial.println("Moving.. Waiting for HLFB");
      while (!SERVO.StepsComplete() || SERVO.HlfbState() != MotorDriver::HLFB_ASSERTED) {
          continue;
      }

      Serial.println("Move Done");
    }
    
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


