#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>

/* GENEREL*/
#define MAX_STRING_LENGTH 250 /* GPS max string */
#define MAX 200
#define DB "database"

/* MAIN */
#define PASSWORD "A319"

/* POLYGONS */
#define POLYGONS "polygons.txt"
#define DEFAULT_SPD_LIMIT 80
#define CHAR_LENGTH 2 

/* INTERFACE */
#define VERSION 0.4
#define TITLE "UBI PROGRAM"
#define BASEPREMIUM 10000

#define ARRAYMAX 100
#define DATAMAX 999

#define FOLDERONE "Versions"
#define FOLDERTWO "Non pusherino kode"
#define FOLDERTHREE ".git"
#define FOLDERFOUR "GPS"

/*PROCESS GPS */
#define NO_OF_GPS_DATA_TYPES 7 /* No of data types in gps point */
#define NO_OF_STRING_TYPES 3 /* No of gps string types */
#define MAX_HDOP 5.0 /* Maximum dilution of precision */
#define MAX_SEC_BETWEEN_POINTS 1.0
#define DAYTIME_START 6
#define DAYTIME_END 22
#define SEC_IN_HOUR 3600
#define SEC_IN_MIN 60
#define ONE_DEGREE 100 /* Get degrees by moving comma 2 times left */
#define RADIUS_EARTH 6371
#define PI 3.14159
    /* Clock format HHMMSS.SSS */
#define ONE_HOUR 10000 /* Get hours by moving comma 4 times left */
#define ONE_MIN 100 /* Get minutes by moving comma 2 times left  */
    /* Date format DDMMYY */
#define ONE_DAY 10000 /* Get days by moving comma 4 times left */
#define ONE_MONTH 100 /* Get months by moving comma 2 times left  */
/*CALC RISK */
#define MIN_SPEED_MARGIN 5
/* CALC PREMIUM */
#define PREMIUM_SETTINGS "Premium Settings.ini"
#define BASE100 100


/*---------POLYGONS.C FUNCTIONS AND STRUCTS:---------*/

typedef struct {
  char name[MAX];
  double numberOfPoints;
  double x[MAX];
  double y[MAX];
  int speedLimit;
  int level;
} polygon;

polygon calcPolygon(double pointX, double pointY, polygon *region, polygon
                  **municipalities, polygon **areas, polygon lastKnownPoly,
                  int munSize, int areaSize);

void generatePolygons(double pointX, double pointY, polygon *regions, polygon
                    **munis, polygon **areas, polygon *current, int *mSize,
                    int *aSize);

polygon generateRegion(double pointX, double pointY, FILE* fp, int *regionMunis, polygon *current);

polygon* generateMunicipalities(double pointX, double pointY, FILE* fp, int regionMunis,
                                int *regionAreas, char name[], polygon *current);

polygon* generateAreas(double pointX, double pointY, FILE* fp, int regionAreas,
                      char name[], polygon *current);

polygon readCoordinates(FILE* fp, int numberOfCoords);
int getSize(FILE *fp);
int pointInPolygon(double pointX, double pointY, polygon poly);

/*------------------------------------------------------*/

/*---------PROCESSGPS.C STRUCTS AND FUNCTIONS ---------*/

typedef struct {
  int dateUTC; /* In Coordinated Universal Time */
  double clockUTC;
  double latitudeDDM; /* In degrees decimal minutes */
  double longitudeDDM;
  char directionLat;
  char directionLon;
  double hdop; /* Horizontal Dilution of Precision */
  int quality;
} GPSdata;

typedef struct{
  /* Local time */
  int date;
  double clock;
  /* Latitude and longitude in decimal degrees*/
  double latitude;
  double longitude;
  /* Speed (km/t), dist (km) and time(sec) since last point */
  double speed;
  double dist;
  double time;
  polygon zone; /* Geofence zone */
} point;

typedef struct{
  double totalKm;
  double totalTime;
  double startTime;
  double endTime;
  double avgSpeed;
  int isNight;
  double isBadweather;
  int noOfPoints;
  int noOfPointsRaw;
  int noOfPointsMissingInfo;
  int noOfPointsPrecisionLow;
  double maxSecBetweenPoints;
} tripdata;

typedef enum fileCheck {fileOK, notNMEA, cannotOpenFile, incorrectStrings} fileCheck;
typedef enum gpsQuality {qualityOK, precisionLow, missingInfo} gpsQuality;

int checkGPSfile (char *userInput);
int isNMEA (char *userInput);
int checkStrings(FILE *fp);
char* getFileError(int fileCheck);
int isGPGSA(char *string);
int isGPGGA(char *string);
int isGPRMC(char *string);
int getGPSQuality(int scanres, double hdop);
GPSdata *makeGPSdata(char *userInput, int *ptrDataSize);
GPSdata *addGPSdata(FILE *fp, int dataSize);
int countPointsInFile(FILE *fp);
int processGPGGA(char *input, int index, GPSdata *gps);
int processGPRMC(char *input, int index, GPSdata *gps);
void printGPSdata(GPSdata *gps, int dataSize);

point  *makePoints(GPSdata *gps, int dataSize, int *ptrMaxPoints);
point makePoint(GPSdata gps);
int getDateInLocalTime(int dateUTC, int offsetDay);
double getClockInSeconds(double clock);
double getClockInLocalTime(double clockUTC, int offsetHour, int *ptrOffsetDay);
double getDecimalDegrees(double degreesinDDM);
double degreesToRadians(double degrees);
double getDirection(double degrees, char direction);
int calcTimeZoneOffsetHour(double longitude);
double calcSpeedKmhBetweenPoints(point point1, point point2);
double calcSecBetweenPoints(point point1, point point2);
double isBadWeather(int date);
double calcKmBetweenPoints(point point1, point point2);
int isNight(double clock);
double calcTripDist(point *points, int dataSize);
void printPoints(point *points, int maxPoints);
char *printClock(double clock);
char *printDate(int date);
char *printSecondsToClockFormat(double seconds);

tripdata makeTripData(point *points, int maxPoints, GPSdata *gps, int dataSize);
void printTripData(tripdata tripdata);

/*---------DATABASE.C FUNCTIONS AND STRUCTS---------*/

typedef struct {
  char email[MAX];
  double riskProfile;
  double basePremium;
  double newPremium;
  double kilometers;
} userProfile;

int checkIfEmail(char email[]);
void storeData(userProfile profile);
int verifyBinary(char string[]);
userProfile getProfileBinary (char target[]);
int findBinary(FILE* fp, char string[], int low, int mid, int high, int last);
int compareProfilesAlphabetical(const void *ip1, const void *ip2);
void dbCheckAndCreate();

/*------------------------------------------------------*/

/*---------LOAD AND SCOREBOARD FUNCTIONS---------*/

void loadProfile();
userProfile* getProfiles();
void printProfile(userProfile profile, int email, int riskProfile,
                int basePremium, int newPremium, int kilometers);
userProfile scanProfile(char *string);
void showProfiles(userProfile *scoreBoard, int count);
void showFriends();
void showScoreBoard();
int isStringNumber(char str[]);
int sameAsPrevious(char email[], userProfile *scoreBoard, int index);
int compareRisk (const void * a, const void * b);
void printFriends(userProfile *input, int size);

/*------------------------------------------------------*/


/*---------CALCULATERISK.C FUNCTIONS AND STRUCTS---------*/

  typedef struct {
  double speedingTime;
  double speedingAvgKmh;
  double speedingRatio;
  double unweighted;
  double factor;
  double base100;
  double weight;
  double weighted;
  } speedRisk;

  typedef struct {
  double unweighted;
  double factor;
  double base100;
  double weight;
  double weighted;
  } riskData;

  typedef struct {
  speedRisk speedRisk;
  riskData timeOfDayRisk;
  riskData timeDrivenRisk;
  riskData tripKmRisk;
  riskData weatherRisk;
  double totalRisk;
  } tripRisk;

  typedef struct {
  double factor;
  double base100;
  double weightedRisk;
  } riskWeightData;

typedef struct {
  double speedRiskLimit;
  double speedRiskWeight;
  double timeOfDayRiskLimit;
  double timeOfDayRiskWeight;
  double timeDrivenRiskLimit;
  double timeDrivenRiskWeight;
  double tripKmRiskLimit;
  double tripKmRiskWeight;
  double seasonRiskLimit;
  double seasonRiskWeight;
} riskSettings;

typedef struct {
  double maxDiscount;
  double lowerRiskLimit;
  double upperRiskLimit;
} premiumSettings;

tripRisk calcRisk(point *points, tripdata tripData);
riskSettings readRiskIni();
speedRisk calcSpeedRisk(point *points, tripdata tripData, riskSettings riskSettings);
double calcSpeedOverAvg(double speedOver, int total);
double calcSpeedWeight(double timeDriven, double timeSpeeding);
riskData calctimeOfDayRisk(tripdata tripData, riskSettings riskSettings);
riskData calcTimeDrivenRisk(tripdata tripData, riskSettings riskSettings);
double totalTimeInHrs(double time);
riskData calcTripKmRisk(tripdata tripData, riskSettings riskSettings);
riskData calcWeatherRisk(tripdata tripData, riskSettings riskSettings);
double weightRisk(userProfile profile, tripdata trip, double tripRisk);
riskWeightData calcWeightedRisk(double riskLimit, double riskUnweighted, double riskWeight);
double getFactor(double t);
double calcBase100Risk(double factor, double value);
double weightBase100Risk(double base100Risk, double weight);
void printTripRisk(tripRisk tripRisk);

/*------------------------------------------------------*/

/*---------CALCPREMIUM.C FUNCTIONS AND STRUCTS---------*/

premiumSettings readPremiumSettingsIni();
double calcPremium(userProfile profile);

/*------------------------------------------------------*/

/*---------INTERFACE.C FUNCTIONS AND STRUCTS---------*/

typedef int(*functionPointer)(const void*, const void*);

void createProfile();
void updateProfile ();
void cls();
void adminInterface();
int checkIfEmail(char email[]);
void userInterface();
void showDircontentCURR();
void showDircontentGPS();
void subNavigator();
userProfile getProfile(FILE *fp, char *string);
void loadUserselection();
void loadUpdateuser();
void dataIndex();
void checkifAdmin();
void dataDisplay();
void dataFilters();
void dbCheckAndCreate();
void searchEmail (FILE *fp);
int cmp_risk_names(const void *a, const void *b);
int cmp_basep_names(const void *a, const void *b);
int cmp_newp_names(const void *a, const void *b);
int cmp_km_names(const void *a, const void *b);
int neg_cmp_risk_names(const void *a, const void *b);
int neg_cmp_basep_names(const void *a, const void *b);
int neg_cmp_newp_names(const void *a, const void *b);
int neg_cmp_km_names(const void *a, const void *b);
void dataIndexSorted(FILE *fp, userProfile *profiles, int dbSize, functionPointer a, functionPointer b);
void dataBaseprem (FILE *fp, userProfile *profiles, int dbSize);
void dataNewprem (FILE *fp, userProfile *profiles, int dbSize);
void dataKilometers (FILE *fp, userProfile *profiles, int dbSize);
void adminPrint (FILE *fp, userProfile *profiles);

/*------------------------------------------------------*/
