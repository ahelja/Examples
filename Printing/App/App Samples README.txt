The App directory of the printing example code contains two sample
Carbon applications and two Printing Dialog Extensions (PDEs).

ROADMAP to the samples:
If you are looking for the absolute simplest example of bringing up the
Printing dialogs and running a sample print loop, look at the sample
code in BasicPrintLoop.

If you want to see a typical implementation of a window and menu based application
with standard printing dialogs, look at the sample code in AppUsingSheets.

If you want to use the printing system to enable Save As PDF functionality in
your application, look at AppUsingSheets.

If you want to have "Print One Copy" functionality, look at AppUsingSheets.

If you are looking for how to build and utilize Printing Dialog Extensions in your application, look at BasicPrintLoop.

If you are a PostScript centric application that generates its own PostScript code
and you need to know how to choose the LaserWriter compatibility path, look at
the sample code in BasicPrintLoop.


The directory BasicPrintLoop contains a Project Builder project and
source to build a very simple sample application that has no windows
or menus but does bring up the Page Setup dialog and the Print Dialog
and has a very basic sample print loop. It also has two example
application printing dialog extensions, one for the Page Setup
dialog and another for the Printing Dialog. This sample also
demonstrates how to select the "LaserWriter compatibility" printing
path for those PostScript centric applications that need to generate
their own PostScript code when printing to PostScript output
devices.

The directory AppUsingSheets contains a Project Builder project and a
Metrowerks 7.0 project to build a Carbon Events window based application
that uses document modal dialogs (aka sheets) when printing. This app
also demonstrates how to leverage your application print loop to enable
"Save As PDF" functionality without using the print dialog. The code
can conditionally be built as CFM and if so, when executing on Mac OS X 
v10.1 and later, the needed routines are looked up dynamically and 
called when appropriate.


