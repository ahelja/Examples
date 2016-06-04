/* 

        IMPORTANT: This Apple software is supplied to you by Apple Computer,
        Inc. ("Apple") in consideration of your agreement to the following terms,
        and your use, installation, modification or redistribution of this Apple
        software constitutes acceptance of these terms.  If you do not agree with
        these terms, please do not use, install, modify or redistribute this Apple
        software.

        In consideration of your agreement to abide by the following terms, and
        subject to these terms, Apple grants you a personal, non-exclusive
        license, under Apple's copyrights in this original Apple software (the
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

#import "WindowController.h"
#import "SubViewController.h"

@implementation WindowController

-(id)init{
    self = [super init];
    if(self)
	{
		_enterTextNibView = nil;
		_setColorNibView = nil;
	}
    return self;
}

- (void)dealloc{
    [super dealloc];
    [_enterTextNibView release];
    [_setColorNibView release];
}

- (void)awakeFromNib{
    // Ensure that the first tab is selected.
    [multipleNibTabView selectTabViewItemWithIdentifier:@"1"];
}

-(NSView*)viewFromNibWithName:(NSString*)nibName{
    NSView * 		newView;
    SubViewController *	subViewController;
    
    subViewController = [SubViewController alloc];
    // Creates an instance of SubViewController which loads the specified nib.
    [subViewController initWithNibName:nibName andOwner:self];
    newView = [subViewController view];
    return newView;
}

- (void)tabView:(NSTabView*)tabView didSelectTabViewItem:(NSTabViewItem*)tabViewItem{
    NSString * 		nibName;
    nibName = nil;
    // The NSTabView will manage the views being displayed but without the NSTabView, you need to use removeSubview: which releases the view and you need to retain it if you want to use it again later.

    // Based on the tab selected, we load the appropriate nib and set the tabViewItem's view to the 
    // view fromt he nib.
    if([[tabViewItem identifier] isEqualToString:@"1"]){
        if(_enterTextNibView) 
            [tabViewItem setView:_enterTextNibView];
        else {
            _enterTextNibView = [[self viewFromNibWithName:@"EnterText"] retain];
            if (_enterTextNibView) {
                [tabViewItem setView:_enterTextNibView];
            }
        }
    }
    if([[tabViewItem identifier] isEqualToString:@"2"]){
        if(_setColorNibView) 
            [tabViewItem setView:_setColorNibView];
        else {
            _setColorNibView = [[self viewFromNibWithName:@"SetColor"] retain];
            if (_setColorNibView) {
                [tabViewItem setView:_setColorNibView];
            }
        }
    }
}

// This methad returns a pointer to the NSTextField on the main window.
- (id)textField{
    return displayTextHere;
}

@end