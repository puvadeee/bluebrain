//
//  JoypadViewContrroller.m
//  BrainDash
//
//  Created by Wayne Keenan on 22/10/2014.
//  Copyright (c) 2014 Cannybots. All rights reserved.
//
// UISlider cusomisations based on: http://www.raywenderlich.com/21703/user-interface-customization-in-ios-6

#import "JoypadViewController.h"
#import "Utilities.h"
#import "RFduinoManager.h"
#define BUTTONS_MAX 8

#define ZAXIS_DEFAULT -80
#define ZAXIS_ZERO    -179

@interface JoypadViewController () {
    NSTimer* timer;
    int xAxisValue,yAxisValue,zAxisValue;
    bool buttonState[BUTTONS_MAX];
    UIImage* currentUISliderMinImage, *currentUISliderMaxImage, *currentUISliderThumbImage;
}
@end

@implementation JoypadViewController

@synthesize rfduino;



- (void)viewDidLoad {
    [super viewDidLoad];
    _mManager = [[CMMotionManager alloc] init];
    _referenceAttitude = nil;
    
    currentUISliderThumbImage = [[UISlider appearance] currentThumbImage];
    currentUISliderMinImage   = [[UISlider appearance] currentMinimumTrackImage];
    currentUISliderMaxImage   = [[UISlider appearance] currentMaximumTrackImage];
    
    self.throttleSlider.transform=CGAffineTransformRotate(self.throttleSlider.transform,270.0/180*M_PI);
    self.throttleSlider.value = ZAXIS_DEFAULT;

    zAxisValue=ZAXIS_DEFAULT;
    
    [_baseImageView addSubview:_knobImageView];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

- (void) viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];

    UIImage *minImage = [[UIImage imageNamed:@"slider_minimum"] resizableImageWithCapInsets:UIEdgeInsetsMake(0, 5, 0, 0)];
    //UIImage *maxImage = [[UIImage imageNamed:@"slider_maximum"] resizableImageWithCapInsets:UIEdgeInsetsMake(0, 5, 0, 0)];
    UIImage *thumbImage = [UIImage imageNamed:@"throttleKnob"];

    //[[UISlider appearance] setMaximumTrackImage:maxImage forState:UIControlStateNormal];
    [[UISlider appearance] setMinimumTrackImage:minImage forState:UIControlStateNormal];
    [[UISlider appearance] setThumbImage:thumbImage forState:UIControlStateNormal];
    


}

- (void) viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    
    [self loadDefaults];
    //[RFduinoManager sharedRFduinoManager].delegate =self;
    self.rfduino=[RFduinoManager sharedRFduinoManager].connectedRFduino;
    [rfduino setDelegate:self];

    if (self.useTilt) {
        [self startUpdateAccelerometer];
    }

    _tiltButton.hidden = !self.useTilt;
    _tiltLabel.hidden = !self.useTilt;
    self.throttleSlider.hidden=self.useTilt;

    
    zAxisValue=ZAXIS_DEFAULT;

    [self startJoypadUpdates];
    
}

- (void) viewWillDisappear:(BOOL)animated {
    [super viewWillDisappear:animated];
    
    //[[UISlider appearance] setMaximumTrackImage:currentUISliderMaxImage forState:UIControlStateNormal];
    [[UISlider appearance] setMinimumTrackImage:currentUISliderMaxImage forState:UIControlStateNormal];
    [[UISlider appearance] setThumbImage:currentUISliderThumbImage forState:UIControlStateNormal];
    
    if (self.useTilt) {
        [self stopCoreMotionUpdate];
    }

    [self performSelector:@selector(stopJoypadUpdates) withObject:nil afterDelay:1.0 ];

}



- (void) loadDefaults {
    NSLog(@"load");
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    
    if (userDefaults) {
        self.xAxisIsInverted   = [userDefaults boolForKey:@"XINV"];
        self.yAxisIsInverted   = [userDefaults boolForKey:@"YINV"];
        self.zAxisIsInverted   = [userDefaults boolForKey:@"ZINV"];
        self.useTilt           = [userDefaults boolForKey:@"USETILT"];
        self.maxX              = (int) [userDefaults integerForKey:@"XMAX"];
        self.maxY              = (int) [userDefaults integerForKey:@"YMAX"];
        
        int sense = 90 - (int)[userDefaults integerForKey:@"ZSENSE"];
        self.minZ = -sense;
        self.maxZ = sense;
    }
}


- (IBAction)handlePan:(UIPanGestureRecognizer *)uigr
{
    int baseW = self.baseImageView.frame.size.width;
    int baseH = self.baseImageView.frame.size.height;
    
    CGPoint point = [uigr locationInView:self.baseImageView];
  
    
    if ( (UIGestureRecognizerStateEnded == uigr.state )|| (UIGestureRecognizerStateCancelled == uigr.state)) {
        point    = CGPointMake(baseW/2, baseH/2);
        
    } else {
        float x = point.x - baseW/2;
        float y = point.y - baseH/2;

        if ( (x*x) + (y*y) > (100*100) ) {
            float bearingRadians = atan2f(x, y); // get bearing in radians
            point.x= ( 100*sin(bearingRadians))+baseW/2;
            point.y= ( 100*cos(bearingRadians))+baseH/2;
        }
    }

    xAxisValue = (int) map(point.x, 0, baseW, -_maxX, _maxX) *  (self.xAxisIsInverted?-1:1);
    yAxisValue = (int) map(point.y, 0, baseH, -_maxY, _maxY) *  (self.yAxisIsInverted?-1:1);
    [self updateKnobPosition:point];

}


- (void) updateKnobPosition:(CGPoint) point {
    int w = self.knobImageView.frame.size.width;
    int h = self.knobImageView.frame.size.height;
    int x = point.x - w/2;
    int y = point.y - h/2;
    [UIView animateWithDuration:0.02 animations:^{
        [self.knobImageView setFrame:CGRectMake(x,y, w,h)];
    }];
   }

// handle buttons

- (IBAction)buttonAction:(UIButton*)sender
{
    [self resetReferenceFrameToCurrent];
    return;
    /*
    long butId = sender.tag;
    bool isPressed = true;
    NSLog(@"Butt pressed:%ld, stade:%d", butId, isPressed);
    if (butId<BUTTONS_MAX) {
        buttonState[butId] = isPressed;
    }
     */
    
}

- (IBAction)buttonReleased:(UIButton*)sender
{
    return;
/*
    long butId = sender.tag;
    bool isPressed = false;
    NSLog(@"Butt pressed:%ld, stade:%d", butId, isPressed);
    if (butId<BUTTONS_MAX) {
        buttonState[butId] = isPressed;
    }
 */
}

- (IBAction)throttleReleased:(UISlider *)sender {
    sender.value= ZAXIS_DEFAULT;
    zAxisValue =  ZAXIS_DEFAULT;

}
- (IBAction)throttleSliderValueChanged:(UISlider *)sender {
    int throttleVal = sender.value;
    if (!self.useTilt)
        zAxisValue = throttleVal;
}

- (IBAction)resetButtonPressed:(UIButton *)sender {
    [self resetReferenceFrameToCurrent];
}

// Core Motion

- (void) resetReferenceFrameToCurrent {
    CMDeviceMotion *deviceMotion = self.mManager.deviceMotion;
    CMAttitude *attitude         = deviceMotion.attitude;
    self.referenceAttitude       = attitude;
}

- (CMMotionManager *)mManager
{
    if (!_mManager) {
        _mManager = [[CMMotionManager alloc] init];
    }
    return _mManager;
}


// @see:  http://wwwbruegge.in.tum.de/lehrstuhl_1/home/98-teaching/tutorials/505-sgd-ws13-tutorial-core-motion
// @see: http://blog.denivip.ru/index.php/2013/07/the-art-of-core-motion-in-ios/?lang=en
// @see: https://github.com/trentbrooks/ofxCoreMotion/blob/master/ofxCoreMotion/src/ofxCoreMotion.mm

- (void)startUpdateAccelerometer
{
    CMDeviceMotion *deviceMotion = self.mManager.deviceMotion;
    CMAttitude *attitude         = deviceMotion.attitude;
    self.referenceAttitude       = attitude;
    self.mManager.deviceMotionUpdateInterval = 0.1;
    
    CMAttitudeReferenceFrame refFrame =CMAttitudeReferenceFrameXArbitraryCorrectedZVertical;
    
    if ( ! ([CMMotionManager availableAttitudeReferenceFrames] & refFrame)) {
        UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Tilt Problem"
                                                        message:@"Unable to use tilting to set the speed on this device."
                                                       delegate:nil
                                              cancelButtonTitle:@"OK"
                                              otherButtonTitles:nil];
        [alert show];
        _useTilt=NO;
    }
    
    [self.mManager startDeviceMotionUpdatesUsingReferenceFrame:refFrame toQueue:[NSOperationQueue mainQueue] withHandler:^(CMDeviceMotion *devMotion, NSError *error) {
        
        CMDeviceMotion *deviceMotion = self.mManager.deviceMotion;
        CMAttitude *attitude = deviceMotion.attitude;
        if (self.referenceAttitude != nil ) {
            [attitude multiplyByInverseOfAttitude:self.referenceAttitude];
        }
        CMQuaternion quat = attitude.quaternion;
        float roll = radiansToDegrees(atan2(2*(quat.y*quat.w - quat.x*quat.z), 1 - 2*quat.y*quat.y - 2*quat.z*quat.z)) ;
        //float pitch = radiansToDegrees(atan2(2*(quat.x*quat.w + quat.y*quat.z), 1 - 2*quat.x*quat.x - 2*quat.z*quat.z));
        //float yaw = radiansToDegrees(asin(2*quat.x*quat.y + 2*quat.w*quat.z));
        [self updateRoll:roll];
    }];
    
}

- (void)stopCoreMotionUpdate
{
    if ([self.mManager isDeviceMotionActive] == YES)
    {
        [self.mManager stopDeviceMotionUpdates];
    }
    
}

- (void) updateRoll:(float)roll {
    
    // Pitch: A pitch is a rotation around a lateral (X) axis that passes through the device from side to side
    // pitch is rotation about the gfx x axis when in portrait mode
    
    // Roll: A roll is a rotation around a longitudinal (Y) axis that passes through the device from its top to bottom
    // roll  is rotation about the gfx y axis when in portrait mode
    
    // Yaw: A yaw is a rotation around an axis (Z) that runs vertically through the device. It is perpendicular to the body of the device, with its origin at the center of gravity and directed toward the bottom of the device
    //NSLog(@"INPUT: roll/throttle=(%.0f)",  roll);
    

    // roll away from body is -ve angles
    
    if (roll < self.minZ) {
        roll=self.minZ;
    }
    if (roll > self.maxZ) {
        roll= self.maxZ;
    }

    roll = map(roll, self.minZ, self.maxZ, -179, 179);

    // Forward/Backward
    if (self.zAxisIsInverted)
        roll = roll * -1;
    zAxisValue = roll;

}

- (void) startJoypadUpdates {
    if (timer)
        [timer invalidate];
    
    timer = [NSTimer scheduledTimerWithTimeInterval:0.1 target:self selector:@selector(sendJoypadUpdate) userInfo:nil repeats:YES];
}

- (void) stopJoypadUpdates {
    if (timer)
        [timer invalidate];
    timer=nil;
    //xAxisValue = 0;
    //yAxisValue = 0;
    //zAxisValue = ZAXIS_ZERO;
    //[self sendJoypadUpdate];

}

- (void) sendJoypadUpdate {
    uint8_t xAxisByte  = map(xAxisValue, -255, 255, 0,255);
    uint8_t yAxisByte  = map(yAxisValue, -255, 255, 0,255);
    uint8_t zAxisByte  = map(-zAxisValue, -180, 180, 0,255);
    uint32_t buttons    = 0;
    for (int i = 0; i<BUTTONS_MAX; i++) {
        buttons = buttons |  ( buttonState[i] << i);
    }
    [self sendJoypadUpdate:xAxisByte y:yAxisByte z:zAxisByte b:buttons];
}

- (void) sendJoypadUpdate:(uint8_t)x y:(uint8_t)y z:(uint8_t)z b:(uint8_t)b {
    char msg[5] = {0};
    snprintf(msg, 5, "%c%c%c%c", x, y, b, z);
    //NSData *data = [NSData dataWithBytesNoCopy:msg length:4 freeWhenDone:NO];
    NSData *data = [NSData dataWithBytes:msg length:4];
    //NSLog(@"SendData: %@", [data hexRepresentationWithSpaces:true]);
    //NSLog(@"SendData: %d\t%d\t%d\t%d", x, y, z, b);
    
    [rfduino send:data];
}

- (void)didReceive:(NSData *)data
{
}



// RFDuino Manager Delegate

- (void)didDiscoverRFduino:(RFduino *)rfduino {
    
}


- (void)didUpdateDiscoveredRFduino:(RFduino *)rfduino {
    
}

- (void)didConnectRFduino:(RFduino *)rfduino {
    
}
- (void)didLoadServiceRFduino:(RFduino *)rfduino {
    
}
- (void)didDisconnectRFduino:(RFduino *)rfduino {
    
    
}

@end
