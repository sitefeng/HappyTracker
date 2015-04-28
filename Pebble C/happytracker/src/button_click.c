#include "pebble.h"
#include "gbitmap_tools.h"
#include "PBConvenience.h"
  
//***********************************************************************
// HappyTracker
//  
// After the app has been opened for the first time, happiness monitoring starts, 
// and asks the user whether they're happy at random time intervals.
// User's responses will be stored in history using Pebble C Persistance storage API,
// as well as sent to the iOS companion app with Pebble C Data Logging API.
//
// - When the app resumes from the background, Short click up if you are happy, 
//   middle if you are feeling okay, and down if you are sad.
//
// - Long Pressing the buttons will have additional functions: up to disable happiness monitoring,
//   long press middle to view history, and down to clear the history
//
//************************************************************************
//  button_click.c
//  HAPPYTRACKER
//  Created by Si Te Feng on 2015-01-09.
//  Copyright (c) 2014 Pebble Technology. All rights reserved.
//************************************************************************
  
#define kWakeupIdKey        21
#define kWakeupReasonNormal 0
  
#define kMinimumWaitTime        5
#define kMaximumRandomTimeRange 10 //in seconds
  
//Variables
static DataLoggingSessionRef logSession;
static AppTimer* mainTimer;

static bool appEnabled;

static Window* window;
static ActionBarLayer* actionBar;
static TextLayer* mainLabel;
static TextLayer* subLabel;

static GBitmap* skylineImg;
static GBitmap* lineImg;
static GBitmap* checkImg;
static GBitmap* equalImg;
static GBitmap* crossImg;

static Window* graphWindow;
static SimpleMenuSection section;
static SimpleMenuLayer* historySimpleMenuLayer;

static void quit_app();
static void graph_window_click_config(void* context);

////////////////////////////////////////
//Wakeup Handler
static void wakeup_handler(WakeupId id, int32_t reason) {
  
  text_layer_set_text(mainLabel, "Are you happy?");
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Question was displayed");
  vibes_short_pulse();
  
  persist_delete(kWakeupIdKey);
}


////////////////////////////////////////
//Button Click Handlers
static void button_clicked(ClickRecognizerRef rec, void* context) {
  
  uint32_t userChoice;
  ButtonId buttonId = click_recognizer_get_button_id(rec);
  
  //PBLOG( "buttonId: %i" , buttonId);
  
  if(buttonId == BUTTON_ID_UP) {
    text_layer_set_text(mainLabel, "Happy Logged");
    userChoice = 1;
  } else if(buttonId == BUTTON_ID_SELECT) {
    text_layer_set_text(mainLabel, "Neutral Logged");
    userChoice = 2;
  } else if(buttonId == BUTTON_ID_DOWN) {
    text_layer_set_text(mainLabel, "Sad Logged");
    userChoice = 3;
  }
  
  //Log the Data 
  logSession = data_logging_create(0x0000, DATA_LOGGING_UINT, 4, true);
  data_logging_log(logSession, &userChoice, 1);
  
  //Store the data locally
  const int32_t currKeyNum = persist_read_int(0);
  persist_write_int(currKeyNum+1, (int)time(NULL) );
  persist_write_int(currKeyNum+2, userChoice);
  persist_write_int(0, currKeyNum+2);
  
  //Quit the app
  mainTimer = app_timer_register(2, quit_app,NULL);
}


static void back_button_pressed(ClickRecognizerRef rec, void* context) {
  window_stack_pop(animated);
}


void erase_data(ClickRecognizerRef rec, void* context) {
  
  vibes_short_pulse();
  int32_t numItems = persist_read_int(0) / 2;
  for(int i=0; i<numItems; i++) {
    persist_delete(i); 
  }
  text_layer_set_text(subLabel, "History Cleared.");
}


void app_enabling_toggle(ClickRecognizerRef rec, void* context) {
 
  vibes_short_pulse();
  
  if(appEnabled) {
    appEnabled = false;
    text_layer_set_text(subLabel, "App Disabled.");
  } else {
    appEnabled = true;
    text_layer_set_text(subLabel, "App Enabled.");
  }
}


//Secondary Screen
static void enter_graph(ClickRecognizerRef rec, void* context) {
  
  graphWindow = window_create();
  window_set_click_config_provider(graphWindow, graph_window_click_config);
  
  int32_t numItems = persist_read_int(0) / 2;
  SimpleMenuItem* items = malloc(sizeof(SimpleMenuItem)*numItems);
  if (!items) {
    text_layer_set_text(subLabel, "No History Available.");
    return;
  }
  
  for(int i=0; i<numItems; i++)
  {
    int32_t timestamp = persist_read_int(i*2+1);
    int32_t buttonId = persist_read_int(i*2+2);
    const char* str = moodStringFromIndex(buttonId);
    const char* timeString = timeStringFromTimestamp(timestamp);
    
    items[i] = (SimpleMenuItem){ .callback = NULL, 
                                 .icon = NULL,
                                 .title = str,
                                 .subtitle = timeString };
  }

  section = (SimpleMenuSection){ .items = items,
                                 .num_items = numItems,
                                 .title = "Happiness History" };
  
  Layer* graphWindowLayer = window_get_root_layer(graphWindow);
  
  GRect historyLayerRect = GRect(0, 0, layer_get_bounds(graphWindowLayer).size.w, layer_get_bounds(graphWindowLayer).size.h);
  historySimpleMenuLayer = simple_menu_layer_create(historyLayerRect, graphWindow, &section, 1, NULL);
  Layer* historyLayer = simple_menu_layer_get_layer(historySimpleMenuLayer);
  layer_add_child( graphWindowLayer, historyLayer);

  window_stack_push(graphWindow, animated);
}
                                          
                                          
////////////////////////////////////////////////////////////////////////////////////////////////////
//Click Conguration Providers

static void bar_click_config(void* context) {
  window_single_click_subscribe(BUTTON_ID_UP, button_clicked);
  window_single_click_subscribe(BUTTON_ID_SELECT, button_clicked);
  window_single_click_subscribe(BUTTON_ID_DOWN, button_clicked);
  
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, enter_graph, NULL);
  window_long_click_subscribe(BUTTON_ID_DOWN, 500, erase_data, NULL);
  window_long_click_subscribe(BUTTON_ID_UP, 500, app_enabling_toggle, NULL);
}


static void main_click_config(void* context) {
}

  
static void graph_window_click_config(void* context) {
  window_single_click_subscribe(BUTTON_ID_BACK, back_button_pressed);
}


////////////////////////////////////////////////
//Application Lifecycle and Window Managment

static void window_did_load(Window* window) {
  
  //Display Msg to the screen
  Layer* windowLayer = window_get_root_layer(window);
  
  mainLabel = text_layer_create(GRect(10, 40, 120, 20));  
  text_layer_set_text(mainLabel, "Loading");
  
  text_layer_set_text_alignment(mainLabel, GTextAlignmentLeft);
  layer_add_child(windowLayer, text_layer_get_layer(mainLabel));  
  
  if(launch_reason()==APP_LAUNCH_WAKEUP) {
    wakeup_handler(0, kWakeupReasonNormal);
  } else {
    text_layer_set_text(mainLabel, "Monitoring");
  }
  
  subLabel = text_layer_create(GRect(10,70, 120, 80));
  text_layer_set_text(subLabel, "Long press middle to view history, down to clear history, up to disable app.");
  layer_add_child(windowLayer, text_layer_get_layer(subLabel));
  
  //Displaying Static Image
  skylineImg = gbitmap_create_with_resource(RESOURCE_ID_kSkyline);
  skylineImg = scaleBitmap(skylineImg, 56, 56);
  BitmapLayer* skylineLayer = bitmap_layer_create( GRect(0,0,112,40) );
  bitmap_layer_set_bitmap(skylineLayer, skylineImg);
  layer_add_child(windowLayer, bitmap_layer_get_layer(skylineLayer));
  
  lineImg = gbitmap_create_with_resource(RESOURCE_ID_kLine);
  lineImg = scaleBitmap(lineImg, 56, 56);
  BitmapLayer* lineLayer = bitmap_layer_create( GRect(0,60,112,6) );
  bitmap_layer_set_bitmap(lineLayer, lineImg);
  layer_add_child(windowLayer, bitmap_layer_get_layer(lineLayer));
  
  //Creating and adding the Action Bar Layer
  //Preparing Bitmap icons
  checkImg = gbitmap_create_with_resource(RESOURCE_ID_kCheckWhite);
  equalImg = gbitmap_create_with_resource(RESOURCE_ID_kEqualWhite);
  crossImg = gbitmap_create_with_resource(RESOURCE_ID_kCrossWhite);
  //Creating action bar
  actionBar = action_bar_layer_create();
  action_bar_layer_set_icon(actionBar, BUTTON_ID_UP, checkImg);
  action_bar_layer_set_icon(actionBar, BUTTON_ID_SELECT, equalImg);
  action_bar_layer_set_icon(actionBar, BUTTON_ID_DOWN, crossImg);
  
  action_bar_layer_set_click_config_provider(actionBar, bar_click_config);
  action_bar_layer_add_to_window(actionBar, window);
  
}


static void window_did_unload(Window* window) {
  
  text_layer_destroy(mainLabel);
}


static void init() {
 
  window = window_create();
  window_set_click_config_provider(window, main_click_config);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_did_load,
    .unload = window_did_unload
  });
  
  window_stack_push(window, true);
  wakeup_service_subscribe(wakeup_handler);
}
  

static void deinit() {
  data_logging_finish(logSession);
  window_destroy(window);
  
  if(appEnabled) {
    static WakeupId wakeup_id;
    srand(time(NULL));
    int randomTime = 5 + rand()%5;
    time_t future_time = time(NULL) + randomTime;
    wakeup_id = wakeup_schedule(future_time, 0, true/*NotifyIfMissed*/);
    persist_write_int(kWakeupIdKey, wakeup_id);
  }
}


static void quit_app() {
  window_stack_pop_all(animated);
}


int main(void) {
  appEnabled = true;
  init();
  app_event_loop();
  deinit();
}


