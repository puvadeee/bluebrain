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
    [self revertPressed:nil];

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
}




-(void)updateVariable:(NSNotification *)notification {
    NSString* name    = [notification name];
    NSData* data      = [notification object];
    const char *bytes = [data bytes];
    
    //NSString* hexString = [data hexRepresentationWithSpaces:YES];
    //NSLog(@"UpVarRcv: %@", hexString);
    
    if ([name isEqualToString:@"IRVAL"]) {
        int ir1= CFSwapInt16BigToHost(( (int16_t*) bytes)[0]);
        int ir2= CFSwapInt16BigToHost(( (int16_t*) bytes)[1]);
        int ir3= CFSwapInt16BigToHost(( (int16_t*) bytes)[2]);

        
    } else if ([name isEqualToString:@"PID"]) {
        _pTextField.text = [NSString stringWithFormat:@"%d", CFSwapInt16BigToHost(( (int16_t*) bytes)[0]) ];;
        //_iTextField.text = [NSString stringWithFormat:@"%d", CFSwapInt16BigToHost(( (int16_t*) bytes)[0]) ];;
        _dTExtField.text = [NSString stringWithFormat:@"%d", CFSwapInt16BigToHost(( (int16_t*) bytes)[1]) ];;
        _pStepper.value = [[_pTextField text] intValue];
        _dStepper.value = [[_dTExtField text] intValue];

        
    } else {
        NSLog(@"Un recognised variable: %@", name);
    }
}

- (void) viewWillDisappear:(BOOL)animated {

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
// LED brightness
- (IBAction)ledBrightnessValueChanged:(UIStepper *)sender {
}
- (IBAction)ledBrightnessTextFieldValueChanged:(id)sender {
}
- (IBAction)deviceIdValueChanged:(id)sender {
}
- (IBAction)ledColourSelected:(id)sender {
}


- (IBAction)savePressed:(id)sender {
    [self sendPIDUpdate:[_pTextField.text intValue] i:0 d:[_dTExtField.text intValue]];

}
- (IBAction)revertPressed:(id)sender {

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
    _debugTextView.text = [NSString stringWithFormat:@"%@\n%@", _debugTextView.text, string];
    //_debugTextView.text = string;
    
    //CGPoint offsetPoint = CGPointMake(0.0, _debugTextView.contentSize.height - _debugTextView.bounds.size.height);
    //[_debugTextView setContentOffset:offsetPoint animated:NO];
    
    NSRange range = NSMakeRange(_debugTextView.text.length - 1, 1);
    [_debugTextView scrollRangeToVisible:range ];
}

- (IBAction)fPressed:(id)sender {
    [self sendString:@"f"];
}

- (IBAction)lPressed:(id)sender {
    [self sendString:@"l"];
}

- (IBAction)rPressed:(id)sender {
    [self sendString:@"r"];
}

- (IBAction)sPressed:(id)sender {
    [self sendString:@"p"];
}

- (IBAction)statsPressed:(id)sender {
    [self sendString:@"i"];

}

- (IBAction)sendPWMPeriodPressed:(UIButton *)sender {
    uint32_t val = [_iTextField.text integerValue];
    
    char msg[21] = {0};
    snprintf(msg, sizeof(msg), "PWM:%c%c%c%c",
             
             (val & 0xFF000000) >> 24,
             (val & 0x00FF0000) >> 16,
             (val & 0x0000FF00) >> 8,
             (val & 0x000000FF) );
    NSData *data = [NSData dataWithBytesNoCopy:msg length:8 freeWhenDone:NO];
    //NSLog(@"SendData: %@", data);
    NSLog(@"SendData: %@", [data hexRepresentationWithSpaces:YES]);
    
    [self.rfduino send:data];



}


@end

@implementation UIProgressView (customView)
- (CGSize)sizeThatFits:(CGSize)size {
    CGSize newSize = CGSizeMake(self.frame.size.width,9);
    return newSize;
}
@end

