//
//  AppDelegate.m
//  BrainDash
//
//  Created by Wayne Keenan on 22/10/2014.
//  Copyright (c) 2014 Cannybots. All rights reserved.
//

#import "AppDelegate.h"

#import "RFduinoManager.h"

@interface AppDelegate()
{
    RFduinoManager *rfduinoManager;
    bool wasScanning;
}
@end


@implementation AppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    rfduinoManager = RFduinoManager.sharedRFduinoManager;
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    if (userDefaults) {
        if (![userDefaults objectForKey:@"marker"]) {
            [userDefaults setBool:false forKey:@"XINV"];
            [userDefaults setBool:false forKey:@"YINV"];
            [userDefaults setBool:true forKey:@"ZINV"];
            [userDefaults setBool:false forKey:@"USETILT"];
            [userDefaults setInteger:128 forKey:@"XMAX"];
            [userDefaults setInteger:128 forKey:@"YMAX"];
            [userDefaults setInteger:90 forKey:@"ZSENSE"];
            [userDefaults setInteger:40 forKey:@"PID_P"];
            [userDefaults setInteger:0 forKey:@"PID_I"];
            [userDefaults setInteger:450 forKey:@"PID_D"];
            [userDefaults setInteger:700 forKey:@"IRWHITE"];
            [userDefaults setObject:@"yes" forKey:@"marker"];
            [userDefaults synchronize];
        }
    }

    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
    NSLog(@"applicationWillResignActive");
    
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
    wasScanning = false;

    if (rfduinoManager.isScanning) {
        wasScanning = true;
        [rfduinoManager stopScan];
    }

}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
    [[NSUserDefaults standardUserDefaults] synchronize];
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    NSLog(@"applicationDidBecomeActive");
    if (wasScanning) {
        [rfduinoManager startScan];
        wasScanning = false;
    }}

- (void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

@end
