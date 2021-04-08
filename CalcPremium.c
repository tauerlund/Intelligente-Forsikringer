#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include"functions.h"

double calcPremium(userProfile profile){
  /*  premiumData premiumData;*/
  premiumSettings premiumSettings;
  double deltaRiskLimit      = 0, discountPenaltyMultiplier = 0;
  double riskPointsOverLimit = 0, adjustedPremium           = 0;
  double discount            = 0; /* Discount % in decimal number */
  double discountPenalty     = 0; /* How much discount should be subtracted */

  premiumSettings = readPremiumSettingsIni(); /* Read premium settings from file */
  /* Get difference between lower and upper limit for risk points.  */
  deltaRiskLimit = premiumSettings.upperRiskLimit - premiumSettings.lowerRiskLimit;
  /* Get multiplier to calculate the proportional discount if between lower and upper limit */
  discountPenaltyMultiplier = premiumSettings.maxDiscount / deltaRiskLimit;
  /* If total risk points is less than lower limit, then give no penalty */
  if (profile.riskProfile <= premiumSettings.lowerRiskLimit) {
    discountPenalty = 0;
  } /* If total risk is between lower and higher limit, then calculate discount */
  else if (premiumSettings.lowerRiskLimit < profile.riskProfile && profile.riskProfile < premiumSettings.upperRiskLimit) {
    /* Get how many risk points exceed lower limit and multiply with penalty multiplier  */
    riskPointsOverLimit = profile.riskProfile - premiumSettings.lowerRiskLimit;
    discountPenalty = riskPointsOverLimit * discountPenaltyMultiplier;
  } /* Else risk points exeed upper limit. Give full penalty */
  else {
    discountPenalty = premiumSettings.maxDiscount;
  }

  /* Get discount */
  discount = premiumSettings.maxDiscount - discountPenalty;
  /* Calculate premium with discount */
  adjustedPremium = profile.basePremium - (profile.basePremium * discount); 

  return adjustedPremium;
}

/* Read premium settings from ini file */
premiumSettings readPremiumSettingsIni() {
  FILE* fp;
  premiumSettings premiumSettings;
  fp = fopen(PREMIUM_SETTINGS, "r");

  if(fp == NULL){ /* Check if file could be opened */
    perror("\nError while opening file with premiumSettings");
    exit(EXIT_FAILURE);
  }

  fscanf(fp, "%*[^0-9] %lf\n", &premiumSettings.maxDiscount);
  fscanf(fp, "%*[^0-9] %lf\n", &premiumSettings.lowerRiskLimit);
  fscanf(fp, "%*[^0-9] %lf", &premiumSettings.upperRiskLimit);
  fclose(fp);
  return premiumSettings;
}
