#!/bin/sh
printenv
mkdir -p "${DERIVED_SOURCES_DIR}"
/Developer/Tools/WSMakeStubs -x ObjC -dir "${DERIVED_SOURCES_DIR}" -name XMethodsStubs -url http://www.xmethods.net/wsdl/query.wsdl
printenv
