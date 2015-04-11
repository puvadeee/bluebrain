//
//  Utilities.h
//  BrainDash
//
//  Created by Wayne Keenan on 25/10/2014.
//  Copyright (c) 2014 Cannybots. All rights reserved.
//

#ifndef BrainDash_Utilities_h
#define BrainDash_Utilities_h
#import "NSData+hex.h"


#define lowByte(v)   ((unsigned char) (v))
#define highByte(v)  ((unsigned char) (((unsigned int) (v)) >> 8))

#define radiansToDegrees(x) (180/M_PI)*x
long map(long x, long in_min, long in_max, long out_min, long out_max);


#endif
