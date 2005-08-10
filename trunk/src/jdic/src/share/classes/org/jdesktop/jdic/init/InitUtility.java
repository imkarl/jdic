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
package org.jdesktop.jdic.init;

import java.io.File;

/**
 * Utility class for JDIC initialization.
 * @author Paul Huang
 * @created August 20, 2004
 */
public class InitUtility {
    static {
        System.loadLibrary("jdic");
    }
    
    /**
     * Sets the environment variable. 
     *
     * @param ennVarName The name of the environment variable.
     * @param envValue The value to be set.
     */
    public static native void setEnv(String envVarName, String envValue);
    
    /**
     * Pre-appends the value to the environment variable.
     *
     * @param envVarName environment variable name.
     * @param appendValue new value to be appended.
     */
    public static void preAppendEnv(String envVarName, String appendValue) {
        String oldValue = System.getenv(envVarName);
        setEnv(envVarName, oldValue == null ? appendValue 
                : appendValue+File.pathSeparator+oldValue);
    }
} 