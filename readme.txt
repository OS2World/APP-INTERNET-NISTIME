This is a recompiled version of NISTIME.EXE 0.1b.

This compile is based on the port to OS/2 by Michael Thompson in 1993.  The
contents of the original port are contained in the \NISTIME directory.  For
information or instructions on using NISTIME.EXE please refer to the original
files.

Aside from SOCKS support and possibly similar side effects of the modern
TCP/IP stacks, there is no functional difference between this version of
NISTIME.EXE and the previous version.  The source code was only changed
where necessary to comply with changes IBM has made to the TCP/IP APIs.

My primary reason for recompiling NISTIME.EXE was to link it with the new
TCP/IP stack in use with any modern version of OS/2.  My primary reason for
doing that was that the newer stacks include support for SOCKS imbedded in the
stack, and I needed to get NISTIME.EXE to work through a SOCKS firewall.

According to the original makefile Michael used IBM CSET/2 at CSD level 046,
the OS/2 2.0 toolkit, and IBM TCP/IP 1.2.1 with CSD's UN37938 and UN37942.

In compiling the updated version I used IBM VisualAge C++ 3.0 at fixpak 8,
the OS/2 3.0 toolkit, and the MPTS toolkit at service level WR08421.

dshields@direct.ca
