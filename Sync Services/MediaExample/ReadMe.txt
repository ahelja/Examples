The MediaExample XCode project contains the following example applications and frameworks:

Events - An example app that shows how to add, modify, delete, and sync custom entity objects.  Use the Import button to import a vCal file which will create local Event objects. Use the Sync button to push the Event objects to the sync engine, and pull Media objects (created by the MediaAssets app below). Follow the tutorial below to view Media objects on a calendar.

MediaAssets - An example app for creating Media objects and linking them to Event objects. Use the Import button to import iPhoto "year" folders. Use the Sync button to push Media objects and pull Event objects. Use the Smart Events button to create relationships between Media and Event objects. Media has a to-one relationship to Event, and Event has a to-many relationship to Media. Use the Sync button to push the relationships you create.

SyncExamples - This framework is used by Events and MediaAssets. It contains shared code for defining an entity model, creating data sources, syncing, and transforming entities to/from sync records. SyncExamples also contains the sync schema for Events and MediaAssets. Cocoa document-based apps can use the EntityDocument class to create a syncable document. If you are writing your own sync methods, refer to the push and pull methods in SyncUtilties for sample code.


Quick Tutorial of Events and MediaAssets apps:

NOTE: the examples work best if you import related photos and calendars. For example, the calendar events correspond to the days you took your photos.

(1) Run Events, import a calendar, and click Sync.
(2) Run MediaAssets, import some photos (select a year folder such as "2003"), and click Sync.
(3) Click "Smart Events" to auto assign calendar events to photos, and click Sync again to push the relationships.
(4) In Events, click Sync to pull the Media objects and relationships.
(5) Click the Calendar button to view the Media objects on the calendar.


Type in this command from a shell in the MediaExample folder to install these targets:

xcodebuild install DSTROOT=/