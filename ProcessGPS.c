#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<time.h>
#include"Functions.h"
#include <assert.h>

char* getFileError(int fileCheck){
  char *standardMessage = "Please enter a new GPS file name.";
  static char errorMessage[MAX];

  switch (fileCheck) {
    case notNMEA:
      sprintf(errorMessage, "Not .nmea file type. %s", standardMessage);
      break;
    case cannotOpenFile:
      sprintf(errorMessage, "Can't open file. %s", standardMessage);
      break;
    case incorrectStrings:
      sprintf(errorMessage, "GPS strings in file are not valid. %s", standardMessage);
      break;
  }
  return errorMessage;
}

  int checkGPSfile (char *userInput){ 
  FILE *fp;
  int fileCheck;

  fp  = fopen(userInput, "r");

  /* MAKE CHECKS */
  if (isNMEA(userInput) == 0){ /* First check correct file type */
    fileCheck = notNMEA;
  }
  else if (fp == 0){  /* Check if file can be opened */
    fileCheck = cannotOpenFile;
  }
  else if (checkStrings(fp) == 0){ /* Check if correct string types and in order */
    fileCheck = incorrectStrings;
  }
  else 
    fileCheck = fileOK;

  fclose(fp);

  return fileCheck;
}

/* Check if NMEA file type */
 int isNMEA (char *temp) {
  char fileType[MAX];
  sscanf(temp, "%*[^.].%s", fileType);
  if(strcmp(fileType, "nmea") == 0) {
    return 1;
  } 
  else {
    return 0;
  }
}

/* Check if GPGGA, GPGSA and GPRMC strings is present and in correct order */
int checkStrings(FILE *fp){
  char temp[MAX];
  int isValid      = 1;
  int round        = 0;
  int gpggaCounter = 0, gpgsaCounter = 0, gprmcCounter = 0;

  while (fgets(temp, MAX, fp) && isValid) {
    switch (round) {
      case 0:
        isValid = isGPGGA(temp);
        ++gpggaCounter;
        break;
      case 1:
        isValid = isGPGSA(temp);
        ++gpgsaCounter;
        break;
      case 2:
        isValid = isGPRMC(temp);
        ++gprmcCounter;
        break;
    }
    /* Count up round number and reset if all string types have been checked */
    ++round;  
    round = round % NO_OF_STRING_TYPES; 
  }

  return isValid;
}

GPSdata *makeGPSdata(char *userInput, int *ptrDataSize){
  int dataSize = 0;
  GPSdata *gps;
  FILE *fp;

  fp  = fopen(userInput, "r");
  dataSize = countPointsInFile(fp);
  rewind(fp);
  gps = addGPSdata(fp, dataSize);
  
  *ptrDataSize = dataSize;

  fclose(fp);

  printf("TEST\n");

  return gps;
}

/* Count no. of GPS points in file. There is a point for every GPGGA string */
int countPointsInFile(FILE *fp){
	int maxPoints = 0;
  char string[MAX];
  while (fgets(string, MAX, fp)) {
    if (isGPGGA(string)) {
		  maxPoints++;
		}
	}
	return maxPoints;
}

/* Check if recieved string is of type GPGGA */
int isGPGGA(char *string){
  char temp[MAX];
  sscanf(string, "%[$GPA]", temp);
  if(strcmp(temp, "$GPGGA") == 0)
    return 1;
  else
    return 0;
}

int isGPGSA(char *string){
    char temp[MAX];
    sscanf(string, "%[$GPSA],", temp);
    if(strcmp(temp, "$GPGSA") == 0)
      return 1;
    else
      return 0;
}

int isGPRMC(char *string){
  char temp[MAX];
  sscanf(string, "%[$GPRMC],", temp);
  if(strcmp(temp, "$GPRMC") == 0)
    return 1;
  else
    return 0;
}

/*Print GPS points */
void printGPSdata(GPSdata *gps, int dataSize){
    int i = 0;

    printf("\n\nGPS DATA ORIGINAL");
    printf("\n------------------------\n");
    printf("\nData size: %d\n\n", dataSize);

    printf("[%5s]\t %-8s %-16s %-16s %-3s %-16s %-3s %-10s %7s\n\n","INDEX", "DATE", "TIME", "LATITUDE", "D" ,"LONGITUDE","D", "HDOP", "QUALITY");
    for(i = 0; i < dataSize; ++i){
    printf("[%5d]\t %-8d %-16lf %-16lf %-3c %-16lf %-3c %-10lf %7d\n",
            i,
            gps[i].dateUTC,
            gps[i].clockUTC,
            gps[i].latitudeDDM,
            gps[i].directionLat,
            gps[i].longitudeDDM,
            gps[i].directionLon,
            gps[i].hdop,
            gps[i].quality);
    }
}

GPSdata *addGPSdata(FILE *fp, int dataSize){
  char temp[MAX];
  int index           = 0;
  int scanresGPGGA    = 0;
  int scanresGPRMC    = 0;
  int scanresTotal    = 0;
  int bothStringsRead = 0;

	GPSdata *gps = (GPSdata*)malloc(sizeof(GPSdata) * dataSize);

	while (fgets(temp, MAX, fp)) {
    if (isGPGGA(temp)) {
		  scanresGPGGA = processGPGGA(temp, index, gps); /* Read data in gps struct and return scanres value */
    }
    else if (isGPRMC(temp)) {
      scanresGPRMC = processGPRMC(temp, index, gps);
      bothStringsRead = 1;
    }

    if (bothStringsRead){
      /* Assign gps quality */
      scanresTotal = scanresGPGGA+scanresGPRMC;
      gps[index].quality = getGPSQuality(scanresTotal, gps[index].hdop);
      ++index; /* Only count up index when data read from both strings */
      bothStringsRead = 0; /* Reset */
    }
    if (gps[index].quality == missingInfo || gps[index].quality == precisionLow){
      /* If bad quality initialize string to zero */
      gps[index].dateUTC      = 0;
      gps[index].clockUTC     = 0;
      gps[index].latitudeDDM  = 0;
      gps[index].directionLat = 0;
      gps[index].longitudeDDM = 0;
      gps[index].directionLon = 0;
      gps[index].hdop         = 0;  
    }
  }
  return gps;
}


int getGPSQuality(int scanres, double hdop){
  int quality = 0;
  /* Check if all data have been read from string */
  if (scanres != NO_OF_GPS_DATA_TYPES) {
    quality = missingInfo; /* If scanres not met, not enough info in strings*/
  } /* Check precision */
  else if (hdop > MAX_HDOP) { 
    quality = precisionLow;
  }
  else{
    quality = qualityOK;
  }
  return quality;
}

/* Read GPGGA string to struct members */
int processGPGGA(char *input, int index, GPSdata *gps){
  int scanres = 0;
  scanres = sscanf(input, " %*[^,],%lf,%lf,%c,%lf,%c,%*f,%*f,%lf",
    &gps[index].clockUTC,
    &gps[index].latitudeDDM,
    &gps[index].directionLat,
    &gps[index].longitudeDDM,
    &gps[index].directionLon,
    &gps[index].hdop);
  return scanres;
}
/* Read GPRMC string to struct member */
int processGPRMC(char *input, int index, GPSdata *gps){
  int scanres = 0;
  scanres = sscanf(input, " %*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%d",
    &gps[index].dateUTC);
  return scanres;
}

/* Make a point from raw gps data */
point makePoint(GPSdata gps){
  int offsetHour = 0;
  int offsetDay = 0;
  point point;

  /* Convert position to decimal degrees */
  point.latitude = getDecimalDegrees(gps.latitudeDDM);
  point.longitude = getDecimalDegrees(gps.longitudeDDM);
  /* Get correct direction. Degrees is negative if direction south or west */
  point.latitude = getDirection(point.latitude, gps.directionLat);
  point.longitude = getDirection(point.longitude, gps.directionLon);
  /* Get correct time */
  offsetHour = calcTimeZoneOffsetHour(point.longitude);
  if(offsetHour == 0){
    point.clock = gps.clockUTC;
    point.date = gps.dateUTC;
  }
  else{
    point.clock = getClockInLocalTime(gps.clockUTC, offsetHour, &offsetDay);
    point.date = getDateInLocalTime(gps.dateUTC, offsetDay);
  }
  return point;
}

/* Make degrees negative if direction is south or west */
double getDirection(double degrees, char direction){
  if(direction == 'S' || direction == 'W'){
    degrees = -degrees;
  }
  return degrees;
}

/* Convert degrees decimal minutes (DDM) to decimal degrees from format DDDMM.MMM */
double getDecimalDegrees(double degreesinDDM){
  double decimalMinutes = 0, decimalDegrees = 0;
  int degrees = 0;

  degrees = degreesinDDM / ONE_DEGREE;   /* Get degrees by moving comma 2 times left */
  degreesinDDM -= degrees * ONE_DEGREE; /* Substract degrees from DDM */

  decimalMinutes = degreesinDDM; /* DDM holds decimal minutes only now */

  /* Calculate decimal degree */
  decimalDegrees = (decimalMinutes / SEC_IN_MIN) + degrees;
  return decimalDegrees;
}

/* Make array of points */
point *makePoints(GPSdata *gps, int dataSize, int *ptrMaxPoints){
  polygon region;
  polygon *municipalities;
  polygon *areas;
  int munSize    = 0;
  int areaSize   = 0;
  int firstPoint = 0;
  int i          = 0;
  int p          = 0;

  point *points;
  /* Allocate mem for array of point structs */
  points = (point*)malloc(sizeof(point) * dataSize);
  if (points == NULL){
    printf("ERROR: Cannot allocate enough memory for %s.",
          "points");
    exit(EXIT_FAILURE);
  }

  /* FILL ARRAY WITH POINTs */
  /* Find first point where quality is OK */
  while(gps[i].quality != qualityOK){
    ++i;
  }
  firstPoint = i;
  /* Initialize first point as first point has no previus point */
  points[0] = makePoint(gps[firstPoint]);
  /* Initilize points not generated in function makePoint */
  points[0].dist  = 0;
  points[0].time  = 0;
  points[0].speed = 0;
  generatePolygons(points[0].latitude, points[0].longitude,
                   &region, &municipalities, &areas, &points[0].zone,
                   &munSize, &areaSize);

  /* Fill rest of array */
  for(i = firstPoint + 1, p = 1; i < dataSize; ++i){ 

    if (gps[i].quality == qualityOK){
    /* Make point data NOT dependent on previous point */
    points[p] = makePoint(gps[i]); 
    
    /* Make point data dependent on previous point */
    points[p].dist = calcKmBetweenPoints(points[p-1], points[p]);
    points[p].time = calcSecBetweenPoints(points[p-1], points[p]);
    points[p].speed = calcSpeedKmhBetweenPoints(points[p-1], points[p]);
    points[p].zone = calcPolygon(points[p].latitude, points[p].longitude,
                                 &region, &municipalities, &areas, points[p-1].zone,
                                 munSize, areaSize);
    ++p; /* Count up points index */
    }
  }

  *ptrMaxPoints = p; /* Return no. of points via pointer */

  free(municipalities);
  free(areas);

  return points;
}

/* Calc distance in km between two points given their latitudes and longitudes in degrees */
double calcKmBetweenPoints(point point1, point point2){
  double deltaLatRad  = 0, deltaLonRad  = 0;
  double point1LatRad = 0, point2LatRad = 0;
  double a = 0, c = 0;
  double distKm = 0;

  /* Convert to radians */
  deltaLatRad  = degreesToRadians(point2.latitude - point1.latitude);
  deltaLonRad  = degreesToRadians(point2.longitude - point1.longitude);
  point1LatRad = degreesToRadians(point1.latitude);
  point2LatRad = degreesToRadians(point2.latitude);

  /* Haversines formular calculates shortest dist between two points on af sphere (earth) */
  a  = pow(sin(deltaLatRad/2),2) + pow(sin(deltaLonRad/2),2) * cos(point1LatRad) * cos(point2LatRad);
  c  = 2 * atan2(sqrt(a), sqrt(1-a)); /* Arc tangent */
  distKm = c * RADIUS_EARTH;

  return distKm;
}

double degreesToRadians(double degrees){
  double radians = degrees * PI / 180;
  return radians;
}

/* Calculate seconds between two points */
double calcSecBetweenPoints(point point1, point point2){
    double seconds = 0;
    seconds = getClockInSeconds(point2.clock) - getClockInSeconds(point1.clock);
    return seconds;
}

/* Convert clock to seconds from format HHMMSS.SSS */
double getClockInSeconds(double clock){
    double totalSeconds = 0, sec = 0;
    int hour = 0, min = 0;

    hour  = clock / ONE_HOUR;
    clock -= hour * ONE_HOUR;

    min   = clock / ONE_MIN;
    clock -= min * ONE_MIN;

    sec = clock; /* Clock holds seconds only now */

    /* Calculate seconds */
    totalSeconds = (hour * SEC_IN_HOUR) + (min * SEC_IN_MIN) + sec;
    return totalSeconds;
}

/* Converts gps time(UTC) to local time zone */
double getClockInLocalTime(double clockUTC, int offsetHour, int *ptrOffsetDay){
  double clockLocal = 0;
  int offsetDay = 0;

  /* Get clock in local time by adding/substracting offset value */
  clockLocal = clockUTC + (offsetHour * ONE_HOUR);

  /* Adjust clock and and offset day if clock is off limits after conversion to local time */
  if(clockLocal >= 24 * ONE_HOUR){
    clockLocal -= 24 * ONE_HOUR;
    ++offsetDay;
  }
  else if(clockLocal < 0){
    clockLocal += 24 * ONE_HOUR;
    --offsetDay;
  }

  *ptrOffsetDay = offsetDay;

  return clockLocal;
}

/* Converts gps date (UTC) to local time zone */
int getDateInLocalTime(int dateUTC, int offsetDay){
  int dateLocal = 0;

  if(offsetDay == 1){
    dateLocal = dateUTC + ONE_DAY;
  }
  else if (offsetDay == -1){
    dateLocal = dateUTC - ONE_DAY;
  }
  else{
    dateLocal = dateUTC;
  }

  return dateLocal;
}

double calcSpeedKmhBetweenPoints(point point1, point point2){
    double kmh = 0;
    kmh = (calcKmBetweenPoints(point1, point2) / calcSecBetweenPoints(point1,point2)) * SEC_IN_HOUR;
    return kmh;
}

/* Calculate offset value in hours */
int calcTimeZoneOffsetHour(double longitude){
  int offsetHour = 0;
  int limitUTCzone = 0;
  /* UTC time zone spans from -7.5 to +7.5 degrees longitude */
  if (longitude > 0) {
    limitUTCzone = 7.5;
  }
  else{
    limitUTCzone = -7.5;
  }
  /* Time zone change 1 hour per 15 degrees */
  offsetHour = (longitude + limitUTCzone) / 15;
  return offsetHour;
}

/* Check if day or night */
int isNight(double clock){
  int hour  = 0;
  /* Check what hour of day */
  hour = clock / ONE_HOUR;
  /* Check if hour outside day time */
  if((DAYTIME_START <= hour && hour <= DAYTIME_END)!=1)
    return 1;
  else
    return 0;
}

/* Check if bad weather season */
double isBadWeather(int date){
  int day = 0, month = 0, monthChanceOfRain = 0, monthChanceOfFrost = 0, randomNumber;
  double riskPoints = 0;
  /* Arrays holding probalities of bad weather in months 1-12 */
  int monthsRain[12] = {29, 25, 22, 20, 21, 26, 26, 27, 27, 30, 31, 31};
  int monthsFrost[12] = {59, 59, 0, 0, 0, 0, 0, 0, 0, 0, 0, 59};

  /* Get month */
  day = date / ONE_DAY;
  date -= day * ONE_DAY;
  month = date / ONE_MONTH;
  month = month - 1;
  /*Create random number based on seed*/
  srand(time(NULL));
  randomNumber = rand() % 100;
  /*The number is now in the range between 0-100*/
  monthChanceOfRain = monthsRain[month];
  monthChanceOfFrost = monthsFrost[month];

  if (randomNumber <= monthChanceOfRain) {
    riskPoints += 0.5;
  }
  randomNumber = rand() % 100; /* Get new random number */
  if (monthChanceOfFrost != 0 && randomNumber <= monthChanceOfFrost) {
    riskPoints += 0.5;
  }
  return riskPoints;
}

/* Print the generated array of points*/
void printPoints(point *points, int maxPoints){
  int i = 0;

  printf("\n\nGENERATED POINTS");
  printf("\n------------------------\n");
  printf("\nMax points: %d\n\n", maxPoints);

  printf("[%5s]\t  %-11s %-14s %-9s %-10s %-9s %-9s %-12s %-18s %s\n\n","INDEX", "DATE", "TIME", "LATITUDE" ,"LONGITUDE", "KM/POINT", "SEC/POINT", "KM/H", "ZONE", "SPEED LIMIT");
  for(i = 0; i < maxPoints; ++i){
    printf("[%5d]\t  %-11s %-14s %-9.3lf %-10.3lf %-9.3lf %-9.3lf %-12.3lf %-18s %d\n",
          i,
          printDate(points[i].date),
          printClock(points[i].clock),
          points[i].latitude,
          points[i].longitude,
          points[i].dist,
          points[i].time,
          points[i].speed,
          points[i].zone.name,
          points[i].zone.speedLimit);
 }
}

/* Print clock in correct format. Used in print functions */
char *printClock(double clock){
  static char printclock[13];
  int hour = 0, min = 0;
  double sec = 0;

  hour = clock / ONE_HOUR;
  clock -= hour * ONE_HOUR;

  min  = clock / ONE_MIN;
  clock -= min * ONE_MIN;

  sec  = clock;

  sprintf(printclock, "%s%d:%s%d:%s%.2lf",
    hour < 10 ? "0" : "", hour, /* Add 0 char in front if needed */
    min  < 10 ? "0" : "", min,
    sec  < 10 ? "0" : "", sec);

  return printclock;
}
/* Print clock in correct format. Used in print functions */
char *printDate(int date){
  static char printdate[8];
  int day = 0, month = 0, year = 0;
  /* Extract date from integer DDMMYY */
  day = date / ONE_DAY;
  date -= day * ONE_DAY;

  month = date / ONE_MONTH;
  date -= month * ONE_MONTH;

  year = date;

  sprintf(printdate, "%s%d-%s%d-%s%d",
    day   < 10 ? "0" : "", day, /* Add 0 char in front if needed */
    month < 10 ? "0" : "", month,
    year  < 10 ? "0" : "", year);

  return printdate;
}
/* Print clock in correct format. Used in print functions */
char *printSecondsToClockFormat(double totalSec){
  static char printclock[13];
  int hour = 0, min = 0;
  double sec = 0;
  /* Extract clock from double HHMMSS.SSS*/
  hour = totalSec / SEC_IN_HOUR;
  totalSec -= hour * SEC_IN_HOUR;

  min = totalSec / SEC_IN_MIN;
  totalSec -= min * SEC_IN_MIN;

  sec = totalSec;

  sprintf(printclock, "%s%d:%s%d:%s%.2lf",
  hour < 10 ? "0" : "", hour, /* Add 0 char in front if needed */
  min  < 10 ? "0" : "", min,
  sec  < 10 ? "0" : "", sec);

  return printclock;
}

/* Fill struct tripdata with information about trip based on generated points */
tripdata makeTripData(point *points, int maxPoints, GPSdata *gps, int dataSize){
  int lastPoint         = maxPoints-1;
  double totalSpeed     = 0;
  int i                 = 0;
  int totalSpeedCounter = 0;
  int firstPoint        = 0;
  tripdata tripdata;
  tripdata.totalKm                = 0;
  tripdata.noOfPointsMissingInfo  = 0;
  tripdata.noOfPointsPrecisionLow = 0;
  tripdata.maxSecBetweenPoints   = 0;

  /* Get data from points */
  for(i = 1; i < maxPoints; ++i){ /* Start at index 1 as first point holds no speed, dist and time since previus point */
    /* Get total km and speed data */
    tripdata.totalKm += points[i].dist;
    totalSpeed += points[i].speed;
    ++totalSpeedCounter;
    /* Get max dist between points*/
    if (points[i].time > tripdata.maxSecBetweenPoints){
      tripdata.maxSecBetweenPoints = points[i].time;
    }
  }
  /* Get no. of discarded gps points divided in missing info and low precision */
  for(i = 0; i < dataSize; ++i){ 
    if(gps[i].quality == missingInfo){
      ++tripdata.noOfPointsMissingInfo;
    } 
    else if(gps[i].quality == precisionLow){
      ++tripdata.noOfPointsPrecisionLow;
    }
  }
  /* Fill rest of struct */
  tripdata.totalTime     = calcSecBetweenPoints(points[firstPoint], points[lastPoint]);
  tripdata.startTime     = points[firstPoint].clock;
  tripdata.endTime       = points[lastPoint].clock;
  tripdata.avgSpeed      = totalSpeed / totalSpeedCounter;
  tripdata.isNight       = isNight(points[firstPoint].clock);
  tripdata.isBadweather  = isBadWeather(points[firstPoint].date);
  tripdata.noOfPoints    = maxPoints;
  tripdata.noOfPointsRaw = dataSize;
  
  return tripdata;
 }


/* Print trip data struct */
void printTripData(tripdata tripdata){

  printf("\nCongratulations! Trip has been processed.\n\n");
  printf("TRIP DATA\n");
  printf("\n%-25s %.2lf km\n", "Total distance:", tripdata.totalKm);
  printf("%-25s %.3lf km/t\n","Average speed:", tripdata.avgSpeed);
  printf("\n%-25s %s\n", "Start time:", printClock(tripdata.startTime));
  printf("%-25s %s\n", "End time:", printClock(tripdata.endTime));
  printf("\n%-25s %s\n", "Total time:", printSecondsToClockFormat(tripdata.totalTime));
  printf("\n%-25s %s", "Time of day", tripdata.isNight == 1 ? "Night driving" : "Day driving");
  printf("\n%-25s %s", "Weather", tripdata.isBadweather == 1 ? "Rain AND frost" :
                                  tripdata.isBadweather == 0.5 ? "Rain OR frost" : "Good");
  printf("\n\n%-28s %4d\n\n", "GPS points in file:", tripdata.noOfPointsRaw);
  printf("%-28s %4d\n", "Missing data in string:", tripdata.noOfPointsMissingInfo);
  printf("%-28s %4d\n", "Inaccurate (HDOP):", tripdata.noOfPointsPrecisionLow);
  printf("%-28s %4d\n", "Total points discarded:", tripdata.noOfPointsMissingInfo + tripdata.noOfPointsPrecisionLow);
  printf("\n%-28s %4d\n", "GPS points, generated:", tripdata.noOfPoints);
  printf("\n%-28s %4.2lf\n", "Sec. between points, max:", tripdata.maxSecBetweenPoints);
  printf("_________________________________________\n");
}

