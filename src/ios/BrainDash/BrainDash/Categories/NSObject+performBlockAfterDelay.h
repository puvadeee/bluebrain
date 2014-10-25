//
//  NSObject+performBlockAfterDelay.h
//  BrainDash
//
//  Created by Wayne Keenan on 06/06/2014.
//  Copyright (c) 2014 CannyBots. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSObject (performBlockAfterDelay)


- (void) performBlock: (dispatch_block_t) block
           afterDelay: (NSTimeInterval) delay;

@end
