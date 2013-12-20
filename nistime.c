/* nistime.c - query and set time from NIST server */
/* icc /O -Sp2 -Q+ -N30 -W3 -Sm -G5 -B"/NOI" nistime.c tcp32dll.lib so32dll.lib */

/* ------------------------------ */
/* Feature test macros            */
/* ------------------------------ */
/* #define _POSIX_SOURCE                  /* Always require POSIX standard */

/* ------------------------------ */
/* Standard include files         */
/* ------------------------------ */
#define INCL_DOSDATETIME
#include <os2.h>

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <types.h>                     /* Will be OS/2 TCP/IP types.h file */
#include <sys/socket.h>                /* More TCP/IP include files        */
#include <netinet/in.h>
/* #include <arpa/inet.h> */

/* ------------------------------ */
/* Local include files            */
/* ------------------------------ */

/* ------------------------------- */
/* My local typedef's and defines  */
/* ------------------------------- */
#define  panic    SysPanic(__FILE__, __LINE__)

#define  NIST_TIME_INET_ADDRESS  "132.163.135.130"    /* time.nist.gov   */
#define  NIST_TIME_PORT          13                   /* specific port   */
#define  UNIX_EPOCH_MJD          40587                /* UNIX epoch date */

/* ------------------------------- */
/* My external function prototypes */
/* ------------------------------- */

/* ------------------------------- */
/* My internal function prototypes */
/* ------------------------------- */
static int sw(int argc, char *arg, char *let, long *val);
static int ShiftTime(time_t diff);
static void PrintUsage(void);

/* ------------------------------- */
/* My usage of other external fncs */
/* ------------------------------- */

/* ------------------------------- */
/* Locally defined global vars     */
/* ------------------------------- */


/* ---------------------------------------------------------------------------
- client process to connect to the daytime service via port 13 from nist
- time server.
-
- This OS/2 implementation is a port of the unix utility by the same name
- obtained by anonymous FTP from time_a.timefreq.bldrdoc.gov.  My apologies
- to Judah Levine for the hack of his code.  For questions about this OS/2
- implementation, contact:
-
-   Michael Thompson
-   Cornell University
-   Dept. of Materials Science
-   129 Bard Hall
-   tommy@msc.cornell.edu
-
-
- The client process uses the time message received to check (and optionally
- to set) the time of the local clock.  the comparison assumes that the local
- clock keeps time in seconds from 1/1/70 which is the UNIX standard, and it
- computes the time difference by converting the received MJD to seconds
- since 1/1/70 and adding the received hr, min and sec, also converted to
- seconds.  If the local machine keeps time in some other way, then the
- comparison method will have to change, but the rest should be okay.
-
- This software was developed with US Government support and it may not be
- sold, restricted or licensed.  You may duplicate this program provided
- that this notice remains in all of the copies, and you may give it to
- others provided they understand and agree to this condition.
-
- This program and the time protocol it uses are under development and the
- implementation may change without notice.
-
- For questions or additional information, contact:
-
-   Judah Levine
-   Time and Frequency Division
-   NIST/847
-   325 Broadway
-   Boulder, Colorado 80303
-   (303) 492 7785
-   jlevine@time_a.timefreq.bldrdoc.gov
--------------------------------------------------------------------------- */
int main(int argc, char *argv[]) {

/* Internet socket access parameters */
   char *cp   = NIST_TIME_INET_ADDRESS;   /* server address in . notation  */
   int  pserv = NIST_TIME_PORT;           /* port for time service         */
   long address;                          /* ip address                    */
   struct sockaddr_in *sin;               /* socket address structure      */
   int s;                                 /* socket number                 */
   int length;                            /* size of message               */
   char buf[132];                         /* holds message                 */

   time_t my_time, nist_time;             /* Time since epoch              */
   time_t diff;                           /* time difference, local - NIST */

/* Values obtained from the internet message */
   long mjd0  = UNIX_EPOCH_MJD;           /* MJD of the UNIX EPOCH         */
   long mjd;                              /* holds parsed received mjd     */
   int yr,mo,dy,hr,min,sec;               /* holds parsed received time    */
   int day_light,                         /* Daylight savings flag         */
       leap_sec,                          /* Leap second status            */
       health;                            /* Health of server status       */

/* Local variables */
   char achr;                             /* Random character              */
   char let;                              /* command-line letter           */
   long val;                              /* command line value            */

/* ---------------------------------------------------------------------------
-  the following variables define what this program should do and what
-  output should be produced.  The values below are the defaults which
-  may be changed by characters on the command line as described below.
-  The effect of each variable is as follows:
-
- Command-Line Switch   effect
-  -m0      msg  = 0 Do not produce any messages; only time difference is
-                    written to standard output.
-  -m1 or -M     = 1 Produce short messages.
-  -m2           = 2 Produce longer messages.
-
-  -s0      set  = 0 Do not adjust local time.
-  -s1 or -S     = 1 Adjust local time if server is healthy.
-  -s2           = 2 Adjust local time even if server is unhealthy.
-  -s3           = 3 Ask operator first.
--------------------------------------------------------------------------- */
int msg = 1;                        /* default is short messages           */
int set = 3;                        /* default is ask before setting clock */

/* parse command line switches */
   ++argv; --argc;                                 /* Skip command name */
   while (sw(argc, argv[0], &let, &val) != 0) {    /* Switch present    */
      switch (let) {
         case '?':
                        case 'h':
                        case 'H':
            PrintUsage();
            return(EXIT_SUCCESS);
         case 'm':
            msg=val;
            break;
         case 's':
            set=val;
            break;
         case 'S':
            set=1;
            break;
         case 'M':
            msg=1;
            break;
         default:
            fprintf(stderr, "\nSwitch %c not recognized.\n", let);
            break;
      }
      argc--;  /* decrement argument counter */
      argv++;  /* and increment pointer      */
   }

/* -----------------------------------------------------------------
-- (1) Make sure we are using the TZ variable
-- (2) Convert xxx.xxx internet address to internal format
-- (3) allocate space for socket info, and fill int
-- (4) Create socket
-- (5) Connect to server.  Note this is a stream server and
--     record boundaries are not preserved.
-- (6) Query local time via time() call
-- (7) Close socket and free allocated memory
----------------------------------------------------------------- */
        _tzset();                                                                                                               /* Get timezone from TZ */

   sock_init();                                       /* Initialize sockets */
   if ( (address=inet_addr(cp)) == -1) {              /* Internal format */
      fprintf(stderr,"Internet address error.\n");
      return(EXIT_FAILURE);
   }
   if ( (sin = calloc(sizeof(*sin), 1)) == NULL) {    /* Allocate space  */
      fprintf(stderr, "Unable to allocate memory for socket info.\n");
      return(EXIT_FAILURE);
   }
   sin->sin_family      = AF_INET;                    /* Fill in request */
   sin->sin_addr.s_addr = address;
   sin->sin_port        = htons(pserv);

/* Now, create socket, open and read */
   if ( (s = socket(AF_INET, SOCK_STREAM, 0) ) < 0) { /* Get a socket    */
      fprintf(stderr,"Socket creation error.\n");
      return(EXIT_FAILURE);
   } else if (connect(s, (struct sockaddr *) sin, sizeof(*sin) ) < 0) {
      fprintf(stderr, "Failed to connect to time server: %d\n", sock_errno());
      return(EXIT_FAILURE);
   }
   if ( (length = recv(s, buf, sizeof(buf)-1, 0)) <= 0) {
      fprintf(stderr, "No response from NIST server.\n");
      soclose(s);
      return(EXIT_FAILURE);
   }
   buf[length] = '\0';                                   /* terminate msg  */

/* Immediately, get local time as well for comparison */
   if ( (my_time = time(NULL)) == (time_t) -1) {
      fprintf(stderr, "Unable to determine local clock time\n");
      return(EXIT_FAILURE);
   }

/* And close socket, free allocated space */
   soclose(s);                      /* Close port and free memory now      */
   free(sin);

/* --------------------------------------------------------------------------
-- convert received time to seconds since EPOCH and subtract this from the
-- local time in the same units.  Since POSIX makes no specification about
-- times less than 1 second, and the internet message only goes to seconds,
-- look only this far and ignore smaller differences.  Fortunately, the delay
-- in the net should be no more than about 0.05 seconds (according to NIST).
--------------------------------------------------------------------------- */
   sscanf(buf," %ld %2d-%2d-%2d %2d:%2d:%2d %d %d %d",
      &mjd, &yr, &mo, &dy, &hr, &min, &sec, &day_light, &leap_sec, &health);
   nist_time = 86400*(mjd-mjd0) + 3600*hr + 60*min + sec;
   diff = my_time - nist_time;

/* --------------------------------------------------------------------------
-- Output various reports to stdout depending on the setting of msg
-- parameter.
--------------------------------------------------------------------------- */
/* Output desired messages */
   if (msg != 0) {
      printf(" Time message received:\n");
      printf("                        D  L\n");
      printf(" MJD  YY MM DD HH MM SS ST S H  Adv.%s", buf);
   }
   if (msg == 2) {                  /* longer messages selected */
      if( (day_light == 0) || (day_light > 51) ) {
         printf("Standard Time now in effect\n");
         if (day_light > 51)
           printf("Change to Daylight Saving Time in %d days\n",day_light-51);
      }
      if( (day_light <= 50) && (day_light > 1) ) {
         printf("Daylight Saving Time now in effect\n");
         if (day_light > 1 && day_light != 50)
            printf("Change to Standard Time in %d days\n", day_light-1);
      }
      if (day_light == 1)  printf("Standard time begins at 2am today\n");
      if (day_light == 51) printf("Daylight Saving Time begins at 2am today\n");
      if (leap_sec == 1)   printf("Leap Second to be added at end of month\n");
      if (leap_sec == 2)   printf("Second to be dropped at end of month\n");
   }
   if (msg == 0) {                           /* no messages except time diff */
      printf("%ld", diff);
   } else {
      printf("Local Clock - NIST = %ld second(s)\n", diff);
   }


/* --------------------------------------------------------------------------
-- Deal with possibly resetting the clock based on setting of set parameter
--------------------------------------------------------------------------- */
   if (set == 0 || diff == 0) {
      if (diff == 0 && msg != 0) printf("\n No adjustment is needed.\n");
      return(EXIT_SUCCESS);
   }

   if( (health != 0) && (msg != 0) )
      printf("\n Server is not healthy, adjustment not recommended.");

   achr = 'n';                         /* default is not to set */
   if (set == 3) {                     /* action depends on answer to query */
      printf("Do you want to adjust the local clock ? [y/n] ");
      fflush(stdout);
      achr = getchar();
   }

   if ( (set==2) || (set==1 && health==0) || (tolower(achr) == 'y') ) {
      if (ShiftTime(diff) != 0) {
         fprintf(stderr, "Unable to modify system clock\n");
         return(EXIT_FAILURE);
                } else {
                        if (msg != 0) printf("Local clock time reset to NIST standard\n");
      }
   }

   return(EXIT_SUCCESS);
}


/* ---------------------------------------------------------------------------
--
-- Inputs:  diff - Time difference from current setting
--------------------------------------------------------------------------- */
static int ShiftTime(time_t diff) {

   DATETIME os2_time;
   struct tm *tm;
   time_t newtime;
   int rc;

   DosGetDateTime(&os2_time);                /* Fill in unknown components */
   newtime = time(NULL) - diff;              /* Get corrected time      */
   tm = localtime(&newtime);                 /* Fill in the structure   */

   os2_time.hours      = tm->tm_hour;
   os2_time.minutes    = tm->tm_min;
   os2_time.seconds    = tm->tm_sec;
   os2_time.hundredths = 0;
   os2_time.day        = tm->tm_mday;
   os2_time.month      = tm->tm_mon  + 1;
   os2_time.year       = tm->tm_year + 1900;
   os2_time.weekday    = tm->tm_wday;

   rc = DosSetDateTime(&os2_time);
   return(rc);
}


/* ---------------------------------------------------------------------------
-  this subroutine parses switches on the command line.
-  switches are of the form -<letter><value>.  If one is found, a pointer to
-  the letter is returned in let and a pointer to the value in val as a
-  long integer.
-
-  parameters argc and argv are passed in from the main calling program.
-  Note that if argc = 0, no arguments are left.
-  if a switch is decoded, the value of the function is 1, otherwise zero.
-
-  a number following the letter is decoded as a decimal value unless it
-  has a leading x in which case it is decoded as hexadecimal.
--------------------------------------------------------------------------- */
static int sw(int argc, char *arg, char *let, long *val) {

/* either nothing is left or what is left is not a switch */
   if( (argc == 0) || (*arg != '-') ) {
     *let = '\0';
     *val = 0;
     return(0);
   }

   ++arg;                        /* Jump over the - sign */
   *let = *arg++;                /* Letter option        */
   if (*arg != 'x') {            /* if next char is not x, decode number */
      *val = atol(arg);
   } else {                      /* Use sscanf to interpret complex value */
      sscanf(arg+1, "%lx", val);
   }
   return(1);
}

/* ---------------------------------------------------------------------------
- Routine to print the usage in response to a -? option.
--------------------------------------------------------------------------- */
static void PrintUsage(void) {

   fputs(
"   Syntax: nistime [ options ]\n"
"\n"
"   Description:\n"
"      This program connects to the daytime service on the NIST time server\n"
"      time_a.timefreq.bldrdoc.gov using tcp/ip port 13.  The time server\n"
"      returns the current time which is then compared with the time of the\n"
"      local clock. The difference may be used to adjust the time of the\n"
"      local clock using DosSetDateTime().\n"
"\n"
"   Options:\n"
"     -m0  Terse mode.  Only the time difference in seconds is written\n"
"          to standard output.  A positive value means local clock is fast.\n"
"     -m1  Display short time messages. (also -M and default)\n"
"     -m2  Display verbose time messages.\n"
"\n"
"     -s0  Do not set the local clock.\n"
"     -s1  Set the local clock if the server is healthy. (also -S)\n"
"     -s2  Set the local clock even if the server is not healthy.\n"
"     -s3  Query operator before setting the clock. (default)\n"
"\n"
"Bugs: (1) This version does not estimate transmission delays in the network.\n"
"          Not a big deal since time is set to the nearest second only.\n"
"      (2) Time is instantaneously reset and thus may be non-monotonic.\n"
   , stdout);

   return;
}
