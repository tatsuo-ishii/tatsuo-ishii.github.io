/* -*-pgsql-c-*- */
/*
 * $Header: /home/t-ishii/repository/pgpool/main.c,v 1.11 2004/05/06 06:30:35 t-ishii Exp $
 *
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003, 2004	Tatsuo Ishii
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of the
 * author not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. The author makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 */
#include "pool.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <sys/wait.h>

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include <signal.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "version.h"

#define PGPOOLMAXLITSENQUEUELENGTH 10000
static void daemonize(void);
static int read_pid_file(void);
static void write_pid_file(void);
static pid_t fork_a_child(int unix_fd, int inet_fd);
static int create_unix_domain_socket(void);
static int create_inet_domain_socket(void);
static void myexit(int code);
static int check_pid_file(char *path);

static RETSIGTYPE exit_handler(int sig);
static RETSIGTYPE reap_handler(int sig);
static RETSIGTYPE failover_handler(int sig);

static void usage(void);
static void stop_me(void);

/* set unix domain socket path */
struct sockaddr_un un_addr;

/* child pid table */
pid_t *pids;

int unix_fd;
int inet_fd;

int exiting = 0;
int switching = 0;

int not_detach = 0;
int debug = 0;

pid_t mypid;

/*
* pgpool main program
*/
int main(int argc, char **argv)
{
	int opt;
	char conf_file[POOLMAXPATHLEN+1];
	int i;
	int pid;

	snprintf(conf_file, sizeof(conf_file), "%s/%s", DEFAULT_CONFIGDIR, POOL_CONF_FILE_NAME);

	while ((opt = getopt(argc, argv, "f:hnd")) != -1)
	{
		switch (opt)
		{
			case 'f':
				if (!optarg)
				{
					usage();
					exit(1);
				}
				strncpy(conf_file, optarg, sizeof(conf_file));
				break;
			case 'h':
				usage();
				exit(0);
				break;
			case 'n':
				not_detach = 1;
				break;
			case 'd':
				debug = 1;
				break;
			default:
				usage();
				exit(1);
		}
	}

	if (pool_get_config(conf_file))
	{
		pool_error("Unable to get configuration. Exiting...");
		myexit(1);
	}

	/* set current PostgreSQL backend */
	pool_config.current_backend_host_name = pool_config.backend_host_name;
	pool_config.current_backend_port = pool_config.backend_port;

	if (optind == (argc - 1) && !strcmp(argv[optind], "stop"))
	{
		stop_me();
		fprintf(stderr, "pgpool stopped.\n");
		exit(0);
	}
	else if (optind == argc)
	{
		pid = read_pid_file();
		if (pid > 0)
		{
			if (kill(pid, 0) == 0)
			{
				fprintf(stderr, "pid file found. is another pgpool(%d) is running?\n", pid);
				exit(1);
			}
			else
				fprintf(stderr, "pid file found but it seems bogus. Trying to start pgpool anyway...\n");
		}
	}
	else if (optind < argc)
	{
		usage();
		exit(1);
	}


	if (not_detach)
		write_pid_file();
	else
		daemonize();

	mypid = getpid();

	/* set unix domain socket path */
	snprintf(un_addr.sun_path, sizeof(un_addr.sun_path), "%s/.s.PGSQL.%d",
			 pool_config.socket_dir,
			 pool_config.port);

	/* set up signal handlers */
	signal(SIGPIPE, SIG_IGN);

	/* create unix domain socket */
	unix_fd = create_unix_domain_socket();

	/* create inet domain socket if any */
	if (pool_config.inetdomain)
	{
		inet_fd = create_inet_domain_socket();
	}

	pids = malloc(pool_config.num_init_children * sizeof(pool_config.num_init_children));
	if (pids == NULL)
	{
		pool_error("failed to allocate pids");
		myexit(1);
	}
	memset(pids, 0, pool_config.num_init_children * sizeof(pool_config.num_init_children));

	/* fork the children */
	for (i=0;i<pool_config.num_init_children;i++)
	{
		pids[i] = fork_a_child(unix_fd, inet_fd);
	}

	/* set up signal handlers */
	signal(SIGTERM, exit_handler);
	signal(SIGINT, exit_handler);
	signal(SIGCHLD, reap_handler);
	signal(SIGUSR1, failover_handler);

	for (;;)
	{
		pause();
	}
	return 0;
}

static void usage(void)
{
	fprintf(stderr, "pgpool version %s, a generic connection pool/replication server for PostgreSQL\n\n", PGPOOLVERSION);
	fprintf(stderr, "usage: pgpool [-f config_file][-n][-d][-h][stop]\n");
	fprintf(stderr, "  config_file default path: %s/%s\n",DEFAULT_CONFIGDIR, POOL_CONF_FILE_NAME);
	fprintf(stderr, "  -n: don't run in daemon mode. does not detatch control tty\n");
	fprintf(stderr, "  -d: debug mode. lots of debug information will be printed\n");
	fprintf(stderr, "  stop: stop pgpool\n");
	fprintf(stderr, "  -h: print this help\n");
}

/*
* detatch control ttys
*/
static void daemonize(void)
{
	int			i;
	pid_t		pid;

	pid = fork();
	if (pid == (pid_t) -1)
	{
		pool_error("fork() failed. reason: %s", strerror(errno));
		exit(1);
		return;					/* not reached */
	}
	else if (pid > 0)
	{			/* parent */
		exit(0);
	}

#ifdef HAVE_SETSID
	if (setsid() < 0)
	{
		pool_error("setsid() failed. reason:%s", strerror(errno));
		exit(1);
	}
#endif

	i = open("/dev/null", O_RDWR);
	dup2(i, 0);
	dup2(i, 1);
	dup2(i, 2);
	close(i);

	write_pid_file();
}

/*
* stop myself
*/
static void stop_me(void)
{
	FILE *fd;
	char path[POOLMAXPATHLEN];
	char pidbuf[128];
	pid_t pid;

	snprintf(path, sizeof(path), "%s/%s", pool_config.logdir, PID_FILE_NAME);
	fd = fopen(path, "r");
	if (!fd)
	{
		pool_error("could not open pid file as %s. reason: %s",
				   path, strerror(errno));
		exit(1);
	}

	memset(pidbuf, 0, sizeof(pidbuf));
	fread(pidbuf, sizeof(pidbuf), 1, fd);
	pid = atoi(pidbuf);
	fclose(fd);

	if (kill(pid, SIGTERM) == -1)
	{
		pool_error("could stop pid: %d. reason: %s", pid, strerror(errno));
		exit(1);
	}
}

/*
* check the existence of pid file. returns 0 if exists non 0 otherwise.
*/
static int check_pid_file(char *path)
{
	struct stat buf;

	return stat(path, &buf);
}

/*
* read the pid file
*/
static int read_pid_file(void)
{
	FILE *fd;
	char path[POOLMAXPATHLEN];
	char pidbuf[128];

	snprintf(path, sizeof(path), "%s/%s", pool_config.logdir, PID_FILE_NAME);
	fd = fopen(path, "r");
	if (!fd)
	{
		return -1;
	}
	if (fread(pidbuf, 1, sizeof(pidbuf), fd) <= 0)
	{
		pool_error("could not read pid file as %s. reason: %s",
				   path, strerror(errno));
		fclose(fd);
		return -1;
	}
	fclose(fd);
	return(atoi(pidbuf));
}

/*
* write the pid file
*/
static void write_pid_file(void)
{
	FILE *fd;
	char path[POOLMAXPATHLEN];
	char pidbuf[128];

	snprintf(path, sizeof(path), "%s/%s", pool_config.logdir, PID_FILE_NAME);
	fd = fopen(path, "w");
	if (!fd)
	{
		pool_error("could not open pid file as %s. reason: %s",
				   path, strerror(errno));
		exit(1);
	}
	snprintf(pidbuf, sizeof(pidbuf), "%d", getpid());
	fwrite(pidbuf, strlen(pidbuf), 1, fd);
	if (fclose(fd))
	{
		pool_error("could not write pid file as %s. reason: %s",
				   path, strerror(errno));
		exit(1);
	}
}

/*
* fork a child
*/
pid_t fork_a_child(int unix_fd, int inet_fd)
{
	pid_t pid;

	pid = fork();

	if (pid == 0)
	{
		/* call child main */
		do_child(unix_fd, inet_fd);
	}
	else if (pid == -1)
	{
		pool_error("fork() failed. reason: %s", strerror(errno));
		myexit(1);
	}
	return pid;
}

/*
* create inet domain socket
*/
static int create_inet_domain_socket(void)
{
	struct sockaddr_in addr;
	int fd;
	int status;
	int one = 1;
	int len;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
	{
		pool_error("Failed to create INET domain socket. reason: %s", strerror(errno));
		myexit(1);
	}
	if ((setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &one,
					sizeof(one))) == -1)
	{
		pool_error("setsockopt() failed. reason: %s", strerror(errno));
		myexit(1);
	}
	memset((char *) &addr, 0, sizeof(addr));
	((struct sockaddr *)&addr)->sa_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(pool_config.port);
	len = sizeof(struct sockaddr_in);
	status = bind(fd, (struct sockaddr *)&addr, len);
	if (status == -1)
	{
		pool_error("bind() failed. reason: %s", strerror(errno));
		myexit(1);
	}

	status = listen(fd, PGPOOLMAXLITSENQUEUELENGTH);
	if (status < 0)
	{
		pool_error("listen() failed. reason: %s", strerror(errno));
		myexit(1);
	}
	return fd;
}

/*
* create UNIX domain socket
*/
static int create_unix_domain_socket(void)
{
	struct sockaddr_un addr;
	int fd;
	int status;
	int len;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1)
	{
		pool_error("Failed to create UNIX domain socket. reason: %s", strerror(errno));
		myexit(1);
	}
	memset((char *) &addr, 0, sizeof(addr));
	((struct sockaddr *)&addr)->sa_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path), un_addr.sun_path);
	len = sizeof(struct sockaddr_un);
	status = bind(fd, (struct sockaddr *)&addr, len);
	if (status == -1)
	{
		pool_error("bind() failed. reason: %s", strerror(errno));
		myexit(1);
	}

	if (chmod(un_addr.sun_path, 0777) == -1)
	{
		pool_error("chmod() failed. reason: %s", strerror(errno));
		myexit(1);
	}

	status = listen(fd, PGPOOLMAXLITSENQUEUELENGTH);
	if (status < 0)
	{
		pool_error("listen() failed. reason: %s", strerror(errno));
		myexit(1);
	}
	return fd;
}

static void myexit(int code)
{
	char path[POOLMAXPATHLEN];

	if (getpid() != mypid)
		return;

	unlink(un_addr.sun_path);
	snprintf(path, sizeof(path), "%s/%s", pool_config.logdir, PID_FILE_NAME);
	unlink(path);

	exit(code);
}

/* notice backend connection error using SIGUSR1 */
void notice_backend_error(void)
{
	pid_t parent = getppid();

	kill(parent, SIGUSR1);
	/* avoid race conditon with SIGCHLD */
	sleep(1);
}

static RETSIGTYPE exit_handler(int sig)
{
	int i;

	exiting = 1;

	for (i = 0; i < pool_config.num_init_children; i++)
	{
		pid_t pid = pids[i];
		if (pid)
			kill(pid, SIGTERM);
	}
	while (wait(NULL) > 0)
		;

	if (errno != ECHILD)
		pool_error("wait() failed. reason:%s", strerror(errno));

	myexit(0);
}


/*
 * handle SIGUSR1 (backend connection error, fail over request, if possible)
 */
static RETSIGTYPE failover_handler(int sig)
{
	int i;
	int replication = 0;

	pool_debug("failover_handler called");

	if (exiting)
		return;

	if (switching)
		return;

	/* secondary backend exists? */
	if (pool_config.secondary_backend_port == 0)
		return;
#ifdef NOT_USED
	/* in replication mode and already degeneration done */
	else if (pool_config.replication_mode && !pool_config.replication_enabled)
		return;
#endif

	/* already failover */
	if (strcmp(pool_config.current_backend_host_name, pool_config.secondary_backend_host_name) ||
		pool_config.current_backend_port != pool_config.secondary_backend_port)
	{
		switching = 1;
		
		if (pool_config.replication_enabled)
		{
			replication = 1;
			pool_log("starting degenration. shutdown secondary host %s(%d)",
					   pool_config.secondary_backend_host_name,
					   pool_config.secondary_backend_port);
		}
		else
		{
			pool_log("starting failover from %s(%d) to %s(%d)",
					   pool_config.current_backend_host_name,
					   pool_config.current_backend_port,
					   pool_config.secondary_backend_host_name,
					   pool_config.secondary_backend_port);
		}

		/* kill all children */
		for (i = 0; i < pool_config.num_init_children; i++)
		{
			pid_t pid = pids[i];
			if (pid)
				kill(pid, SIGTERM);
		}
		while (wait(NULL) > 0)
			;

		if (errno != ECHILD)
			pool_error("wait() failed. reason:%s", strerror(errno));

		if (pool_config.replication_enabled)
		{
			/* disable replicaton mode */
			pool_config.replication_enabled = 0;
		}
		else
		{
			/* fail over to secondary */
			pool_config.current_backend_host_name = pool_config.secondary_backend_host_name;
			pool_config.current_backend_port = pool_config.secondary_backend_port;
		}

		/* fork the children */
		for (i=0;i<pool_config.num_init_children;i++)
		{
			pids[i] = fork_a_child(unix_fd, inet_fd);
		}

		/*
		 * do not close unix_fd and inet_fd here. if a child dies we
		 * need to fork a new child which should inherit these fds.
		 */

		switching = 0;

		if (replication)
		{
			pool_log("degenration done. shutdown secondary host %s(%d)",
					   pool_config.secondary_backend_host_name,
					   pool_config.secondary_backend_port);
		}
		else
		{
			pool_log("failover from %s(%d) to %s(%d) done.",
					   pool_config.backend_host_name,
					   pool_config.backend_port,
					   pool_config.secondary_backend_host_name,
					   pool_config.secondary_backend_port);
		}
	}
}

/*
 * handle SIGCHLD
 */
static RETSIGTYPE reap_handler(int sig)
{
	pid_t pid;
	int status;
	int i;

	if (exiting)
		return;

	if (switching)
		return;

#ifdef HAVE_WAITPID
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
	{
#else
		while ((pid = wait3(&status, WNOHANG, NULL)) > 0)
		{
#endif

			/* look for exiting child's pid */
			for (i=0;i<pool_config.num_init_children;i++)
			{
				if (pid == pids[i])
				{
					/* if found, fork a new child */
					if (!switching && !exiting)
					{
						pids[i] = fork_a_child(unix_fd, inet_fd);
						break;
					}
				}
			}
		}
	}
