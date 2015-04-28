#pragma once

#define PBLOG(fmt, args...)  APP_LOG(APP_LOG_LEVEL_DEBUG,fmt, ## args)  

#define kSteelWidth      144
#define kSteelHeight     168
#define kSteelMainHeight 152 //excluding the status bar
  
#define animated true

void printVar( void* var );  
  
const char* timeStringFromTimestamp(int32_t time);
const char* moodStringFromIndex(int32_t num);




