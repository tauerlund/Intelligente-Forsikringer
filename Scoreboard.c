#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<ctype.h>
#include"Functions.h"

/*Function that prompts the user for an amount of friends, they wish top compare
themselves up against, then able to insert their emails to do so*/
void showFriends(userProfile userinput)
{
    char message[MAX];
    char choice[MAX];
    char email[MAX];
    char inputNumber[MAX];
    int size      = 0;
    int i         = 0;
    userProfile *scoreBoard;

    while(!isStringNumber(inputNumber)) /*While input is not a digit. isdigit comes from ctype.h*/
    {
      cls();
      printf("Enter amount of friends you wish to compare yourself to:\n\nInput: ");
      scanf(" %s", inputNumber);
    }

    size = atoi(inputNumber); /*Converts the char to int for further usage */
    size++;
    scoreBoard = (userProfile *)malloc(sizeof(userProfile) * size);

    scoreBoard[0] = userinput;
    strcat(scoreBoard[0].email, " (YOU)");
    sprintf(message, "Please enter friend's email. (%d of %d)", i+1, size-1);

    for(i = 1; i < size; i++) {
      cls();
      printf("%s\n\nInput: ", message);
      scanf(" %s", email);
      scoreBoard[i] = getProfileBinary(email); /* Function called from DATABASE.C*/
      if (strcmp(scoreBoard[i].email, "\0") == 0) {
        sprintf(message, "Cannot find the email \"%s\" in database! Please enter a new email for your friend. (%d of %d)", email, i, size-1);
        i--;
      }
      else if (strcmp(scoreBoard[i].email, userinput.email) == 0) {
        sprintf(message, "Cannot compare with yourself! Please enter a new email for your friend. (%d of %d)", i, size-1);
        i--;
      }
      else if (i > 1 && !sameAsPrevious(email, scoreBoard, i)) {
        sprintf(message, "Already added that friend! Please enter a new email for your friend. (%d of %d)", i, size-1);
        i--;
      }
      else {
        sprintf(message, "Please enter friend's email. (%d of %d)", i+1, size-1);
      }
    }
    do {
      cls();
      printFriends(scoreBoard, size); /* Function called from this file */
      printf("Do you wish to go back? (0)\n\nInput: ");
      scanf(" %s", choice);
    } while (strcmp(choice, "0") != 0);
    free(scoreBoard);
}


int isStringNumber(char str[]) {
  int size = strlen(str);
  int i    = 0;

  for (i = 0; i < size; i++) {
    if (!isdigit(str[i])) {
      return 0;
    }
  }
  return 1;
}

int sameAsPrevious(char email[], userProfile *scoreBoard, int index) {
  int i = 0;

  for (i = index-1; i > 0; i--) {
    if (strcmp(email, scoreBoard[i].email) == 0) {
      return 0;
    }
  }
  return 1;
}

/*This function prints all friends based on the input of "showFriends" */
void printFriends(userProfile *input, int size){
    int i = 0;

    qsort(input, size, sizeof(userProfile), compareRisk);
    for (i = 0; i < size; i++) {
      printf("%d of %d\n", i + 1, size);
      if (input[i].riskProfile != 0) {
        printProfile(input[i], 1, 1, 0, 0, 0);
        printf("\n");
      }
    }
}

/*Compare function that sorts after lowest number of "riskProfile"*/
int compareRisk (const void * a, const void * b){
    userProfile *userProfileA = (userProfile *)a;
    userProfile *userProfileB = (userProfile *)b;
    return -( userProfileB->riskProfile - userProfileA->riskProfile);
}
