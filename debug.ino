// debugging related functions.  

void letsWait(char *message) { //pause and wait for serial input
  char t;
  debugln(message);

  while (!Serial.available()) {
    delay(1);
  }
  while (Serial.available()) {
    t = Serial.read();
  }
  debugln(" ->");
  delay(100);
}

void printValues(int v){ //send readable weather to serial monitor

  debugln();
  debug("Index            : ");
  debugln(v);
  debug("Timestamp        : ");
  debug(strTime(wxData[v].timeStamp));
  debug("City             : ");
  debugln(wxData[0].city);
  debug("Temp             : ");
  debugln(wxData[v].temperature);
  debug("Pressure         : ");
  debugln(wxData[v].pressure);
  debug("Humidity         : ");
  debugln(wxData[v].humidity);
  debug("Wind Speed       : ");
  debugln(wxData[v].windSpeed);
  debug("Wind Direction   : ");
  debugln(wxData[v].windDirection);
  debug("Forecast ID      : ");
  debugln(wxData[v].conditionID);
  debug("Description      : ");
  debugln(wxData[v].description);
  debug("Moonphase        : ");
  debugln(wxData[v].moonphase);
  debug("Meter value      : ");
  debugln(map((v), meters[0].lscale, meters[0].uscale, meters[0].ltrim, meters[0].utrim));
  debugln();
}


String strTime(time_t unixTime) {
  long TIME_OFFSET = -14400;
  unixTime += TIME_OFFSET;
  return ctime(&unixTime);
}

