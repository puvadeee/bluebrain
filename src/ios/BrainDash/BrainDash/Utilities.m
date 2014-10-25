//
//  Utilities.m
//  BrainDash
//
//  Created by Wayne Keenan on 25/10/2014.
//  Copyright (c) 2014 Cannybots. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Utilities.h"


long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
