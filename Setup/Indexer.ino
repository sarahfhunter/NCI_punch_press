/********* INDEXER ***********/

void UpdateIndexer() {
  if (digitalRead(INDEXER_MODE_ENABLE)) {
    if (digitalRead(INDEXER_GEMCO)) { //cycle indexer according to press position
      //if we are past 190 degrees ish
      MoveDistance(1); //move one stop?
    }
    else if (digitalRead(INDEXER_FW)) { //jog forward
      MoveDistance(0.1); //move 0.1 stop
    }
    else if (digitalRead(INDEXER_REV)) { //jog backward
      //reverse
      MoveDistance(-0.1); //move 0.1 stop
    }
    //if neither FW or REV, don't move.
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
  
    //compute distance based on selector switch
    int distance = 6400 * GEAR_REDUCER * INDEXER_REDUCER * numStops / GetTotalStops();  
    // pulses = pulses per motor revolution (6400) * gear box ratio (5) * indexer ratio (12 or 6) number of stops / total stops 

    Serial.print("Commanding ");
    Serial.print(distance);
    Serial.println(" pulses");

    // Command the move of incremental distance
    SERVO.Move(distance);

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
    else { return 12; } //12 positions
}


