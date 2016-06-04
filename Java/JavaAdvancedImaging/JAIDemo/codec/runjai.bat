REM
REM A batch file to run the demo.
REM

set PATH=..\..\lib;%PATH%
set JARPATH=..\..\lib\jai_core.jar;..\..\lib\jai_codec.jar;..\..\lib\mlibwrapper_jai.jar
java -Xmx48m -classpath %JARPATH%;. FormatDemo ..\images\*.*
