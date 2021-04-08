#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include"functions.h"
#define POLYGONS "polygons.txt"
#define DEFAULT_SPD_LIMIT 80

/* Finds the area and municipality of the region the position is in */
polygon calcPolygon(double pointX, double pointY, polygon *region,
                    polygon **municipalities, polygon **areas,
                    polygon lastKnownPoly, int munSize, int areaSize) {
  polygon currentPolygon; /* Polygon struct that gets updated and returned */
  polygon r;
  polygon *a;
  polygon *m;
  int found = 0;
  int i     = 0;
  /* Assigns the current generated region, municipalities and areas allocated in the memory */
  r = *region;
  a = *areas;
  m = *municipalities;

  /* Checks if still in last known polygon */
  if (pointInPolygon(pointX, pointY, lastKnownPoly)) {
    currentPolygon = lastKnownPoly;
    if(currentPolygon.level == 3) {
      found = 1;
    }
  }
  if (!found) { /* If not in last known polygon */
    /* Finding area position is in */
    for (i = 0; i < areaSize; i++) {
      if (pointInPolygon(pointX, pointY, a[i])) {
        currentPolygon = a[i];
        i = areaSize;
        found = 1;
      }
    }
  }
  /* Area not found, therefore finding municipality the position is in */
  if (!found) {
    for (i = 0; i < munSize; i++) {
      if(pointInPolygon(pointX, pointY, m[i])){
        currentPolygon = m[i];
        i = munSize;
        found = 1;
      }
    }
  }
  /* municipality not found, therefore ask if region is the same? */
  if (!found) {
    if (pointInPolygon(pointX, pointY, r)) {
      currentPolygon = r;
    }
    else { /* Region not the same, position has changed region */
      /* Region has changed, therefore we check for all and generates new polygons for that region */
      free(municipalities);
      free(areas);
      generatePolygons(pointX, pointY, &r, &m, &a, &currentPolygon, &munSize, &areaSize);
      *region = r;
      **municipalities = *m;
      **areas = *a;
    }
  }
  return currentPolygon;
}

/* Gets regions, municipalities, areas polygons */
void generatePolygons(double pointX, double pointY, polygon *regions,
                      polygon **munis, polygon **areas, polygon *current,
                      int *m_size, int *a_size) {

  /* VARIABLES */
  FILE* fp;
  int regionMunis = 0;
  int regionAreas = 0;
  /* STRUCT */
  polygon tempRegion;
  polygon *tempMunicipalities;
  polygon *tempAreas;
  /* Opens the Database with the polygons */
  fp = fopen(POLYGONS, "r");

  /* Gets Region */
  tempRegion = generateRegion(pointX, pointY, fp,
                             &regionMunis, current);
  *m_size    = regionMunis; /* Gets amount of municipalities in region found */

  /* Gets municipalities */
  tempMunicipalities = generateMunicipalities(pointX, pointY, fp,
                                              regionMunis, &regionAreas,
                                              tempRegion.name, current);
  *a_size = regionAreas; /* Gets amount of areas in region found */

  /* Gets Areas */
  tempAreas = generateAreas(pointX, pointY, fp,
                            regionAreas, tempRegion.name,
                            current);

  /* Assigns via pointers to struct */
  *regions = tempRegion;
  *munis   = tempMunicipalities;
  *areas   = tempAreas;

  fclose(fp);
}

/* Generates the region the point is in */
polygon generateRegion(double pointX, double pointY, FILE* fp,
  int *regionMunis, polygon *current) {

  int i                = 0;
  int size             = 0;
  int foundRegion      = 0;
  int totalRegionMunis = 0;
  int coords           = 0;
  char currentChar;
  char regionName[MAX];
  polygon tempPoly, region;

  size = getSize(fp);

  /* Finds regions in database and finds what Region the car is in */
  while (!foundRegion && i < size) {
    /* Goes through the file 1 char per loop */
    currentChar = fgetc(fp);

    /* Gets how many municipalities in the region, in database is in [] */
    if (currentChar == '[') {
      fscanf(fp, "%d]", &totalRegionMunis);
    }
    /* Gets region name in "" by database  */
    if (currentChar == '\"') {
      fscanf(fp, "%[^\"]\"\n", regionName);
    }
    else if (currentChar == '{') {
      /* Gets how many coordinates it has */
      fscanf(fp, " %d\n", &coords);

      /* Gets and sets the coordinates, name, how many corners for the regions polygon */
      tempPoly                = readCoordinates(fp, coords);
      tempPoly.numberOfPoints = coords;
      tempPoly.speedLimit     = DEFAULT_SPD_LIMIT;
      strcpy(tempPoly.name, regionName);

      /* Checks if the car is in the regions polygon */
      if (pointInPolygon(pointX, pointY, tempPoly)) {
        /* Is region, therefore assigns the polygon to region and goes out of while loop */
        region       = tempPoly;
        region.level = 1;
        *current     = region;
        foundRegion  = 1;
      }
    }
    else if (currentChar == '}') {  /* Has gone through a set of a region therefore increment */
      i++;
    }
  } /* END OF LOOP */

  *regionMunis = totalRegionMunis; /* Returns amount of municipalities in region */
  return region;  /* Return region polygon */
}

/* Generates the municipalities in the region the point is in */
polygon* generateMunicipalities(double pointX, double pointY, FILE* fp,
                                int regionMunis, int *regionAreas, char name[],
                                polygon *current) {
  int i         = 0;
  int j         = 0;
  int size      = 0;
  int coords    = 0;
  int counter   = 0;
  int foundMuni = 0;
  char currentChar;
  char regionName[MAX], muniName[MAX];
  polygon *municipalities;

  /* Gets the size of the database */
  size = getSize(fp);

  /* Allocates memory for municipalities, ammount is known in find region function */
  municipalities = (polygon*)malloc(sizeof(polygon) * regionMunis);
  /* Resets region name */
  strcpy(regionName, "\0");

  /* Finds municipality loop */
  while (!foundMuni && i < size) {
    currentChar = fgetc(fp);

    /* In database ouside from a municipality is the ammount of Areas in [ammount] */
    if (currentChar == '[') {
      fscanf(fp, "%d]", &counter); /* Reads the [ammount] */
      if ((strcmp(regionName, name) == 0)) {
        *regionAreas += counter; /* Adds up the ammount of areas in the found region */
      }
    }

    /* Finds the found region's municipalities */
    /* Finds "" for region name, and checks if region name has not been found and set */
    if ((currentChar == '\"') && (strcmp(regionName, "\0") == 0)) {
      fscanf(fp, "%[^\"]\"\n", regionName); /* SETS REGION NAME */
    }

    /* Checks to see if we are in the right region */
    if (strcmp(regionName, name) == 0) {
      /* Gets municipality Name */
      if (currentChar == '\"') {
        fscanf(fp, "%[^\"]\"\n", muniName);
      /* Checks when the sets{} of coordinates begin in database */
      }
      else if (currentChar == '{') {
         /* Gets ammount of coordinates */
         fscanf(fp, " %d\n", &coords);

         /* Gets and Sets municipalities coordinates and name */
         municipalities[j]                = readCoordinates(fp, coords);
         municipalities[j].numberOfPoints = coords;
         municipalities[j].speedLimit     = DEFAULT_SPD_LIMIT;
         municipalities[j].level          = 2;
         strcpy(municipalities[j].name, muniName);

         if (pointInPolygon(pointX, pointY, municipalities[j])) {
           *current = municipalities[j];
         }
         j++;

         if (j >= regionMunis) {  /* If gone through all municipalities, activate sentinel */
           foundMuni = 1; /* Sentinel */
         }
      }
    }
    if (currentChar == '}') { /* Gone through a set{} in database */
      strcpy(regionName, "\0");  /* Resets region name */
      i++;
    }
  }/* END OF LOOP, Found municipalities */
  return municipalities;
}

/* Generates the Areas in the region the point is in */
polygon* generateAreas(double pointX, double pointY, FILE* fp,
                      int regionAreas, char name[], polygon *current) {

  /* Finds areas in all found municipalities */
  int i         = 0;
  int j         = 0;
  int size      = 0;
  int spdLimit  = 0;
  int foundArea = 0;
  int coords    = 0;
  polygon *areas;
  char currentChar;
  char regionName[MAX];
  char muniName[MAX];
  char areaName[MAX];

  areas = (polygon*)malloc(sizeof(polygon) * regionAreas);
  size = getSize(fp);
  strcpy(regionName, "\0");

  /* Finds the areas for the region */
  while (!foundArea && i < size) {
    /* Goes through DB 2 charateres at a time */
    currentChar = fgetc(fp);

    /* Speed limit in area */
    if (currentChar == '[') {
      fscanf(fp, "%d]", &spdLimit);
    }
    /* Checks if region name has not been found */
    if ((currentChar == '\"') && (strcmp(regionName, "\0") == 0)) {
      fscanf(fp, "%[^\"]\"\n", regionName);
    }
    /* If the region found is the correct set region */
    if (strcmp(regionName, name) == 0) {
      /* Reads municipality and area name */
      if (currentChar == '#') { /* # indicates start of municipality and area name in DB */
        fscanf(fp, "%[^#]# \"%[^\"]\"", muniName, areaName);
      }
      else if (currentChar == '{') {  /* End of Set{} */

         /* Sets the area to the struct */
         fscanf(fp, " %d\n", &coords);
         areas[j]                = readCoordinates(fp, coords); /* Sets coordinates of area */
         areas[j].numberOfPoints = coords; /* Ammount of corners/points in polygon */
         areas[j].level          = 3;
         areas[j].speedLimit     = spdLimit; /* Sets speed limit */
         strcpy(areas[j].name, areaName); /* Sets area name */

         if (pointInPolygon(pointX, pointY, areas[j])) {
           *current = areas[j];
         }
         j++;

         if(j >= regionAreas) {
           foundArea = 1;
         }
      }
    }
    if (currentChar == '}') { /* Gone through a set{} in database */
      strcpy(regionName, "\0");
      i++;
    }
  } /* END OF LOOP, Found all areas for the municipalities */
  return areas;
}

/* Reads coordinates in DB and returns it */
/* OBS! Filepointer needs to be correctly set before use */
polygon readCoordinates(FILE* fp, int numberOfCoords) {
  int i = 0;
  polygon tempPoly;
  for (i = 0; i < numberOfCoords; i++) {
    fscanf(fp, "%lf , %lf\n", &tempPoly.x[i], &tempPoly.y[i]);
  }
  return tempPoly;
}

/* Get a size of the database */
/* OBS! Filepointer needs to be correctly set before use */
int getSize(FILE *fp) {
  int size = 0;
  char tempString[MAX];
  while ((size == 0) && !(feof(fp))) {
    fgets(tempString, CHAR_LENGTH, fp);
    if (strcmp(tempString, "(") == 0) {
      fscanf(fp, "%d)", &size);
    }
  }
  return size;
}

/* Finds out if within a polygon from two degree-coordinates */
int pointInPolygon(double pointX, double pointY, polygon poly) {

  int currentCorner = 0;
  int nextCorner    = 0;
  int isInPoly      = 0;
  double lineSlope  = 0;
  double linePoint  = 0;

  for (currentCorner = 0; currentCorner < poly.numberOfPoints; currentCorner++){
    /* Moves the vertex checkers */
    if(currentCorner == poly.numberOfPoints - 1){
      nextCorner = 0;
    }
    else {
      /* If we reached the final corner, the second vertex checker goes to start point */
      nextCorner = currentCorner + 1;
    }
    /* Checks if pointY is below current corner of the polygon */
    /* X_OR is "^" */
    if ((pointY < poly.y[currentCorner]) ^ (pointY < poly.y[nextCorner])) {
      /* Calculates the slope between two corners */
      lineSlope = (poly.x[nextCorner] - poly.x[currentCorner]) / (poly.y[nextCorner] - poly.y[currentCorner]);
      /* Calculates the x-coordinate for the point between the two corners,
      that has the same y-coordinate as the point we are looking for */
      linePoint = lineSlope * (pointY - poly.y[currentCorner]) + poly.x[currentCorner];

      /* If the point x is outside polygon */
      if(pointX < linePoint){
        /* Is left to the line */
        isInPoly = !isInPoly;
      }
    }
  }
  return isInPoly;
}
