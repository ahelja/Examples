#import <InterfaceBuilder/InterfaceBuilder.h>
#import "ClockControl.h"

@interface ClockControlPalette : IBPalette {
}
@end

@interface ClockControl (ClockControlPaletteInspector)
- (NSString *)inspectorClassName;
@end
