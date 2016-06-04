/**
 * @(#)SharpenToolWidget.java	15.2 03/05/20
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



import javax.swing.*;
import javax.swing.event.*;
import javax.swing.text.*;
import javax.swing.border.*;
import javax.swing.colorchooser.*;
import javax.swing.filechooser.*;
import javax.accessibility.*;

import java.awt.*;
import java.awt.event.*;
import java.beans.*;
import java.util.*;
import java.io.*;
import java.applet.*;
import java.net.*;

/**
 * SharpenToolWidget.java
 *
 * This widget is used to control an image sharpening widget.
 *
 * The user must implement PropertyChangeListener to receive
 * notification of any changes on the sharpening values.
 *
 * @version 1.0 5/31/01
 */


public class SharpenToolWidget extends JFrame {

    public static final String DEFAULT_TITLE = new String("Image Sharpener");
    public static final int SLIDER_DEFAULT_VALUE = 20;
    public static final int SLIDER_MIN_DEFAULT_VALUE = 0;
    public static final int SLIDER_MAX_DEFAULT_VALUE = 100;

    private String title;

    private JButton resetBtn;
    private JButton helpBtn;
    private JButton hideBtn;

    private JSlider topSldr;
    private JPanel basePanel;

    private int sliderValue;
    private int sliderDefault;
    private int sliderMin;
    private int sliderMax;

    private int oldSliderValue; // Used to monitor change

    private PropertyChangeListener changeEventListener = null;
    private String propertyId;

    // Main method allowing a standalone run
    public static void main(String[] args) {
	SharpenToolWidget stw = new SharpenToolWidget();
	stw.show();
    }

    // Constructor for a JFrame with some widgets (most general case)
    public SharpenToolWidget(String userTitle,
                             int userSliderMin, int userSliderMax,
                             int userSliderDefault,
                             PropertyChangeListener listener,
                             String userPropertyId) {
        super(userTitle);
        title = userTitle;

        // Set the size for this widget
        setBounds(100,100,220,90);

        // Set up the base panel
        basePanel = new JPanel();
        basePanel.setLayout(new BoxLayout(basePanel, BoxLayout.Y_AXIS));
        getContentPane().add(basePanel);

        // Load the slider values from the user
        sliderDefault = userSliderDefault;
        sliderMin     = userSliderMin;
        sliderMax     = userSliderMax;
        sliderValue   = sliderDefault;

        // Place the items in the gui onto base panel
        createSliderAndButtons();
 
        // Store value to avoid repeating duplicates
        oldSliderValue = sliderValue;

        // Get the user's propertyId to pass this widget's properties
        this.changeEventListener = listener;
        addPropertyChangeListener(userPropertyId, listener);
        this.propertyId = userPropertyId;
        this.pack();

    } // SharpenToolWidget

    // Constructor for a JFrame with some widgets (pseudocolor applications)
    public SharpenToolWidget() {
        super(DEFAULT_TITLE);

        title = DEFAULT_TITLE;

        // Set the size for this widget
        setBounds(100,100,220,90);

        // Set up the base panel
        basePanel = new JPanel();
        basePanel.setLayout(new BoxLayout(basePanel, BoxLayout.Y_AXIS));
        getContentPane().add(basePanel);

        // Load default slider values
        sliderDefault = SLIDER_DEFAULT_VALUE;
        sliderMin     = SLIDER_MIN_DEFAULT_VALUE;
        sliderMax     = SLIDER_MAX_DEFAULT_VALUE;
        sliderValue   = sliderDefault;

        // Place the items in the gui onto base panel
	createSliderAndButtons();

        // Store value to avoid repeating duplicates
        oldSliderValue = sliderValue;

        // No listener, so this is just running in "toy mode"
        System.out.println("no parameters!");

        this.pack();
    }

    /** processChange
     * 
     * Send changed values to listeners and repaint the drawing area
     */


    protected void processChange() {

        if (sliderValue == oldSliderValue) return;
        oldSliderValue = sliderValue;
        firePropertyChange(propertyId, null, (Object)(new Integer(sliderValue)));

    } // processChange()

    public void createSliderAndButtons() {

	// Create panels to hold the layout:
        //
	//   ----------------------------
        //   | Blur             Sharpen |
	//   | <--+++---------slider--> |
	//   ---------------------------
        //   |  <btn1>  <btn2>  <btn3>  |
	//   ---------------------------
	// 
	JPanel topPanel = new JPanel();
        topPanel.setLayout(new BoxLayout(topPanel, BoxLayout.Y_AXIS));

        // Labels
        JPanel labelPanel = new JPanel();
        labelPanel.setLayout(new BoxLayout(labelPanel, BoxLayout.X_AXIS));
        JLabel blurLabel = new JLabel("<== Blur ---------------- Sharpen ==>");
        labelPanel.add(blurLabel);

        // Top slider
        SliderListener sListener = new SliderListener();
	topSldr = new JSlider(sliderMin, sliderMax, sliderValue);
        topSldr.setPaintTicks(false);
        topSldr.setPaintLabels(false);
	topSldr.addChangeListener(sListener);
        topSldr.setOrientation(javax.swing.SwingConstants.HORIZONTAL);
        topPanel.add(topSldr);

        JPanel btnPanel = new JPanel();
        btnPanel.setLayout(new BoxLayout(btnPanel, BoxLayout.X_AXIS));

        ButtonListener bListener = new ButtonListener();

        // Construct reset, help, and hide buttons
        resetBtn = new JButton("Reset");
        resetBtn.addActionListener(bListener);
        btnPanel.add(resetBtn);
        helpBtn = new JButton("Help");
        helpBtn.addActionListener(bListener);
        btnPanel.add(helpBtn);
        hideBtn = new JButton("Hide");
        hideBtn.addActionListener(bListener);
        btnPanel.add(hideBtn);

        // Attach everything to the base panel
        basePanel.add(labelPanel);
        basePanel.add(topPanel);
        basePanel.add(btnPanel);

    } // void createSlidersAndDrawingArea

    private void resetWidget() {

        // Set default parameter
        sliderValue = sliderDefault;
        topSldr.setValue(sliderValue);
        processChange();

    }

    public void showHelp() {

        JFrame frame = new JFrame("Sharpen Tool Help");
        JOptionPane.showMessageDialog(frame,
            "Use the slider to sharpen an image.  Slide to\n"+
            "the left to blur. Slide right to sharpen more.\n",
            "User Help",
            JOptionPane.INFORMATION_MESSAGE);

    } // void showHelp

    // A class to handle button presses
    class ButtonListener implements ActionListener {
	public ButtonListener() {}
	public void actionPerformed(ActionEvent e) {
	    JButton b = (JButton)e.getSource();
            if (b.equals(resetBtn)) {

                // Reset the widget to the nominal setting
                resetWidget();

            } else if (b.equals(helpBtn)) {

                // Display an information dialog
                showHelp();

	    } else if (b.equals(hideBtn)) {

                // hide this widget from view
                setVisible(false);

 	    }
        }
    }  // class ButtonListener

    // A class to handle slider changes
    class SliderListener implements ChangeListener {

	public SliderListener() {}
        public void stateChanged(ChangeEvent e) {
            JSlider s = (JSlider)e.getSource();

            if (s.equals(topSldr)) {
                sliderValue = s.getValue();
            }

            // If the user is dragging, wait until they
            // let go before sending updates to client.
            if (!s.getValueIsAdjusting()) {
                processChange();
            }
        }
    }  // class SliderListener

}
