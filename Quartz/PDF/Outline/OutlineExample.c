/* Extract the outline from a PDF file.
 * Author: Derek B Clegg
 * 21 March 2003
 *
 * Copyright (c) 2003-2004 Apple Computer, Inc.
 * All rights reserved.
 */

/* IMPORTANT: This Apple software is supplied to you by Apple Computer,
 * Inc. ("Apple") in consideration of your agreement to the following
 * terms, and your use, installation, modification or redistribution of
 * this Apple software constitutes acceptance of these terms.  If you do
 * not agree with these terms, please do not use, install, modify or
 * redistribute this Apple software.
 *
 * In consideration of your agreement to abide by the following terms, and
 * subject to these terms, Apple grants you a personal, non-exclusive
 * license, under Apple's copyrights in this original Apple software (the
 * "Apple Software"), to use, reproduce, modify and redistribute the Apple
 * Software, with or without modifications, in source and/or binary forms;
 * provided that if you redistribute the Apple Software in its entirety and
 * without modifications, you must retain this notice and the following
 * text and disclaimers in all such redistributions of the Apple Software.
 * Neither the name, trademarks, service marks or logos of Apple Computer,
 * Inc. may be used to endorse or promote products derived from the Apple
 * Software without specific prior written permission from Apple. Except as
 * expressly stated in this notice, no other rights or licenses, express or
 * implied, are granted by Apple herein, including but not limited to any
 * patent rights that may be infringed by your derivative works or by other
 * works in which the Apple Software may be incorporated.
 *
 * The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 * MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 * THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 * OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 *
 * IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 * MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 * AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 * STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE. */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ApplicationServices/ApplicationServices.h>

static void set_program_name(const char *);
static CFURLRef get_url(const char *);
static void print_outline(CGPDFDocumentRef, FILE *);

static const char *program_name = NULL;

static const char usage_string[] =
  "Extract the outline from a PDF file.\n"
  "Usage: %s [OPTIONS] FILE\n"
  "Options:\n"
  "--help                 Print this usage message.\n";

static const struct option long_options[] = {
    {"help", no_argument, 0, '?'},
    {0},
};

static const char short_options[] = "?";

static void
usage(void)
{
    fprintf(stderr, usage_string, program_name);
}

/* Extract the outline from a PDF file specified on the command-line. */

int
main(int argc, char **argv)
{
    int optc;
    CFURLRef url;
    CGPDFDocumentRef document;
    const char *filename, *password;
    char buffer[128];
    
    set_program_name(argv[0]);

    while (1) {
	optc = getopt_long(argc, argv, short_options, long_options, 0);
	if (optc == EOF)
	    break;
	switch (optc) {
	case '?':
	    usage();
	    return EXIT_SUCCESS;
	    break;
	default:
	    fprintf(stderr, "%s: invalid option: %c.\n", program_name, optc);
	    usage();
	    return EXIT_FAILURE;
	}
    }
    
    switch (argc - optind) {
    case 1:
	filename = argv[optind];
	break;
    default:
	fprintf(stderr, "%s: invalid number of arguments.\n", program_name);
	usage();
	return EXIT_FAILURE;
    }

    url = get_url(filename);
    if (url == NULL)
	return EXIT_FAILURE;

    document = CGPDFDocumentCreateWithURL(url);
    if (document == NULL) {
	fprintf(stderr, "%s: can't open `%s'.\n", program_name, filename);
	CFRelease(url);
	return EXIT_FAILURE;
    }
    CFRelease(url);
    
    if (CGPDFDocumentIsEncrypted(document)) {
	if (!CGPDFDocumentUnlockWithPassword(document, "")) {
	    printf("Enter password: ");
	    fflush(stdout);
	    password = fgets(buffer, sizeof(buffer), stdin);
	    if (password != NULL) {
		buffer[strlen(buffer) - 1] = '\0';
		if (!CGPDFDocumentUnlockWithPassword(document, password))
		    fprintf(stderr, "%s: invalid password.\n", program_name);
	    }
	}
	if (!CGPDFDocumentIsUnlocked(document)) {
	    fprintf(stderr, "%s: can't unlock `%s'.\n", program_name,
		    filename);
	    CGPDFDocumentRelease(document);
	    return EXIT_FAILURE;
	}
    }

    print_outline(document, stdout);

    CGPDFDocumentRelease(document);

    return EXIT_SUCCESS;
}

static void
set_program_name(const char *p)
{
    char *s;

    s = strrchr(p, '/');
    program_name = (s != 0) ? (s + 1) : p;
}

static CFURLRef
get_url(const char *filename)
{
    CFURLRef url;
    CFStringRef path;

    path = CFStringCreateWithCString(NULL, filename, kCFStringEncodingUTF8);
    if (path == NULL) {
	fprintf(stderr, "%s: can't create CFString.\n", program_name);
	return NULL;
    }

    url = CFURLCreateWithFileSystemPath(NULL, path, kCFURLPOSIXPathStyle, 0);
    CFRelease(path);
    if (url == NULL) {
	fprintf(stderr, "%s: can't create CFURL.\n", program_name);
	return NULL;
    }

    return url;
}

#define regular    0
#define italic     1
#define bold       2
#define boldItalic 3

static bool
lookup_numbers(CGPDFDictionaryRef dict, const char *key, float *values,
	       size_t count)
{
    size_t k;
    CGPDFReal v;
    CGPDFArrayRef array;

    if (!CGPDFDictionaryGetArray(dict, key, &array))
	return false;

    if (CGPDFArrayGetCount(array) != count)
	return false;

    for (k = 0; k < count; k++) {
	if (!CGPDFArrayGetNumber(array, k, &v))
	    return false;
	values[k] = v;
    }

    return true;
}

static void
print_item(FILE *file, int indent, CFStringRef title, bool isOpen,
	   const float color[], int style)
{
    int k;
    char buffer[1024];

    for (k = 0; k < indent; k++)
	fprintf(file, " ");

    CFStringGetCString(title, buffer, sizeof(buffer), kCFStringEncodingUTF8);
    fprintf(file, "%s", buffer);

    if (!isOpen)
	fprintf(file, " <closed>");

    if (color[0] != 0 || color[1] != 0 || color[2] != 0)
	fprintf(file, " <color: %g %g %g>", color[0], color[1], color[2]);

    switch (style) {
    case regular:
	break;
    case italic:
	fprintf(file, " <style: Italic>");
	break;
    case bold:
	fprintf(file, " <style: Bold>");
	break;
    case boldItalic:
	fprintf(file, " <style: BoldItalic>");
	break;
    }
    fprintf(file, "\n");
}

static void
print_outline_items(FILE *file, int indent, CGPDFDocumentRef document,
		    CGPDFDictionaryRef outline)
{
    int style;
    bool isOpen;
    float color[3];
    CGPDFStringRef string;
    CGPDFDictionaryRef first;
    CGPDFInteger count, flags;
    CFStringRef title;

    if (document == NULL || outline == NULL)
	return;

    do {
	title = NULL;
	if (CGPDFDictionaryGetString(outline, "Title", &string))
	    title = CGPDFStringCopyTextString(string);

	isOpen = true;
	if (CGPDFDictionaryGetInteger(outline, "Count", &count))
	    isOpen = (count < 0) ? false : true;

	if (!lookup_numbers(outline, "C", color, 3))
	    color[0] = color[1] = color[2] = 0;

	style = regular;
	if (CGPDFDictionaryGetInteger(outline, "F", &flags)) {
	    switch (flags & 3) {
	    case 1:
		style = italic;
		break;
	    case 2:
		style = bold;
		break;
	    case 3:
		style = boldItalic;
		break;
	    default:
		style = regular;
		break;
	    }
	}

	print_item(file, indent, title, isOpen, color, style);

	if (CGPDFDictionaryGetDictionary(outline, "First", &first))
	    print_outline_items(file, indent + 2, document, first);

    } while (CGPDFDictionaryGetDictionary(outline, "Next", &outline));
}

static void
print_outline(CGPDFDocumentRef document, FILE *file)
{
    CGPDFDictionaryRef catalog, outline, first;

    if (document == NULL || file == NULL)
	return;

    catalog = CGPDFDocumentGetCatalog(document);
    if (!CGPDFDictionaryGetDictionary(catalog, "Outlines", &outline))
	return;

    if (!CGPDFDictionaryGetDictionary(outline, "First", &first))
	return;

    print_outline_items(file, 0, document, first);
}
