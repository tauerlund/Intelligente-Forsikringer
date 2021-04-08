#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include"Functions.h"

tripRisk calcRisk(point *points, tripdata tripData) {
  riskSettings riskSettings;
  tripRisk tripRisk;
  /* Read risk settings from file */
  riskSettings   = readRiskIni();
  /* Calc risk */
  tripRisk.speedRisk      = calcSpeedRisk(points, tripData, riskSettings);
  tripRisk.timeOfDayRisk  = calctimeOfDayRisk(tripData, riskSettings);
  tripRisk.timeDrivenRisk = calcTimeDrivenRisk(tripData, riskSettings);
  tripRisk.tripKmRisk     = calcTripKmRisk(tripData, riskSettings);
  tripRisk.weatherRisk    = calcWeatherRisk(tripData, riskSettings);
  /* Get total risk */
  tripRisk.totalRisk = tripRisk.speedRisk.weighted + tripRisk.timeDrivenRisk.weighted
                     + tripRisk.tripKmRisk.weighted + tripRisk.weatherRisk.weighted
                     + tripRisk.timeOfDayRisk.weighted;
  return tripRisk;
}

speedRisk calcSpeedRisk(point *points, tripdata tripData, riskSettings riskSettings) {
  int i = 0, total = 0;
  double speedOver = 0, timeSpeeding = 0, speedOverAvg = 0, speedweight = 0;
  double timeDriven = 0, riskUnweighted = 0;
  speedRisk speedRisk;
  riskWeightData riskWeightData;
  /* Calc how much over speed limit and for how long */
  for (i = 0; i < tripData.noOfPoints; i++) {
    if (points[i].speed > MIN_SPEED_MARGIN) {
    /*Get total time actually driving (over min speed)*/
    timeDriven += points[i].time;
    }
    if (points[i].speed > points[i].zone.speedLimit) {
      speedOver += points[i].speed - points[i].zone.speedLimit;
      timeSpeeding += points[i].time;
      ++total;
    }
  }

  /*Calculate risk if driver has been speeding */
  if (speedOver != 0){
    speedOverAvg = calcSpeedOverAvg(speedOver, total);
    speedweight = calcSpeedWeight(timeDriven, timeSpeeding);
    riskUnweighted = speedOverAvg * speedweight;
    if (riskUnweighted > riskSettings.speedRiskLimit) {
      riskUnweighted = riskSettings.speedRiskLimit;
    }
    riskWeightData = calcWeightedRisk(riskSettings.speedRiskLimit, riskUnweighted, riskSettings.speedRiskWeight);
  }
  else{
    riskWeightData.factor       = 0;
    riskWeightData.base100      = 0;
    riskWeightData.weightedRisk = 0;
  }
  /* Assign data to struct before returning */
  speedRisk.speedingTime   = timeSpeeding;
  speedRisk.speedingAvgKmh = speedOverAvg;
  speedRisk.speedingRatio  = speedweight;
  speedRisk.unweighted     = riskUnweighted;
  speedRisk.factor         = riskWeightData.factor;
  speedRisk.base100        = riskWeightData.base100;
  speedRisk.weight         = riskSettings.speedRiskWeight * 100;
  speedRisk.weighted       = riskWeightData.weightedRisk;

  return speedRisk;
}

double calcSpeedOverAvg(double speedOver, int total) {
  double result;
  result = speedOver/total;
  return result;
}

double calcSpeedWeight(double timeDriven, double timeSpeeding) {
    double result;
    result = timeSpeeding/timeDriven;
    return result;
}

riskData calctimeOfDayRisk(tripdata tripData, riskSettings riskSettings) {
  riskData timeOfDayRisk;
  riskWeightData riskWeightData;

  riskWeightData = calcWeightedRisk(riskSettings.timeOfDayRiskLimit, tripData.isNight, riskSettings.timeOfDayRiskWeight);
  /* Assign data to struct before returning */
  timeOfDayRisk.unweighted = tripData.isNight;
  timeOfDayRisk.factor     = riskWeightData.factor;
  timeOfDayRisk.base100    = riskWeightData.base100;
  timeOfDayRisk.weight     = riskSettings.timeOfDayRiskWeight * 100;
  timeOfDayRisk.weighted   = riskWeightData.weightedRisk;

  return timeOfDayRisk;
}

riskData calcTimeDrivenRisk(tripdata tripData, riskSettings riskSettings) {
  double time = 0;
  riskData timeDrivenRisk;
  riskWeightData riskWeightData;

  time = totalTimeInHrs(tripData.totalTime);
  if (time > riskSettings.timeDrivenRiskLimit) {
    time = riskSettings.timeDrivenRiskLimit;
  }
  riskWeightData = calcWeightedRisk(riskSettings.timeDrivenRiskLimit, time, riskSettings.timeDrivenRiskWeight);
  /* Assign data to struct before returning */
  timeDrivenRisk.unweighted = time;
  timeDrivenRisk.factor     = riskWeightData.factor;
  timeDrivenRisk.base100    = riskWeightData.base100;
  timeDrivenRisk.weight     = riskSettings.timeDrivenRiskWeight * 100;
  timeDrivenRisk.weighted   = riskWeightData.weightedRisk;

  return timeDrivenRisk;
}

riskData calcTripKmRisk(tripdata tripData, riskSettings riskSettings) {

  double distance = 0;
  riskData tripKmRisk;
  riskWeightData riskWeightData;

  distance = tripData.totalKm;
  if (distance > riskSettings.tripKmRiskLimit) {
    distance = riskSettings.tripKmRiskLimit;
  }
  riskWeightData = calcWeightedRisk(riskSettings.tripKmRiskLimit, distance, riskSettings.tripKmRiskWeight);
  /* Assign data to struct before returning */
  tripKmRisk.unweighted  = distance;
  tripKmRisk.factor  = riskWeightData.factor;
  tripKmRisk.base100 = riskWeightData.base100;
  tripKmRisk.weight      = riskSettings.tripKmRiskWeight * 100;
  tripKmRisk.weighted    = riskWeightData.weightedRisk;

  return tripKmRisk;
}

riskData calcWeatherRisk(tripdata tripData, riskSettings riskSettings) {
  riskData weatherRisk;
  riskWeightData riskWeightData;

  riskWeightData = calcWeightedRisk(riskSettings.seasonRiskLimit, tripData.isBadweather, riskSettings.seasonRiskWeight);
  /* Assign data to struct before returning */
  weatherRisk.unweighted = tripData.isBadweather;
  weatherRisk.factor  = riskWeightData.factor;
  weatherRisk.base100 = riskWeightData.base100;
  weatherRisk.weight = riskSettings.seasonRiskWeight * 100;
  weatherRisk.weighted = riskWeightData.weightedRisk;

  return weatherRisk;
}

double weightRisk(userProfile profile, tripdata tripData, double tripRisk) {
  double oldRiskWeight = 0, tripRiskWeight = 0, newRisk = 0;
  double totalKilometers = profile.kilometers + tripData.totalKm;

  /* Calc risk weight for this trip and risk already in database (if any) */
  tripRiskWeight = tripData.totalKm / totalKilometers;
  oldRiskWeight = profile.kilometers / totalKilometers;

  newRisk = (profile.riskProfile * oldRiskWeight) + (tripRisk * tripRiskWeight);
  return newRisk;
}

/* Get risk settings to calculate risk */
riskSettings readRiskIni() {
  FILE *fp;
  riskSettings riskSettings;
  fp = fopen("risk factors.ini", "r");
  if(fp == NULL){ /* Check if file could be opened */
    perror("\nError while opening file with riskSettings");
    exit(EXIT_FAILURE);
  }

  fscanf(fp, "%*s%lf , %lf", &riskSettings.speedRiskLimit, &riskSettings.speedRiskWeight);
  fscanf(fp, "%*s%lf , %lf", &riskSettings.timeOfDayRiskLimit, &riskSettings.timeOfDayRiskWeight);
  fscanf(fp, "%*s%lf , %lf", &riskSettings.timeDrivenRiskLimit, &riskSettings.timeDrivenRiskWeight);
  fscanf(fp, "%*s%lf , %lf", &riskSettings.tripKmRiskLimit, &riskSettings.tripKmRiskWeight);
  fscanf(fp, "%*s%lf , %lf", &riskSettings.seasonRiskLimit, &riskSettings.seasonRiskWeight);

  fclose(fp);
  return riskSettings;
}

/* Get weighted risk */
riskWeightData calcWeightedRisk(double riskLimit, double riskUnweighted, double riskWeight) {
  riskWeightData riskWeightData;

  double factor = 0, base100 = 0, weightedRisk = 0;
  factor = getFactor(riskLimit);
  base100 = calcBase100Risk(factor, riskUnweighted);
  weightedRisk = weightBase100Risk(base100, riskWeight);

  riskWeightData.factor = factor;
  riskWeightData.base100 = base100;
  riskWeightData.weightedRisk = weightedRisk;

  return riskWeightData;
}

double getFactor(double riskLimit) {
    double factor = 0;
    factor = BASE100/riskLimit;
    return factor;
}

double calcBase100Risk(double factor, double riskUnweighted) {
    double base100Risk = 0;
    base100Risk = factor * riskUnweighted;
    return base100Risk;
}

double weightBase100Risk(double base100Risk, double riskWeight) {
    double weightedBase100 = 0;
    weightedBase100 = base100Risk * riskWeight;
    return weightedBase100;
}

double totalTimeInHrs(double totalTime) {
    double hours;
    hours = totalTime / SEC_IN_HOUR;
    return hours;
}

/* Print all available data in struct tripRisk */
void printTripRisk(tripRisk tripRisk){

  printf("\nRISK DATA\n");
  /*  printf("\n%-16s %s\n", "", "Weighted 0-100");*/
  printf("\n%-20s\n", "SPEED RISK:");
  printf("%-2s%-25s %6.2lf km/h\n","", "Avg. speed over limit", tripRisk.speedRisk.speedingAvgKmh);
  printf("%-2s%-25s %6.2lf sec.\n","", "Time speeding", tripRisk.speedRisk.speedingTime);
  printf("%-2s%-25s %6.2lf\n","", "Speeding/time ratio", tripRisk.speedRisk.speedingRatio);
  printf("%-2s%-25s %6.2lf\n","", "Unweighted", tripRisk.speedRisk.unweighted);
  printf("%-2s%-25s %6.2lf\n","", "Factor", tripRisk.speedRisk.factor);
  printf("%-2s%-25s %6.2lf\n","", "Base100", tripRisk.speedRisk.base100);
  printf("%-2s%-25s %6.2lf %%\n","", "Weight", tripRisk.speedRisk.weight);
  printf("%-2s%-37s %6.2lf\n","", "Weighted 0-100", tripRisk.speedRisk.weighted);

  printf("\n%-20s\n", "TIME DRIVEN RISK:");
  printf("%-2s%-25s %6.2lf hours\n", "", "Unweighted", tripRisk.timeDrivenRisk.unweighted);
  printf("%-2s%-25s %6.2lf\n","", "Factor", tripRisk.timeDrivenRisk.factor);
  printf("%-2s%-25s %6.2lf\n","", "Base100", tripRisk.timeDrivenRisk.base100);
  printf("%-2s%-25s %6.2lf %%\n", "","Weight", tripRisk.timeDrivenRisk.weight);
  printf("%-2s%-37s %6.2lf\n", "", "Weighted 0-100", tripRisk.timeDrivenRisk.weighted);

  printf("\n%-20s\n", "TRIP KM RISK:");
  printf("%-2s%-25s %6.2lf km.\n", "", "Unweighted", tripRisk.tripKmRisk.unweighted);
  printf("%-2s%-25s %6.2lf\n","", "Factor", tripRisk.tripKmRisk.factor);
  printf("%-2s%-25s %6.2lf\n","", "Base100", tripRisk.tripKmRisk.base100);
  printf("%-2s%-25s %6.2lf %%\n", "", "Weight", tripRisk.tripKmRisk.weight);
  printf("%-2s%-37s %6.2lf\n", "", "Weighted 0-100", tripRisk.tripKmRisk.weighted);

  printf("\n%-20s\n", "WEATHER RISK:");
  printf("%-2s%-25s %6.2lf\n", "", "Unweighted", tripRisk.weatherRisk.unweighted);
  printf("%-2s%-25s %6.2lf\n","", "Factor", tripRisk.weatherRisk.factor);
  printf("%-2s%-25s %6.2lf\n","", "Base100", tripRisk.weatherRisk.base100);
  printf("%-2s%-25s %6.2lf %%\n", "", "Weight", tripRisk.weatherRisk.weight);
  printf("%-2s%-37s %6.2lf\n", "", "Weighted 0-100", tripRisk.weatherRisk.weighted);

  printf("\n%-20s\n", "TIME OF DAY RISK:");
  printf("%-2s%-25s %6.2lf\n", "", "Unweighted", tripRisk.timeOfDayRisk.unweighted);
  printf("%-2s%-25s %6.2lf\n","", "Factor", tripRisk.timeOfDayRisk.factor);
  printf("%-2s%-25s %6.2lf\n","", "Base100", tripRisk.timeOfDayRisk.base100);
  printf("%-2s%-25s %6.2lf %%\n", "", "Weight", tripRisk.timeOfDayRisk.weight);
  printf("%-2s%-37s %6.2lf\n", "", "Weighted 0-100", tripRisk.timeOfDayRisk.weighted);

  printf("\n%-39s %6.2lf", "TOTAL RISK", tripRisk.totalRisk);
  printf("\n_________________________________________\n");
}