/**
 * @(#)JAIAddConstPanel.java	15.2 03/05/20
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


import java.awt.*;
import java.awt.event.*;
import java.awt.image.renderable.ParameterBlock;
import java.awt.image.renderable.RenderedImageFactory;
import java.util.Hashtable;
import java.util.Vector;
import javax.media.jai.*;
import javax.swing.*;
import javax.swing.event.*;

public class JAIAddConstPanel extends JAIDemoPanel
    implements ChangeListener, ItemListener {

    boolean zeroState = true;

    int param1 = 0;
    int param2 = 0;
    int param3 = 0;

    JSlider p1Slider;
    JSlider p2Slider;
    JSlider p3Slider;

    public JAIAddConstPanel(Vector sourceVec) {
        super(sourceVec);
        masterSetup();
    }

    public String getDemoName() {
        return "AddConst";
    }


    public void makeControls(JPanel controls) {
        p1Slider = new JSlider(JSlider.HORIZONTAL, -255, 255, 0);
        p2Slider = new JSlider(JSlider.HORIZONTAL, -255, 255, 0);
        p3Slider = new JSlider(JSlider.HORIZONTAL, -255, 255, 0);

        Hashtable labels = new Hashtable();
        labels.put(new Integer(-255), new JLabel("-255"));
        labels.put(new Integer(0), new JLabel("0"));
        labels.put(new Integer(255), new JLabel("255"));
        p1Slider.setLabelTable(labels);
        p2Slider.setLabelTable(labels);
        p3Slider.setLabelTable(labels);

        p1Slider.setPaintLabels(true);
        p2Slider.setPaintLabels(true);
        p3Slider.setPaintLabels(true);
        
        p1Slider.addChangeListener(this);
        p2Slider.addChangeListener(this);
        p3Slider.addChangeListener(this);

        JPanel p1SliderPanel = new JPanel();
        p1SliderPanel.setLayout(new BoxLayout(p1SliderPanel, BoxLayout.X_AXIS));
        JLabel p1Label = new JLabel("Red");
        p1SliderPanel.add(p1Label);
        p1SliderPanel.add(p1Slider);

        JPanel p2SliderPanel = new JPanel();
        p2SliderPanel.setLayout(new BoxLayout(p2SliderPanel, BoxLayout.X_AXIS));
        JLabel p2Label = new JLabel("Green");
        p2Label.setPreferredSize(p1Label.getPreferredSize());
        p2SliderPanel.add(p2Label);
        p2SliderPanel.add(p2Slider);

        JPanel p3SliderPanel = new JPanel();
        p3SliderPanel.setLayout(new BoxLayout(p3SliderPanel, BoxLayout.X_AXIS));
        JLabel p3Label = new JLabel("Blue");
        p3Label.setPreferredSize(p1Label.getPreferredSize());
        p3SliderPanel.add(p3Label);
        p3SliderPanel.add(p3Slider);

        JPanel sliderPanel = new JPanel();
        sliderPanel.setLayout(new BoxLayout(sliderPanel, BoxLayout.Y_AXIS));
        sliderPanel.add(p1SliderPanel);
        sliderPanel.add(p2SliderPanel);
        sliderPanel.add(p3SliderPanel);

        controls.setLayout(new BorderLayout());
        controls.add("Center", sliderPanel);
    }

    public boolean supportsAutomatic() {
        return true;
    }

    public PlanarImage process() {
        PlanarImage im = getSource(0);
 
        ParameterBlock pb = new ParameterBlock();
        pb.addSource(im);
        double consts[] = {param1, 
                           param2,
                           param3};
        pb.add(consts);
        return JAI.create("addconst", pb, renderHints);
    }

    public void startAnimation() {
    }

    int sliderDelta1 = 1;
    int sliderDelta2 = 1;
    int sliderDelta3 = 1;
    boolean isAutoInit = false;

    public void animate() {
        if(zeroState) {
            int delta =
                (int)((float)(p1Slider.getMaximum() -
                              p1Slider.getMinimum())/3.0F);
            p2Slider.setValue(Math.max(p2Slider.getMinimum(),
                                       p1Slider.getValue() - delta));
            p3Slider.setValue(Math.min(p3Slider.getMaximum(),
                                       p1Slider.getValue() + delta));
            zeroState = false;
        }

        int value = p1Slider.getValue();
        int newValue = value + sliderDelta1;

        if (newValue < p1Slider.getMinimum() ||
            newValue > p1Slider.getMaximum()) {
            sliderDelta1 = -sliderDelta1;
        }
        p1Slider.setValue(value + sliderDelta1);

        int value2 = p2Slider.getValue();
        int newValue2 = value2 + sliderDelta2;

        if (newValue2 < p2Slider.getMinimum() ||
            newValue2 > p2Slider.getMaximum()) {
            sliderDelta2 = -sliderDelta2;
        }
        p2Slider.setValue(value2 + sliderDelta2);

        int value3 = p3Slider.getValue();
        int newValue3 = value3 + sliderDelta3;

        if (newValue3 < p3Slider.getMinimum() ||
            newValue3 > p3Slider.getMaximum()) {
            sliderDelta3 = -sliderDelta3;
        }
        p3Slider.setValue(value3 + sliderDelta3);
    }

    public void reset() {
        param1 = 0;
        param2 = 0;
        param3 = 0;
        p1Slider.setValue(param1);
        p2Slider.setValue(param2);
        p3Slider.setValue(param2);
        zeroState = true;
    }

    public void stateChanged(ChangeEvent e) {
        JSlider source = (JSlider)e.getSource();
        int value = source.getValue();

        if (source == p1Slider) {
             param1 = value;
        } else if (source == p2Slider) {
             param2 = value;
        } else if (source == p3Slider) {
             param3 = value;
        }
        repaint();
    }

    public void itemStateChanged(ItemEvent e) {
    }
}
