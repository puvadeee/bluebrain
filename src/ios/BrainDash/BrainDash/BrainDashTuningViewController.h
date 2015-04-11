//
//  BrainDashTuningViewController.h
//  BrainDash
//
//  Created by Wayne Keenan on 04/06/2014.
//  Copyright (c) 2014 CannyBots. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "RFduino.h"

@interface BrainDashTuningViewController : UIViewController <UITextFieldDelegate, RFduinoDelegate>
@property (weak, nonatomic) IBOutlet UITextField *pTextField;
@property (weak, nonatomic) IBOutlet UITextField *iTextField;
@property (weak, nonatomic) IBOutlet UITextField *dTExtField;
@property (weak, nonatomic) IBOutlet UIStepper *pStepper;
@property (weak, nonatomic) IBOutlet UIStepper *iStepper;
@property (weak, nonatomic) IBOutlet UIStepper *dStepper;
-(IBAction)textFieldReturn:(id)sender;
- (IBAction)pStepperValueChanged:(UIStepper*)sender;
- (IBAction)iStepperValueChanged:(UIStepper*)sender;
- (IBAction)dStepperValueChanged:(UIStepper *)sender;
@property (weak, nonatomic) IBOutlet UITextView *debugTextView;
@property (weak, nonatomic) IBOutlet UILabel *whiteThresholdLable;
@property (weak, nonatomic) IBOutlet UISlider *whiteThresholdSlider;

@property (strong, nonatomic) RFduino *rfduino;

- (IBAction)statsPressed:(id)sender;

@end
