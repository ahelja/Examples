
import  wx
import  images

#---------------------------------------------------------------------------

class TestToolBar(wx.Frame):
    def __init__(self, parent, log):
        wx.Frame.__init__(self, parent, -1, 'Test ToolBar', size=(500, 300))
        self.log = log
        self.timer = None
        self.Bind(wx.EVT_CLOSE, self.OnCloseWindow)

        wx.Window(self, -1).SetBackgroundColour(wx.NamedColour("WHITE"))

        # Use the wxFrame internals to create the toolbar and associate it all
        # in one tidy method call.
        tb = self.CreateToolBar( wx.TB_HORIZONTAL
                                 | wx.NO_BORDER
                                 | wx.TB_FLAT
                                 | wx.TB_TEXT
                                 )

        # Here's a 'simple' toolbar example, and how to bind it using SetToolBar()
        #tb = wx.ToolBarSimple(self, -1, wx.DefaultPosition, wx.DefaultSize,
        #               wx.TB_HORIZONTAL | wx.NO_BORDER | wx.TB_FLAT)
        #self.SetToolBar(tb)
        # But we're doing it a different way here.

        log.write("Default toolbar tool size: %s\n" % tb.GetToolBitmapSize())

        self.CreateStatusBar()

        tb.AddSimpleTool(10, images.getNewBitmap(), "New", "Long help for 'New'")
        #tb.AddLabelTool(10, "New", images.getNewBitmap(), shortHelp="New", longHelp="Long help for 'New'")
        self.Bind(wx.EVT_TOOL, self.OnToolClick, id=10)
        self.Bind(wx.EVT_TOOL_RCLICKED, self.OnToolRClick, id=10)

        tb.AddSimpleTool(20, images.getOpenBitmap(), "Open", "Long help for 'Open'")
        self.Bind(wx.EVT_TOOL, self.OnToolClick, id=20)
        self.Bind(wx.EVT_TOOL_RCLICKED, self.OnToolRClick, id=20)

        tb.AddSeparator()
        tb.AddSimpleTool(30, images.getCopyBitmap(), "Copy", "Long help for 'Copy'")
        self.Bind(wx.EVT_TOOL, self.OnToolClick, id=30)
        self.Bind(wx.EVT_TOOL_RCLICKED, self.OnToolRClick, id=30)

        tb.AddSimpleTool(40, images.getPasteBitmap(), "Paste", "Long help for 'Paste'")
        self.Bind(wx.EVT_TOOL, self.OnToolClick, id=40)
        self.Bind(wx.EVT_TOOL_RCLICKED, self.OnToolRClick, id=40)

        tb.AddSeparator()

        tool = tb.AddCheckTool(50, images.getTog1Bitmap(),
                               shortHelp="Toggle this")
        self.Bind(wx.EVT_TOOL, self.OnToolClick, id=50)

##         tb.AddCheckTool(60, images.getTog1Bitmap(), images.getTog2Bitmap(),
##                         shortHelp="Toggle with 2 bitmaps")
##         self.Bind(EVT_TOOL, self.OnToolClick, id=60)

        self.Bind(wx.EVT_TOOL_ENTER, self.OnToolEnter)
        self.Bind(wx.EVT_TOOL_RCLICKED, self.OnToolRClick) # Match all
        self.Bind(wx.EVT_TIMER, self.OnClearSB)

        tb.AddSeparator()
        cbID = wx.NewId()

        tb.AddControl(
            wx.ComboBox(
                tb, cbID, "", choices=["", "This", "is a", "wxComboBox"],
                size=(150,-1), style=wx.CB_DROPDOWN
                ))
                
        self.Bind(wx.EVT_COMBOBOX, self.OnCombo, id=cbID)
        tb.AddControl(wx.TextCtrl(tb, -1, "Toolbar controls!!", size=(150, -1)))

        # Final thing to do for a toolbar is call the Realize() method. This
        # causes it to render (more or less, that is).
        tb.Realize()


    def OnToolClick(self, event):
        self.log.WriteText("tool %s clicked\n" % event.GetId())
        tb = self.GetToolBar()
        tb.EnableTool(10, not tb.GetToolEnabled(10))

    def OnToolRClick(self, event):
        self.log.WriteText("tool %s right-clicked\n" % event.GetId())

    def OnCombo(self, event):
        self.log.WriteText("combobox item selected: %s\n" % event.GetString())

    def OnToolEnter(self, event):
        self.log.WriteText('OnToolEnter: %s, %s\n' % (event.GetId(), event.GetInt()))

        if self.timer is None:
            self.timer = wx.Timer(self)

        if self.timer.IsRunning():
            self.timer.Stop()

        self.timer.Start(2000)
        event.Skip()


    def OnClearSB(self, event):  # called for the timer event handler
        self.SetStatusText("")
        self.timer.Stop()
        self.timer = None


    def OnCloseWindow(self, event):
        if self.timer is not None:
            self.timer.Stop()
            self.timer = None
        self.Destroy()

#---------------------------------------------------------------------------

class TestPanel(wx.Panel):
    def __init__(self, parent, log):
        self.log = log
        wx.Panel.__init__(self, parent, -1)

        b = wx.Button(self, -1, "Show the ToolBar sample", (50,50))
        self.Bind(wx.EVT_BUTTON, self.OnButton, b)


    def OnButton(self, evt):
        win = TestToolBar(self, self.log)
        win.Show(True)


#---------------------------------------------------------------------------


def runTest(frame, nb, log):
    win = TestPanel(nb, log)
    return win

#---------------------------------------------------------------------------




overview = """\
wx.ToolBar is a narrow strip of icons on one side of a frame (top, bottom, sides)
that acts much like a menu does, except it is always visible. Additionally, actual
wxWindows controls, such as wx.TextCtrl or wx.ComboBox, can be added to the toolbar
and used from within it.

Toolbar creation is a two-step process. First, the toolbar is defined using the
various Add* methods of wx.ToolBar. Once all is set up, then wx.Toolbar.Realize()
must be called to render it.

wx.Toolbar events are also propogated as Menu events; this is especially handy when
you have a menu bar that contains items that carry out the same function. For example,
it is not uncommon to have a little 'floppy' toolbar icon to 'save' the current file 
(whatever it is) as well as a FILE/SAVE menu item that does the same thing. In this
case, both events can be captured and acted upon using the same event handler
with no ill effects.

If there are cases where a toolbar icon should *not* be associated with a menu item,
use a unique ID to trap it.

There are a number of ways to create a toolbar for a wx.Frame. wx.Frame.CreateToolBar() 
does all the work except it adds no buttons at all unless you override the virtual method
OnCreateToolBar(). On the other hand, you can just subclass wx.ToolBar and then use
wx.Frame.SetToolBar() instead.

Note that wx.TB_DOCKABLE is only supported under GTK. An attempt to alleviate this
is provided in wx.lib.floatbar, but it is not formally supported.
"""


if __name__ == '__main__':
    import sys,os
    import run
    run.main(['', os.path.basename(sys.argv[0])] + sys.argv[1:])

