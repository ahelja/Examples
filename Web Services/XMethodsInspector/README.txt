XMethodsInspector.app

This is a simple example of using the /Developer/Tools/WSMakeStubs
tool to integrate web services into a Cocoa application.
www.xmethods.net is a listing service for web services and provides a
programmatic API via WSDL and SOAP for listing web services.

The URL for the WSDL document is: http://www.xmethods.net/wsdl/query.wsdl

Building the project requires an active internet connection as it
fetches the WSDL from the net when it builds.  If this is a problem,
download the WSDL manually and update the WSMakeStubs build phase to
specify -file <path> instead of -url <locataion>.

Steve Zellers
Mon Apr 22 10:14:47 2002

