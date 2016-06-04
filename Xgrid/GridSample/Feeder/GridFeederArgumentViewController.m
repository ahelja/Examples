/*
 
 File: GridFeederArgumentViewController.m
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Computer, Inc. ("Apple") in consideration of your agreement to the
 following terms, and your use, installation, modification or
 redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use,
 install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple.  Except
 as expressly stated in this notice, no other rights or licenses, express
 or implied, are granted by Apple herein, including but not limited to
 any patent rights that may be infringed by your derivative works or by
 other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright 2002, 2005 Apple Computer, Inc., All Rights Reserved
 
*/

#import "GridFeederArgumentViewController.h"
#import "GridFeederArgumentContainerController.h"

#define STATIC_POPUPITEM_TAG 1
#define SEQUENCE_POPUPITEM_TAG 2
#define RANDOM_POPUPITEM_TAG 3

@implementation GridFeederArgumentViewController

- (id)initWithParent:(GridFeederArgumentContainerController *)parent;
{
    if (self = [super init]) {
	
        _parent = parent;
        
        [NSBundle loadNibNamed:@"ArgumentView" owner:self];
    
        [_typePopUp setAutoenablesItems:NO];
        [_typePopUp selectItemAtIndex:[_typePopUp indexOfItemWithTag:STATIC_POPUPITEM_TAG]];

        NSRect frame = [_staticTypeSubview frame];
        _currentTypeSubview = _staticTypeSubview;

        [_typePopUp setNextKeyView:_staticTypeTextField];
        [_staticTypeTextField setNextKeyView:_addButton];
        
        [_typeView addSubview:_currentTypeSubview];
        [_currentTypeSubview setFrame:frame];
    }

    return self;
}

- (void)dealloc;
{
    [_view removeFromSuperview];
    
    [_staticTypeSubview release];
    [_sequenceTypeSubview release];
    [_randomTypeSubview release];

    [_view release];

    [super dealloc];
}

- (GridFeederArgumentContainerController *)parent;
{
    return _parent;
}

- (NSView *)view;
{
    return _view;
}

- (NSView *)firstKeyView;
{
    return _typePopUp;
}

- (NSView *)lastKeyView;
{
    return _removeButton;
}

- (void)setEditable:(BOOL)editable;
{
    [_typePopUp setEnabled:editable];
    
    [_staticTypeTextField setEditable:editable];
    
    [_sequenceTypeFromTextField setEditable:editable];
    [_sequenceTypeToTextField setEditable:editable];
    [_sequenceTypeByTextField setEditable:editable];
    
    [_randomTypeSizeTextField setEditable:editable];
    [_randomTypeFromTextField setEditable:editable];
    [_randomTypeToTextField setEditable:editable];

    [_addButton setEnabled:editable];
    [_removeButton setEnabled:editable];
}

- (void)setRemoveEnabled:(BOOL)enabled;
{
    [_removeButton setEnabled:enabled];
}

- (void)setTitle:(NSString *)title;
{
    [_titleField setStringValue:title];
}

- (NSArray *)argumentStringArray;
{
    NSMutableArray *array = nil;
	
    int selectedItemTag = [[_typePopUp selectedItem] tag];

    switch (selectedItemTag) {
	
        case STATIC_POPUPITEM_TAG: {
		
            NSString *string = [_staticTypeTextField stringValue];

            if (string != nil && [string isEqualToString:@""] == NO) {
			
                array = [NSMutableArray arrayWithObject:string];
            }
            else {
			
                array = [NSMutableArray array];
            }
			
            break;
        }
        case SEQUENCE_POPUPITEM_TAG: {
		
            NSString *fromString = [_sequenceTypeFromTextField stringValue];
            NSString *toString = [_sequenceTypeToTextField stringValue];

            if (fromString == nil ||
                toString == nil ||
                [fromString isEqualToString:@""] == YES ||
                [toString isEqualToString:@""] == YES) {

                array = [NSMutableArray array];
                break;
            }
            
            long long fromLongLong = 0;
            long long toLongLong = 0;
            long long byLongLong = 0;
            
            [[NSScanner scannerWithString:[_sequenceTypeFromTextField stringValue]] scanLongLong:&fromLongLong];
            
            [[NSScanner scannerWithString:[_sequenceTypeToTextField stringValue]] scanLongLong:&toLongLong];
            
            [[NSScanner scannerWithString:[_sequenceTypeByTextField stringValue]] scanLongLong:&byLongLong];
            
            if (byLongLong < 1) {
			
                byLongLong = 1;
            }
            
            if (fromLongLong > toLongLong) {
			
                long long temp = toLongLong;
                toLongLong = fromLongLong;
                fromLongLong = temp;
            }

            long long rangeLongLong = ((toLongLong - fromLongLong + 1) / byLongLong) + 1;
            array = [NSMutableArray arrayWithCapacity:rangeLongLong];

            long long i;
            
            for (i = fromLongLong; i <= toLongLong; i += byLongLong) {
			
                NSNumber *number = [NSNumber numberWithLongLong:i];
                NSString *string = [number stringValue];

                if (string != nil) {
                    [array addObject:string];
                }
            }
			
            break;
        }
        case RANDOM_POPUPITEM_TAG: {
		
            NSString *sizeString = [_randomTypeSizeTextField stringValue];
            NSString *fromString = [_randomTypeFromTextField stringValue];
            NSString *toString = [_randomTypeToTextField stringValue];

            if (sizeString == nil ||
                fromString == nil ||
                toString == nil ||
                [sizeString isEqualToString:@""] == YES ||
                [fromString isEqualToString:@""] == YES ||
                [toString isEqualToString:@""] == YES) {

                array = [NSMutableArray array];
                break;
            }
            
            int sizeInt = [_randomTypeSizeTextField intValue];
            int fromInt = [_randomTypeFromTextField intValue];
            int toInt = [_randomTypeToTextField intValue];
            int i;

            if (sizeInt == 0) {
			
                array = [NSMutableArray array];
                break;
            }

            if (fromInt > toInt) {
			
                int temp = toInt;
                toInt = fromInt;
                fromInt = temp;
            }

            int rangeInt = (toInt - fromInt + 1);
            
            if (sizeInt > rangeInt) {
			
                sizeInt = rangeInt;
            }

            array = [NSMutableArray arrayWithCapacity:sizeInt];
            
            srandom(time(NULL));

            for (i = 0; i < sizeInt; i++) {
			
                int randomInt = (random() % rangeInt) + fromInt;

                [array addObject:[[NSNumber numberWithInt:randomInt] stringValue]];
            }
			
            break;
        }
        default: {
		
            break;
        }
    }

    return array;
}

- (IBAction)typePopUpAction:(id)sender;
{
    int selectedItemTag = [[sender selectedItem] tag];

    switch (selectedItemTag) {
	
        case STATIC_POPUPITEM_TAG: {
		
            if (_currentTypeSubview != _staticTypeSubview) {
			
                [_typeView replaceSubview:_currentTypeSubview with:_staticTypeSubview];
                _currentTypeSubview = _staticTypeSubview;
                
                [_typePopUp setNextKeyView:_staticTypeTextField];
                [_staticTypeTextField setNextKeyView:_addButton];
            }
			
            break;
        }
        case SEQUENCE_POPUPITEM_TAG: {
		
            if (_currentTypeSubview != _sequenceTypeSubview) {
			
                [_typeView replaceSubview:_currentTypeSubview with:_sequenceTypeSubview];
                _currentTypeSubview = _sequenceTypeSubview;

                [_typePopUp setNextKeyView:_sequenceTypeFromTextField];
                [_sequenceTypeByTextField setNextKeyView:_addButton];
            }
            break;
			
        }
        case RANDOM_POPUPITEM_TAG: {
		
            if (_currentTypeSubview != _randomTypeSubview) {
			
                [_typeView replaceSubview:_currentTypeSubview with:_randomTypeSubview];
                _currentTypeSubview = _randomTypeSubview;

                [_typePopUp setNextKeyView:_randomTypeSizeTextField];
                [_randomTypeToTextField setNextKeyView:_addButton];
            }
			
            break;
        }
        default: {
		
            break;
        }
    }
}
    
- (IBAction)addButtonAction:(id)sender;
{
    [[self parent] addArgument:self];
}

- (IBAction)removeButtonAction:(id)sender;
{
    [[self parent] removeArgument:self];
}

@end
