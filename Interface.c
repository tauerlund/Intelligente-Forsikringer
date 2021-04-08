#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <math.h>
#include "functions.h"

void cls () { /* CLEAR SCREEN */
  int i = 0;
  for(i = 0; i < 10; i++) {
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
  }
}

/* Checks if input is a valid email */
int checkIfEmail(char email[]) {
  int scanres = 0;
  char string1[MAX], string2[MAX], string3[MAX];

  scanres = sscanf(email, "%[^@]@%[^.].%s", string1, string2, string3);

  if (scanres == 3)
    return 1;
  else
    return 0;
}

/* Show GPS-files in folder GPS */
void showDircontentGPS() {
  struct dirent *dirEntry; /* Struct declared in dirent.h. Holding member d_name (name of entry)*/
  DIR *dir = opendir("./GPS"); /* Open GPS directory */
  printf("\nFolder: GPS\n\n");

   if (dir == NULL) {
     printf("Failed to load DIR" );
     return;
   }
   while ((dirEntry = readdir(dir)) != NULL) { /* Read dir content */
      printf("%s\n", dirEntry->d_name); /* Print file names */
  }
   printf("\n");
   closedir(dir);
}

/* Creates or updates profile */
void updateProfile (userProfile profile) {
  int dataSize     = 0;
  int maxPoints    = 0;
  int state        = 0;
  int fileCheck    = 0;
  int existingUser = 0;
  char inputGPS[MAX];
  char temp[MAX];
  char messageGPS[MAX] = "Please enter GPS file name.";
  char choice[MAX];
  GPSdata *gps;
  point *points;
  tripdata trip;
  tripRisk tripRisk;

  do {
    cls();

    showDircontentGPS();
    printf("%s\n\nInput: ", messageGPS);
    scanf(" %s", temp);
    sprintf(inputGPS, "GPS/%s", temp);

    /* Check file */
    fileCheck = checkGPSfile(inputGPS);
    if (fileCheck == fileOK){
      state = 1;
    }
    else {
      sprintf(messageGPS, "%s", getFileError(fileCheck));
    }
  } while (state == 0);

  if (state == 1) {
    cls();
    /* Make points of GPS-data from file*/
    gps = makeGPSdata(inputGPS, &dataSize);
    printGPSdata(gps, dataSize); 
    /* Make converted points from GPS-data points*/
    points = makePoints(gps, dataSize, &maxPoints);
    printPoints(points, maxPoints);
    /* Make trip data overwiew from converted points*/
    trip = makeTripData(points, maxPoints, gps, dataSize);
    printTripData(trip);
    /* Check if sec between points exceed tolerance */
    if (trip.maxSecBetweenPoints > MAX_SEC_BETWEEN_POINTS){
      state = 0;
      while (state == 0){
        printf("\nWARNING!!\n\nDATA INACCURATE: MAX SECONDS BETWEEN POINTS EXCEED TOLERANCE (%.2lf SEC.)\n", MAX_SEC_BETWEEN_POINTS);
        printf("\nGo back  (0)\nContinue (1)\n\nInput: ");
        scanf(" %s", choice);
        if (strcmp(choice, "1") == 0) {
          state = 1;
        }
        if (strcmp(choice, "0") == 0){
          state = 2;
        }
      }
    }

    if(state == 1){
      /* Calculate risk from points and trip data*/
      tripRisk = calcRisk(points, trip);
      printTripRisk(tripRisk);

      /* Check if user exist */
      existingUser = profile.kilometers != 0;

      /* Only print old profile and calculate weighted risk if existing user */
      if(existingUser) {
        printf("\nOLD PROFILE\n\n");
        printProfile(profile,1,1,1,1,1);
        printf("_________________________________________\n");
        profile.riskProfile = weightRisk(profile, trip, tripRisk.totalRisk);
      }
      else {
        profile.riskProfile = tripRisk.totalRisk;
      }

      profile.kilometers += trip.totalKm; /* Add km of this trip to profile total km*/
      profile.newPremium = calcPremium(profile); /* Get premium*/

      printf("\n%s PROFILE\n\n", existingUser ? "UPDATED" : "NEW");
      printProfile(profile,1,1,1,1,1);
      printf("_________________________________________\n");
    }
    storeData(profile); /* Save data in database */
    free(gps);
    free(points);
  }

  while (state == 1) {
    printf("\nDo you wish to go back? (0)\n\nInput: ");
    scanf(" %s", choice);
    if (strcmp(choice, "0") == 0) {
      state = 2;
    }
  }
}

void createProfile () {
  char inputEmail[MAX];
  char message[MAX] = "Please enter a valid email.";
  int state = 0;
  userProfile profile;

  do {
    cls();
    printf("%s\n\nInput: ", message);
    scanf(" %s", inputEmail);

    if (checkIfEmail(inputEmail)) {
      profile = getProfileBinary(inputEmail);

      if (strcmp(profile.email, "\0") == 0) {
        strcpy(profile.email, inputEmail);
        profile.riskProfile = 0;
        profile.basePremium = BASEPREMIUM;
        profile.newPremium  = 0;
        profile.kilometers  = 0;
        updateProfile(profile);
        state = 1;
      }
      else {
        sprintf(message, "Email \"%s\" already exists. Please enter a new email.", inputEmail);
      }
    }
    else {
      sprintf(message, "Entered email is not a valid email. Please enter a new email.");
    }
  } while (state == 0);
}

/* Email riskprof baseprem newprem km */
void dataIndex (FILE *fp) {  /* ADMIN DATABASE (FULL SORTED LISTS) */
  int dbSize = 0;
  int state  = 0;
  char choice[MAX];
  userProfile *profiles;

  profiles = getProfiles();
  rewind(fp);
  fscanf(fp, "(%d)\n", &dbSize);	/* Gets index ammount in DB */
  do {
    cls();
    rewind(fp);
    printf("SORT BY:\n1) Risk profile\n2) Base premium\n3) Adj. premium\n4) Kilometers\n0) Back\n\nInput: ");
    scanf(" %s", choice);

    if (strcmp(choice, "1") == 0) {
      dataIndexSorted(fp, profiles, dbSize, &neg_cmp_risk_names, &cmp_risk_names);
    }
    else if (strcmp(choice, "2") == 0) {
      dataIndexSorted(fp, profiles, dbSize, &neg_cmp_basep_names, &cmp_basep_names);
    }
    else if (strcmp(choice, "3") == 0) {
      dataIndexSorted(fp, profiles, dbSize, &neg_cmp_newp_names, &cmp_newp_names);
    }
    else if (strcmp(choice, "4") == 0) {
      dataIndexSorted(fp, profiles, dbSize, &neg_cmp_km_names, &cmp_km_names);
    }
    else if (strcmp(choice, "0") == 0) {
      state = 1;
    }
  } while (state == 0);
}


void dataIndexSorted(FILE *fp, userProfile *profiles, int dbSize, functionPointer a, functionPointer b) {
  char choice[MAX];
  int state = 0;

  do {
    cls();
    printf("1) Ascending\n2) Descending\n0) Back\n\nInput: ");
    scanf(" %s", choice);

    if(strcmp(choice, "1") == 0) {
      state = 1;
      qsort(profiles, dbSize, sizeof(userProfile), a);
      adminPrint(fp, profiles);
    }
    else if (strcmp(choice, "2") == 0) {
      state = 1;
      qsort(profiles, dbSize, sizeof(userProfile), b);
      adminPrint(fp, profiles);
    }
    else if (strcmp(choice, "0") == 0) {
      state = 1;
    }
  } while (state == 0);
}

/* Print profiles in database  */
void adminPrint (FILE *fp, userProfile *profiles) { /* PRINTER FOR ADMIN DATABASE */
  int i      = 0;
  int dbSize = 0;
  char choice[MAX];

  rewind(fp);
  fscanf(fp, "(%d)\n", &dbSize);
  while (strcmp(choice, "0") != 0) {
    cls();
    for(i = 0; i < dbSize; i++) {
      printProfile(profiles[i], 1, 1, 1, 1, 1);
      printf("\n\n");
    }
    printf("Do you wish to go back? (0)\n\nInput: ");
    scanf(" %s", choice);
  }
}

/* Compares by risk then emails, Ascending */
int cmp_risk_names(const void *a, const void *b) {
  int res = 0;
  userProfile *riskA = (userProfile *)a;
  userProfile *riskB = (userProfile *)b;

  if ((*riskA).riskProfile < (*riskB).riskProfile)
  {
    res = 1;
  }
  else if ((*riskA).riskProfile > (*riskB).riskProfile)
  {
    res = -1;
  } else if ((*riskA).riskProfile == (*riskB).riskProfile) {
    if ((*riskA).email < (*riskB).email) {
      res = 1;
    } else if ((*riskA).email < (*riskB).email) {
      res = -1;
    }
  }
  return res;
}
/* Compares by base premium, Ascending */
int cmp_basep_names(const void *a, const void *b) {
  int res = 0;
  userProfile *baseA = (userProfile *)a;
  userProfile *baseB = (userProfile *)b;

  if ((*baseA).basePremium < (*baseB).basePremium)
  {
    res = 1;
  }
  else if ((*baseA).basePremium > (*baseB).basePremium)
  {
    res = -1;
  }
  else if ((*baseA).basePremium == (*baseB).basePremium) {
    if ((*baseA).email < (*baseB).email) {
      res = 1;
    }
    else if ((*baseA).email < (*baseB).email) {
      res = -1;
    }
  }
  return res;
}

/* Compare by newPremium, Ascending */
int cmp_newp_names(const void *a, const void *b) {
  int res = 0;
  userProfile *newA = (userProfile *)a;
  userProfile *newB = (userProfile *)b;

  if ((*newA).newPremium < (*newB).newPremium)
  {
    res = 1;
  }
  else if ((*newA).newPremium > (*newB).newPremium)
  {
    res = -1;
  }
  else if ((*newA).newPremium == (*newB).newPremium) {
    if ((*newA).email < (*newB).email) {
      res = 1;
    }
    else if ((*newA).email < (*newB).email) {
      res = -1;
    }
  }
  return res;
}

/* Compare by kilometers then emails, Ascending */
int cmp_km_names(const void *a, const void *b)
{
  int res = 0;
  userProfile *kmA = (userProfile *)a;
  userProfile *kmB = (userProfile *)b;

  if ((*kmA).kilometers < (*kmB).kilometers)
  {
    res = 1;
  }
  else if ((*kmA).kilometers > (*kmB).kilometers)
  {
    res = -1;
  }
  else if ((*kmA).kilometers == (*kmB).kilometers) {
    if ((*kmA).email < (*kmB).email) {
      res = 1;
    }
    else if ((*kmA).email < (*kmB).email) {
      res = -1;
    }
  }
  return res;
}


/* Compares by risk then emails, Descending */
int neg_cmp_risk_names(const void *a, const void *b) {
  int res = 0;
  userProfile *riskA = (userProfile *)a;
  userProfile *riskB = (userProfile *)b;

  if ((*riskA).riskProfile > (*riskB).riskProfile)
  {
    res = 1;
  }
  else if ((*riskA).riskProfile < (*riskB).riskProfile)
  {
    res = -1;
  }
  else if ((*riskA).riskProfile == (*riskB).riskProfile) {
    if ((*riskA).email < (*riskB).email) {
      res = 1;
    }
    else if ((*riskA).email < (*riskB).email) {
      res = -1;
    }
  }
  return res;
}
/* Compares by base premium, Descending */
int neg_cmp_basep_names(const void *a, const void *b) {
  int res = 0;
  userProfile *baseA = (userProfile *)a;
  userProfile *baseB = (userProfile *)b;

  if ((*baseA).basePremium > (*baseB).basePremium)
  {
    res = 1;
  }
  else if ((*baseA).basePremium < (*baseB).basePremium)
  {
    res = -1;
  }
  else if ((*baseA).basePremium == (*baseB).basePremium) {
    if ((*baseA).email < (*baseB).email) {
      res = 1;
    }
    else if ((*baseA).email < (*baseB).email) {
      res = -1;
    }
  }
  return res;
}

/* Compare by newPremium, Descending */
int neg_cmp_newp_names(const void *a, const void *b) {
  int res = 0;
  userProfile *newA = (userProfile *)a;
  userProfile *newB = (userProfile *)b;

  if ((*newA).newPremium > (*newB).newPremium)
  {
    res = 1;
  }
  else if ((*newA).newPremium < (*newB).newPremium)
  {
    res = -1;
  }
  else if ((*newA).newPremium == (*newB).newPremium) {
    if ((*newA).email < (*newB).email) {
      res = 1;
    }
    else if ((*newA).email < (*newB).email) {
      res = -1;
    }
  }
  return res;
}

/* Compare by kilometers then emails, Descending */
int neg_cmp_km_names(const void *a, const void *b){
  int res = 0;
  userProfile *kmA = (userProfile *)a;
  userProfile *kmB = (userProfile *)b;

  if ((*kmA).kilometers > (*kmB).kilometers) {
    res = 1;
  }
  else if ((*kmA).kilometers < (*kmB).kilometers) {
    res = -1;
  }
  else if ((*kmA).kilometers == (*kmB).kilometers) {
    if ((*kmA).email < (*kmB).email) {
      res = 1;
    }
    else if ((*kmA).email < (*kmB).email) {
      res = -1;
    }
  }
  return res;
}

void dbCheckAndCreate () {
  FILE *fc;
  char datastring[200] = "\"email@host.abr\" | 0.0 , 0.0 , 0.0 , 0.0 |\n";
  FILE *fp = fopen(DB, "r");

  if(fp == NULL) {
    printf("NEW DATABASE CREATED\n");
    fclose(fp);

    fc = fopen(DB, "w");
    fputs("(1)\n", fc);
    fputs(datastring, fc);
    fclose(fc);
  }
  fclose(fp);
}
