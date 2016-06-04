/**
 * @(#)IconJAI.java	15.2 03/05/20
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



import java.awt.Color;
import java.awt.Component;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.Transparency;
import java.awt.color.ColorSpace;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.ComponentColorModel;
import java.awt.image.DataBuffer;
import java.awt.image.Raster;
import java.awt.image.RenderedImage;
import java.awt.image.SampleModel;
import java.awt.image.WritableRaster;
import java.awt.image.renderable.ParameterBlock;
import java.awt.geom.*;
import javax.media.jai.*;
import javax.swing.*;

public class IconJAI implements Icon {

    /** The source RenderedImage. */


    protected RenderedImage im;
    /** The image's SampleModel. */


    protected SampleModel sampleModel;
    /** The image's ColorModel or one we supply. */


    protected ColorModel colorModel;

    protected int minX;
    protected int minY;
    protected int width;
    protected int height;
    protected Rectangle imageBounds;

    /** The image's min X tile. */


    protected int minTileX;
    /** The image's max X tile. */


    protected int maxTileX;
    /** The image's min Y tile. */


    protected int minTileY;
    /** The image's max Y tile. */


    protected int maxTileY;
    /** The image's tile width. */


    protected int tileWidth;
    /** The image's tile height. */


    protected int tileHeight;
    /** The image's tile grid X offset. */


    protected int tileGridXOffset;
    /** The image's tile grid Y offset. */


    protected int tileGridYOffset;

    /** The pixel to display in the upper left corner or the canvas. */ 


    protected int originX;
    /** The pixel to display in the upper left corner or the canvas. */ 


    protected int originY;

    private Color grayColor = new Color(192, 192, 192);

    private Color backgroundColor = null;
    
    public IconJAI(RenderedImage im) {
        int mx = im.getMinX();
        int my = im.getMinY();
        if ((mx < 0) || (my < 0)) {
            ParameterBlock pb = new ParameterBlock();
            pb.addSource(im);
            pb.add((float)Math.max(-mx, 0));
            pb.add((float)Math.max(-my, 0));
            pb.add(new InterpolationNearest());
            im = JAI.create("translate", pb, null);
        }
	
        this.im = im;
        this.sampleModel = im.getSampleModel();
        this.colorModel = im.getColorModel();
        if (this.colorModel == null) {
            this.colorModel =
                PlanarImage.createColorModel(im.getSampleModel());
        }
        if (this.colorModel == null) {
            throw new IllegalArgumentException(
                "IconJAI is unable to display supplied RenderedImage.");
        }

        if (this.colorModel.getTransparency() != Transparency.OPAQUE) {
            Object col = im.getProperty("background_color");
            if (col != null) {
                backgroundColor = (Color)col;
            } else {
                backgroundColor = new Color(0, 0, 0);
            }
        }

        minX = im.getMinX();
        minY = im.getMinY();
        width = im.getWidth();
        height = im.getHeight();
        imageBounds = new Rectangle(minX, minY, width, height);

        minTileX = im.getMinTileX();
        maxTileX = im.getMinTileX() + im.getNumXTiles() - 1;
        minTileY = im.getMinTileY();
        maxTileY = im.getMinTileY() + im.getNumYTiles() - 1;
        tileWidth  = im.getTileWidth();
        tileHeight = im.getTileHeight();
        tileGridXOffset = im.getTileGridXOffset();
        tileGridYOffset = im.getTileGridYOffset();

        originX = originY = 0;
    }

    public int getIconWidth() {
        return im.getWidth();
    }

    public int getIconHeight() {
        return im.getHeight();
    }

    public RenderedImage getImage() {
        return im;
    }

    public void paintIcon(Component c, Graphics g, int x, int y) {
        Graphics2D g2D = null;
        if (g instanceof Graphics2D) {
            g2D = (Graphics2D)g;
        } else {
            System.err.println("IconJAI: not a Graphics2D.");
            return;
        }

        // Get the clipping rectangle and translate it into image coordinates. 
        Rectangle clipBounds = g.getClipBounds();

        int transX = x;
        int transY = y;

        // Determine the extent of the clipping region in tile coordinates.
        int txmin, txmax, tymin, tymax;
        int ti, tj;

        // Constrain drawing to the active image area
        Rectangle imageRect = new Rectangle(minX + transX,
                                            minY + transY,
                                            width, height);
        if (clipBounds == null) {
     	    clipBounds = new Rectangle(0,0,c.getWidth(), c.getHeight());
            // old way - may be calculating the entire image
	    // txmin = minTileX;
            // txmax = maxTileX;
            // tymin = minTileY;
            // tymax = maxTileY;
	    // g2D.setClip(imageRect);
	}
	  
	//txmin = XtoTileX(clipBounds.x);
	txmin = PlanarImage.XToTileX(clipBounds.x - transX, tileGridXOffset, tileWidth);
	txmin = Math.max(txmin, minTileX);
	txmin = Math.min(txmin, maxTileX);
	    


	//txmax = XtoTileX(clipBounds.x + clipBounds.width - 1);
	txmax = PlanarImage.XToTileX(clipBounds.x + clipBounds.width - 1 - transX,
			    tileGridXOffset, tileWidth );
	txmax = Math.max(txmax, minTileX);
	txmax = Math.min(txmax, maxTileX);
	
	//            tymin = YtoTileY(clipBounds.y);
	tymin = PlanarImage.YToTileY(clipBounds.y - transY, tileGridYOffset, tileHeight);
	tymin = Math.max(tymin, minTileY);
	tymin = Math.min(tymin, maxTileY);
	
	//            tymax = YtoTileY(clipBounds.y + clipBounds.height - 1);
	tymax = PlanarImage.YToTileY(clipBounds.y + clipBounds.height - 1 - transY,
			    tileGridYOffset, tileHeight);
	tymax = Math.max(tymax, minTileY);
	tymax = Math.min(tymax, maxTileY);
	
	g2D.clip(imageRect);
	
        if (backgroundColor != null) {
            g2D.setColor(backgroundColor);
        }

        // Loop over tiles within the clipping region
        for (tj = tymin; tj <= tymax; tj++) {
            for (ti = txmin; ti <= txmax; ti++) {
                int tx = PlanarImage.tileXToX(ti, tileGridXOffset, tileWidth);
                int ty = PlanarImage.tileYToY(tj, tileGridYOffset, tileHeight);

                Raster tile = im.getTile(ti, tj);
                DataBuffer dataBuffer = tile.getDataBuffer();

                Point origin = new Point(0, 0);
                WritableRaster wr =
                    tile.createWritableRaster(sampleModel,
                                              dataBuffer,
                                              origin);

                BufferedImage bi = new BufferedImage(colorModel,
                                                     wr,
                                                     false, null);

                AffineTransform transform =
                    AffineTransform.getTranslateInstance(tx + transX,
                                                         ty + transY);

                if (backgroundColor != null) {
                    g2D.fillRect(tx + transX, ty + transY,
                                 tileWidth, tileHeight);
                }
                g2D.drawImage(bi, transform, null);
            }
        }
    }
}
