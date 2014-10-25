//
//  JoypadViewContrroller.h
//  BrainDash
//
//  Created by Wayne Keenan on 22/10/2014.
//  Copyright (c) 2014 Cannybots. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <CoreMotion/CoreMotion.h>
#import "RFduino.h"

@interface JoypadViewController : UIViewController<RFduinoDelegate>

@property (weak, nonatomic) IBOutlet UIImageView *baseImageView;
@property (weak, nonatomic) IBOutlet UIImageView *knobImageView;

@property (nonatomic, strong) CMMotionManager *mManager;
@property (nonatomic, strong) CMAttitude* referenceAttitude;

@property (nonatomic, assign) int  minZ;
@property (nonatomic, assign) int  maxZ;

@property (nonatomic, assign) bool xAxisIsInverted;
@property (nonatomic, assign) bool yAxisIsInverted;
@property (nonatomic, assign) bool zAxisIsInverted;


@property (strong, nonatomic) RFduino *rfduino;


@end

