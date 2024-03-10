void showDisplay(byte newState, bool refresh) {       // employs servo 'easing' to slow down the movement from the old value to the new value
  const unsigned long METER_SPEED = 1;                // in millis.  Higher is slower.
  debugStep("Entering showDisplay");

  if (refresh) {
    for (byte i = 0; i < MAX_METERS; i++) meterBuffer[i][0] = meterBuffer[i][1];
    convertMeterData(newState, 1);                    //store the new values
  }

  //debug("Start update...");
  if ( ( (meterBuffer[0][0] != meterBuffer[1][0]) || 
        (meterBuffer[0][1] != meterBuffer[1][1]) || 
        (meterBuffer[0][2] != meterBuffer[1][2]) || 
        (meterBuffer[0][3] != meterBuffer[1][3]) || 
        (meterBuffer[0][4] != meterBuffer[1][4]) || 
        (meterBuffer[0][5] != meterBuffer[1][5]) || 
        (meterBuffer[0][6] != meterBuffer[1][6]) ) ) 
    {

    for (byte i = 0; i < MAX_METERS; i++) {     
      if (meterBuffer[0][i] < meterBuffer[1][i]) {    // increment or decrement toward new value
        meterBuffer[0][i]++;
      } else if (meterBuffer[0][i] > meterBuffer[1][i]) {
        meterBuffer[0][i]--;
      }
      analogWrite(meters[i].pin, meterBuffer[0][i]);  // output
    }
    delay(METER_SPEED);
  }
}

bool alertStatus(int wxid) {
  switch (wxid) {                                     //these were arbitrarily chosen by me
    case 780:
    case 622:
    case 602:
    case 600:
    case 601:
    case 212:
      return true;
    default:
      return false;
  }
}

void netStatus(byte state) {
#if (DISPLAYENABLE > 0)
  switch (state) {
    case 0:
      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(RED_PIN, LOW);
      break;
    case 1:
      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(RED_PIN, HIGH);
      break;
    case 2:
      digitalWrite(GREEN_PIN, HIGH);
      digitalWrite(RED_PIN, LOW);
      break;
  }
#endif
}

void sweepDisplay() {
  delay(1);
  debugln("Begin sweep");
#if (DISPLAYENABLE > 1)
  debugStep("Turning on Green LED");
  netStatus(GREEN);
  int x = 1;
  for (int i = 0; i > -1; i = i + x) {
    for (byte a = 0; a < MAX_METERS; a++) analogWrite(meters[a].pin, i); //ignores individual lower and upper meter trim settings
    if (i == 255) {
      x = -1;  // switch direction at peak
    }
    delay(5);
  }
  debugStep("Turning on Red LED");
  netStatus(RED);
  delay(500);
  debugStep("Turning off LED");
  netStatus(OFF);
#endif
}

int convertCond(int wxid) {  //use the weather ID number determine which icon to point to (ordered bad to good going clockwise)

  if (wxid == 800) {
    return 100;  //clear and sunny
  }
  if (wxid >= 801 && wxid <= 804) {
    return 84;  //partly cloudy
  }
  if (wxid >= 300 && wxid <= 321) {
    return 72;  //drizzle
  }
  if (wxid >= 500 && wxid <= 501) {
    return 60;  //light rain
  }
  if (wxid >= 502 && wxid <= 531) {
    return 48;  //heavy rain
  }
  if ((wxid >= 200 && wxid <= 232) && (wxid != 212)) {
    return 36;  //thunderstorm
  }
  if (wxid == 212) {
    return 36;  //thunderstorm
  }
  if (wxid >= 611 && wxid <= 622) {
    return 24;  //snow+rain
  }
  if (wxid >= 600 && wxid <= 601) {
    return 12;  //snow
  }
  if (wxid == 602) {
    return 12;  //
  }
  if (wxid == 622) {
    return 12;  //
  }
  if (wxid >= 701 && wxid <= 780) {
    return 0;  //fog, smoke, etc..
  }

  return 100;  //if nothing matches, it's sunny and clear
}

void convertMeterData(byte state, byte index) {

  meterBuffer[index][0] = mymap((state), meters[0].lscale, meters[0].uscale, meters[0].ltrim, meters[0].utrim);
  meterBuffer[index][1] = mymap(convertCond(wxData[state].conditionID), meters[1].lscale, meters[1].uscale, meters[1].ltrim, meters[1].utrim);
  meterBuffer[index][2] = mymap(wxData[state].temperature, meters[2].lscale, meters[2].uscale, meters[2].ltrim, meters[2].utrim);
  meterBuffer[index][3] = mymap(wxData[state].windSpeed, meters[3].lscale, meters[3].uscale, meters[3].ltrim, meters[3].utrim);
  meterBuffer[index][4] = mymap(wxData[state].windDirection, meters[4].lscale, meters[4].uscale, meters[4].ltrim, meters[4].utrim);
  meterBuffer[index][5] = mymap(wxData[state].pressure, meters[5].lscale, meters[5].uscale, meters[5].ltrim, meters[5].utrim);
  meterBuffer[index][6] = mymap(wxData[state].humidity, meters[6].lscale, meters[6].uscale, meters[6].ltrim, meters[6].utrim);
      debug("lscale: ");
      debugln(meters[0].lscale);
      debug("uscale: ");
      debugln(meters[0].uscale);
      debug("ltrim: ");
      debugln(meters[0].ltrim);
      debug("utrim: ");
      debugln(meters[0].utrim);

debug("Index: ");
      debugln(state);
      debugln(mymap((state), meters[0].lscale, meters[0].uscale, meters[0].ltrim, meters[0].utrim));
debug("Condition: ");
  debugln(wxData[state].conditionID);
  debugln(mymap(convertCond(wxData[state].conditionID), meters[1].lscale, meters[1].uscale, meters[1].ltrim, meters[1].utrim));
debug("Temp: ");
  debugln(wxData[state].temperature);
debug("Speed: ");
  debugln(wxData[state].windSpeed);
debug("Direction: ");
  debugln(wxData[state].windDirection);
debug("Pressure: ");
  debugln(wxData[state].pressure);
debug("Humidity: ");
  debugln(wxData[state].humidity);

  for (byte i = 0; i < WX_BUFFER; i++) meterBuffer[index][i] = constrain(meterBuffer[index][i], meters[i].lscale, meters[i].uscale);
      debug("Time index: ");
      debugln(state);
      debug("Time meter buffer is currently: ");
      debugln(meterBuffer[index][0]);
      debug("Temp meter buffer is currently: ");
      debugln(meterBuffer[index][2]);
      debug("Condition meter buffer is currently: ");
      debugln(meterBuffer[index][1]);
      debug("Wind Speed meter buffer is currently: ");
      debugln(meterBuffer[index][3]);
      debug("Wind Direction meter buffer is currently: ");
      debugln(meterBuffer[index][4]);
      debug("Pressure meter buffer is currently: ");
      debugln(meterBuffer[index][5]);
}