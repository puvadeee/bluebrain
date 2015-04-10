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

    _pStepper.value = [[_pTextField text] intValue];
    _iStepper.value = [[_iTextField text] intValue];
    _dStepper.value = [[_dTExtField text] intValue];
    _debugTextView.text = @"";
    
    [self statsPressed:nil];
}


-(IBAction)textFieldReturn:(id)sender
{
    [sender resignFirstResponder];
}

- (BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string {
    if (textField==_pTextField) {
        _pStepper.value = [[textField text] intValue];
    } else if (textField==_iTextField) {
        _iStepper.value = [[textField text] intValue];
    } else if (textField==_dTExtField) {
        _dStepper.value = [[textField text] intValue];
    }
    return true;
}

- (void)textFieldDidBeginEditing:(UITextField *)textField {
    self.currentResponder = textField;
}

- (void)resignOnTap:(id)iSender {
    [self.currentResponder resignFirstResponder];
}

- (IBAction)pStepperValueChanged:(UIStepper *)sender {
    _pTextField.text = [NSString stringWithFormat:@"%d",(int16_t)sender.value];
}
- (IBAction)iStepperValueChanged:(UIStepper *)sender {
    _iTextField.text = [NSString stringWithFormat:@"%d",(int16_t)sender.value];

}
- (IBAction)dStepperValueChanged:(UIStepper *)sender {
    _dTExtField.text = [NSString stringWithFormat:@"%d",(int16_t)sender.value];

}


- (IBAction)savePressed:(id)sender {
    [self sendPIDUpdate:[_pTextField.text intValue] i:0 d:[_dTExtField.text intValue]];

}


#define lowByte(v)   ((unsigned char) (v))
#define highByte(v)  ((unsigned char) (((unsigned int) (v)) >> 8))

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
    [self sendString:@"i"];
    
}


- (IBAction)whiteThresholdSliderValueChanged:(UISlider *)sender {
    self.whiteThresholdLable.text=[NSString stringWithFormat:@"%.0f", sender.value];
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

