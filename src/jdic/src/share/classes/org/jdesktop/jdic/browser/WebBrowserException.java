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
 
package org.jdesktop.jdic.browser;


/**
 * A <code>WebBrowserException</code> is thrown by certain methods of 
 * <code>WebBrowser</code> class whenever something fails internally (eg. <code>
 * WebBrowser</code> fails to initialize, or a goForward call was made before
 * <code>WebBrowser</code> finishes initialization.
 * 
 * @see WebBrowser
 */
public class WebBrowserException extends Exception {

    /**
     * Constructs a <code>WebBrowserException</code> without a detail message.
     */
    public WebBrowserException() {
        super();
    }
  
    /**
     * Constructs a <code>WebBrowserException</code> with a detail message.
     * 
     * @param msg the detail message pertaining to this exception.
     */
    public WebBrowserException(String msg) {
        super(msg);
    }  
}