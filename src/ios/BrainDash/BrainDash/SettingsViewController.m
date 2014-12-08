//
//  SettingsViewController.m
//  BrainDash
//
//  Created by Wayne Keenan on 22/10/2014.
//  Copyright (c) 2014 Cannybots. All rights reserved.
//

#import "SettingsViewController.h"
#import "RFduino.h"
#import "RFduinoManager.h"


@interface SettingsViewController ()

@end

@implementation SettingsViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    [self loadDefaults];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}


- (void) viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    [self loadDefaults];
}

- (void) viewWillDisappear:(BOOL)animated {
    [super viewWillDisappear:animated];
    [self saveDefaults];
}

- (void) loadDefaults {
    NSLog(@"load");
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    
    if (userDefaults) {
        _invertX.on   = [userDefaults boolForKey:@"XINV"];
        _invertY.on   = [userDefaults boolForKey:@"YINV"];
        _invertZ.on   = [userDefaults boolForKey:@"ZINV"];
        _useTilt.on   = [userDefaults boolForKey:@"USETILT"];
        _xSense.value = [userDefaults integerForKey:@"XMAX"];
        _ySense.value = [userDefaults integerForKey:@"YMAX"];
        _zSense.value = [userDefaults integerForKey:@"ZSENSE"];
    }
}


- (void) saveDefaults {
    NSLog(@"save");
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    if (userDefaults) {
        [userDefaults setBool:_invertX.on forKey:@"XINV"];
        [userDefaults setBool:_invertY.on forKey:@"YINV"];
        [userDefaults setBool:_invertZ.on forKey:@"ZINV"];
        [userDefaults setBool:_useTilt.on forKey:@"USETILT"];
        [userDefaults setInteger:_xSense.value forKey:@"XMAX"];
        [userDefaults setInteger:_ySense.value forKey:@"YMAX"];
        [userDefaults setInteger:_zSense.value forKey:@"ZSENSE"];
        [userDefaults setObject:@"yes" forKey:@"marker"];
        [userDefaults synchronize];
    } else {
        NSLog(@"no user defaults");
    }
}

- (IBAction)whiteThresholdSliderValueChanged:(UISlider *)sender {
    self.whiteThresholdLable.text=[NSString stringWithFormat:@"%.0f", sender.value];
    RFduino *rfduino;
    rfduino=[RFduinoManager sharedRFduinoManager].connectedRFduino;
    if (rfduino) {
        char msg[6] = {0};
        snprintf(msg, sizeof(msg), "%c%c%c%c%c", 0x7F, 0x7F, 0, 0x7F, (uint8_t)sender.value);
        NSData *data = [NSData dataWithBytesNoCopy:msg length:sizeof(msg)-1 freeWhenDone:NO];
        NSLog(@"SendData: %@", data);
    
        [rfduino send:data];
    }
}

- (IBAction)valChanged:(id)sender {
    [self saveDefaults];
}
@end
