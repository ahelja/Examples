/**
 * @(#)JAIFileBrowser.java	15.2 03/05/20
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
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.image.renderable.ParameterBlock;
import java.util.Vector;
import javax.media.jai.*;
import javax.swing.*;
import javax.swing.event.*;

public class JAIFileBrowser
    extends WindowAdapter
    implements ListSelectionListener {

    String[] filenames;

    JFrame frame;
    JFrame frame2;

    PlanarImage image = null;
    JLabel picture = null;
    JPanel picturePanel;
    JSplitPane splitPane;
    JTable propertyTable = null;

    int width, height;

    public JAIFileBrowser(String[] filenames) {
        this.filenames = filenames;

        frame = new JFrame("JAI File Browser");
        frame2 = new JFrame(filenames[0]);
        
        // Read first image
        image = JAIImageReader.readImage(filenames[0]);
        width = image.getWidth();
        height = image.getHeight();

	// Create the filename list
        Vector filenameVector = new Vector();
        for (int i = 0; i < filenames.length; i++) {
            filenameVector.add(filenames[i]);
        }

	JList filenameList = new JList(filenameVector);
	filenameList.setSelectedIndex(0);
	filenameList.addListSelectionListener(this);
        JScrollPane filenameScrollPane = new JScrollPane(filenameList);

        // Create the property table
        propertyTable = createPropertyTable();
        JScrollPane propertyScrollPane = new JScrollPane(propertyTable);

        splitPane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT);
        splitPane.setLeftComponent(filenameScrollPane);
        splitPane.setRightComponent(propertyScrollPane);

        // Picture
        picture = new JLabel();
        picture.setHorizontalAlignment(SwingConstants.CENTER);
        Icon icon = new IconJAI(image);
        picture.setIcon(icon);

        int w = icon.getIconWidth();
        int h = icon.getIconHeight();
        picture.setPreferredSize(new Dimension(w, h));

        picturePanel = new JPanel();
        picturePanel.setLayout(new BorderLayout());
        JScrollPane pictureScrollPane = new JScrollPane(picture);
        picturePanel.setPreferredSize(new Dimension(Math.min(w + 3, 800),
                                                    Math.min(h + 3, 800)));
        picturePanel.add(new JScrollPane(picture), BorderLayout.CENTER);

        frame.getContentPane().setLayout(new BorderLayout());
        frame.getContentPane().add(splitPane);
        frame.addWindowListener(this);
        frame.setLocation(100, 100);
        frame.pack();
        frame.setVisible(true);

        frame2.getContentPane().setLayout(new BorderLayout());
        frame2.getContentPane().add(picturePanel);
        frame2.addWindowListener(this);
        frame2.setLocation(100, 500);
        frame2.pack();
        frame2.setVisible(true);
    }

    public JTable createPropertyTable() {
        String[] columnNames = { "Property Name", "Value" };
        String[] propertyNames = image.getPropertyNames();

        int numProperties;
        if (propertyNames != null) {
            numProperties = propertyNames.length;
        } else {
            numProperties = 0;
        }
        String[][] values = new String[numProperties][2];

        for (int i = 0; i < numProperties; i++) {
            String name = propertyNames[i];

            values[i][0] = name;
            Object property = image.getProperty(name);
            if (property == null) {
                values[i][1] = "<null>";
            } else if (property instanceof int[]) {
                int[] nums = (int[])property;
                String s = "[";
                for (int j = 0; j < nums.length - 1; j++) {
                    s += nums[j] + ", ";
                }
                s += nums[nums.length - 1] + "]";
                values[i][1] = s;
            } else {
                values[i][1] = property.toString();
            }
        }

        return new JTable(values, columnNames);
    }

    public void valueChanged(ListSelectionEvent e) {
        if (!e.getValueIsAdjusting()) {
            JList theList = (JList)e.getSource();
            int index = theList.getSelectedIndex();

            frame2.setTitle(filenames[index]);

            image = JAIImageReader.readImage(filenames[index]);
            width = image.getWidth();
            height = image.getHeight();
            Icon icon = new IconJAI(image);
            picture.setIcon(icon);

            int w = icon.getIconWidth();
            int h = icon.getIconHeight();
            picture.setPreferredSize(new Dimension(w, h));
            picturePanel.setPreferredSize(new Dimension(Math.min(w + 3, 800),
                                                        Math.min(h + 3, 800)));
            picture.revalidate();

            propertyTable = createPropertyTable();
            splitPane.setRightComponent(new JScrollPane(propertyTable)); 
            frame.pack();
            frame2.pack();
        }
    }

    public void windowClosing(WindowEvent e) {
        System.exit(0);
    }

    public static void main(String[] args) {
        if (args.length < 1) {
            System.err.println("usage: java JAFileBrowser filename [filename ...]");
            System.exit(0);
        }

        new JAIFileBrowser(args);
    }
}
