//
//  ViewController.h
//  HappyTracker
//
//  Created by Si Te Feng on 1/6/15.
//  Copyright (c) 2015 Si Te Feng. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <PebbleKit/PebbleKit.h>

@interface ViewController : UIViewController <PBDataLoggingServiceDelegate>
{
    PBWatch* pebble;
    NSInteger sessionNum;
}

@property (weak, nonatomic) IBOutlet UITextView *mainTextView;
@property (nonatomic,strong) NSMutableString* historyString;


- (IBAction)getDataButton:(UIButton *)sender;

@end

