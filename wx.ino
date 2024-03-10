bool getWx() {
  const char *API_KEY = "3f40ac2a10bdef039fa4bbbf2c8c5e64";
  const char *UNITS = "imperial";                     // "metric" or "imperial"
  const char *LANG = "en";                            // this doesn't affect anything
  const char *COUNTRY = "US";                         // United states model

  if ((WiFi.status() != WL_CONNECTED)) {
    debugln("I can't get the forecast because I don't have an internet connection.");
    netState = false;
    delay(3000);
    return false;
  }

  OW_forecast *forecast = new OW_forecast;            // Create the structures that hold the retrieved weather

  debug("Getting forecast for ");
  debug(owLat);
  debug(", ");
  debugln(owLong);

  if ( !(ow.getForecast(forecast, API_KEY, owLat, owLong, UNITS, LANG)) ) {
    debugln("I tried to get the weather, but it didn't work.");
    delete forecast;
    return false;
  }
  
  if (forecast) {
    debugStep("I got the 3-hourly forecast.  Send a command to continue.");
    netState = true;
    for (int i = 0; i < (17); i++) {                  //retrieve 48 hours of forecasts (48 / 3 hourly + 1)
      switch (i) {
        case 0:
          wxData[i].temperature = forecast->temp[i];
          wxData[i].conditionID = forecast->id[i];
          wxData[i].humidity = forecast->humidity[i];
          wxData[i].windSpeed = forecast->wind_speed[i];
          wxData[i].alert = alertStatus(forecast->id[i]);
          wxData[i].timeStamp = forecast->dt[i];
          wxData[i].description = forecast->description[i];
          wxData[i].pressure = forecast->pressure[i];
          wxData[i].city = forecast->city_name;
          wxData[i].windDirection = forecast->wind_deg[i];
          wxData[i].moonphase = forecast->moon_phase[0];
          printValues(i);
          debugStep("This is index 0 (now).  Send a command to continue.");
          break;
        case 1:  //+6 hours
        case 3:  //+12 hours
        case 5:  //+18 hours
        case 7:  //+24 hours
          wxData[(i + 1) / 2].temperature = forecast->temp[(i + 1)];
          wxData[(i + 1) / 2].conditionID = forecast->id[(i + 1)];
          wxData[(i + 1) / 2].humidity = forecast->humidity[(i + 1)];
          wxData[(i + 1) / 2].windSpeed = forecast->wind_speed[(i + 1)];
          wxData[(i + 1) / 2].alert = alertStatus(forecast->id[(i + 1)]);
          wxData[(i + 1) / 2].timeStamp = forecast->dt[(i + 1)];
          wxData[(i + 1) / 2].description = forecast->description[(i + 1)];
          wxData[(i + 1) / 2].pressure = forecast->pressure[(i + 1)];
          wxData[(i + 1) / 2].city = forecast->city_name;
          wxData[(i + 1) / 2].windDirection = forecast->wind_deg[(i + 1)];
          wxData[(i + 1) / 2].moonphase = forecast->moon_phase[0];
          printValues((i + 1) / 2);
          
          debugStep("This is index 1, 2, 3, or 4.  Send a command to continue.");
          break;
        case 15:  //+48 hours
          wxData[5].temperature = forecast->temp[15];
          wxData[5].conditionID = forecast->id[15];
          wxData[5].humidity = forecast->humidity[15];
          wxData[5].windSpeed = forecast->wind_speed[15];
          wxData[5].alert = alertStatus(forecast->id[15]);
          wxData[5].timeStamp = forecast->dt[15];
          wxData[5].description = forecast->description[15];
          wxData[5].pressure = forecast->pressure[15];
          wxData[5].city = forecast->city_name;
          wxData[5].windDirection = forecast->wind_deg[15];
          wxData[5].moonphase = forecast->moon_phase[15];
          printValues(5);
          debugStep("This is index 5 (+48 hours). Send a command to continue.");
          break;
      }
      debugStep("We've completed a loop.  Send a command to continue.");
    }
    delete forecast;

    debugStep("I'm done here.  Send a command to head back to the main loop.");
    getLocalWx();
    printValues(WX_BUFFER-1); //when BME280 is disabled, this shows the last forecast
    return true;
  } else {
    getLocalWx();
    printValues(6);
    netState = false;
    return false;
  }
}

void getLocalWx() {
  #if (BME280ENABLE)
    float temp(NAN), hum(NAN), pres(NAN);
    BME280::TempUnit tempUnit(BME280::TempUnit_Fahrenheit);
    BME280::PresUnit presUnit(BME280::PresUnit_hPa);
    bme.read(pres, temp, hum, tempUnit, presUnit);
    wxData[(WX_BUFFER-1)] = wxData[0];                //copy everything from the current internet wx if available
    wxData[(WX_BUFFER-1)].city = "Indoor";
    wxData[(WX_BUFFER-1)].temperature = temp;         // then add the BME280 sensor values on top
    wxData[(WX_BUFFER-1)].pressure = pres;
    wxData[(WX_BUFFER-1)].humidity = hum;
  #endif
}

bool getTestWx() {
  for (int i = 0; i < 6; i++){
    wxData[i].temperature = 72.0;
    wxData[i].conditionID = 800;
    wxData[i].humidity = 50;
    wxData[i].windSpeed = 5.0;
    wxData[i].alert = false;
    wxData[i].description = "Its Nice";
    wxData[i].pressure = 1013.0;
    wxData[i].city = "Home";
    wxData[i].windDirection = 90;
    wxData[i].moonphase = 0.0;
  }
  return true;
}
