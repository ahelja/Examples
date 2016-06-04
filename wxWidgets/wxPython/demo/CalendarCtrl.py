
import  wx
import  wx.calendar

#----------------------------------------------------------------------

class TestPanel(wx.Panel):
    def __init__(self, parent, ID, log):
        wx.Panel.__init__(self, parent, ID)
        self.log = log

        cal = wx.calendar.CalendarCtrl(self, -1, wx.DateTime_Now(), pos = (25,50),
                             style = wx.calendar.CAL_SHOW_HOLIDAYS
                             | wx.calendar.CAL_SUNDAY_FIRST
                             | wx.calendar.CAL_SEQUENTIAL_MONTH_SELECTION
                             )

        self.Bind(wx.calendar.EVT_CALENDAR, self.OnCalSelected, id=cal.GetId())

        b = wx.Button(self, -1, "Destroy the Calendar", pos = (275, 50))
        self.Bind(wx.EVT_BUTTON, self.OnButton, id= b.GetId())
        self.cal = cal

        # Set up control to display a set of holidays:
        self.Bind(wx.calendar.EVT_CALENDAR_MONTH, self.OnChangeMonth, cal)
        self.holidays = [(1,1), (10,31), (12,25) ]    # (these don't move around)
        self.OnChangeMonth()

    def OnButton(self, evt):
        if self.cal is not None:
            self.cal.Destroy()
            self.cal = None

    def OnCalSelected(self, evt):
        self.log.write('OnCalSelected: %s\n' % evt.GetDate())

    def OnChangeMonth(self, evt=None):
        cur_month = self.cal.GetDate().GetMonth() + 1   # convert wxDateTime 0-11 => 1-12

        for month, day in self.holidays:
            if month == cur_month:
                self.cal.SetHoliday(day)

#----------------------------------------------------------------------

def runTest(frame, nb, log):
    win = TestPanel(nb, -1, log)
    return win

#----------------------------------------------------------------------


overview = """\
<html><body>
<h2>CalendarCtrl</h2>

Yet <i>another</i> calendar control.  This one is a wrapper around the C++
version described in the docs.  This one will probably be a bit more efficient
than the one in wxPython.lib.calendar, but I like a few things about it better,
so I think both will stay in wxPython.
"""


if __name__ == '__main__':
    import sys,os
    import run
    run.main(['', os.path.basename(sys.argv[0])] + sys.argv[1:])

