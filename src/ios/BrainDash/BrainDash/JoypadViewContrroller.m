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

@interface JoypadViewController () {
    NSTimer* timer;
    int xAxisValue,yAxisValue,zAxisValue;
    bool buttonState[BUTTONS_MAX];
    UIImage* currentUISliderMinImage, *currentUISliderMaxImage, *currentUISliderThumbImage;
}
@end

@implementation JoypadViewController

@synthesize rfduino;

+ (void)load {
    customUUID = @"7e400001-b5a3-f393-e0a9-e50e24dcca9e";
}


- (void)viewDidLoad {
    [super viewDidLoad];
    _mManager = [[CMMotionManager alloc] init];
    _referenceAttitude = nil;
    //[self.throttleSlider removeConstraints:self.throttleSlider.constraints];
    //[self.throttleSlider setTranslatesAutoresizingMaskIntoConstraints:YES];
    
    currentUISliderThumbImage = [[UISlider appearance ]currentThumbImage];
    currentUISliderMinImage   = [[UISlider appearance ]currentMinimumTrackImage];
    currentUISliderMaxImage   = [[UISlider  appearance ] currentMaximumTrackImage];
    
    self.throttleSlider.transform=CGAffineTransformRotate(self.throttleSlider.transform,270.0/180*M_PI);

}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

- (void) viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    


    UIImage *minImage = [[UIImage imageNamed:@"slider_minimum"] resizableImageWithCapInsets:UIEdgeInsetsMake(0, 5, 0, 0)];
    UIImage *maxImage = [[UIImage imageNamed:@"slider_maximum"] resizableImageWithCapInsets:UIEdgeInsetsMake(0, 5, 0, 0)];
    UIImage *thumbImage = [UIImage imageNamed:@"throttleKnob"];

    //[[UISlider appearance] setMaximumTrackImage:maxImage forState:UIControlStateNormal];
    [[UISlider appearance] setMinimumTrackImage:minImage forState:UIControlStateNormal];
    [[UISlider appearance] setThumbImage:thumbImage forState:UIControlStateNormal];
}

- (void) viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    

    
    [self loadDefaults];
    self.rfduino=[RFduinoManager sharedRFduinoManager].connectedRFduino;
    [rfduino setDelegate:self];
    if (self.useTilt)
        [self startUpdateAccelerometer];
    [self startJoypadUpdates];
    
}

- (void) viewWillDisappear:(BOOL)animated {
    
    //[[UISlider appearance] setMaximumTrackImage:currentUISliderMaxImage forState:UIControlStateNormal];
    [[UISlider appearance] setMinimumTrackImage:currentUISliderMaxImage forState:UIControlStateNormal];
    [[UISlider appearance] setThumbImage:currentUISliderThumbImage forState:UIControlStateNormal];
    
    [super viewWillDisappear:animated];
    if (self.useTilt)
        [self stopCoreMotionUpdate];
    [self stopJoypadUpdates];
}



- (void) loadDefaults {
    NSLog(@"load");
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    
    if (userDefaults) {
        self.xAxisIsInverted   = [userDefaults boolForKey:@"XINV"];
        self.yAxisIsInverted   = [userDefaults boolForKey:@"YINV"];
        self.zAxisIsInverted   = [userDefaults boolForKey:@"ZINV"];
        self.useTilt           = [userDefaults boolForKey:@"USETILT"];
        
        int zSense = (int) [userDefaults integerForKey:@"ZSENSE"];
        self.minZ = -(180-zSense);
        self.maxZ = 180-zSense;
    }
}

- (IBAction)handlePan:(UIPanGestureRecognizer *)uigr
{
    CGPoint point = [uigr locationInView:self.baseImageView];
    int baseX = self.baseImageView.frame.origin.x;
    int baseY = self.baseImageView.frame.origin.y;
    int baseW = self.baseImageView.frame.size.width;
    int baseH = self.baseImageView.frame.size.height;
    
    if (uigr.state == UIGestureRecognizerStateEnded) {
        point    = CGPointMake(baseX + baseW/2, baseY+ baseH/2);
    } else {
        if (point.x<baseX) return;
        if (point.x>baseX+baseW) return;
        if (point.y<baseY) return;
        if (point.y>baseY+baseH) return;
    }
    xAxisValue = (int) map(point.x, baseX, baseX+baseW, -255,255) *  (self.xAxisIsInverted?-1:1);
    yAxisValue = (int) map(point.y, baseY, baseY+baseH, -255,255) *  (self.yAxisIsInverted?-1:1);
    
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
    sender.value=75;
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
- (IBAction)throttleSliderValueChanged:(UISlider *)sender {
    //[self updateRoll:sender.value];
    int throttleVal = sender.value;
    if (throttleVal < 0) throttleVal = 0;
    if (throttleVal > 255) throttleVal = 255;
    zAxisValue = throttleVal;
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
        UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"CoreMotion Error"
                                                        message:@"Request reference frame not available."
                                                       delegate:nil
                                              cancelButtonTitle:@"OK"
                                              otherButtonTitles:nil];
        [alert show];
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
    
    
    if (roll < self.minZ) roll = self.minZ;
    if (roll > self.maxZ) roll = self.maxZ;
    //NSLog(@"CLAMP: roll/throttle=(%.0f)",  roll);
    
    if (roll !=0.0)
        roll = map(roll, self.minZ, self.maxZ, -180, 180);
    //NSLog(@"MAP  : roll/throttle=(%.0f)",  roll);
    

    // Forward/Backward
    if (self.zAxisIsInverted)
        roll = roll * -1;
    //NSLog(@"INV? : roll/throttle=(%.0f)",  roll);
    zAxisValue = roll;

}

- (void) startJoypadUpdates {
    if (timer)
        [timer invalidate];
    
    timer = [NSTimer scheduledTimerWithTimeInterval:0.05 target:self selector:@selector(sendJoypadUpdate) userInfo:nil repeats:YES];
}

- (void) stopJoypadUpdates {
    if (timer)
        [timer invalidate];
    timer=nil;
    [self sendJoypadUpdate:127 y:127 z:127  b:0];

}

- (void) sendJoypadUpdate {
    uint8_t xAxisByte  = map(xAxisValue, -255, 255, 0,255);
    uint8_t yAxisByte  = map(yAxisValue, -255, 255, 0,255);
    uint8_t zAxisByte  = map(zAxisValue, -180, 180, 0,255);
    uint32_t buttons    = 0;
    for (int i = 0; i<BUTTONS_MAX; i++) {
        buttons = buttons |  ( buttonState[i] << i);
    }
    [self sendJoypadUpdate:xAxisByte y:yAxisByte z:zAxisByte b:buttons];
}

- (void) sendJoypadUpdate:(uint8_t)x y:(uint8_t)y z:(uint8_t)z b:(uint8_t)b {
    char msg[5] = {0};
    snprintf(msg, sizeof(msg), "%c%c%c%c", x, y, b, z);
    NSData *data = [NSData dataWithBytesNoCopy:msg length:sizeof(msg)-1 freeWhenDone:NO];
    //NSLog(@"SendData: %@", data);

    [rfduino send:data];
}

- (void)didReceive:(NSData *)data
{
    NSString *hexString = [data hexRepresentationWithSpaces:YES];
    NSLog(@"RecievedData: %@", hexString);
    self.message.text = [NSString stringWithUTF8String:[data bytes]];
// TODO:     [rfduino disconnect];
}

@end
