/* vim:set ts=4 sw=4 sts=4 et cin: */
/*
 * Copyright (C) 2004 Sun Microsystems, Inc. All rights reserved. Use is
 * subject to license terms.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Common.h"
#include "ProfileDirServiceProvider.h"
#include "MsgServer.h"
#include "Message.h"

// from the Gecko SDK
#include "nsXPCOM.h"
#include "nsXPCOMCID.h"
#include "nsEmbedString.h"
#include "nsMemory.h"
#include "nsIServiceManager.h"
#include "nsIComponentManager.h"
#include "nsIProperties.h"
#include "nsIPrefService.h"
#include "nsILocalFile.h"
#include "plstr.h"

// below files are copied from the mozilla source tree (not part of the Gecko 
// SDK and subject to change in future versions of Mozilla)

// copied from the mozilla 1.7 source tree to support 1.7+.
#include "nsIProfileInternal.h"

// copied from the mozilla 1.4 source tree to support 1.4 through 1.6.
#include "nsIHttpProtocolHandler.h"
#include "nsIProfileInternalOld.h"

// from nsAppDirectoryServiceDefs.h (which is not part of the
// Gecko SDK v1.4)
#define NS_APP_USER_PROFILES_ROOT_DIR "DefProfRt"

// Check the Mozilla version that is being used.
nsresult GetMozillaVersion(char* versionBuf, size_t versionBufSize)
{
    nsresult rv;

    nsCOMPtr<nsIHttpProtocolHandler> httpHandler;
    rv = GetService("@mozilla.org/network/protocol;1?name=http",
                    NS_GET_IID(nsIHttpProtocolHandler),
                    getter_AddRefs(httpHandler));
    NS_ENSURE_SUCCESS(rv, rv);

    // UserAgent string looks like:
    //   Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.7) Gecko/20040616 
    //
    // Extract the "rv" field, which is returned by the GetMisc method.

    nsEmbedCString misc;
    rv = httpHandler->GetMisc(misc);
    if (NS_FAILED(rv)) {
        return rv;
    }        

    const char* miscString = misc.get(); 
    if (miscString[0] != 'r' || miscString[1] != 'v' || miscString[2] != ':')
        return NS_ERROR_UNEXPECTED;

    PL_strncpyz(versionBuf, &miscString[3], versionBufSize);
    return NS_OK;
}

nsresult GetSpecialDirectory(const char *key, nsIFile **result)
{
    nsresult rv;

    nsCOMPtr<nsIProperties> dirSvc;
    rv = GetService(NS_DIRECTORY_SERVICE_CONTRACTID,
                    NS_GET_IID(nsIProperties),
                    getter_AddRefs(dirSvc));
    if (NS_FAILED(rv))
        return rv;

    return dirSvc->Get(key, NS_GET_IID(nsIFile), (void **) result);
}

static nsresult GetPrivateProfileDir(nsIFile **profileDir)
{
    nsresult rv;

    nsCOMPtr<nsIFile> profileRootDir;
    rv = GetSpecialDirectory(NS_APP_USER_PROFILES_ROOT_DIR, 
                             getter_AddRefs(profileRootDir));
    NS_ENSURE_SUCCESS(rv, rv);
    rv = profileRootDir->AppendNative(nsEmbedCString("WebBrowser"));
    NS_ENSURE_SUCCESS(rv, rv);

    *profileDir = profileRootDir;
    NS_IF_ADDREF(*profileDir);
    return NS_OK;
}

static nsresult CopyPrefs(nsIFile *fromFile, nsIFile *toFile)
{
    nsresult rv;

    nsCOMPtr<nsILocalFile> fromLocalFile
            = do_QueryInterface(fromFile, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsILocalFile> toLocalFile
            = do_QueryInterface(toFile, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    FILE *from_fp, *to_fp;

    rv = fromLocalFile->OpenANSIFileDesc("r", &from_fp);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = toLocalFile->OpenANSIFileDesc("w", &to_fp);
    if (NS_FAILED(rv)) {
        NS_WARNING("unable to open output file");
        fclose(from_fp);
        return rv;
    }

    static const char kHeader[] = 
        "# Mozilla User Preferences\n\n";
    fwrite(kHeader, sizeof(kHeader) - 1, 1, to_fp);

    static const char *interestedSettings[] = {
        "accessibility.typeaheadfind.",
        "browser.display.",
        "browser.enable_automatic_image_resizing",
        "config.use_system_prefs",
        "font.",
        "network.",
        "security.",
    };

    PRInt32 numSettings = sizeof(interestedSettings) / sizeof(char *);

    char buf[1024];
    while (fgets(buf, sizeof(buf), from_fp) != NULL) {
        for (PRInt32 i = 0; i < numSettings; i++) {
            if (strstr(buf, interestedSettings[i]) != NULL)
                fwrite(buf, strlen(buf), 1, to_fp);
        }
    }

    fclose(from_fp);
    fclose(to_fp);
    return NS_OK;
}

nsresult InitializeProfile()
{
    nsresult rv;
         
    nsEmbedCString copiedPrefs("copiedprefs.js");

    nsCOMPtr<nsIFile> privateProfileDir;
    rv = GetPrivateProfileDir(getter_AddRefs(privateProfileDir));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIFile> privateProfilePrefs;
    rv = privateProfileDir->Clone(getter_AddRefs(privateProfilePrefs));
    NS_ENSURE_SUCCESS(rv, rv);
    rv = privateProfilePrefs->AppendNative(copiedPrefs);
    NS_ENSURE_SUCCESS(rv, rv);

    // activate our private profile
    nsCOMPtr<ProfileDirServiceProvider> locProvider;
    NS_NewMyProfileDirServiceProvider(getter_AddRefs(locProvider));
    NS_ENSURE_TRUE(locProvider, NS_ERROR_FAILURE);
    rv = locProvider->Register();
    NS_ENSURE_SUCCESS(rv, rv);
    rv = locProvider->SetProfileDir(privateProfileDir);
    NS_ENSURE_SUCCESS(rv, rv);

    // XXX because nsIProfileInternal was altered breaking binary 
    //     compatibility, and the interface UUID was not changed.  
    //     thus there is no way to use QueryInterface to get around 
    //     this problem.

    // Check the Mozilla version that is being used.
    char versionBuf[32];
    rv = GetMozillaVersion(versionBuf, sizeof(versionBuf));
    NS_ENSURE_SUCCESS(rv, rv);

    int MOZILLA_1_7 = 1;
    if (strncmp(versionBuf, "1.7", 3) < 0) {
        MOZILLA_1_7 = 0;
    }

    nsCOMPtr<nsIProfileInternal> profileService;
    nsCOMPtr<nsIProfileInternalOld> profileServiceOld;
    if (MOZILLA_1_7) {
        rv = GetService(NS_PROFILE_CONTRACTID,
                        NS_GET_IID(nsIProfileInternal),
                        getter_AddRefs(profileService));
    } else {
        rv = GetService(NS_PROFILE_CONTRACTID,
                        NS_GET_IID(nsIProfileInternalOld),
                        getter_AddRefs(profileServiceOld));
    }    
    NS_ENSURE_SUCCESS(rv, rv);
        
    // get the current (last used) profile
    PRUnichar *currProfileName;
    if (MOZILLA_1_7) {
        rv = profileService->GetCurrentProfile(&currProfileName);
    } else {
        rv = profileServiceOld->GetCurrentProfile(&currProfileName);
    }    
    if (NS_SUCCEEDED(rv)) {
        nsCOMPtr<nsIFile> currProfileDir;
        if (MOZILLA_1_7) {
            profileService->GetProfileDir(currProfileName, getter_AddRefs(currProfileDir));
        } else {
            profileServiceOld->GetProfileDir(currProfileName, getter_AddRefs(currProfileDir));
        }    
        nsMemory::Free(currProfileName);
        NS_ENSURE_SUCCESS(rv, rv);
        if (NS_FAILED(rv)) {
            nsMemory::Free(currProfileName);
            return rv;
        }

        rv = currProfileDir->AppendNative(nsEmbedCString("prefs.js"));
        NS_ENSURE_SUCCESS(rv, rv);
        PRBool exists;
        rv = currProfileDir->Exists(&exists);

        if (NS_SUCCEEDED(rv) && exists) {
            CopyPrefs(currProfileDir, privateProfilePrefs);
        }
    }

    // get prefs
    nsCOMPtr<nsIPrefService> pref;
    rv = GetService("@mozilla.org/preferences-service;1",
                    NS_GET_IID(nsIPrefService),
                    getter_AddRefs(pref));
    NS_ENSURE_SUCCESS(rv, rv);

    // regenerate the nsIFile object here, because ReadUserPrefs needs to get the file size
    rv = privateProfileDir->Clone(getter_AddRefs(privateProfilePrefs));
    NS_ENSURE_SUCCESS(rv, rv);
    rv = privateProfilePrefs->AppendNative(copiedPrefs);
    NS_ENSURE_SUCCESS(rv, rv);

    // activate our copied prefs.js
    pref->ReadUserPrefs(privateProfilePrefs);

    return NS_OK;
}

void ReportError(const char* msg)
{
    SendSocketMessage(-1, CEVENT_INIT_FAILED, msg);
}

// copy ASCII characters from |str| into |result|.  use a temporary buffer.
PRBool
ConvertAsciiToUtf16(const char *str, nsAString &result)
{
    int len = strlen(str);
    PRUnichar *buf = (PRUnichar *) malloc(len * sizeof(PRUnichar));
    if (!buf)
        return PR_FALSE;
    for (int i=0; i<len; ++i)
        buf[i] = PRUnichar(str[i]);
    result.Assign(buf, len);
    free(buf);
    return PR_TRUE;
}

// helper function for converting host endian UTF-16 chars to a Mozilla nsACString
PRBool
ConvertUtf16ToUtf8(const PRUnichar *str, nsACString &result)
{
    // XXX insert proper UTF-16 to UTF-8 conversion here
    // XXX for now, we just do a lossy conversion to ASCII

    int len = 0;
    for (const PRUnichar *p = str; *p; ++p)
        ++len;
    char *buf = (char *) malloc(len * sizeof(char));
    if (!buf)
        return PR_FALSE;
    for (int i=0; i<len; ++i)
    {
        buf[i] = (char) str[i];
        if (((unsigned char ) buf[i]) & 0x80)
        {
            fprintf(stderr, "FIXME: lossy conversion!!\n");
            buf[i] = '?';
        }
    }
    result.Assign(buf, len);
    free(buf);
    return PR_TRUE;
}

// helper function for getting xpcom services
nsresult
GetService(const char *aContractID, const nsIID &aIID, void **aResult)
{
    nsCOMPtr<nsIServiceManager> svcMgr;
    nsresult rv = NS_GetServiceManager(getter_AddRefs(svcMgr));
    if (NS_FAILED(rv))
        return rv;

    return svcMgr->GetServiceByContractID(aContractID, aIID, aResult);
}

nsresult
CreateInstance(const char *aContractID, const nsIID &aIID, void **aResult)
{
    nsCOMPtr<nsIComponentManager> compMgr;
    nsresult rv = NS_GetComponentManager(getter_AddRefs(compMgr));
    if (NS_FAILED(rv))
        return rv;

    return compMgr->CreateInstanceByContractID(aContractID, nsnull, aIID, aResult);
}
