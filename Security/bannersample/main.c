//
// bannersample - simple example of GUI plugin for auth api
//

#include <Security/Authorization.h>
#include <Security/AuthorizationTags.h>
#include <Security/AuthorizationPlugin.h>

#include <stdlib.h>
#include <string.h>
#include <syslog.h>

OSStatus initializeBanner(AuthorizationMechanismRef inMechanism, int modal);
OSStatus setResult(AuthorizationMechanismRef inMechanism);

typedef struct PluginRef
{
    const AuthorizationCallbacks *callbacks;
} PluginRef;

typedef enum MechanismId
{
    kMechNone,
    kMechTest,
	kMechModalTest
} MechanismId;

typedef struct MechanismRef
{
    const PluginRef *plugin;
    AuthorizationEngineRef engine;
    MechanismId mechanismId;
} MechanismRef;


static OSStatus pluginDestroy(AuthorizationPluginRef inPlugin)
{
    PluginRef *plugin = (PluginRef *)inPlugin;
    free(plugin);
    return 0;
}

static OSStatus mechanismCreate(AuthorizationPluginRef inPlugin,
        AuthorizationEngineRef inEngine,
        AuthorizationMechanismId mechanismId,
        AuthorizationMechanismRef *outMechanism)
{
    const PluginRef *plugin = (const PluginRef *)inPlugin;
    MechanismRef *mechanism = calloc(1, sizeof(MechanismRef));

	// Enable this to allow time to attach to SecurityAgent with gdb
//	sleep(20);

    mechanism->plugin = plugin;
    mechanism->engine = inEngine;
    /* Check that we support the requested mechanismId. */
    if (!strcmp(mechanismId, "none"))
        mechanism->mechanismId = kMechNone;
    else if (!strcmp(mechanismId, "test"))
        mechanism->mechanismId = kMechTest;
    else if (!strcmp(mechanismId, "modaltest"))
        mechanism->mechanismId = kMechModalTest;
    else
        return errAuthorizationInternal;

    *outMechanism = mechanism;

    return 0;
}

static OSStatus mechanismInvoke(AuthorizationMechanismRef inMechanism)
{
    MechanismRef *mechanism = (MechanismRef *)inMechanism;
    OSStatus status;

    switch (mechanism->mechanismId)
    {
    case kMechNone:
        break;
    case kMechTest:
        status = initializeBanner(inMechanism,false);
        if (status)
            return status;
		// In the warning banner case, we return good immediately so that
		// the loginwindow UI will show.
		setResult(inMechanism);
        break;
	case kMechModalTest:
        status = initializeBanner(inMechanism,true);
        if (status)
            return status;
		// Usually a UI plugin will set the result only when the user has
		// authenticated. In this sample the user "authenticates" when they
		// click the OK button
		break;
    default:
        return errAuthorizationInternal;
    }

    return noErr;
}

static OSStatus mechanismDeactivate(AuthorizationMechanismRef inMechanism)
{
    return 0;
}

static OSStatus mechanismDestroy(AuthorizationMechanismRef inMechanism)
{
    MechanismRef *mechanism = (MechanismRef *)inMechanism;
	finalizeBanner(inMechanism);
    free(mechanism);

    return 0;
}

AuthorizationPluginInterface pluginInterface =
{
    kAuthorizationPluginInterfaceVersion, //UInt32 version;
    pluginDestroy,
    mechanismCreate,
    mechanismInvoke,
    mechanismDeactivate,
    mechanismDestroy
};

OSStatus AuthorizationPluginCreate(const AuthorizationCallbacks *callbacks,
    AuthorizationPluginRef *outPlugin,
    const AuthorizationPluginInterface **outPluginInterface)
{
    PluginRef *plugin = calloc(1, sizeof(PluginRef));

    plugin->callbacks = callbacks;
    *outPlugin = (AuthorizationPluginRef) plugin;
    *outPluginInterface = &pluginInterface;
    return 0;
}

OSStatus setResult(AuthorizationMechanismRef inMechanism)
{
    MechanismRef *mechanism = (MechanismRef *)inMechanism;

	if (!mechanism)
        return errAuthorizationInternal;
		
    return mechanism->plugin->callbacks->SetResult(mechanism->engine, kAuthorizationResultAllow);
}


/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation,
 modification or redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.

 In consideration of your agreement to abide by the following terms, and subject to these
 terms, Apple grants you a personal, non-exclusive license, under Apple's copyrights in
 this original Apple software (the "Apple Software"), to use, reproduce, modify and
 redistribute the Apple Software, with or without modifications, in source and/or binary
 forms; provided that if you redistribute the Apple Software in its entirety and without
 modifications, you must retain this notice and the following text and disclaimers in all
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your
 derivative works or by other works in which the Apple Software may be incorporated.

 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES,
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
          OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE,
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


