=======================================================================
nistime        for OS/2 2.0
-----------------------------------------------------------------------
version 0.1b


Ported by Michael Thompson, 1993
Original code from NIST - see liscence below
=======================================================================


Description
===========

This program access the NIST time/date server via TCP/IP and obtains the
standard time.  This time is compared to the current setting of the
internal clock, and the difference is reported to stadard output.  With
appropriate flags, this program will also reset the internal clock
to agree with the NIST standard.  TCP/IP must already be installed and
configured for this program to work.

The OS/2 implementation is a port of the UNIX equivalent named program
available from anonymous ftp at time_a.timefreq.bldrdoc.gov.  It has been
appropriately hacked (quite substantially, though unnecessarily) to work
with the IBM CSET/2 compiler and the IBM TCP/IP 1.2.1 package.

NISTIME uses the timezone information set in the TZ variable to convert
from UTC to local time.  Copy the value from one of your UNIX systems
or create form "set TZ=CST6CDT" where the 6 represents number of hours
from UTC (6 for Central Time in the US).  Default if no TZ is set is
EST5EDT, Eastern Standard Time.

The following help may be obtained by typing "nistime -?".

  Syntax: nistime [ options ]

  Description:
      This program connects to the daytime service on the NIST time server
      time_a.timefreq.bldrdoc.gov using tcp/ip port 13.  The time server
      returns the current time which is then compared with the time of the
      local clock. The difference may be used to adjust the time of the
      local clock using DosSetDateTime().

  Options:
      -m0  Terse mode.  Only the time difference in seconds is written
           to standard output.  A positive value means local clock is fast.
      -m1  Display short time messages. (also -M and default)
      -m2  Display verbose time messages.

      -s0  Do not set the local clock.
      -s1  Set the local clock if the server is healthy. (also -S)
      -s2  Set the local clock even if the server is not healthy.
      -s3  Query operator before setting the clock. (default)

  Bugs: (1) This version does not estimate transmission delays in the network.
            Not a big deal since time is set to the nearest second only.
        (2) Time is instantaneously reset and thus may be non-monotonic.

The default call will access the server, compare the time, and if different,
query if you want to reset the internal clock.


Installation
============
NISTIME.EXE should be placed a directory such as \tcpip\bin, accessible from
the path.  To run automatically, modify either STARTUP.CMD to run NISTIME
following TCPSTART.CMD, or hack TCPSTART.CMD so this is run last.


Contents of Archive
===================
nistime.exe             The executable
nistime.c               Source code
nistime.man             Original man page accompanying the UNIX version
makefile                Makefile to create nistime.exe given TCP/IP 1.2.1
                        programming package and 32 bit header file CSD.
readme                  This file


Licensing
=========
From the original code:
   This software was developed with US Government support and it may not be
   sold, restricted or licensed.  You may duplicate this program provided
   that this notice remains in all of the copies, and you may give it to
   others provided they understand and agree to this condition.

Comments and suggestions for changes in this port may be sent to me at:
      Michael Thompson
      Cornell University
      Dept. of Materials Science
      129 Bard Hall
      tommy@msc.cornell.edu


NO WARRANTY
===========
As you guess, I disclaim all responsibility for any problems arising from
the use of this program.  This program is provided "AS IS" without warranty
of any kind, either expressed or implied, including, but not limited to, the
implied warranties of merchantability and fitness for a particular purpose.


Know bugs
=========
- This version does not estimate transmission delays in the network.
  Not a big deal since time is set to the nearest second only.

- The time is instantaneously reset using DosSetDateTime.  If the local
  clock was ahead of the standard, then time will be momentarily 
  non-monotonic.  This has the potential to screw up file timestamps used
  in makefiles.  I don't think it is worth the effort to create the
  equivalent of adjtime (2) to avoid this problem.

- There is no icon attached to this program.  If someone wants to make an
  appropriate one, please send it to me to be added in any subsequent
  revisions.


History
=======

Version 0.1  1993 May 22 (Initial release to public)
----------------------------------------------------
- Initial release

Version 0.1b  1993 June 11 (Whoops bug fix)
--------------------------------------------
- Original version failed to use the TZ variable to access
  the local timezone.  Always set clock assuming you lived
  on the East Coast (no surprise it always worked for me!)
