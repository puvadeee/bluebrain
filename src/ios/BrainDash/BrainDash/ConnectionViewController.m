//
//  ConnectionViewController.m
//  BrainDash
//
//  Created by Wayne Keenan on 24/10/2014.
//  Copyright (c) 2014 Cannybots. All rights reserved.
//

#import "ConnectionViewController.h"
#import "ScanViewController.h"
#import "RfduinoManager.h"
#import "Rfduino.h"

@interface ConnectionViewController ()
{
    RFduinoManager *rfduinoManager;
}
@end

@implementation ConnectionViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    rfduinoManager = RFduinoManager.sharedRFduinoManager;

    ScanViewController *scanViewController = [[ScanViewController alloc] init];

    [self addChildViewController:scanViewController];
    [self.selectionView addSubview: scanViewController.view];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

- (void) viewDidAppear:(BOOL)animated {
    [rfduinoManager.connectedRFduino disconnect];
}
@end
