#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include"Functions.h"

/* Load a single profile */
void loadProfile() {
  userProfile profile;
  int state = 0;
  char inputEmail[MAX];
  char choice[MAX];

  /* Open database */
  FILE *fp = fopen(DB, "r");
  if(fp == NULL){
    printf("Database doesn't exist. Exiting program.\n");
    exit(EXIT_FAILURE);
  }

  cls();
  /* Prompt for email for desired profile */
  printf("Please enter your email:\n\nInput: ");
  scanf("%s", inputEmail);

  /* Search for profile by binary search */
  profile = getProfileBinary(inputEmail);
  while (strcmp(profile.email,"\0") == 0 && state == 0) {
    cls();
    printf("Could not find profile, please try again with a new email or 0) to go back.\n\nInput: ");
    scanf("%s", inputEmail);
    if (strcmp("0",inputEmail) == 0) {
      state = 1;
    }
    profile = getProfileBinary(inputEmail);
  }
  fclose(fp);

  if (state == 0) {
    while ((strcmp(choice, "1") != 0) && (strcmp(choice, "0") != 0))
    {
      cls();
      printProfile(profile, 1, 1, 1, 1, 0);
      printf("\nWould you like compare yourself with your friends (1) or go back (0)?\n\nInput: ");
      scanf("%s", choice);
    }

    if (strcmp(choice, "1") == 0)
    {
      showFriends(profile);
    }
  }
}

/*Prints profile, with parameters to choose what parts of the array is wished to
be printed.*/
void printProfile(userProfile profile, int email, int riskProfile, int basePremium, int newPremium, int kilometers){
  if (email) {
    printf("%s%s %s\n\n", "Email", ":", profile.email);
  }
  if (riskProfile) {
    printf("%-12s%2s %10.2lf\n","Risk profile", ":", profile.riskProfile);
  }
  if (basePremium) {
    printf("%-12s%2s %10.2lf\n","Base premium", ":", profile.basePremium);
  }
  if (newPremium) {
    printf("%-12s%2s %10.2lf\n", "Adj. premium", ":", profile.newPremium);
  }
  if (kilometers) {
	 printf("%-12s%2s %10.2lf\n", "Kilometers", ":", profile.kilometers);
  }
}

/* Simple for loop, that prints profiles */
void showProfiles(userProfile *scoreBoard, int count)
{
    int i;
    printf("%s\t%-20s %5s\n", "RANK", "EMAIL", "POINTS");
    for (i = 0; i < count; i++)
    {
        printf("%d\t%-20s %5.0f\n", i+1, scoreBoard[i].email, scoreBoard[i].riskProfile);
    }
}

/*Looks through a file and scans them in with the help of "scanProfile".*/
userProfile* getProfiles() {
  FILE *fp;
  userProfile *profiles;
  int dbSize = 0, i = 0;

  fp = fopen(DB, "r");
  fscanf(fp, "(%d)\n", &dbSize);  /* Gets index ammount in DB */
  profiles = (userProfile*)malloc(sizeof(userProfile) * dbSize);
  for (i = 0; i < dbSize; i++) {
    fscanf(fp, "\"%[^\"]\" | %lf , %lf , %lf , %lf |\n", profiles[i].email,
          &profiles[i].riskProfile, &profiles[i].basePremium, &profiles[i].newPremium,
          &profiles[i].kilometers);
  }
  return profiles;
  fclose(fp);
}



userProfile scanProfile(char *string){
  userProfile a;
    sscanf(string, "\"%[^\"]\" | %lf , %lf , %lf , %lf |\n", a.email, &a.riskProfile, &a.basePremium, &a.newPremium, &a.kilometers);
    return a;
}
