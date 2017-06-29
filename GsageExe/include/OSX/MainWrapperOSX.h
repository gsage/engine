#ifndef _MainWrapperOSX_H_
#define _MainWrapperOSX_H_
#
/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2016 Artem Chernyshev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#if GSAGE_PLATFORM == GSAGE_APPLE
#ifndef nil
#define nil NULL
#endif
#include <MacTypes.h>
#include <QuartzCore/CVDisplayLink.h>
#include <AppKit/NSWindow.h>
#include "CoreFoundation/CoreFoundation.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>
{
    NSTimer *mTimer;
    Gsage::GsageFacade *mFacade;

    NSDate *mDate;
    NSTimeInterval mLastFrameTime;
    CVDisplayLinkRef mDisplayLink; //display link for managing rendering thread
}

- (id)initWithClass:(Gsage::GsageFacade*)facade;
- (void)go;
- (void)update:(id)sender;
- (void)shutdown;
@property (retain, atomic) NSTimer *mTimer;
@property (nonatomic) NSTimeInterval mLastFrameTime;
@property Gsage::GsageFacade *mFacade;

@end

@implementation AppDelegate

@synthesize mTimer;
@dynamic mLastFrameTime;
@synthesize mFacade;

- (id) initWithClass:(Gsage::GsageFacade*)facade
{
  self = [super init];
  if (self) {
    self.mFacade = facade;
  }
  return self;
}

- (NSTimeInterval)mLastFrameTime
{
  return mLastFrameTime;
}

- (void)setLastFrameTime:(NSTimeInterval)frameInterval
{
  // Frame interval defines how many display frames must pass between each time the
  // display link fires. The display link will only fire 30 times a second when the
  // frame internal is two on a display that refreshes 60 times a second. The default
  // frame interval setting of one will fire 60 times a second when the display refreshes
  // at 60 times a second. A frame interval setting of less than one results in undefined
  // behavior.
  if (frameInterval >= 1)
  {
    mLastFrameTime = frameInterval;
  }
}

- (void)go {
  NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
  mLastFrameTime = 1;
  mTimer = [[NSTimer timerWithTimeInterval: 0.001 target:self selector:@selector(update:) userInfo:self repeats:true] retain];
  [[NSRunLoop currentRunLoop] addTimer:mTimer forMode: NSDefaultRunLoopMode];
  [[NSRunLoop currentRunLoop] addTimer:mTimer forMode: NSEventTrackingRunLoopMode]; // Ensure timer fires during resize
  [pool release];
}

- (void)applicationDidFinishLaunching:(NSNotification *)application {
  mLastFrameTime = 1;
  mTimer = nil;

  [self go];
}

- (void)shutdown {
  if(mDisplayLink)
  {
    //CVDisplayLinkStop(mDisplayLink);
    //CVDisplayLinkRelease(mDisplayLink);
    mDisplayLink = nil;
  }

  [NSApp terminate:nil];
}

- (void)update:(id)sender
{
  if(!mFacade->update())
  {
    if(mTimer)
    {
      [mTimer invalidate];
      mTimer = nil;
    }

    [NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
  }
}

@end

#endif
#endif
