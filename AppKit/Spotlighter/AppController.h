/*
 AppController.h

 This is the main controller for the user interface. Most of the work is done by bindings.
 
*/

#import <Cocoa/Cocoa.h>

@interface AppController : NSObject
{
    NSMetadataQuery *_query;
    NSString *_searchKey;
    BOOL _searchContent;
}

- (void)queryNote:(NSNotification *)note;

// Through the miracle of bindings, by exposing the query in the controller, we can bind to anything in the query with expressions such as "query.results". The NSArrayController in the nib file named "AllResults" binds to the query results in this matter.
- (NSMetadataQuery *)query;

// Expose searchKey so that the NSTextField for searching can easily be typed into and update the query as needed
- (NSString *)searchKey;
- (void)setSearchKey:(NSString *) value;

- (BOOL)searchContent;
- (void)setSearchContent:(BOOL)value;

@end


