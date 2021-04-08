#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include"functions.h"

int main(void) {
  char choice[MAX];
  int state = 0;
  dbCheckAndCreate();
  do {
    cls();
    printf("%s v%.1f\n", TITLE, VERSION);
    printf("1) User\n2) Admin\n0) Exit program\n\nInput: ");
    scanf("%s", choice);

    if (strcmp(choice, "1") == 0) {
      userInterface();
    }
    else if (strcmp(choice, "2") == 0) {
      checkifAdmin();
    }
    else if (strcmp(choice, "0") == 0) {
      cls();
      state = 1;
      printf("Exiting program...\n");
    }
  } while (state == 0);
  return EXIT_SUCCESS;
}

void userInterface () { /* USER INTERFACE */
  char choice[MAX];
  int state = 0;
  do {
    cls();
    printf("%s v%.1f\n", TITLE, VERSION);
    printf("1) Sign in\n");
    printf("0) Back\n\n");
      /*prompt*/
    printf("Input: ");
    scanf("%s", choice);

    /*conditions*/
    if (strcmp(choice, "1") == 0) {
      loadProfile();
    }
    else if (strcmp(choice, "0") == 0) {
      state = 1;
    }
  } while (state == 0);
}

void checkifAdmin () { /* ADMIN PASSWORD REQUEST */
  char adminPass[MAX];
  char choice[MAX];

  cls();
  printf("ENTER PASSWORD: ");
  scanf("%s", adminPass);

  if (strcmp(adminPass, PASSWORD) == 0) {
    adminInterface();
  }
  else {
    do {
      cls();
      printf("ACCESS DENIED\nDo you want to go back to main menu? (0)\n\nInput: ");
      scanf("%s", choice);
    } while (strcmp(choice, "0") != 0);
  }
}

void adminInterface () { /* ADMIN INTERFACE */
  char choice[MAX];
  char inputEmail[MAX] = "\0";
  char emailMessage[MAX] = "Enter email of profile to update.";
  int state = 0;
  userProfile profile;
  do {
    strcpy(inputEmail, "\0");
    strcpy(emailMessage, "Enter email of profile to update.");
    cls();
    printf("%s v%.1f\n", TITLE,  VERSION);
    printf("1) Create user\n2) Update user\n3) Show profiles in database \n0) Back\n\nInput: ");
    scanf("%s", choice);
    if (strcmp(choice, "1") == 0) {
      createProfile();
    }
    else if (strcmp(choice, "2") == 0) {
      while (strcmp(inputEmail, "\0") == 0) {
        cls();
        printf("%s\n\nInput: ", emailMessage);
        scanf(" %s", inputEmail);
        profile = getProfileBinary(inputEmail);
        if (strcmp(profile.email, "\0") != 0) {
            updateProfile(profile);
        }
        else {
          sprintf(emailMessage, "Could not find email \"%s\"! Please enter a new email.", inputEmail);
          strcpy(inputEmail, "\0");
        }
      }
    }
    else if (strcmp(choice, "3") == 0) {
      FILE *fp = fopen(DB, "r");
      if (fp == NULL) {
        printf("Database doesn't exist. Exiting program.\n");
        exit(EXIT_FAILURE);
      }
      else {
        dataIndex(fp);
      }
    }
    else if (strcmp(choice, "0") == 0) {
      state = 1;
    }
  } while (state == 0);
}
