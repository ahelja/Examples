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
//
//  strings.h
//  ZenSync
//
//  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
//

#define DEFAULT_CUSTOM_COMMENT_LINE @"# Run this tool with -h or --help for an explanation of this file\n\
# The fields for records are RecordId, Name, Team, Time1, Time2\n\
# As you'll see if you read the -h text, you'll want to remove the C: line for the first sync to a blank sync server"
#define DEFAULT_CUSTOM_TAG_LINE @"T: "
#define DEFAULT_CUSTOM_CHANGES @"C: "
#define DEFAULT_CUSTOM_RECORDS @"\
R: 00001,Paul,Kegs,1:04,59\n\
R: 00002,Scott,Owls,55,1:03\n\
R: 00003,Maggie,Sharks,53,1:00\n\
R: 00004,Kelsie,Padres,1:02,1:01\n\
R: 00005,Ray,Engineers,1:10,1:13\n\
R: 00006,Patricia,Bombers,57,1:02\n\
R: 00007,Brian,Knolls Gang,1:12,1:02"

#define DEFAULT_STOCK_COMMENT_LINE @"# Run this tool with -h or --help for an explanation of this file\n\
# The fields for calendars are RecordId, title\n\
# The fields for events are RecordId, summary, start date, end date, calendar\n\
# As you'll see if you read the -h text, you'll want to remove the C: line for the first sync to a blank sync server"
#define DEFAULT_STOCK_TAG_LINE @"T: "
#define DEFAULT_STOCK_CHANGES_1 @"CC: "
#define DEFAULT_STOCK_CHANGES_2 @"CE: "
#define DEFAULT_STOCK_RECORDS @"\
RC: 00001,Test Calendar\n\
RC: 00002,Test Calendar 2\n\
RE: 00003,Mothers Day,3pm today,4pm today,00001\n\
RE: 00004,Sunday,10am sunday,8pm sunday,00002"

#define DEFAULT_CUSTOM_SCHEMA @"~/Library/Application Support/SyncExamples/ZenSync/ZenCustomSchema.syncschema"
#define DEFAULT_CUSTOM_DATA_1 @"~/Library/Application Support/SyncExamples/ZenSync/Custom1.txt"
#define DEFAULT_CUSTOM_DATA_2 @"~/Library/Application Support/SyncExamples/ZenSync/Custom2.txt"
#define DEFAULT_STOCK_DATA_1 @"~/Library/Application Support/SyncExamples/ZenSync/Stock1.txt"
#define DEFAULT_STOCK_DATA_2 @"~/Library/Application Support/SyncExamples/ZenSync/Stock2.txt"
#define DEFAULT_WORKING_DIR @"~/Library/Application Support/SyncExamples/ZenSync"
#define DEFAULT_CUSTOM_CLIENT_PLIST @"~/Library/Application Support/SyncExamples/ZenSync/CustomClientDescription.plist"
#define DEFAULT_STOCK_CLIENT_PLIST @"~/Library/Application Support/SyncExamples/ZenSync/StockClientDescription.plist"
#define DEFAULT_CLIENT_IMAGE @"~/Library/Application Support/SyncExamples/ZenSync/ZenSyncClient.png"

#define HELP_TEXT @"\
This tool will show the basics of the SyncServices functionality.\n\
\n\
The arguments consist of the following:\n\
-h or --help\n\
  print this message and exit\n\
--custom\n\
--stock\n\
  These arguments indicate whether the tool will use a custom schema, or a stock\n\
  schema, respectively. The custom schema is represented by a file at\n\
  ~/Library/Application Support/SyncExamples/ZenSync/CustomSchema.plist\n\
  and is created when compiling this tool.  When using a stock schema, this\n\
  program will use the contact schema, defined in\n\
  /System/Library/SyncServices/Schemas/Contacts.syncschema/Contents/Resources/Schema.plist\n\
  If neither argument is supplied, the tool will assume --custom\n\
--dump-data\n\
  This argument will instruct the tool to write two sample data stores.  If\n\
  combined with --custom, these files will correspond to the custom schema,\n\
  and be written to\n\
  ~/Library/Application Support/SyncExamples/ZenSync/Custom[1|2].txt\n\
  If combined with --stock, these files will correspond to the contacts schema,\n\
  and be written to\n\
  ~/Library/Application Support/SyncExamples/ZenSync/Stock[1|2].txt\n\
  In each case, the *1.txt file should be synced first, as it's written to slow\n\
  sync data into the server.\n\
--sync FILE\n\
  This argument will instruct the tool to do a sync, using FILE as its data store.\n\
  The FILE argument should be the name of a file in\n\
  ~/Library/Application Support/SyncExamples/ZenSync/\n\
  This must be used with either the --custom or --stock argument, to indicate which\n\
  schema the file's data corresponds to.\n\
--refresh FILE\n\
  This argument will instruct the tool to do a refresh sync, using FILE as its (write only) data store.\n\
  The FILE argument should be the name of a file in\n\
  ~/Library/Application Support/SyncExamples/ZenSync/\n\
  This must be used with either the --custom or --stock argument, to indicate which\n\
  schema the file's data corresponds to.  This overwrites the file with contents from the truth\n\
\n\
Layout of the Custom data files:\n\
\n\
  The data written during --dump-data represents the ZenSync's data.  Lines beginning\n\
  with a # are comments.  You may edit the file as you see fit.  In fact, hand editing\n\
  the file, followed by running this tool with --sync is the intended usage.\n\
  Each non-comment line must start with \"T: \", \"C: \", or \"R: \", which\n\
  stand for Tag, Change, and Record, respectively. The order doesn't matter.\n\
\n\
  The lines will take the following form:\n\
  T: Tag_number\n\
  C: [add all_fields_(no_record_id)_with_comma_separators | delete recordId | modify recordId [field,new-value]*]\n\
  C: recordId new_values_for_all_fields_(no_record_id)\n\
  R: recordId,all_fields_with_comma_separators\n\
\n\
  The particular fields can be determined by reading the schema files for the custom\n\
  and stock schemas.  You may only have 1 Tag line (the value of which cannot be -1)\n\
  This Tag line will be incremented on every sync where data is pushed (more on that\n\
  in a second), and serves as a version indicator.\n\
\n\
  There are three syncing modes employed by this tool.  The mode used will be determined\n\
  by the state of C: lines in the data store.  If the file being used has no C: lines, a\n\
  slow sync will be used.  Each record in the file will be pushed to the sync server, and\n\
  any changes received will be written to the file.  If the file has one C: line that\n\
  has no data following the C: , it will be treated as a no-push case.  In this case, no\n\
  data will be written to the server, but all changes received from the server will be\n\
  written to the file.  Think of it as a read-only sync.  In the case where there is at\n\
  least one C: line with data, the change indicated by those lines will be pushed to the\n\
  sync server.  If the form is recordId,[add/delete/modify], the change will be pushed as\n\
  an ISyncChange.  If the form is a recordId followed by new data, the entire record will\n\
  be pushed to the sync server\n\
\n\
Layout of the Stock data files:\n\
\n\
  The data written during --dump-data represents the ZenSync's data.  Lines beginning with\n\
  a # are comments.  You may edit the file as you see fit.  In fact, hand editing the file,\n\
  followed by running this tool with --sync is the intended usage.\n\
  Each non-comment line must start with \"T: \", \"CC: \", \"CE: \", \"RC: \", or \"RE: \",\n\
  which stand for Tag, ChangeCalendar, ChangeEvent, RecordCalendar, and RecordEvent,\n\
  respectively.  The order doesn't matter.\n\
\n\
  The lines will take the following form:\n\
  T: Tag_number\n\
  CC: [add all_fields_(no_record_id)_with_comma_separators | delete recordId | modify recordId [field,new-value]*]\n\
  CC: recordId new_values_for_all_fields_(no_record_id)\n\
  CE: [add all_fields_(no_record_id)_with_comma_separators | delete recordId | modify recordId [field,new-value]*]\n\
  CE: recordId new_values_for_all_fields_(no_record_id)\n\
  RC: recordId,all_fields_with_comma_separators\n\
  RE: recordId,all_fields_with_comma_separators\n\
\n\
  The particular fields can be determined by reading the schema files for the custom and\n\
  stock schemas You may only have 1 Tag line (the value of which cannot be -1).  This Tag\n\
  line will be incremented on every sync where data is pushed (more on that in a second),\n\
  and serves as a version indicator.\n\
\n\
  There are three syncing modes employed by this tool.  The mode used will be determined by\n\
  the state of CC: and CE: lines in the data store.  The stock schema involves syncing each\n\
  entity type separately.  Therefore, it the file being used has no CC: lines, a slow sync\n\
  will be used for calendars.  If the file has no CE: lines, a slow sync will be used for\n\
  events.  In a slow sync, each record in the file will be pushed to the sync server, and\n\
  any changes received will be written to the file.  If the file has one change line for a\n\
  particular entity that has no data following the CC: or CE:, it will be treated as a no-push\n\
  case.  In this case, no data for that entity type will be written to the server, but all\n\
  changes received from the server will be written to the file.  Think of it as a read-only\n\
  sync.  In the case where there is at least one change line with data for a particular\n\
  entity type, the change indicated by those lines will be pushed to the sync server.  If\n\
  the form is recordId,[add/delete/modify], the change will be pushed as a ISyncChange.\n\
  If the formis a recordId followed by new data, the entire record will be pushed to the\n\
  sync server\n"
