#!/bin/sh

/usr/bin/osascript \
-e 'tell application "examples" of machine "http://www.soapware.org"' \
-e '	call soap {method name:"getStateName", parameters: { statenum: 41 }, method namespace uri: "http://www.soapware.org", SOAPAction: "/examples" }' \
-e 'end tell'
