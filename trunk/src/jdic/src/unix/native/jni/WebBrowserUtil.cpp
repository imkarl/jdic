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

#include <jawt_md.h>
#include <jawt.h>
#include <X11/Xlib.h>
#include "WebBrowserUtil.h"
#include <stdlib.h>
#include <limits.h>

#include <sys/stat.h>
#include <gconf/gconf-client.h>

#define GCONF_URL_HANDLER_PATH "/desktop/gnome/url-handlers/"

#ifndef PATH_MAX
#define PATH_MAX 512
#endif

/*
 * Class:     org_jdesktop_jdic_browser_internal_WebBrowserUtil
 * Method:    nativeGetBrowserPath
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_org_jdesktop_jdic_browser_internal_WebBrowserUtil_nativeGetBrowserPath
  (JNIEnv *env, jclass)
{    
    // Get environment variable MOZILLA_FIVE_HOME value.
    char *moz5Home = getenv("MOZILLA_FIVE_HOME");
    if (moz5Home != NULL)
        return env->NewStringUTF(moz5Home);

    /*
     * MOZILLA_FIVE_HOME not set. Search the Mozilla binary path to set it:
     *   - First check Gconf to find the default browser path; 
     *   - If fails, then check the "mozilla" command under $PATH.
     *
     * If the mozilla binary path is found, check if it points to a valid path:
     *   - If not, return NULL; 
     *   - If yes, check if libxpcom.so is located at the same path:
     *     - If yes, set MOZILLA_FIVE_HOME to the mozilla binary path;
     *     - If not, scan the mozilla binary file, which should be a Bourne 
     *       shell script file, for the MOZILLA_FIVE_HOME setting.
     *
     *  If MOZILLA_FIVE_HOME is found and set, add it to LD_LIBRARY_PATH.
     */
    gboolean result;
    char *schemes[2] = {"http", "unknown"};
    char *path;
    char *command;
    char *mozpath = NULL;
                                                                                                          
    // Check GConf to find the default browser path.
    g_type_init();
    GConfClient *client = gconf_client_get_default ();
    for (int i = 0; i < sizeof(schemes); i++) {
        path = g_strconcat (GCONF_URL_HANDLER_PATH, schemes[i], "/command", NULL);
        command = gconf_client_get_string (client, path, NULL);
        g_free(path);
        if (command != NULL) {
#ifdef DEBUG
            printf("The default browser path set in GConf: %s\n", command);
#endif
            break;
        }
    }
                                                                                                          
    // Check if the command is or points to a Mozilla binary path.
    if (command != NULL) {
        char *p = g_strstr_len(command, strlen(command), "mozilla");
        if (p) {
            // Remove the "%s" string in the command string.
            mozpath = g_strdup_printf (command, "");
            // Remove the leading and trailing whilespaces.
            if (mozpath != NULL) {
                mozpath = g_strstrip(mozpath);
            } 
        }
    }

    // Check if the Mozilla path is valid. Or else, check the mozilla
    // command under $PATH.
    struct stat stat_p;
    if ((mozpath == NULL) || (stat (mozpath, &stat_p) != 0)) {
        char *pathenv = getenv("PATH");
        char **pathfields = g_strsplit(pathenv, ":", -1);    
        for (int index = 0; pathfields[index] != NULL; index++) {
            mozpath = g_strconcat (pathfields[index], "/mozilla", NULL);
            if (stat (mozpath, &stat_p) == 0) {
#ifdef DEBUG
                printf("Found mozilla binary under $PATH: %s\n", mozpath);
#endif
                break; 
            } else {
                g_free(mozpath); // Release the newly-allocated string.
                mozpath = NULL;
            }
        }
    }

    if (mozpath == NULL) 
        return NULL;

    // Check if libxpcom.so is located at the Mozilla parent path. If not, 
    // scan the Mozilla command file for the MOZILLA_FIVE_HOME setting.
    char *moz5home_value = NULL;
  
    // the file mozpath maybe a link file.
    char *real_mozpath = (char *)malloc(PATH_MAX);
    realpath(mozpath, real_mozpath);
    free(mozpath);
    mozpath = real_mozpath;

    // Check if libxpcom.so exists at the same path.
    char *str_p = g_strrstr(mozpath, "/");
    char *parentpath = g_strndup(mozpath, str_p - mozpath);
    char *libpath = g_strconcat (parentpath, "/libxpcom.so", NULL);
    if (stat (libpath, &stat_p) == 0) {
        moz5home_value = g_strdup(parentpath);
#ifdef DEBUG
        printf("Found libxpcom.so under mozilla binary path: %s\n", parentpath);
#endif
    } else {
        g_free(libpath); // Release the newly-allocated string.

        // Scan the file, which (or the symbol link target file) should
        // be a Bourne shell script, for the MOZILLA_FIVE_HOME setting.
        char buf[1024]; 
        FILE *fp = fopen(mozpath, "r");
        if (fp) {
            while (fgets(buf, 1024, fp)) {
                char* substr_p = g_strstr_len(buf, strlen(buf), 
                                          "MOZILLA_FIVE_HOME=");
                if (substr_p) {
                    moz5home_value = 
                      g_strdup(substr_p + strlen("MOZILLA_FIVE_HOME=")); 
                    if (moz5home_value && strlen(moz5home_value)) {
                        moz5home_value = g_strstrip(moz5home_value);
                        if (moz5home_value) {
                            // Remove the leading '"' character. 
                            for (int i = 0; i < strlen(moz5home_value); i++) {
                                if (moz5home_value[i] != '"') {
                                    moz5home_value += i; 
                                    break;
                                }
                            }
                            // Remove the trailing '\n' and '"' characters.
                            for (int i = (strlen(moz5home_value) - 1); i > 0; i--) {
                                if ((moz5home_value[i] == '\n') 
                                    || moz5home_value[i] == '"') { 
                                    moz5home_value[i] = '\0'; 
                                } else { 
                                    break;
                                }
                            }
#ifdef DEBUG
                            printf("Scaned MOZILLA_FIVE_HOME setting from %s as: %s\n", 
                                mozpath, moz5home_value);
#endif
                        }
                    }
                    break;
                } // end of "if (substr_p)"
            }
            fclose(fp);
        } // end of "if (p)"
    }
    g_free(mozpath);

    return (moz5home_value == NULL) ? 
        NULL : env->NewStringUTF(moz5home_value);
}