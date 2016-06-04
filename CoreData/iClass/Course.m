/*

File: Course.m

Abstract: Implementation for NSManagedObject subclass representing a Course

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

Copyright © 2005 Apple Computer, Inc., All Rights Reserved

*/

#import "iClassAppDelegate.h"
#import "Course.h"
#import "Student.h"

@implementation Course

- (void)dealloc {
    [super dealloc];
}

- (void)awakeFromEverything {
    [self setUpRelationships];
    
    // Because courses is a fetched property on teacher, we won't be notified if the teacher is deleted.
    // We need to register for a notification to be able to notice it and nil out our teacher.
    [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(contextDidChange:) name: NSManagedObjectContextObjectsDidChangeNotification object: [self managedObjectContext]];
}

- (void)awakeFromInsert {
    [super awakeFromInsert];
    [self awakeFromEverything];
}

- (void)awakeFromFetch {
    [super awakeFromFetch];
    [self awakeFromEverything];
}

- (void)didTurnIntoFault {
    [[NSNotificationCenter defaultCenter] removeObserver: self];
}

- (void)willSave {
    [self saveStudents];
}

- (void)contextDidChange:(NSNotification *)aNotification {
    Teacher *aTeacher = [self primitiveValueForKey: @"teacher"];
    
    if (nil != aTeacher) {
        if ([aTeacher isDeleted]) {
            [self setTeacher: nil];
        }
    }
}

- (void)setTeacher:(NSManagedObject *)aTeacher {
    [self willChangeValueForKey: @"teacher"];
    [self setPrimitiveValue: aTeacher forKey: @"teacher"];
    [self didChangeValueForKey: @"teacher"];
    
    // Teacher is a to one relationship, so when the teacher relationship is set, also fill in the underlying storage
    NSString *teacherID = [[[aTeacher objectID] URIRepresentation] description];
    [self willChangeValueForKey: @"teacherURI"];
    [self setPrimitiveValue: teacherID forKey: @"teacherURI"];
    [self didChangeValueForKey: @"teacherURI"];
}

- (void)setTeacherID:(NSNumber *)anID {
    @throw [NSException exceptionWithName: @"InvalidStateChangeException" reason: @"teacherID must be set indirectly by setting a teacher." userInfo: nil];
}

-(void)saveStudents {
    // turn our students to many relationship into an NSData containing the studentIDs for all of the students
    // enrolled in this class, then put that in my studentIDs attribute
    NSSet *localStudents = [self primitiveValueForKey: @"students"];
    
    if ([localStudents count] == 0) {
        [self setPrimitiveValue: nil forKey: @"studentURIs"];
    } else {
        NSMutableArray *localIDs = [[NSMutableArray alloc] initWithCapacity: [localStudents count]];
        Student *student = nil;
        NSEnumerator *objectEnumerator = [localStudents objectEnumerator];
        
        while (student = (Student *)[objectEnumerator nextObject]) {
            [localIDs addObject: [[student objectID] URIRepresentation]];
        }
        
        NSData *aData = [NSArchiver archivedDataWithRootObject: localIDs];
        [self setPrimitiveValue: aData forKey: @"studentURIs"];
    }
}

- (void)setUpRelationships {
    NSString *teacherID = [self primitiveValueForKey: @"teacherURI"];
    
    // Find and fetch the teacher whose id corresponds to my teacherID, then set that object as the 
    // destination of my teacher relationship
    if (nil != teacherID) {
        NSManagedObjectContext *context = [self managedObjectContext];
        NSPersistentStoreCoordinator *coordinator = [context persistentStoreCoordinator];
        
        NSManagedObjectID *teacherOID = [coordinator managedObjectIDForURIRepresentation: [NSURL URLWithString: teacherID]];
        
        NSManagedObject *myTeacher = [context objectWithID: teacherOID];
        [self setTeacher: myTeacher];
    }
    
    NSData *_studentIDData = [self primitiveValueForKey: @"studentURIs"];
    
    // Deserialize my _studentIDData attribute to get the studentIDs for the students enrolled in this class
    // Then create those students and populate the relationship
    if (nil != _studentIDData) {
        NSMutableArray *localURIS = [NSUnarchiver unarchiveObjectWithData: _studentIDData];
        int i = 0, count = [localURIS count];
        NSMutableSet *localStudents = [self primitiveValueForKey: @"students"];
        
        NSManagedObjectContext *context = [self managedObjectContext];
        NSPersistentStoreCoordinator *coordinator = [context persistentStoreCoordinator];
        
        for ( ; i < count ; i++) {
            NSManagedObjectID *oid = [coordinator managedObjectIDForURIRepresentation: [localURIS objectAtIndex: i]];
            NSManagedObject *managedObject = [context objectWithID: oid];
            [localStudents addObject: managedObject];
        }
        
    }        
    
}

@end
