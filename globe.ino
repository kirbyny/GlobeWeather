

void globe() {
  if (millis() - mouseTimer > MOUSE_INTERVAL) { //refresh every MOUSE_INTERVAL ms
    mouseTimer = millis();
#if (GLOBEENABLE) 
      mouse.get_data();
      mousePoint = {(double)mouse.x_movement(), (double)(mouse.y_movement() * -1)}; //invert y axis
      currentLatLong = calculateDestination(currentLatLong.x, currentLatLong.y, getVector(ORIGIN, mousePoint).x, getVector(ORIGIN, mousePoint).y); //compute our new lat long
      dtostrf(currentLatLong.x, 6, 6, owLat);
      dtostrf(currentLatLong.y, 6, 6, owLong);
#else
    dtostrf(currentLatLong.x, 6, 6, owLat);
    dtostrf(currentLatLong.y, 6, 6, owLong);
#endif
  }
}

Point getVector(Point point1, Point point2) {
  return {toDegrees(calculateBearing(point1, point2)), calculateDistance(point1, point2)};
}

// Calculate destination point given a starting point, bearing, and distance
Point calculateDestination(double startLat, double startLon, double bearing2, double distance2) {
    double delta = distance2 / GLOBE_SCALE; // Angular distance in radians using scaling factor
    double lat1 = toRadians(startLat);
    double lon1 = toRadians(startLon);
    double brng = toRadians(bearing2);
    double lat2 = asin(sin(lat1) * cos(delta) + cos(lat1) * sin(delta) * cos(brng));
    double lon2 = lon1 + atan2(sin(brng) * sin(delta) * cos(lat1), cos(delta) - sin(lat1) * sin(lat2));

    // Normalize longitude to be between -180 and 180 degrees
    lon2 = fmod(lon2 + 3 * PI, 2 * PI) - PI;
    return {toDegrees(lat2), toDegrees(lon2)};
}

// Convert degrees to radians
double toRadians(double degrees) {
    return degrees * PI / 180.0;
}

// Convert radians to degrees
double toDegrees(double radians) {
    return radians * 180.0 / PI;
}
  
// Calculate bearing between two points
double calculateBearing(Point oldP, Point newP) {
    double dx = newP.x - oldP.x;
    double dy = newP.y - oldP.y;
    double angle = atan2(dy, dx);
    return fmod(toRadians(90.0) - angle, 2 * PI); // Convert to standard bearing
}

// Calculate distance between two points
double calculateDistance(Point oldP, Point newP) {
    double dx = newP.x - oldP.x;
    double dy = newP.y - oldP.y;
    return sqrt(dx*dx + dy*dy);
}