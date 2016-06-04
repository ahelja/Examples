
#include "krb5-auth.h"
#include <pwd.h>
#include <unistd.h>

KLStatus KLoginPrincipal(const char *principal, const char *username, const char *password, bool verifykdc)
{
    KLStatus      err = 0;
    krb5_context  context = NULL;
    KLBoolean     gotKrb5 = false;
    krb5_creds    v5Creds;
    KLBoolean     gotKrb4 = false;
    CREDENTIALS v4Creds;
	KLPrincipal klPrincipal = NULL;
	KLBoolean     verify = verifykdc ? true : false;

    if (!err) 
	{
        err = krb5_init_context (&context);
    }

	err = KLCreatePrincipalFromString(principal ? principal : username, kerberosVersion_V5, &klPrincipal);

    if (!err) 
	{
        err = KLAcquireNewInitialTicketCredentialsWithPassword (klPrincipal, NULL, password, context, &gotKrb4, &gotKrb5, &v4Creds, &v5Creds);
    }

    if (!err) 
	{
        err = KLVerifyInitialTicketCredentials (gotKrb4 ? &v4Creds : NULL, gotKrb5 ? &v5Creds : NULL, verify);
    }

    if (!err) 
	{
		if (geteuid() == 0)
		{
			struct passwd *user = getpwnam(username);
			if (user && user->pw_uid)
				seteuid(user->pw_uid);
		}
		KLStoreNewInitialTicketCredentials (klPrincipal, context, gotKrb4 ? &v4Creds : NULL, gotKrb5 ? &v5Creds : NULL, NULL /*outCredCacheName*/);
		
		seteuid(0);
	}

	if (klPrincipal) 
		KLDisposePrincipal(klPrincipal);
		
    if (gotKrb5)
		krb5_free_cred_contents (context, &v5Creds);
		
    if (context != NULL)
		krb5_free_context (context);
		
	return err;
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

