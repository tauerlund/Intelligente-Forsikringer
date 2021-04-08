#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include"functions.h"
#define DIVISOR 2

/* Store data in database */
void storeData(userProfile profile) {
  FILE *fp;
  int dbSize    = 0;
  int dbSizeNew = 0;
  int i         = 0;
  userProfile *buffer;

  /* Verifies if in database, if not in database we allow storing indicated by incrementing database size by one */
  if (!verifyBinary(profile.email)) {
    dbSizeNew++;
  }

  fp = fopen(DB, "r"); /* Opens database */
  fscanf(fp, "(%d)\n", &dbSize);	/* Gets index ammount in database */
  dbSizeNew += dbSize;	/* adds the size of the database together with the increment */
  buffer = (userProfile*)malloc(sizeof(userProfile) * dbSizeNew); /* Allocates the precise ammount needed */
  /* Gets information about users into the buffer struct */
  for (i = 0; i < dbSize; i++) {
    fscanf(fp, "\"%[^\"]\" | %lf , %lf , %lf , %lf |\n", buffer[i].email,
          &buffer[i].riskProfile, &buffer[i].basePremium, &buffer[i].newPremium,
          &buffer[i].kilometers);
  }
  fclose(fp);

  /* Sets intput profile into the right buffer */
  i = 0;
  /* If the database is larger than the original ammount, create profile at last index */
  if(dbSizeNew > dbSize){
   /* Create a new profile */
   strcpy(buffer[dbSize].email, profile.email);
   buffer[dbSize].riskProfile = profile.riskProfile;
   buffer[dbSize].basePremium = profile.basePremium;
   buffer[dbSize].newPremium  = profile.newPremium;
   buffer[dbSize].kilometers  = profile.kilometers;

   qsort(buffer, dbSizeNew, sizeof(userProfile), compareProfilesAlphabetical);
  }
  else {
  	/* Update profile on its previous index */
  	while (i < dbSize) {
  	  if (strcmp(profile.email, buffer[i].email) == 0){
	    	strcpy(buffer[i].email, profile.email);
      	buffer[i].riskProfile = profile.riskProfile;
      	buffer[i].basePremium = profile.basePremium;
      	buffer[i].newPremium  = profile.newPremium;
      	buffer[i].kilometers  = profile.kilometers;
        i = dbSize;
      }
    i++;
  	}
  }

  /* Writes the newly created or updated profile into database by passing the profile from buffer to the database file */
  /* Overwrites database, so we pass all old profiles plus the new updated or created profile */
  fp = fopen(DB, "w");
  fprintf(fp, "(%d)\n", dbSizeNew);
  for (i = 0; i < dbSizeNew; i++) {
    fprintf(fp, "\"%s\" | %.2lf , %.2lf , %.2lf , %.2lf |\n", buffer[i].email,
            buffer[i].riskProfile, buffer[i].basePremium, buffer[i].newPremium,
            buffer[i].kilometers);
  }
  free(buffer);
  fclose(fp);
}

int verifyBinary(char target[]) {
  FILE *fp;
  int low    = 0;
  int mid    = 0;
  int high   = 0;
  int dbSize = 0;
  char str[MAX];

  fp = fopen(DB, "r"); /* Opens database */
  fscanf(fp, "(%d)\n", &dbSize); /* Gets index ammount in database */
  /* Below is a failsafe, in case the database only has one profile, in which
     case the binary search will fail. */
  if (dbSize <= 1) {
    fscanf(fp, "\n\"%[^\"]\"", str);
    if (strcmp(str, target) == 0) {
      return 1;
    }
    else return 0;
  }
  rewind(fp);
  /* START PARAMETERES */
  low = ftell(fp);  /* Sets to start of file */
  fseek(fp, 0, SEEK_END); /* Finds the end of file, SEEK_END is end of file */
  high = ftell(fp); /* Sets high to end of file */
  mid = high / DIVISOR; /* Sets middle of file */

  /* Recursive function binary search with the start parameteres */
  if (findBinary(fp, target, low, mid, high, high)) {
    return 1;
  }
  else {
    return 0;
  }
}

/* Gets the profile through binary search in Database */
userProfile getProfileBinary (char target[]) {
  userProfile profile;
  FILE *fp;
  int low    = 0;
  int mid    = 0;
  int high   = 0;
  int dbSize = 0;
  char str[MAX];

  fp = fopen(DB, "r");
  fscanf(fp, "(%d)\n", &dbSize); /* Gets index ammount in database */
  /* Below is a failsafe, in case the database only has one profile, in which
     case the binary search will fail. */
  if (dbSize <= 1) {
    fscanf(fp, "\n\"%[^\"]\"", str);
    if (strcmp(str, target) == 0) {
      strcpy(profile.email, target);
      fscanf(fp, " | %lf , %lf , %lf , %lf |\n", &profile.riskProfile, &profile.basePremium,
            &profile.newPremium, &profile.kilometers);
      return profile;
    }
    strcpy(profile.email, "\0");
    return profile;
  }
  rewind(fp);
  /* Sets the start parameteres */
  low = ftell(fp);
  fseek(fp, 0, SEEK_END);
  high = ftell(fp);
  mid = high / DIVISOR;
  /* Finds profile */
  if (findBinary(fp, target, low, mid, high, high)) {
    /* Found profile */
    strcpy(profile.email, target);
    fscanf(fp, " | %lf , %lf , %lf , %lf |\n", &profile.riskProfile, &profile.basePremium,
          &profile.newPremium, &profile.kilometers);
    return profile;
  }
  else {  /* No profile found */
    strcpy(profile.email, "\0");
    return profile;
  }
}

/* Finds the profile by using a binary search function, with start parameteres */
int findBinary(FILE* fp, char target[], int low, int mid, int high, int last) {
  int current = 0; /* Represents a bit in int-form */
  char str[MAX];
  /* Sets filepointer to point to middle value */
  fseek(fp, mid, SEEK_SET);
  /* Gets the bit of current filepointer position */
  current = ftell(fp);
  /* Moves filepointer to new line, cause could be in middle of line */
  fgets(str, MAX, fp);
  /* Gets Email at filepointer position */
  fscanf(fp, "\n\"%[^\"]\"", str);

  /* If we arrive on the same bit, the email does not exist */
  if (current == last) {
    return 0;
  }
  /* If we found the String we are looking for */
  if (strcmp(target, str) == 0) {
    return 1;
  }
  else if (strcmp(target, str) > 0) { /* Not found string yet. Also target is higher than current string(str) */
    /* Sets new search variables, elimnates bitrange below current bit */
    /* And sets new middle from which to start the search */
    low = current;
    mid = (low + high) / DIVISOR;
    findBinary(fp, target, low, mid, high, current); /* Recursive run with new start parameteres */
  }
  else if (strcmp(target, str) < 0){  /* Target is lower than current string(str) */
    /* Sets new search variables, elimnates bitrange higher than current bit */
    /* And sets new middle from which to start the search */
    high = current;
    mid  = (low + high) / DIVISOR;
    findBinary(fp, target, low, mid, high, current); /* Recursive run with new start parameteres */
  }
  else {
    return 0;
  }
}

/*  Comparison function for use with qsort. */
int compareProfilesAlphabetical(const void *ip1, const void *ip2) {
  int result;
  userProfile *ipi1 = (userProfile *)ip1;
  userProfile *ipi2 = (userProfile *)ip2;

  if (strcmp((*ipi1).email, (*ipi2).email) > 0) {
    result = 1;
  }
  else if (strcmp((*ipi1).email, (*ipi2).email) < 0) {
    result = -1;
  }
  return result;
}
