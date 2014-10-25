//
//  NSObject+performBlockAfterDelay.m
//  BrainDash
//
//  Created by Wayne Keenan on 06/06/2014.
//  Copyright (c) 2014 CannyBots. All rights reserved.
//

#import "NSObject+performBlockAfterDelay.h"

@implementation NSObject (performBlockAfterDelay)

- (void) performBlock: (dispatch_block_t) block afterDelay: (NSTimeInterval) delay
{
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, delay * NSEC_PER_SEC), dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                   block);
}

@end
