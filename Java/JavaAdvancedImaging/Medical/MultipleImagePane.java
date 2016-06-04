/**
 * @(#)MultipleImagePane.java	15.2 03/05/20
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


import java.awt.Component;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.LayoutManager;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.awt.image.RenderedImage;
import java.awt.image.renderable.ParameterBlock;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.Hashtable;
import javax.swing.JPanel;

/**
 * This class defines a <code>JPanel</code> which contains the current
 *  displayed images (each image is placed in a separate
 *  <code>ImagePane</code>).  Also this class monitors the focus change.
 *
 */



public class MultipleImagePane extends JPanel implements FocusListener,
						     MedicalAppConstants {

    /**
     * The grid layout for this panel: the number of rows/columns, and the
     * capacity.
     *
     * @see ImageGridLayout
     */


    private ImageGridLayout imageViewLayout;

    /** The array of display panes located in this image view panel.
     */


    private MedicalImagePane[] singleImagePanes;

    /** The current focused <code>ImagePane</code>. The default is 0.
     */


    private int focusedPane = 0;

    /** The default constructor.
     */


    MultipleImagePane() {}

    /**
     * The constructor accepts the grid layout defined in the property file
     *  and the images to be displayed when this container is created.
     *
     *  @param layout The grid layout defined in the property file.
     *  @param images The images to be displayed.
     */


    MultipleImagePane(int layout, RenderedImage[] images) {
	setImageGridLayout(layout);
	createLayout(images);
    }

    /**
     * Set a layout to this view panel. The layout is defined by the provided
     *  number corresponding the layout definition in the property file.
     */


    public void setImageGridLayout(int layoutNo) {
	imageViewLayout = new ImageGridLayout("Layout" + layoutNo);
    }

    /** Get the layout of this view panel. */


    public ImageGridLayout  getImageGridLayout() {
	return imageViewLayout;
    }

    /** Create the layout and display images. */


    public void createLayout(RenderedImage[] images) {

	// Clear the old contents.
	Component[] components = getComponents();
	if (components.length > 0)
	    for (int i = 0; i < components.length; i++)
		remove(components[i]);

	// Set the layout consistent with the chosen layout.
	setLayout(createGridLayout());

	// Get the total image number for this layout.
	int imageNum = imageViewLayout.getImageNum();
	imageNum = Math.min(imageNum, images.length);

	// Create the panes to contain these images, add them to this
	// panel, and prepare to monitor the focus changing.
	singleImagePanes = new MedicalImagePane[imageNum];
	for (int i = 0; i < imageNum; i++) {
            ResizePolicy policy =
                new ResizePolicy(ResizePolicy.POLICY_CENTER);

            singleImagePanes[i] =
                new MedicalImagePane(images[i], policy,
					    "Image " + (i + 1));
	    add(singleImagePanes[i]);
	    singleImagePanes[i].addFocusListener(this);
	}

	// Define the prefered size
	int width = (images[0].getWidth() + 4) *
		    imageViewLayout.getImageNumInARow();
	int height = (images[0].getHeight() + 4) *
		    imageViewLayout.getImageNumInAColumn();

        setFocused(0, true);
        singleImagePanes[0].setFocused(true);

	setPreferredSize(new Dimension(width, height));

	// Repaint
	revalidate();
	repaint();
    }

    /** Change images for display. Set the focus to the first image. */


    public void set(RenderedImage[] images) {
	set(images, true);
    }

    /**
     * Change images for display.  The boolean parameter indicates that
     *  the focus will be on the first image or not.
     */


    public void set(RenderedImage[] images, boolean changeFocus) {
	for (int i = 0; i < singleImagePanes.length; i++)
	    if (i < images.length)
		singleImagePanes[i].set(images[i]);

	if (changeFocus) {
	    setFocused(0, true);
	    singleImagePanes[0].setFocused(true);
	}
    }

    /** Return the image in the <code>i</code>th image display panel. */


    public RenderedImage getImage(int i) {
	if (i >= 0 && i < singleImagePanes.length)
	    return singleImagePanes[i].getImage();
	return null;
    }

    /** Return all the <code>ImagePanel</code>s. */


    public MedicalImagePane[] getImagePanels() {
	return singleImagePanes;
    }

    /** Change the image displayed in the focused panel. */


    public void setFocusedPane(RenderedImage image) {
        singleImagePanes[focusedPane].set(image);
    }

    /** return the serial number of the focused image display panel. */


    public int getFocused() {
	return focusedPane;
    }

    /** Change the focus status of the <code>i</code>th image display panel. */


    private void setFocused(int pane, boolean b) {
	if (b)
	    focusedPane = pane;
	singleImagePanes[pane].setFocused(b);
    }

    /**
     * This class delegates to process the <code>FocusEvent</code> for the image
     *  display panels.
     */


    public void focusGained(FocusEvent evt) {
	Object source = evt.getSource();
	if (source instanceof ImagePane) {
	    for (int i = 0; i < singleImagePanes.length; i++)
		setFocused(i, singleImagePanes[i].equals(source));
	}
    }

    /** This class delegates to process the FocusEvent for the image
     *  display panes.
     */


    public void focusLost(FocusEvent evt) {
    }

    /**
     * Create a GridLayout based on the <code>ImageGridLayout</code> of this
     *  panel.
     */


    private LayoutManager createGridLayout() {
	return new GridLayout(imageViewLayout.getImageNumInARow(),
			      imageViewLayout.getImageNumInAColumn());
    }
}
