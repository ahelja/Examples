#include <Carbon/Carbon.h>

#include "TString.h"

#include "CFMethods.h"

void
PrintTestHeader(
	const char*	inTestName )
{
	fprintf( stderr, "=======================\n%s\n=======================\n", inTestName );
}

void
TestTString()
{
	TString			string( CFSTR("joe1") );
	TString			string2( NULL, CFSTR( "joe%d\n" ), 2 );
	//TMutableString	string3( "joe3" );
	TMutableString	string3;
	UniChar			uniChars[ 3 ] = { 0x00A2, 0x00A9, 0x00AE };
	TString			string4( uniChars, sizeof( uniChars ) / sizeof( UniChar ) );
	CFStringRef		typeDesc;
	
	PrintTestHeader( "TestTString" );

	//string = CFSTR("joe1-redo");
	
	string.Show();
	string2.Show();
	string3.Show();
	string4.Show();

	fprintf( stderr, "string3 typeID: %d\n", string3.GetTypeID() );
	fprintf( stderr, "string4.Length: %d\n", (int)string4.GetLength() );
	typeDesc = string3.CopyTypeDescription();
	fprintf( stderr, "string3 typeDescription: " );
	CFShow( typeDesc );
	CFRelease( typeDesc );
	
	string3.Append( CFSTR(" schmoe") );
	//string3 += TString( CFSTR( " Yo!" ) ) + TString( CFSTR( " Wow!" ) );
	string3.Show();
}

void
TestCFStringMethods()
{
	CFStringRef			string = CFSTR( "joe1" );
	CFStringRef			string2 = CFString::Create( NULL, CFSTR( "joe%d\n" ), 2 );
	CFMutableStringRef	string3 = CFString::CreateMutable( "joe3" );
	UniChar				uniChars[ 3 ] = { 0x00A2, 0x00A9, 0x00AE };
	CFStringRef			string4 = CFString::Create( uniChars, sizeof( uniChars ) / sizeof( UniChar ) );
	CFStringRef			typeDesc;
	
	PrintTestHeader( "TestCFStringMethods" );

	string->Release();
	string = CFSTR( "joe1-redo" );
	
	string->Show();
	string2->Show();
	string3->Show();
	string4->Show();

	fprintf( stderr, "string3 typeID: %d\n", string3->GetTypeID() );
	typeDesc = string3->CopyTypeDescription();
	fprintf( stderr, "string3 typeDescription: " );
	typeDesc->Show();
	typeDesc->Release();
	fprintf( stderr, "string4.Length: %d\n", string4->GetLength() );
	
	string3->Append( CFSTR(" schmoe") );
	//string3 += TString( CFSTR( " Yo!" ) ) + TString( CFSTR( " Wow!" ) );
	string3->Show();
}

#if 0
void
TestTArray()
{
	PrintTestHeader( "TestTArray" );
}

void
TestCFArrayMethods()
{
	PrintTestHeader( "TestCFArrayMethods" );
}

void
TestTBag()
{
	PrintTestHeader( "TestTBag" );
}

void
TestCFBagMethods()
{
	PrintTestHeader( "TestCFBagMethods" );
}

void
TestTData()
{
	PrintTestHeader( "TestTData" );
}

void
TestCFDataMethods()
{
	PrintTestHeader( "TestCFDataMethods" );
}

void
TestTDictionary()
{
	PrintTestHeader( "TestTDictionary" );
}

void
TestCFDictionaryMethods()
{
	PrintTestHeader( "TestCFDictionaryMethods" );
}

void
TestTNumber()
{
	PrintTestHeader( "TestTNumber" );
}

void
TestCFNumberMethods()
{
	PrintTestHeader( "TestCFNumberMethods" );
}

void
TestTSet()
{
	PrintTestHeader( "TestTSet" );
}

void
TestCFSetMethods()
{
	PrintTestHeader( "TestCFSetMethods" );
}

void
TestTURL()
{
	PrintTestHeader( "TestTURL" );
}

void
TestCFURLMethods()
{
	PrintTestHeader( "TestCFURLMethods" );
}
#endif

int main(
	int			argc,
	char*		argv[])
{
    OSStatus	err = noErr;

#if 0
	TestTArray();
	TestCFArrayMethods();
	
	TestTBag();
	TestCFBagMethods();
	
	TestTData();
	TestCFDataMethods();

	TestTDictionary();
	TestCFDictionaryMethods();

	TestTNumber();
	TestCFNumberMethods();

	TestTSet();
	TestCFSetMethods();

	TestTURL();
	TestCFURLMethods();
#endif

	TestTString();
	TestCFStringMethods();

	return err;
}

