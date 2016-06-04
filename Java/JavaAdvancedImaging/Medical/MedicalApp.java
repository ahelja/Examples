/**
 * @(#)MedicalApp.java	15.2 03/05/20
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


import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Event;
import java.awt.event.ActionEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.image.RenderedImage;
import java.io.File;
import java.util.Hashtable;
import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.BorderFactory;
import javax.swing.ImageIcon;
import javax.swing.KeyStroke;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JToolBar;
import javax.media.jai.JAI;

/**
 * The main entrance of the Medical Image Application for JAI 1.1.
 *
 * Before load any data set, this class displays a medical image slice with
 * the title for this demo in a single view image display panel.
 *
 * This class handles the events generated from the menu items: open and exit.
 * Start the loading thread in <code>MedicalAppState</code>, adjust the size
 * of the <code>JFrame</code> based on the image size and the layout.
 *
 */



public class MedicalApp extends JFrame implements MedicalAppConstants  {

    /** The singleton instance of this singleton class. */


    private static MedicalApp instance;

    /** Cache the screen size. */


    private Dimension screenSize;

    /** The toolbar of this application. */


    private JToolBar toolBar = new JToolBar();

    /** The panel to display a group of images. */


    private MultipleImagePane multipleImagePane;

    /** Indicates it is the first-time load or not. */


    private boolean firstLoad = true;

    /**
     * Cache the height of the header: the summation of the heights of the
     *  title bar, the menu bar, and the tool bar.
     */


    private int headHeight;

    /** Return the single instance of this class. */


    public static synchronized MedicalApp getInstance() {
	if (instance == null)
	    instance = new MedicalApp();
	return instance;
    }

    /** The main entrance of this application. */


    public static void main(String[] args) {
	MedicalApp f = MedicalApp.getInstance();
	f.show();
	f.pack();
    }

    /** The constructor. Define as private to guarantee it is singleton. */


    private MedicalApp() {
	super(JaiI18N.getString("AppTitle"));

	// define the original size as half of the screen and locate it
	// at the center.
	screenSize = this.getToolkit().getScreenSize();
	setBounds(screenSize.width / 4, screenSize.height / 4,
		  screenSize.width / 2, screenSize.height / 2);

	// create the open/exit actions.
	FileAction openAction =
            new FileAction("Open",
                           KeyStroke.getKeyStroke('O',Event.CTRL_MASK),
                           "Open 12-bit Dicom dataset");
	FileAction exitAction =
            new FileAction("Exit",
                           KeyStroke.getKeyStroke('X', Event.CTRL_MASK),
                           "Exit Medical Image Application");

	// Create the file menu.
	JMenu fileMenu = new JMenu(JaiI18N.getString("File"));
	addMenuItem(fileMenu, openAction);
	fileMenu.addSeparator();
	addMenuItem(fileMenu, exitAction);
	JMenuBar menuBar = new JMenuBar();
	menuBar.add(fileMenu);
	setJMenuBar(menuBar);

	// add actions to the toolbar.
	addToolBarButton(openAction);
	addToolBarButton(exitAction);

        addWindowListener(new WindowHandler()); // Add window listener

	// get the layout number before loading any data set.
	int layout =
	    new Integer(JaiI18N.getString("LayoutBeforeLoad")).intValue();

       
	// create an image view pane containing only one image.
        multipleImagePane =
	    new MultipleImagePane(layout,
			      new RenderedImage[] {JAI.create("url",
					 getClass().getResource("/" + JaiI18N.getString("TitleImage")) )});
        
	this.getContentPane().add(multipleImagePane, BorderLayout.CENTER);

	toolBar.setFloatable(true);
	this.getContentPane().add(toolBar, BorderLayout.NORTH);
    }

    /** Return the current image view panel. */


    public MultipleImagePane getMultipleImagePane() {
	return multipleImagePane;
    }

    /** Add a new menu item. */


    private JMenuItem addMenuItem(JMenu menu, Action action) {
        JMenuItem item = menu.add(action);   // Add the menu item

        KeyStroke keystroke = (KeyStroke)action.getValue(action.ACCELERATOR_KEY);
        if(keystroke != null)
          item.setAccelerator(keystroke);
        item.setIcon(null);
        return item;
    }

    /** Add a new tool button to the tool bar. */


    private JButton addToolBarButton(Action action) {
        JButton button = toolBar.add(action);      // Add toolbar button
        button.setBorder(BorderFactory.createRaisedBevelBorder()); // Add button border
        button.setText(null);
        return button;
    }

    // Handler class for window events
    class WindowHandler extends WindowAdapter {
	// Handler for window closing event
	public void windowClosing(WindowEvent e) {
	    // Code to be added here later...
	    System.exit(0);
	}
    }

    /** Resize the frame after change the layout. */


    public void resizeAfterChangeLayout() {
	// define the utility menu pane.
	UtilityMenuPane utilityPane = UtilityMenuPane.getInstance();

	// first time load, define the height of the header.
        if (firstLoad) {
	    headHeight = getHeight() - multipleImagePane.getHeight();
            getContentPane().add(utilityPane, BorderLayout.EAST);
	}

	// define the utility pane size
	Dimension utilitySize = utilityPane.getPreferredSize();
	int utilityWidth =
	    (new Integer(JaiI18N.getString("UtilityMenuWidth"))).intValue();
	utilityWidth = Math.max(utilityWidth, utilitySize.width);
	int utilityHeight =
	    (new Integer(JaiI18N.getString("UtilityMenuHeight"))).intValue();
	utilityHeight = Math.max(utilityHeight, utilitySize.height);

	// define the width/ height of the frame.
	Dimension viewSize = multipleImagePane.getPreferredSize();
        int width = utilityWidth + viewSize.width;
        int height = Math.max(utilityHeight, viewSize.height) +
                    headHeight;

	// cannot be larger than the screen.
	width = Math.min(width, screenSize.width);
	height = Math.min(height, screenSize.height);

	// centerize the frame.
	setLocation((screenSize.width - width) / 2,
		    (screenSize.height - height) / 2);
        setSize(new Dimension(width, height));
	firstLoad = false;
    }

    /** An inner class to define the FileAction. */


    class FileAction extends AbstractAction {
        // The file chooser to get the dictionary name of the data set.
	private JFileChooser inputFileChooser;

        // Initialization block to intialize the file chooser.
	{
	    inputFileChooser = new JFileChooser("." + File.separator + "data");
	    inputFileChooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
	}

        // Constructor
        FileAction(String name) {
            super(JaiI18N.getString(name));
            String iconFileName = "/images/" + name.toLowerCase() + ".gif";
            putValue(SMALL_ICON, new ImageIcon(getClass().getResource(iconFileName)));
        }

        // Constructor
        FileAction(String name, KeyStroke keystroke) {
            this(name);
            if (keystroke != null)
                putValue(ACCELERATOR_KEY, keystroke);
        }

        // Constructor
        FileAction(String name, KeyStroke keystroke, String tooltip) {
            this(name, keystroke);                          // Call the other constructor
            if (tooltip != null)                             // If there is tooltip text
                putValue(SHORT_DESCRIPTION, tooltip);         // ...squirrel it away
        }

        // Constructor
        FileAction(String name, String tooltip) {
            this(name);                                     // Call the other constructor
            if(tooltip != null)                             // If there is tooltip text
            putValue(SHORT_DESCRIPTION, tooltip);         // ...squirrel it away
        }

        // Event handler
        public void actionPerformed(ActionEvent e) {
	    // process the action events.
	    String name = (String)getValue("Name");
	    if (name.equals(JaiI18N.getString("Open"))) {
		int returnVal = inputFileChooser.showOpenDialog(MedicalApp.this);
		if (returnVal == JFileChooser.APPROVE_OPTION) {
		    // after the file chosen, load it.
		    File file = inputFileChooser.getSelectedFile();
		    MedicalAppState medicalAppState =
			MedicalAppState.getInstance();
		    medicalAppState.loadDataSet(file);
		}
	    } else if (name.equals(JaiI18N.getString("Exit"))) {
		// exit the application.
		System.exit(0);
	    }
        }
    }
}

