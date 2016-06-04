/*
        IMPORTANT: This Apple software is supplied to you by Apple Computer,
        Inc. ("Apple") in consideration of your agreement to the following terms,
        and your use, installation, modification or redistribution of this Apple
        software constitutes acceptance of these terms.  If you do not agree with
        these terms, please do not use, install, modify or redistribute this Apple
        software.
        
        In consideration of your agreement to abide by the following terms, and
        subject to these terms, Apple grants you a personal, non-exclusive
        license, under Apple’s copyrights in this original Apple software (the
        "Apple Software"), to use, reproduce, modify and redistribute the Apple
        Software, with or without modifications, in source and/or binary forms;
        provided that if you redistribute the Apple Software in its entirety and
        without modifications, you must retain this notice and the following text
        and disclaimers in all such redistributions of the Apple Software.
        Neither the name, trademarks, service marks or logos of Apple Computer,
        Inc. may be used to endorse or promote products derived from the Apple
        Software without specific prior written permission from Apple. Except as
        expressly stated in this notice, no other rights or licenses, express or
        implied, are granted by Apple herein, including but not limited to any
        patent rights that may be infringed by your derivative works or by other
        works in which the Apple Software may be incorporated.
        
        The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES
        NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
        IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
        PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
        ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
        
        IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
        CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
        SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
        INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
        MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
        WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
        LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY
        OF SUCH DAMAGE.  
*/

#import "SMWSecondWindowController.h"
#import "SMWAppDelegate.h"


@implementation SMWAppDelegate
/*
 * Rather than put all of the application's windows and controllers in the MainMenu nib, 
 * we separate them. MainMenu.nib is listed as the Main nib file in the Application 
 * Settings for this target, so it will be loaded at start up.
 *
 * The second window is only instantiated and shown when the user clicks the Second 
 * Window button. By delaying the creation of the second window, we improve our launch
 * times and make the MainMenu nib less complicated.
 */
 
//----------------------------------------------------------------------------------------------
 
/* 
 * This action is connected to the Second Window button. It looks to see if the window has already
 * been created. If it has, it's simply told to show. Otherwise we create a new instance of our
 * NSWindowController subclass.
 */
- (IBAction)displaySecondWindow:(id)sender
{
    if (_controller == nil) // Check to see if we've already loaded the second window controller
    {
        // If not, instantiate a new controller. The controller is told to initWithWindowNibName. The
        // new instance will use NSBundle to find and load the correct SecondWindow.nib file. When
        // the nib is loaded, the resultField outlet described in SMWSecondWindow.h will be filled.
        _controller = [[SMWSecondWindowController alloc] initWithWindowNibName:@"SecondWindow"]; 
    }
    //Now that we have a controller, tell it to show the window.
    [_controller showWindow:self];
}

//----------------------------------------------------------------------------------------------

/*
 * Although the application is closing anyway, it's a good idea to clean
 * up after ourselves.
 */
- (void)dealloc
{
    [_controller release]; 	//Release the controller we created above.
    [_answerList release]; 	//Release the array of answers we created before.
    [super dealloc];
}

// When a question is answered in the other nib, the Application controller gets the message to answer it.  The question is send to this method.
-(void)answerQuestion:(NSString *)question
{
    [[_controller window] orderOut:nil];	// The second window is ordered out of the screen.
    [_firstWindow makeKeyAndOrderFront:nil];	// Make sure the window is open and ordered to the front.
    if (!_answerList) {
        // A list of answers is loaded into an array from a plist.
        _answerList= [[NSArray arrayWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"answerFile" ofType:@"plist"]] retain];
    }
    // If no question was asked, we ask a question of our own.
    if ([question length]==0)
    {
        NSString * locDefaultQuestion = [[NSBundle bundleForClass:[self class]] localizedStringForKey:@"How hard is it to enter a question?" value:@"How hard is it to enter a question?" table:@"SMW"];
        [_question setStringValue:locDefaultQuestion];
    }
    else
    {
        [_question setStringValue:question];	// The question field is populated with the question.
    }
    // The answer field is populated with an answer selected randomly from the array.
    [_answer setStringValue:[_answerList objectAtIndex: rand() % [_answerList count]]];
}

@end
