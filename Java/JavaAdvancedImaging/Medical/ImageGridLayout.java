/**
 * @(#)ImageGridLayout.java	15.2 03/05/20
 *
 * Copyright (c) 2003 Sun Microsystems, Inc.
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * -Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * -Redistribution in binary form must reproduct the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of Sun Microsystems, Inc. or the names of contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * This software is provided "AS IS," without a warranty of any kind. ALL
 * EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING
 * ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN AND ITS LICENSORS
 * SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF
 * USING, MODIFYING OR DISTRIBUTING THE SOFTWARE OR ITS DERIVATIVES. IN NO
 * EVENT WILL SUN OR ITS LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT
 * OR DATA, OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR
 * PUNITIVE DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY OF
 * LIABILITY, ARISING OUT OF THE USE OF OR INABILITY TO USE SOFTWARE, EVEN
 * IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 *
 * You acknowledge that Software is not designed,licensed or intended for
 * use in the design, construction, operation or maintenance of any nuclear
 * facility.
 */


/**
 * A class defines an grid layout for an <code>MultipleImagePane</code>.  The
 *  valid layouts for this application are defined in the property file.  An
 *  example for a layout is listed below:
 *  <pre>
 *              Layout1Label="     4-Views     "
 *              Layout1ImageNum=4
 *              Layout1ImageNumInARow=2
 *              Layout1ImageNumInAColumn=2
 *  </pre>
 *
 * <p> The first item is the name of the layout displayed on the GUI of the
 *  application.  The second item is the total image number to be displayed in
 *  the <code>MultipleImagePane</code>.  The third and fourth items is the number of
 *  images to be displayed on each row or column.  If the total image number is
 *  less than the total grid number, some of the grid will be displayed as empty
 *  panels.
 *
 */



public class ImageGridLayout {
    /** The total number of images that will be displayed within this layout. */


    private int imageNum;

    /** The number of images that will be located in each row. */


    private int imageNumInARow;

    /** The number of images that will be located in each column. */


    private int imageNumInAColumn;

    /** The layout name based on its serial number. */


    private String name;

    /** The layout name to be displayed in the layout list in the GUI. */


    private String label;

    /**
     * Construct an <code>ImageGridLayout</code> object based on the name
     *  composed with the string "Layout" and the serial number of this layout
     *  when it is defined in the property file.  Retrieve the
     *  parameters of this layout from the property file.
     */


    public ImageGridLayout(String name) {
	String s = JaiI18N.getString(name + "ImageNum");
	imageNum = (new Integer(s)).intValue();

	s = JaiI18N.getString(name + "ImageNumInARow");
	imageNumInARow = (new Integer(s)).intValue();

	s = JaiI18N.getString(name + "ImageNumInAColumn");
	imageNumInAColumn = (new Integer(s)).intValue();

	this.name = name;
	label = JaiI18N.getString(name + "Label");
    }

    /** Return the capacity of this layout. */


    public int getImageNum() {
	return imageNum;
    }

    /** Return the number of columns. */


    public int getImageNumInARow() {
	return imageNumInARow;
    }

    /** Return the number of rows. */


    public int getImageNumInAColumn() {
	return imageNumInAColumn;
    }

    /** Return the name to be displayed in the layout list of the GUI. */


    public String getLabel() {
	return label;
    }

    /** Stringify for print for debugging.  */


    public String toString() {
	return name + imageNum + "  " + imageNumInARow + "  " +
		imageNumInAColumn;
    }
}
