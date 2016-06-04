#import <Cocoa/Cocoa.h>
#import "AppointmentData.h"
#import "ClockControl.h"

@interface AppController : NSObject {
    IBOutlet NSTableView  *myTable;       // Outlet to the table view, connected in IB.
    IBOutlet NSTextField  *timeReadout;   // Outlet to the text field displaying the clock time.
    NSMutableArray        *appointments;  // An array of AppointmentData objects.
}

- (IBAction)addAppointment:(id)sender;
- (IBAction)removeSelectedAppointment:(id)sender;
- (IBAction)clockTimeChanged:(id)sender;

@end
