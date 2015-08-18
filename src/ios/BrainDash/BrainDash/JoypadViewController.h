//
//  JoypadViewContrroller.h
//  BrainDash
//
//  Created by Wayne Keenan on 22/10/2014.
//  Copyright (c) 2014 Cannybots. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <CoreMotion/CoreMotion.h>
#import "RfduinoManager.h"

#import "RFduino.h"

@interface JoypadViewController : UIViewController<RFduinoDelegate,  UIGestureRecognizerDelegate, RFduinoManagerDelegate>

@property (weak, nonatomic) IBOutlet UIImageView *baseImageView;
@property (weak, nonatomic) IBOutlet UIImageView *knobImageView;

@property (nonatomic, strong) CMMotionManager *mManager;
@property (nonatomic, strong) CMAttitude* referenceAttitude;

@property (nonatomic, assign) int  maxX;
@property (nonatomic, assign) int  maxY;

@property (nonatomic, assign) int  minZ;
@property (nonatomic, assign) int  maxZ;

@property (nonatomic, assign) bool xAxisIsInverted;
@property (nonatomic, assign) bool yAxisIsInverted;
@property (nonatomic, assign) bool zAxisIsInverted;
@property (nonatomic, assign) bool useTilt;


@property (strong, nonatomic) RFduino *rfduino;

@property (weak, nonatomic) IBOutlet UISlider *throttleSlider;
@property (weak, nonatomic) IBOutlet UILabel *message;
@property (weak, nonatomic) IBOutlet UIButton *tiltButton;
@property (weak, nonatomic) IBOutlet UIButton *tiltLabel;

@end

