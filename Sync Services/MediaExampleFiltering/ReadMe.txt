The MediaExampleFiltering XCode project contains the following example application:

Calendar - An example app that shows how to create a schema extension and filter records. This is a modified version of the Events application from the MediaExamples project. This application is intended to be used in combination with the Events and MediaAssets example applications.

The Calendar app is identical to Events except that it adds a new entity called Calendar with an inverse to-many relationship from Calendar to Event. The Calendar app has only one Calendar record and displays only the events belonging to it. Likewise, when displaying the CalendarView, only the events that belong to the Calendar are displayed.

In addition, the Calendar app filters all of the Event records, so that only the event records belonging to the Calendar record are synced. See the Console output of Calendar to verify that filtering is working.

The Calendar app is also an example of schema extensions. The Calendar entity and to-many relationship to Event are defined in a SyncExamplesExtension.syncschema located in the application Resources folder. The original SyncExamples.syncschema needs to be installed or registered with the sync engine before using the Calendar app.

You need to install this framework, located in MediaExample, before using Calendar:

SyncExamples - Contains all the data source/entity relationship model classes, and syncing logic. This framework is used by Events, MediaAssets and Calendar.

Build the MediaExample project and this project into a common build directory.


Quick Tutorial of Calendar:

NOTE: the examples work best if you import related photos and calendars. For example, the calendar events correspond to the days you took your photos.

(1) Import and sync arbitrary events via original Events app.

(2) Import and sync more events (that correspond to some photos) via the new Calendar app (a filtering version of Events). The single Calendar object has an inverse to-many to its Event objects.

(3) Import and sync media objects via the original MediaAssets app. (MediaAssets shows ALL the Event objects in the pulldown because it is NOT filtering them.)

(4) Click Smart Events to create the relationship between some event and media objects.

(5) Sync MediaAssets again to push the relationships.

(6) Sync the Calendar app to pull relationships and test filtering. Only the changes to the events that belong to the Calendar object are pulled.

