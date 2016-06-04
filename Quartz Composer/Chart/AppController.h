@interface AppController : NSObject
{
	IBOutlet NSTableView*		tableView;
	IBOutlet QCView*			view;
	
	NSMutableArray*				_data;
}
- (void) updateChart;
@end
