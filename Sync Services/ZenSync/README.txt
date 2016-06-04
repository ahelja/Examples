QUICK START:

A good way to get started working with this example is:

1.  Build.
2.  Open a terminal, cd to your build directory.
3.  Run:
./ZenSync --custom --dump-data
./ZenSync --sync Custom1.txt
./ZenSync --sync Custom2.txt

While running the above steps, you can monitor the contents of Custom1.txt and Custom2.txt in ~/Library/Application\ Support/SyncExamples/ZenSync to see what ZenSync is doing.

The description of these files' layout (and other information relative to this tool), can be found by running this tool with -h or --help as its option.  This will log a copious amount of useful information regarding ZenSync.


MORE INFO:

This example builds a Foundation Tool that handles the very basics of syncing.  ZenSync handles sycning of simple Calendar Data ("--stock") or its own custom Zen Objects ("--custom").  ZenSync has the ability to both create default data sets ("--dump-data"), and sync them ("--sync").

ZenSync is different from most full fledged syncing applications. Unlike your final application, ZenSync registers no alert handler, nor can it be run as an "alert tool".  Most syncing applications (including the other Sync Services examples), will register an alert mechanism, so that they can join syncs initiated by other clients syncing similar data.  This departure from a "normal" sync application was done in the interest of simplicity.