//
//  SettingsViewController.h
//  BrainDash
//
//  Created by Wayne Keenan on 22/10/2014.
//  Copyright (c) 2014 Cannybots. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface SettingsViewController : UIViewController

@property (weak, nonatomic) IBOutlet UISwitch *invertX;
@property (weak, nonatomic) IBOutlet UISwitch *invertY;
@property (weak, nonatomic) IBOutlet UISwitch *invertZ;
@property (weak, nonatomic) IBOutlet UISlider *xSense;
@property (weak, nonatomic) IBOutlet UISlider *ySense;
@property (weak, nonatomic) IBOutlet UISlider *zSense;
@property (weak, nonatomic) IBOutlet UISwitch *useTilt;
- (IBAction)valChanged:(id)sender;
@property (weak, nonatomic) IBOutlet UILabel *whiteThresholdLable;
@property (weak, nonatomic) IBOutlet UILabel *reverseTiltLable;
@property (weak, nonatomic) IBOutlet UILabel *tiltSenseLabel;

@end

