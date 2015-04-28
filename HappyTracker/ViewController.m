//
//  ViewController.m
//  HappyTracker
//
//  Created by Si Te Feng on 1/6/15.
//  Copyright (c) 2015 Si Te Feng. All rights reserved.
//

#import "ViewController.h"
#import "AppDelegate.h"
#import "NSDate+HNConvenience.h"


@interface ViewController ()

@end

@implementation ViewController


- (void)viewDidLoad {
    [super viewDidLoad];
    
    sessionNum = 0;
    
    AppDelegate* delegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
    pebble = delegate.pebble;
    if(pebble)
    {
        [self watchConnected];
    }
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(watchConnected) name:@"PBWatchDidConnectNotification" object:nil];
    
    //Data Logging Services
    [[[PBPebbleCentral defaultCentral] dataLoggingService] setDelegate:self];
    [self appendHistoryString:@"Polling Data..."];
    uuid_t uuidBytes;
    NSUUID* uuid = [[NSUUID alloc] initWithUUIDString:
                    @"385233be-d6aa-43a9-8a3a-26500e384a8c"];
    
    [uuid getUUIDBytes:uuidBytes];
    [[PBPebbleCentral defaultCentral] setAppUUID:[NSData dataWithBytes:uuidBytes length:16]];
    
    //Other Setups
    self.historyString = [@"" mutableCopy];

    self.mainTextView.editable = NO;
    self.mainTextView.selectable = NO;
}


#pragma mark - Other Methods

- (void)watchConnected
{
    NSLog(@"Watch Connected, proceeding");
    [[[PBPebbleCentral defaultCentral] dataLoggingService] pollForData];
    
}

- (void)appendHistoryString: (NSString*)string
{
    NSString* tempString = [self.historyString copy];
    
    self.historyString = [string mutableCopy];
    if(tempString!=nil)
        [self.historyString appendString:tempString];

    self.mainTextView.text = self.historyString;
}


#pragma mark - PB Data Logging Service Delegates

- (BOOL)dataLoggingService:(PBDataLoggingService *)service hasSInt32s:(const SInt32 [])data numberOfItems:(UInt16)numberOfItems forDataLoggingSession:(PBDataLoggingSessionMetadata *)session
{
    
    
    
    
    return YES;
}


- (BOOL)dataLoggingService:(PBDataLoggingService *)service hasUInt32s:(const UInt32 [])data numberOfItems:(UInt16)numberOfItems forDataLoggingSession:(PBDataLoggingSessionMetadata *)session
{
    NSLog(@"received %u data", numberOfItems);
    sessionNum++;
    
    NSDate* logDate = [NSDate dateWithTimeIntervalSince1970: session.timestamp];
    
    [self appendHistoryString: [NSString stringWithFormat:@"**Session #%ld (%@)\n", (long)sessionNum, logDate.description]];
    
    for(UInt16 i=0; i<numberOfItems; i++)
    {
        [self appendHistoryString: [NSString stringWithFormat:@"  - %@\n", [self happinessStringFromButtonId:(NSUInteger)data[i]]]];
    }
    
    return YES;
}


- (void)dataLoggingService:(PBDataLoggingService *)service sessionDidFinish:(PBDataLoggingSessionMetadata *)session
{
    NSLog(@"Data Logging Finished");
    
}


- (IBAction)getDataButton:(UIButton *)sender {
    [[[PBPebbleCentral defaultCentral] dataLoggingService] pollForData];
    [self appendHistoryString:@"Polling New Data...\n"];
}


- (NSString*)happinessStringFromButtonId: (NSUInteger)uid
{
    if(uid == 1) {
        return @"Happy";
    } else if(uid == 2) {
        return @"Neutral";
    } else if(uid == 3) {
        return @"Sad";
    } else {
        return @"unknown";
    }
    
}


#pragma mark - Other Methods



- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}




@end
