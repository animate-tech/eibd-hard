/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005-2011 Martin Koegler <mkoegler@auto.tuwien.ac.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

extern "C" {
#include <argp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <netdb.h>
}
#include <fcntl.h>
#include <sys/stat.h>
#include "layer3.h"
#include "localserver.h"
#include "inetserver.h"
#include "eibnetserver.h"
#include "groupcacheclient.h"
#include "eibdstate.h"
extern "C" {
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
};
#include <unistd.h>
#include "../bcu/addrtab.h"
#include "ip/ipv4net.h"

#include <vector>
#include <sstream>

static class Logs logger;

#define OPT_BACK_TUNNEL_NOQUEUE 1
#define OPT_BACK_TPUARTS_ACKGROUP 2
#define OPT_BACK_TPUARTS_ACKINDIVIDUAL 3
#define OPT_BACK_TPUARTS_DISCH_RESET 4


/** structure to store the arguments */
struct arguments
{
  /** port to listen */
  int port;
  /** path for unix domain socket */
  const char *name;
  /** path to pid file */
  const char *pidfile;
  /** path to trace log file */
  const char *daemon;
  /** trace level */
  int tracelevel;
  /** error level */
  int errorlevel;
  /** warning level */
  int warnlevel;
  /** infolog level */
  int infolevel;
  /** EIB address (for some backends) */
  eibaddr_t addr;
  /* EIBnet/IP server */
  bool tunnel;
  bool route;
  bool discover;
  bool groupcache;
  int backendflags;
  const char *serverip;

  int inbusqlen;
  int outbusqlen;
  int peerqlen;
  int clientsmax;
  int maxpacketspersecond;
  bool dropclientsoninterfaceloss;
  IPv4NetList ipnetfilters;

  uid_t userid;
  gid_t groupid;

  bool resetaddrtable;
};
/** storage for the arguments*/
struct arguments arg;

#define LOGANDDIE(args...) { \
    ERRORLOGSHAPE (&logger, LOG_EMERG, Logging::NOPOLICY, NULL, Logging::MSGNOHASH, ##args); \
                                                                        \
    if (arg.pidfile)                                                    \
       unlink (arg.pidfile);                                            \
                                                                        \
    exit (1);                                                           \
}

/** aborts program with a printf like message */
void
die (const char *msg, ...)
{
  va_list ap;
  va_start (ap, msg);
  vprintf (msg, ap);
  printf ("\n");
  va_end (ap);

  if (arg.pidfile)
    unlink (arg.pidfile);

  exit (1);
}


#include "layer2conf.h"

/** structure to store layer 2 backends */
struct urldef
{
  /** URL-prefix */
  const char *prefix;
  /** factory function */
  Layer2_Create_Func Create;
  /** cleanup function */
  void (*Cleanup) ();
};

/** list of URLs */
struct urldef URLs[] = {
#undef L2_NAME
#define L2_NAME(a) { a##_PREFIX, a##_CREATE, a##_CLEANUP },
#include "layer2create.h"
  {0, 0, 0}
};

void (*Cleanup) ();

/** determines the right backend for the url and creates it */
Layer2Interface *
Create (const char *url, int flags, Logs * t, int inquemaxlen, int outquemaxlen, int maxpacketspersecond, int peerquemaxlen,
    IPv4NetList &ipnetfilters)
{
  unsigned int p = 0;
  struct urldef *u = URLs;
  while (url[p] && url[p] != ':')
    p++;
  if (url[p] != ':')
    die ("not a valid url");
  while (u->prefix)
    {
      if (strlen (u->prefix) == p && !memcmp (u->prefix, url, p))
	{
	  Cleanup = u->Cleanup;
	  return u->Create (url + p + 1,  flags, t, inquemaxlen, outquemaxlen, maxpacketspersecond, peerquemaxlen,ipnetfilters);
	}
      u++;
    }
  die ("url not supported");
  return 0;
}

/** parses an EIB individual address */
eibaddr_t
readaddr (const char *addr)
{
  int a, b, c;
  sscanf (addr, "%d.%d.%d", &a, &b, &c);
  return ((a & 0x0f) << 12) | ((b & 0x0f) << 8) | ((c & 0xff));
}

/** version */
const char *argp_program_version = "eibd " VERSION;
/** documentation */
static char doc[] =
  "eibd -- a communication stack for EIB\n"
  "(C) 2005-2011 Martin Koegler <mkoegler@auto.tuwien.ac.at>\n"
  "    2006-     Dr. A. Przygienda <prz@zeta2.ch>, hard-hat rewrite & extensions"
  "supported URLs are:\n"
#undef L2_NAME
#define L2_NAME(a) a##_URL
#include "layer2create.h"
  "\n"
#undef L2_NAME
#define L2_NAME(a) a##_DOC
#include "layer2create.h"
  "\n";

/** documentation for arguments*/
static char args_doc[] = "URL";

/** option list */
static struct argp_option options[] = {
  {"listen-tcp", 'i', "PORT", OPTION_ARG_OPTIONAL,
   "listen at TCP port PORT (default 6720)"},
  {"listen-local", 'u', "FILE", OPTION_ARG_OPTIONAL,
   "listen at Unix domain socket FILE (default /tmp/eib)"},
  {"trace", 't', "LEVEL", 0, "set trace (debugging) level mask"},
  {"warn",  'w', "LEVEL", 0, "set warning level mask"},
  {"info",  'l', "LEVEL", 0, "set info log level mask"},
  {"error", 'f', "LEVEL", 0, "set error level"},
  {"eibaddr", 'e', "EIBADDR", 0,
   "set our own EIB-address to EIBADDR (default 0.0.1), for drivers, which need an address"},
   {"reset-addr-table", 'r', 0, 0, "reset the size of BCU interface address table on start"},
  {"pid-file", 'p', "FILE", 0, "write the PID of the process to FILE"},
  {"daemon", 'd', "FILE", OPTION_ARG_OPTIONAL,
   "start the programm as daemon, the output will be written to FILE, if the argument present"},
#ifdef HAVE_EIBNETIPSERVER
  {"Tunnelling", 'T', 0, 0,
   "enable EIBnet/IP Tunneling in the EIBnet/IP server"},
  {"Routing", 'R', 0, 0, "enable EIBnet/IP Routing in the EIBnet/IP server"},
  {"Discovery", 'D', 0, 0,
   "enable the EIBnet/IP server to answer discovery and description requests (SEARCH, DESCRIPTION)"},
  {"Server", 'S', "ip[:port]", OPTION_ARG_OPTIONAL,
   "starts the EIBnet/IP server part"},
#endif
#ifdef HAVE_GROUPCACHE
  {"GroupCache", 'c', 0, 0,
   "enable caching of group communication network state"},
#endif
#ifdef HAVE_EIBNETIPTUNNEL
  {"no-tunnel-client-queuing", OPT_BACK_TUNNEL_NOQUEUE, 0, 0,
   "do not assume KNXnet/IP Tunneling bus interface can handle parallel cEMI requests"},
#endif
#ifdef HAVE_TPUARTs
  {"tpuarts-ack-all-group", OPT_BACK_TPUARTS_ACKGROUP, 0, 0,
   "tpuarts backend should generate L2 acks for all group telegrams"},
  {"tpuarts-ack-all-individual", OPT_BACK_TPUARTS_ACKINDIVIDUAL, 0, 0,
   "tpuarts backend should generate L2 acks for all individual telegrams"},
  {"tpuarts-disch-reset", OPT_BACK_TPUARTS_DISCH_RESET, 0, 0,
   "tpuarts backend should should use a full interface reset (for Disch TPUART interfaces)"},
#endif
  {"InQueueMax", 'Q', "INT", OPTION_ARG_OPTIONAL,
   "restrict incoming queue length, will drop & warn after queue length from bus is exceeded, without argument default 255"},
  {"OutQueueMax", 'q', "INT", OPTION_ARG_OPTIONAL,
   "restrict outgoing queue length, will drop & warn after queue length to bus interface is exceeded, without argument default 255"},
  {"PeerQueueMax", 'P', "INT", OPTION_ARG_OPTIONAL,
   "restrict in/out queue length per client, start to drop when exceeded, without argument default 32"},
  {"ClientsMax", 'C', "INT", OPTION_ARG_OPTIONAL,
   "restrict maximum number of concurrent clients on server, without argument default 32"},
  {"PacketsPerSecond", 'B', "INT", OPTION_ARG_OPTIONAL,
   "throttling of sending to EIB interface to maximum telegrams per seconds, without argument default 10"},
   {"DropClientsOnInterfaceLoss", 'X', 0, 0,
       "drop attached clients when the underlying interface becomes unavailable",
   },
  {"run-as", 'U', "user[:group]", 0,
   "change to run as a specific user (and optional group)" },
   {"FilterSubnets", 'F', "comma separated list of IP subnets", OPTION_ARG_OPTIONAL,
       "enable filtering on subnets. Subnets (in 9.9.9.0/24 notation) to restrict all clients' source addresses. "
       "All servers (including EIB tunneling & routing) are restricting arriving clients, "
       "empty list filters for local interface subnets and 127.0.0.1/32. 127.0.0.1/32 is ALWAYS allowed." },
  {0}
};

static int _subversion_version = SVN_REVISION; // this will prevent compilation until a clean numeric, checked-in version!

int daemon_become_user(uid_t uid, gid_t gid, char *user)
{
        gid_t gids[10];
        int g = 0;

        if (setgroups(0, NULL) == -1 || (g = getgroups(0, NULL)) != 0)
        {
                /* FreeBSD always returns the primary group */

                if (g != 1 || getgroups(10, gids) != 1 || gids[0] != getgid())
                        return -1;
        }

        if (setgid(gid) == -1 || getgid() != gid || getegid() != gid)
                return -1;

        if (user && initgroups(user, gid) == -1)
                return -1;

        if (setuid(uid) == -1 || getuid() != uid || geteuid() != uid)
                return -1;

        return 0;
}


void addLocalInterfaces( IPv4NetList &filterlist )
{
  struct ifaddrs *ifaddr, *ifa;
  int family, s;
  char host[NI_MAXHOST];

  if (getifaddrs(&ifaddr) == -1)
    {
      perror("getifaddrs failed");
      exit(EXIT_FAILURE);
    }

  /* Walk through linked list, maintaining head pointer so we
   can free list later */

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
      family = ifa->ifa_addr->sa_family;

      /* Display interface name and family (including symbolic
       form of the latter for the common families) */

      if (family == AF_INET)
        {
          IPv4Net *na = new IPv4Net(*ifa->ifa_addr,
                                    IPv4(*ifa->ifa_netmask).mask_len());
          // std::string s;
          // std::cout << "O" << get_addr_str(ifa->ifa_addr) << "/" << get_addr_str(ifa->ifa_netmask) << std::endl;
          // s = *na;
          // std::cout << s << std::endl;
          filterlist.push_back(*na);
        }

#if 0
      printf("%s  address family: %d%s\n", ifa->ifa_name, family,
          (family == AF_PACKET) ? " (AF_PACKET)" :
          (family == AF_INET) ? " (AF_INET)" :
          (family == AF_INET6) ? " (AF_INET6)" : "");
#endif

      /* For an AF_INET* interface address, display the address */

#if 0
      if (family == AF_INET || family == AF_INET6)
        {
          s = getnameinfo(ifa->ifa_addr,
              (family == AF_INET) ? sizeof(struct sockaddr_in) :
              sizeof(struct sockaddr_in6),
              host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
          if (s != 0)
            {
              printf("getnameinfo() failed: %s\n", gai_strerror(s));
              exit(EXIT_FAILURE);
            }
          printf("\taddress: <%s>\n", host);
        }
#endif
    }

  freeifaddrs(ifaddr);
  return;
}

struct split
{
  enum empties_t { empties_ok, no_empties };
};

template <typename Container> Container& split(
  Container&                                 result,
  const typename Container::value_type&      s,
  typename Container::value_type::value_type delimiter,
  split::empties_t                           empties = split::empties_ok )
{
  result.clear();
  std::istringstream ss( s );
  while (!ss.eof())
  {
    typename Container::value_type field;
    getline( ss, field, delimiter );
    if ((empties == split::no_empties) && field.empty()) continue;
    result.push_back( field );
  }
  return result;
}

/** parses and stores an option */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  struct arguments *arguments = (struct arguments *) state->input;
  static IPNet<IPv4> loopback( IPv4Constants::loopback, 31 );
  switch (key)
    {
    case 'T':
      arguments->tunnel = 1;
      break;
    case 'R':
      arguments->route = 1;
      break;
    case 'D':
      arguments->discover = 1;
      break;
    case 'S':
      arguments->serverip = (arg ? arg : "224.0.23.12");
      break;
    case 'u':
      arguments->name = (char *) (arg ? arg : "/tmp/eib");
      break;
    case 'i':
      arguments->port = (arg ? atoi (arg) : 6720);
      break;
    case 't':
      arguments->tracelevel = (arg ? atoi (arg) : 0);
      break;
    case 'f':
      arguments->errorlevel = (arg ? atoi (arg) : 0);
      break;
    case 'w':
      arguments->warnlevel = (arg ? atoi (arg) : 0);
      break;
    case 'l':
    	arguments->infolevel = (arg ? atoi (arg) : 0);
    	break;
    case 'e':
      arguments->addr = readaddr (arg);
      break;
    case 'r':
      arguments->backendflags |= FLAG_B_RESET_ADDRESS_TABLE;
      break;
    case 'p':
      arguments->pidfile = arg;
      break;
    case 'd':
      arguments->daemon = (char *) (arg ? arg : "/dev/null");
      break;
    case 'c':
      arguments->groupcache = 1;
      break;
    case OPT_BACK_TUNNEL_NOQUEUE:
      arguments->backendflags |= FLAG_B_TUNNEL_NOQUEUE;
      break;
    case OPT_BACK_TPUARTS_ACKGROUP:
      arguments->backendflags |= FLAG_B_TPUARTS_ACKGROUP;
      break;
    case OPT_BACK_TPUARTS_ACKINDIVIDUAL:
      arguments->backendflags |= FLAG_B_TPUARTS_ACKINDIVIDUAL;
      break;
    case OPT_BACK_TPUARTS_DISCH_RESET:
      arguments->backendflags |= FLAG_B_TPUARTS_DISCH_RESET;
      break;

    case 'Q':
      arguments->inbusqlen= (arg ? atoi (arg) : 255);
      break;
    case 'q':
      arguments->outbusqlen= (arg ? atoi (arg) : 255);
      break;
    case 'P':
      // fprintf(stderr,"P %s.\n",arg);

      arguments->peerqlen= (arg ? atoi (arg) : 32);
      break;
    case 'C':
      arguments->clientsmax =  (arg ? atoi (arg) : 32);
      break;
    case 'B':
      //      fprintf(stderr,"B %s.\n",arg);
      arguments->maxpacketspersecond = (arg ? atoi (arg) : 10);
      break;
    case 'X':
      arguments->dropclientsoninterfaceloss=1;
      break;
    case 'F':
    {
      // let's see & build a list of all allowed subnets
      try
        {
          std::vector <string>  nets;
          split( nets, string(arg ? arg : ""), ',', split::no_empties );

          for (std::vector <string>::const_iterator ii= nets.begin();
              ii!=nets.end(); ii++) {
                arguments->ipnetfilters.push_back(IPNet<IPv4>(ii->c_str()));
          }
          addLocalInterfaces(arguments->ipnetfilters);
        }
      catch (IPException &e)
        {
          std::cerr << "cannot parse IPv4 subnet: ";
          std::cerr << e.what() << " '" << e.strvalue() << "'";
          std::cerr << std::endl;
          std::cerr.flush();
          exit(4);
        }
    }
    break;
  case 'U':
    {
      struct passwd *pwd = NULL;
      struct group *grp = NULL;
      char *pos;
      char userbuf[BUFSIZ] = "", groupbuf[BUFSIZ] = "";

      if ((pos = strchr(arg, ':')) || (pos = strchr(arg, '.')))
        {
          if (pos > arg)
            snprintf(userbuf, BUFSIZ, "%*.*s", (int) (pos - arg),
                (int) (pos - arg), arg);
          if (*++pos)
            snprintf(groupbuf, BUFSIZ, "%s", pos);
        }
      else
        {
          snprintf(userbuf, BUFSIZ, "%s", arg);
        }

      if (!(pwd = getpwnam(userbuf)))
        die("unknown user");

      if (*groupbuf && !(grp = getgrnam(groupbuf)))
        die("unknown group");

      arguments->userid=  pwd->pw_uid;
      arguments->groupid= grp ? grp->gr_gid : 0;
    }
    	break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

/** information for the argument parser*/
static struct argp argp = { options, parse_opt, args_doc, doc };

#ifdef HAVE_EIBNETIPSERVER
EIBnetServer *
startServer (Layer3 * l3, Logs * t)
{
  EIBnetServer *c;
  char *ip;
  int port;
  if (!arg.serverip)
    return 0;

  char *a = strdup (arg.serverip);
  char *b;
  if (!a)
    die ("out of memory");
  for (b = a; *b; b++)
    if (*b == ':')
      break;
  if (*b == ':')
    {
      *b = 0;
      port = atoi (b + 1);
    }
  else
    port = 3671;
  c = new EIBnetServer (a, port, arg.tunnel, arg.route, arg.discover, l3, t,
		  	  	  	  	arg.inbusqlen, arg.outbusqlen, arg.peerqlen, arg.clientsmax, arg.ipnetfilters);
  if (!c->init ())
    die ("initilization of the EIBnet/IP server failed");
  free (a);
  return c;
}
#endif

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

EIBDInstance *eibdinstance = NULL;

void _diehandler(int sig)
{
  die ("die on signal %d", sig);
}
void _dumpthreads(int i)
{
    pth_ctrl(PTH_CTRL_DUMPSTATE, stdout);
}

#define MTRACING  0

#if MTRACING
#include <mcheck.h>
#endif

int
main (int ac, char *ag[])
{
  int index;
  Queue < Server * > server;
  Server *s;
  eibdinstance=new EIBDInstance();
#ifdef HAVE_EIBNETIPSERVER
  EIBnetServer *serv = 0;
#endif

#if MTRACING
  mtrace();
#endif

  memset (&arg, 0, sizeof (arg));
  arg.addr = 0x0001;
  arg.errorlevel = LOG_WARNING;
  arg.warnlevel=255;
  arg.tracelevel=1;
  arg.infolevel=255;

  logger.setTraceLevel (arg.tracelevel);
  logger.setWarnLevel  (arg.warnlevel);
  logger.setInfoLevel  (arg.infolevel);

  arg.tracelevel=0;
  arg.userid=0;
  arg.groupid=0;

  argp_parse (&argp, ac, ag, 0, &index, &arg);
  if (index > ac - 1)
    die ("url expected");
  if (index < ac - 1)
    die ("unexpected parameter");

  if (arg.port == 0 && arg.name == 0 && arg.serverip == 0)
    die ("No listen-address given");

  signal (SIGPIPE, SIG_IGN);
  signal (SIGINT, _diehandler);
  signal (SIGTERM, _diehandler);
  pth_init ();

  pth_mutex_acquire(&eibdinstance->lock,false,NULL);

  logger.setTraceLevel (arg.tracelevel);
  logger.setWarnLevel  (arg.warnlevel);

  if (arg.daemon)
    {
      int fd = open (arg.daemon, O_WRONLY | O_APPEND | O_CREAT, FILE_MODE);
      if (fd == -1)
	die ("Can not open file %s", arg.daemon);
      int i = fork ();
      if (i < 0)
	die ("fork failed");
      if (i > 0)
	exit (0);
      close (1);
      close (2);
      close (0);
      dup2 (fd, 1);
      dup2 (fd, 2);
      close (fd);
      setsid ();
    }

  FILE *pidf;
  if (arg.pidfile)
    if ((pidf = fopen (arg.pidfile, "w")) != NULL)
      {
	fprintf (pidf, "%d", getpid ());
	fclose (pidf);
      }

  // change to desired group
  if (arg.userid)
          {
            if (daemon_become_user(arg.userid, arg.groupid ? arg.groupid : NULL, NULL)
                == -1)
              {
                die("failed to set user/group");
              }
          }

  if (getuid () == 0)
          LOGANDDIE ( "EIBD must not run as root");

  pth_mutex_release(&eibdinstance->lock);

  INFOLOG(&logger, LOG_INFO, NULL, "%s started successfully with pid %d", argp_program_version, getpid ());

  try
  {
    eibdinstance->l2 = Create (ag[index],  arg.backendflags, &logger, arg.inbusqlen,
        arg.outbusqlen, arg.maxpacketspersecond, arg.peerqlen, arg.ipnetfilters);
    // printf ("%d %d %d %d\n",  arg.inbusqlen,arg.outbusqlen, arg.maxpacketspersecond, arg.peerqlen);

    if (!eibdinstance->l2) {
    	// should have thrown exception ?
    	LOGANDDIE ("Layer 2 interface cannot be opened");
    }

    eibdinstance->l3 = new Layer3 (eibdinstance->l2, &logger,
        arg.backendflags & FLAG_B_RESET_ADDRESS_TABLE,
        arg.dropclientsoninterfaceloss, arg.ipnetfilters );
    if (arg.port)
      eibdinstance->inetserver = new InetServer (eibdinstance->l3, &logger, eibdinstance, arg.port, arg.inbusqlen, arg.outbusqlen,
          arg.peerqlen, arg.clientsmax, arg.ipnetfilters);
    if (arg.name)
      eibdinstance->localserver = new LocalServer (eibdinstance->l3,arg.name, &logger, eibdinstance,  arg.inbusqlen,
          arg.outbusqlen, arg.peerqlen, arg.clientsmax, arg.ipnetfilters);
#ifdef HAVE_EIBNETIPSERVER
    eibdinstance->serv = startServer (eibdinstance->l3, &logger);
#endif
#ifdef HAVE_GROUPCACHE
  if (!CreateGroupCache (eibdinstance->l3, &logger, arg.groupcache))
    LOGANDDIE ("initialisation of the group cache failed");
#endif
  }
  catch (Exception &e)
  {
    LOGANDDIE ("hardware or IP interface initialisation failed, exiting");
  }

  signal (SIGINT, SIG_IGN);
  signal (SIGTERM, SIG_IGN);
  signal (SIGUSR1, SIG_IGN);

  int sig;
  do
    {
      sigset_t t1;
      pth_event_t tic= pth_event(PTH_EVENT_RTIME, pth_time(60*60, 0));
      sigemptyset (&t1);
      sigaddset (&t1, SIGINT);
      sigaddset (&t1, SIGHUP);
      sigaddset (&t1, SIGTERM);
      sigaddset (&t1, SIGUSR1);
      sig = 0;

      pth_sigwait_ev (&t1, &sig, tic);
      pth_event_isolate(tic);

      if (sig)
        {
          INFOLOG(&logger, LOG_INFO, NULL, "caught signal %d", (int) sig);
          if (sig == SIGHUP && arg.daemon)
            {
              int fd = open(arg.daemon, O_WRONLY | O_APPEND | O_CREAT,
                  FILE_MODE);
              if (fd == -1)
                {
                  LOGANDDIE( "can't open log file %s", arg.daemon);
                  continue;
                }
              close(1);
              close(2);
              dup2(fd, 1);
              dup2(fd, 2);
              close(fd);
            }
          else if (sig == SIGUSR1)
            {
              _dumpthreads(0);
            }
        }
      if (pth_event_status(tic) == PTH_STATUS_OCCURRED) {
          eibdinstance->l2->logtic();
      }
      pth_event_free(tic,PTH_FREE_THIS);
    }
  while (sig == SIGHUP || sig==SIGUSR1 || !sig);

  signal (SIGINT, SIG_DFL);
  signal (SIGTERM, SIG_DFL);
  signal (SIGUSR1, _dumpthreads);

  TRACEPRINTF(&logger, 2, NULL, "main thread cleaning up");

  while (!server.isempty ())
    delete server.get ();
#ifdef HAVE_EIBNETIPSERVER
  if (serv)
    delete serv;
#endif
#ifdef HAVE_GROUPCACHE
  DeleteGroupCache ();
#endif

  delete eibdinstance->l3;
  // delete eibdinstance->l2; done by layer3, generally each layer should clean the lower one first
  if (Cleanup)
    Cleanup ();

  if (arg.pidfile)
    unlink (arg.pidfile);

  signal (SIGUSR1, SIG_DFL);

  INFOLOG(&logger, LOG_INFO, NULL, "eibd exiting pid %d", getpid ());
  logger.flush();
  std::cout.flush();
  std::cerr.flush();
  close (1);
  close (2);
  close (0);
  pth_kill (); // well, some servers are left hanging as is interface
  pth_exit (0);
  exit(0);
}
