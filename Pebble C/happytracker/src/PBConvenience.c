#include <pebble.h>
#include "PBConvenience.h"

//************************************************************************
//  PBConvenience.c
//  HAPPYTRACKER
//  Created by Si Te Feng on 2015-01-09.
//  Copyright (c) 2014 Pebble Technology. All rights reserved.
//************************************************************************
  
inline const char* moodStringFromIndex(int32_t num) {
  
  switch(num) {
    case 1:
      return "Happy";
      break;
    case 2:
      return "Neutral";
      break;
    case 3:
      return "Sad";
      break;
    default:
      return "Unknown";
      break;
  }
}                                       

const char* timeStringFromTimestamp(int32_t time) {
  
  const time_t currTime = (time_t)time;
  const struct tm* tStruct = localtime(&currTime);
  char* timeString = malloc( sizeof(char)*19 );
  strftime(timeString, 30, "%x, %X", tStruct);
  return timeString;
}
                                          
                                          

