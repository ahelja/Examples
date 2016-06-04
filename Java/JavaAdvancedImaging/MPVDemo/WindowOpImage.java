/**
 * @(#)WindowOpImage.java	15.3 03/05/20
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


 
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.awt.image.*;
import java.util.*;
import javax.media.jai.*;
import javax.swing.*;

/** An <code>OpImage</code> to cache tiles for display.  The reservoir (cache) of
 *  tiles includes a corona of tiles around the display area.  Heuristics are
 *  added to control when tiles are kept in a local <code>HashTable</code>.
 * 
 */



public class WindowOpImage extends PointOpImage {

    private Hashtable tileQueue = new Hashtable();

    // The display area has coordinates and extent defined by <code>window</code>.
    // The display plus a corona are defined by <code>largeWindow</code>.
    private Rectangle window = new Rectangle();
    private Rectangle largeWindow;

    /** Tunable parameters (see setWindow method)
     *  Project: tune these parameter values for best performance
     *           or allow public access modification by users
     */


    private static final int TILE_THRESH = 10;
    private static final float AREA_THRESH = 0.2F;
    private static final int WINDOW_CORONA = 300;

    /** Constructor with a <code>window</code>.
     *
     * @param source image
     * @param window dislay rectangle coordinates and dimensions
     */


    public WindowOpImage(RenderedImage source, Rectangle window) {
	super(source, null, null, false);
	setWindow(window);
    }

    /** Constructor without a source <code>window</code>.
     *  Note: the default window has zero width and height.
     *
     * @param source image
     */


    public WindowOpImage(RenderedImage source) {
	super(source, null, null, false);
    }

    /** getTile fetches a source tile (using its getTile method) and
     *  stores the tile in the tileQueue.
     */


    public Raster getTile(int tileX, int tileY) {
	ArrayList v = new ArrayList(2);
	v.add(0, new Integer(tileX));
	v.add(1, new Integer(tileY));

	Raster tile = (Raster)tileQueue.get(v);
	if (tile == null) {
	    tile = this.getSourceImage(0).getTile(tileX, tileY);
	    if (tile != null)
		tileQueue.put(v, tile);
	}

	return tile;

    } // getTile

    /** This method re-assigns the display window and refreshes the local tile cache 
     *  <code>tileQueue</code> under several conditions described below.
     *
     * setWindow uses a heuristic to cache tiles for a slightly larger window
     * than what is currently requested.  Think of this op in the chain acting
     * as a tile reservoir (or cache).  The display update heuristic goes like this:
     *
     *    1) If the combined area (number of pixels) of the previous display
     *       position and the new display position contains less than TILE_THRESH
     *       tiles, then let JAI pull tiles all the way through the op chain to
     *       display the image.  TILE_THRESH is a tunable parameter.
     *
     *    2) If the overlap area between the previous and current display windows
     *       is a small fraction (AREA_THRESH) of the combined previous and new
     *       display areas, then add tiles to this nodes reservoir to cover the
     *       display plus a WINDOW_CORONA (also tunable) around the edges.
     *
     * If <code>setWindow</code> were to simply do nothing and return, then the JAI
     * pull mechanism would handle updates but it wouldn't add the corona tiles around
     * the display and it would add useless overhead.
     *
     * @param window the current display location within the image
     */


    public void setWindow(Rectangle window) {
	Rectangle oldWindow = this.window;

	Rectangle overlap = oldWindow.intersection(window);
	double overlapArea = ((double)overlap.width) * overlap.height;
	double totalArea = ((double)oldWindow.width) * oldWindow.height +
			   ((double)window.width) * window.height;
	
	this.window = window;
	this.largeWindow = new Rectangle(window);

        // Extend the display rectangle by WINDOW_CORONA (tunable parameter)
	largeWindow.grow(WINDOW_CORONA, WINDOW_CORONA);
	removeTilesFromQueue();

	double tileSize = ((double)getTileWidth()) * getTileHeight();

        // If the number of tiles is small, let the JAI pull mechanism handle
	// fetching tiles.  TILE_THRESH is a tunable parameter.
	if (totalArea < TILE_THRESH * tileSize)
	    return;

        // If the overlap area is a small fraction of the total previous and
	// current display areas, add to the reservoir.  If the overlap were high
	// then it is likely that the loaded corona would have the needed tiles.
	// AREA_THRESH is a tunable parameter.
	if (overlapArea / (totalArea + 1) < AREA_THRESH) 
	    addTilesToQueue();

    } // setWindow

    /** addTilesToQueue refreshes the local tile cache <code>tileQueue</code>.
     *  Tile indicies are computed for all the tiles that cover the
     *  <code>largeWindow</code> rectangle.  It also displays a progress bar
     *  when tiles are being loaded.
     */


    public void addTilesToQueue() {

        // Compute tile indicies for the tiles that overlap the largeWindow
	int minXTileInWindow = XToTileX(largeWindow.x);
	int maxXTileInWindow = XToTileX(largeWindow.x + largeWindow.width);
	int minYTileInWindow = YToTileY(largeWindow.y);
	int maxYTileInWindow = YToTileY(largeWindow.y + largeWindow.height);

        // Set up and display a simple progress bar
	JFrame bar = new JFrame("Loading tiles ......");
	bar.setBounds(100, 100, 300, 20);
	bar.setResizable(false);
	bar.show();

        // Fetch tiles that overlap the largeWindow and keep track of them
	// in a queue.  See above for getTile declaration.
	for (int y = minYTileInWindow, finished = 1; y <= maxYTileInWindow; y++)
	    for (int x = minXTileInWindow; x <= maxXTileInWindow; x++, finished++) {
		getTile(x, y);
	    }

        // Remove the progress bar
	bar.dispose();

    } // addTilesToQueue

    /** removeTilesFromQueue iterates over the tile queue entries saved in getTile
     *  (above) and tests them for overlap with the largeWindow.  If tiles in the
     *  queue don't overlap the largeWindow, they are removed from the tileQueue.
     */ 


    private void removeTilesFromQueue() {
	if (tileQueue != null && !tileQueue.isEmpty()) {
	    Set keySet = tileQueue.keySet();

	    if (!keySet.isEmpty()) {
		Iterator keyIterator = keySet.iterator();

		while (keyIterator.hasNext()) {
		    Object key = keyIterator.next();
		    Raster tile = (Raster)tileQueue.get(key);

		    if (tile == null)
			continue;
		    if (!largeWindow.intersects(tile.getBounds())) {
			keyIterator.remove();
		    }
		}
	    }
	}

    } // removeTilesFromQueue

    /** return the current window that includes a corona around the current display. */


    public Rectangle getLoadedRectangle() {
       return largeWindow;
    }

} // WindowOpImage
