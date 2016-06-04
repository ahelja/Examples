
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <pwd.h>
#include <stdbool.h>

#include <Security/AuthorizationTags.h>
#include <Security/AuthorizationPlugin.h>

#include "dir_svc_client.h"
#include "krb5-auth.h"

typedef enum { login, authenticate, authnoverify } KerberosMode;

typedef struct PluginRef
{
    const AuthorizationCallbacks *callbacks;
} PluginRef;

typedef enum MechanismId
{
    kMechNone,
    kMechKerberosAuthenticate,
	kMechKerberosAuthenticateNoVerify,
	kMechKerberosLogin
} MechanismId;

typedef struct MechanismRef
{
    const PluginRef *plugin;
    AuthorizationEngineRef engine;
    MechanismId mechanismId;
} MechanismRef;

/**
 * Terrible heuristic, but since we're only applying it to UTF-8 encoded values we're okay
 */
static char *fromNullTerminatedAuthValue(const AuthorizationValue *authvalue)
{
	char *value = NULL;
	
	if (authvalue->data && authvalue->length)
	{
		size_t allocLength = authvalue->length;
		if ( *(uint8_t*)(authvalue->data + authvalue->length - 1) != '\0' )
			allocLength++;
		
		value = calloc(1, allocLength);
		
		if (value)
		{
			memcpy(value, authvalue->data, authvalue->length);
			return value;
		}
	}
	
	return NULL;
}

static bool invoke(MechanismRef *mechanism, int mode)
{
	bool verifyKDC = (mode == authenticate); // only in this mode require kdc to be authenticated
	bool successfulAuthentication = false;
	char *username = NULL, *password = NULL, *principal = NULL;
	
	do 
	{
		const AuthorizationValue *usernameAuthValue, *passwordAuthValue;
		AuthorizationContextFlags contextFlags;
		
	    if (mechanism->plugin->callbacks->GetContextValue(mechanism->engine, kAuthorizationEnvironmentUsername, &contextFlags, &usernameAuthValue))
			break;
		
	    if (mechanism->plugin->callbacks->GetContextValue(mechanism->engine, kAuthorizationEnvironmentPassword, &contextFlags, &passwordAuthValue))
			break;
			
		// these values shouldn't have a terminating NULL but they do
		username = fromNullTerminatedAuthValue(usernameAuthValue);
		password = fromNullTerminatedAuthValue(passwordAuthValue);

		// pass qualified principal if needed
		successfulAuthentication = (0 == KLoginPrincipal(principal, username, password, verifyKDC));

		// if we require authentication
		if ( ((mode == authenticate) || (mode == authnoverify))
			&& !successfulAuthentication)
		{
			// fallback and do DS auth
			successfulAuthentication = (eDSNoErr == checkpw(username, password));
		}

	} while (0);
	
	if (username)
		free(username);
		
	if (password)
	{
		memset(password, '\0', strlen(password));
		free(password);
	}
	
	// we don't decide whether the user can log in
	if (mode == login)
		return kAuthorizationResultAllow;

	// verify successful kerberos authentication
	if (successfulAuthentication)
	{
		struct passwd *user = getpwnam(username);
		if (user)
		{
			uid_t cred_uid = user->pw_uid;
			gid_t cred_gid = user->pw_gid;
			endpwent();

			AuthorizationContextFlags contextFlags = kAuthorizationContextFlagExtractable;
			AuthorizationValue uidAuthValue = { sizeof(cred_uid), &cred_uid };
			mechanism->plugin->callbacks->SetContextValue(mechanism->engine, "uid", contextFlags, &uidAuthValue);
			AuthorizationValue gidAuthValue = { sizeof(cred_gid), &cred_gid };
			mechanism->plugin->callbacks->SetContextValue(mechanism->engine, "gid", contextFlags, &gidAuthValue);

		}
		
		return kAuthorizationResultAllow;
	}

	return kAuthorizationResultDeny;
}


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

    mechanism->plugin = plugin;
    mechanism->engine = inEngine;

    if (!strcmp(mechanismId, "none"))
        mechanism->mechanismId = kMechNone;
    else if (!strcmp(mechanismId, "authenticate"))
        mechanism->mechanismId = kMechKerberosAuthenticate;
    else if (!strcmp(mechanismId, "authenticate-noverify"))
        mechanism->mechanismId = kMechKerberosAuthenticateNoVerify;
    else if (!strcmp(mechanismId, "login"))
        mechanism->mechanismId = kMechKerberosLogin;
    else
        return errAuthorizationInternal;

    *outMechanism = mechanism;

    return 0;
}

static OSStatus mechanismInvoke(AuthorizationMechanismRef inMechanism)
{
    MechanismRef *mechanism = (MechanismRef *)inMechanism;
	AuthorizationResult result = kAuthorizationResultDeny;

    switch (mechanism->mechanismId)
    {
		case kMechNone:
			result = kAuthorizationResultAllow;
			break;
		case kMechKerberosAuthenticate:
			result = invoke(inMechanism, authenticate);
			break;
		case kMechKerberosAuthenticateNoVerify:
			result = invoke(inMechanism, authnoverify);
			break;
		default:
			return errAuthorizationInternal;
    }

    return mechanism->plugin->callbacks->SetResult(mechanism->engine, result);
}

/**
 *   Since SetResult() is called within Invoke(), we don't have to 
 *   deal with being interrupted.
 */
static OSStatus mechanismDeactivate(AuthorizationMechanismRef inMechanism)
{
    return 0;
}


/**
 *   Clean up resources.
 */
static OSStatus mechanismDestroy(AuthorizationMechanismRef inMechanism)
{
    MechanismRef *mechanism = (MechanismRef *)inMechanism;
    free(mechanism);

    return 0;
}


/**
 *  The interface our plugin advertises.  Notice that it uses a constant 
 *  to specify which interface version we compiled against.
 */
AuthorizationPluginInterface pluginInterface =
{
    kAuthorizationPluginInterfaceVersion, //UInt32 version;
    pluginDestroy,
    mechanismCreate,
    mechanismInvoke,
    mechanismDeactivate,
    mechanismDestroy
};


/**
 *  Entry point for all plugins.  Plugin and the host loading it exchange interfaces.
 *  Normally you'd allocate resources shared amongst all mechanisms here.
 *  When a plugin is created it may not necessarily be used, so be lazy.
 */
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
