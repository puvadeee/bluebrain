//
//  BrainDashTuningViewController.m
//  BrainDash
//
//  Created by Wayne Keenan on 04/06/2014.
//  Copyright (c) 2014 CannyBots. All rights reserved.
//

#import "BrainDashTuningViewController.h"
#import "RFduinoManager.h"
#import "NSData+hex.h"
#import "Utilities.h"

@interface BrainDashTuningViewController () {

    int16_t pidP, pidD, whiteThreshold;
}

@property (nonatomic, assign) id currentResponder;

@end

@implementation BrainDashTuningViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {

    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    UITapGestureRecognizer *singleTap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(resignOnTap:)];
    [singleTap setNumberOfTapsRequired:1];
    [singleTap setNumberOfTouchesRequired:1];
    [self.view addGestureRecognizer:singleTap];

}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/


- (void) viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    
    self.rfduino=[RFduinoManager sharedRFduinoManager].connectedRFduino;
    [self.rfduino setDelegate:self];

    [self loadSettings];
    [self applyTuningParametersToUI];

    _debugTextView.text = @"";
    
    [self performSelector:@selector(statsPressed:) withObject:nil afterDelay:1.0 ];

}


-(IBAction)textFieldReturn:(id)sender
{
    [sender resignFirstResponder];
}

- (BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string {
    if (textField==_pTextField) {
        pidP = [[textField text] intValue];
        
    } else if (textField==_iTextField) {
        //_iStepper.value = [[textField text] intValue];
    } else if (textField==_dTExtField) {
        pidD = [[textField text] intValue];
    }
    [self applyTuningParametersToUI];
    return true;
}

- (void)textFieldDidBeginEditing:(UITextField *)textField {
    self.currentResponder = textField;
}

- (void)resignOnTap:(id)iSender {
    [self.currentResponder resignFirstResponder];
}

- (IBAction)pStepperValueChanged:(UIStepper *)sender {
    //_pTextField.text = [NSString stringWithFormat:@"%d",(int16_t)sender.value];
    pidP =(int16_t)sender.value;
    [self applyTuningParametersToUI];

}
- (IBAction)iStepperValueChanged:(UIStepper *)sender {
    //_iTextField.text = [NSString stringWithFormat:@"%d",(int16_t)sender.value];

}
- (IBAction)dStepperValueChanged:(UIStepper *)sender {
    //_dTExtField.text = [NSString stringWithFormat:@"%d",(int16_t)sender.value];
    pidD = (int16_t)sender.value;
    [self applyTuningParametersToUI];
}


- (IBAction)savePressed:(id)sender {
    [self updateTuningParametersFromUI];
    [self sendPIDUpdate:pidP i:0 d:pidD];
    [self saveSettings];

}

- (void) applyTuningParametersToUI {
    _pTextField.text = [NSString stringWithFormat:@"%d", pidP];
    _dTExtField.text = [NSString stringWithFormat:@"%d", pidD];
    _pStepper.value = pidP;
    _dStepper.value = pidD;
    self.whiteThresholdLable.text=[NSString stringWithFormat:@"%d", whiteThreshold];
    self.whiteThresholdSlider.value = whiteThreshold;
}

- (void) updateTuningParametersFromUI {
    pidP= [[_pTextField text] intValue];
    pidD= [[_dTExtField text] intValue];
    whiteThreshold = [_whiteThresholdLable.text intValue];
}

- (void) loadSettings {
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    
    if (userDefaults) {
        pidP = [userDefaults integerForKey:@"PID_P"];
        pidD = [userDefaults integerForKey:@"PID_D"];
        whiteThreshold = [userDefaults integerForKey:@"IRWHITE"];
    }
}



- (void) saveSettings {
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    if (userDefaults) {
        [userDefaults setInteger:pidP forKey:@"PID_P"];
        //[userDefaults setInteger:0 forKey:@"PID_I"];
        [userDefaults setInteger:pidD forKey:@"PID_D"];
        [userDefaults setInteger:whiteThreshold forKey:@"IRWHITE"];
        [userDefaults synchronize];
    } else {
        NSLog(@"no user defaults");
    }
   
}





- (void) sendPIDUpdate:(int)p i:(int)i d:(int)d {
    char msg[21] = {0};
    snprintf(msg, sizeof(msg), "PID:%c%c%c%c", highByte(p), lowByte(p), highByte(d), lowByte(d) );
    NSData *data = [NSData dataWithBytesNoCopy:msg length:9 freeWhenDone:NO];
    //NSLog(@"SendData: %@", data);
    NSLog(@"SendData: %s", msg);
    
    [self.rfduino send:data];
}

- (void) sendString:(NSString*) str {
    NSLog(@"SendString: %@", str);
    NSData* data = [str dataUsingEncoding:NSUTF8StringEncoding];
    //data = [data subdataWithRange:NSMakeRange(0, [data length] - 1)];
    
    [self.rfduino send:data];
    
}

- (void)didReceive:(NSData *)data
{
    NSString *string =[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    NSLog(@"Recv: %@", string);
    NSRange range = NSMakeRange(_debugTextView.text.length - 1, 1);
    [_debugTextView scrollRangeToVisible:range ];
    _debugTextView.text = [NSString stringWithFormat:@"%@\n%@", _debugTextView.text, string];

    //_debugTextView.text = string;
    
}



- (IBAction)statsPressed:(id)sender {
    _debugTextView.text = @"";
    RFduino *rfduino;
    rfduino=[RFduinoManager sharedRFduinoManager].connectedRFduino;
    if (rfduino) {
        [self sendString:@"i"];
    } else {
        _debugTextView.text = @"Not connected.";
    }
    
}
- (IBAction)whiteThresholdSliderFinishedSliding:(id)sender {
    NSLog(@"whiteThresholdSlideEditingDidEnd");
    [self applyTuningParametersToUI];
    [self saveSettings];
}


- (IBAction)whiteThresholdSliderValueChanged:(UISlider *)sender {
    whiteThreshold = sender.value;
    [self applyTuningParametersToUI];

    RFduino *rfduino;
    rfduino=[RFduinoManager sharedRFduinoManager].connectedRFduino;
    if (rfduino) {
        uint8_t whiteVal = map(sender.value, 0, 1000, 0,255);
        char msg[6] = {0};
        snprintf(msg, sizeof(msg), "%c%c%c%c%c", 0x7F, 0x7F, 0, 0x7F, whiteVal);
        NSData *data = [NSData dataWithBytesNoCopy:msg length:sizeof(msg)-1 freeWhenDone:NO];
        NSLog(@"SendData: %@", data);
        
        [rfduino send:data];
    }
}



@end

@implementation UIProgressView (customView)
- (CGSize)sizeThatFits:(CGSize)size {
    CGSize newSize = CGSizeMake(self.frame.size.width,9);
    return newSize;
}
@end

