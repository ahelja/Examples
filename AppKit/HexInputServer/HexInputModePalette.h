/*
 HexInputServer.h
*/

#import <AppKit/AppKit.h>

@interface HexInputModeMatrix : NSMatrix
@end

@interface HexInputModePalette : NSPanel {
    HexInputModeMatrix *_buttonMatrix;
}

- (void)buttonSelected:(id)sender;
- (void)updateModePaletteWithState:(BOOL)isHexInput;
- (void)setEnabled:(BOOL)flag;
@end
