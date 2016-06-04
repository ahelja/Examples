#!/bin/sh

/usr/bin/osascript \
-e 'tell application "RPC2" of machine "http://betty.userland.com/"' \
-e 'call xmlrpc { method name: "examples.getStateName", parameters: { 41 } }' \
-e 'end tell'
