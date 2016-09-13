/* -*-pgsql-c-*- */
/*
 * $Header: /home/t-ishii/repository/pgpool/pool_connection_pool.c,v 1.23 2004/07/19 01:28:44 t-ishii Exp $
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
 *
 * poo_connection_pool.c: connection pool stuff
 */
#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#include <netdb.h>

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "pool.h"

POOL_CONNECTION_POOL *pool_connection_pool;	/* connection pool */

static POOL_CONNECTION_POOL_SLOT *create_cp(POOL_CONNECTION_POOL_SLOT *cp, int secondary_backend);
static POOL_CONNECTION_POOL *new_connection(POOL_CONNECTION_POOL *p);

/*
* initialize connection pools. this should be called once at the startup.
*/
int pool_init_cp(void)
{
	pool_connection_pool = (POOL_CONNECTION_POOL *)malloc(sizeof(POOL_CONNECTION_POOL)*pool_config.max_pool);
	if (pool_connection_pool == NULL)
	{
		pool_error("pool_init_cp: malloc() failed");
		return -1;
	}
	memset(pool_connection_pool, 0, sizeof(POOL_CONNECTION_POOL)*pool_config.max_pool);

	return 0;
}

/*
* find connection by user and database
*/
POOL_CONNECTION_POOL *pool_get_cp(char *user, char *database, int protoMajor)
{
	int i;

	POOL_CONNECTION_POOL *p = pool_connection_pool;

	if (p == NULL)
	{
		pool_error("pool_get_cp: pool_connection_pool is not initialized");
		return NULL;
	}

	for (i=0;i<pool_config.max_pool;i++)
	{
		if (MASTER_CONNECTION(p) &&
			MASTER_CONNECTION(p)->sp->major == protoMajor &&
			MASTER_CONNECTION(p)->sp->user != NULL &&
			strcmp(MASTER_CONNECTION(p)->sp->user, user) == 0 &&
			strcmp(MASTER_CONNECTION(p)->sp->database, database) == 0)
		{
			/* mark this connection is under use */
			MASTER_CONNECTION(p)->closetime = 0;
			return p;
		}
		p++;
	}
	return NULL;
}

/*
 * disconnect and release a connection to the database
 */
void pool_discard_cp(char *user, char *database, int protoMajor)
{
	POOL_CONNECTION_POOL *p = pool_get_cp(user, database, protoMajor);

	if (p == NULL)
	{
		pool_error("pool_discard_cp: cannot get connection pool for user %s datbase %s", user, database);
		return;
	}

	free(MASTER_CONNECTION(p)->sp->user);
	free(MASTER_CONNECTION(p)->sp->database);
	free(MASTER_CONNECTION(p)->sp->startup_packet);
	pool_close(MASTER_CONNECTION(p)->con);

	if (pool_config.replication_enabled)
	{
		/* do not free memory! we did not allocate them */
		pool_close(SECONDARY_CONNECTION(p)->con);
	}

	memset(p, 0, sizeof(POOL_CONNECTION_POOL));
}


/*
* create a connection pool by user and database
*/
POOL_CONNECTION_POOL *pool_create_cp(void)
{
	int i;
	time_t closetime;
	POOL_CONNECTION_POOL *oldestp;

	POOL_CONNECTION_POOL *p = pool_connection_pool;

	if (p == NULL)
	{
		pool_error("pool_create_cp: pool_connection_pool is not initialized");
		return NULL;
	}

	for (i=0;i<pool_config.max_pool;i++)
	{
		if (MASTER_CONNECTION(p) == NULL)
			return new_connection(p);
		p++;
	}

	pool_debug("no empty connection slot was found");

	/*
	 * no empty connection slot was found. look for the oldest connection and discard it.
	 */
	oldestp = p = pool_connection_pool;
	closetime = MASTER_CONNECTION(p)->closetime;
	for (i=0;i<pool_config.max_pool;i++)
	{
		pool_debug("user: %s database: %s closetime: %d",
				   MASTER_CONNECTION(p)->sp->user,
				   MASTER_CONNECTION(p)->sp->database,
				   MASTER_CONNECTION(p)->closetime);
		if (MASTER_CONNECTION(p)->closetime < closetime)
		{
			closetime = MASTER_CONNECTION(p)->closetime;
			oldestp = p;
		}
		p++;
	}

	p = oldestp;
	pool_send_frontend_exits(p);

	pool_debug("discarding old %d th connection. user: %s database: %s", 
			   oldestp - pool_connection_pool,
			   MASTER_CONNECTION(p)->sp->user,
			   MASTER_CONNECTION(p)->sp->database);

	free(MASTER_CONNECTION(p)->sp->user);
	free(MASTER_CONNECTION(p)->sp->database);
	free(MASTER_CONNECTION(p)->sp->startup_packet);
	pool_close(MASTER_CONNECTION(p)->con);
	memset(p, 0, sizeof(POOL_CONNECTION_POOL));

	if (pool_config.replication_enabled)
	{
		free(SECONDARY_CONNECTION(p)->sp->user);
		free(SECONDARY_CONNECTION(p)->sp->database);
		free(SECONDARY_CONNECTION(p)->sp->startup_packet);
		pool_close(SECONDARY_CONNECTION(p)->con);
		memset(p, 0, sizeof(POOL_CONNECTION_POOL));
	}

	return new_connection(p);
}

/*
 * set backend connection close timer
 */
void pool_connection_pool_timer(POOL_CONNECTION_POOL *backend)
{
	POOL_CONNECTION_POOL *p = pool_connection_pool;
	int i;

	pool_debug("pool_connection_pool_timer: called");

	MASTER_CONNECTION(backend)->closetime = time(NULL);		/* set connection close time */

	if (pool_config.connection_life_time == 0)
		return;

	/* look for any other timeout */
	for (i=0;i<pool_config.max_pool;i++, p++)
	{
		if (!MASTER_CONNECTION(p))
			continue;
		if (MASTER_CONNECTION(p)->sp->user == NULL)
			continue;

		if (p != backend && MASTER_CONNECTION(p)->closetime)
			return;
	}

	/* no other timer found. set my timer */
	pool_debug("pool_connection_pool_timer: set alarm after %d seconds", pool_config.connection_life_time);
	signal(SIGALRM, pool_backend_timer_handler);
	alarm(pool_config.connection_life_time);
}

/*
 * backend connection close timer handler
 */
RETSIGTYPE pool_backend_timer_handler(int sig)
{
#define TMINTMAX 0x7fffffff

	POOL_CONNECTION_POOL *p = pool_connection_pool;
	int i;
	time_t now;
	time_t nearest = TMINTMAX;

	now = time(NULL);

	pool_debug("pool_backend_timer_handler called at %d", now);

	for (i=0;i<pool_config.max_pool;i++, p++)
	{
		if (!MASTER_CONNECTION(p))
			continue;
		if (MASTER_CONNECTION(p)->sp->user == NULL)
			continue;

		/* timer expire? */
		if (MASTER_CONNECTION(p)->closetime)
		{
			pool_debug("pool_backend_timer_handler: expire time: %d",
					   MASTER_CONNECTION(p)->closetime+pool_config.connection_life_time);

			if (now >= (MASTER_CONNECTION(p)->closetime+pool_config.connection_life_time))
			{
				/* discard expired connection */
				pool_debug("pool_backend_timer_handler: expires user %s database %s", MASTER_CONNECTION(p)->sp->user, MASTER_CONNECTION(p)->sp->database);

				pool_send_frontend_exits(p);

				free(MASTER_CONNECTION(p)->sp->user);
				free(MASTER_CONNECTION(p)->sp->database);
				free(MASTER_CONNECTION(p)->sp->startup_packet);
				pool_close(MASTER_CONNECTION(p)->con);
				memset(p, 0, sizeof(POOL_CONNECTION_POOL));

				if (pool_config.replication_enabled)
				{
					free(SECONDARY_CONNECTION(p)->sp->user);
					free(SECONDARY_CONNECTION(p)->sp->database);
					free(SECONDARY_CONNECTION(p)->sp->startup_packet);
					pool_close(SECONDARY_CONNECTION(p)->con);
					memset(p, 0, sizeof(POOL_CONNECTION_POOL));
				}
			}
			else
			{
				/* look for nearest timer */
				if (MASTER_CONNECTION(p)->closetime < nearest)
					nearest = MASTER_CONNECTION(p)->closetime;
			}
		}
	}

	/* any remaining timer */
	if (nearest != TMINTMAX)
	{
		nearest = pool_config.connection_life_time - (now - nearest);
		if (nearest <= 0)
		  nearest = 1;
		signal(SIGALRM, pool_backend_timer_handler);
		alarm(nearest);
	}
}

int connect_inet_domain_socket(int secondary_backend)
{
	int fd;
	int len;
	int on = 1;
	struct sockaddr_in addr;
	struct hostent *hp;
	int port;
	char *host;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		pool_error("connect_inet_domain_socket: socket() failed: %s", strerror(errno));
		return -1;
	}

	/* set nodelay */
	if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
				   (char *) &on,
				   sizeof(on)) < 0)
	{
		pool_error("connect_inet_domain_socket: setsockopt() failed: %s", strerror(errno));
		close(fd);
		return -1;
	}

	memset((char *) &addr, 0, sizeof(addr));
	((struct sockaddr *)&addr)->sa_family = AF_INET;

	host = secondary_backend?pool_config.secondary_backend_host_name:pool_config.current_backend_host_name;

	port = secondary_backend?pool_config.secondary_backend_port:pool_config.current_backend_port;
	addr.sin_port = htons(port);
	len = sizeof(struct sockaddr_in);

	hp = gethostbyname(host);
	if ((hp == NULL) || (hp->h_addrtype != AF_INET))
	{
		pool_error("connect_inet_domain_socket: gethostbyname() failed: %s host: %s", strerror(errno), host);
		close(fd);
		return -1;
	}
	memmove((char *) &(addr.sin_addr),
			(char *) hp->h_addr,
			hp->h_length);

	if (connect(fd, (struct sockaddr *)&addr, len) < 0)
	{
		pool_error("connect_inet_domain_socket: connect() failed: %s",strerror(errno));
		close(fd);
		return -1;
	}
	return fd;
}

int connect_unix_domain_socket(int secondary_backend)
{
	struct sockaddr_un addr;
	int fd;
	int len;
	int port;
	char *socket_dir;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1)
	{
		pool_error("connect_unix_domain_socket: setsockopt() failed: %s", strerror(errno));
		return -1;
	}

	port = secondary_backend?pool_config.secondary_backend_port:pool_config.current_backend_port;
	socket_dir = pool_config.backend_socket_dir;
	memset((char *) &addr, 0, sizeof(addr));
	((struct sockaddr *)&addr)->sa_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path), "%s/.s.PGSQL.%d", socket_dir, port);
	pool_debug("connecting postmaster Unix domain socket: %s", addr.sun_path);

	len = sizeof(struct sockaddr_un);

	if (connect(fd, (struct sockaddr *)&addr, len) < 0)
	{
		pool_error("connect_unix_domain_socket: connect() failed: %s", strerror(errno));
		close(fd);
		return -1;
	}
	pool_debug("connected to postmaster Unix domain socket: %s fd: %d", addr.sun_path, fd);
	return fd;
}

static POOL_CONNECTION_POOL_SLOT *create_cp(POOL_CONNECTION_POOL_SLOT *cp, int secondary_backend)
{
	int fd;

	if (secondary_backend)
	{
		if (*pool_config.secondary_backend_host_name == '\0')
			fd = connect_unix_domain_socket(1);
		else
			fd = connect_inet_domain_socket(1);
	}
	else
	{
		if (*pool_config.current_backend_host_name == '\0')
			fd = connect_unix_domain_socket(0);
		else
			fd = connect_inet_domain_socket(0);
	}

	if (fd < 0)
	{
		/* fatal error, notice to parent and exit */
		notice_backend_error();
		exit(1);
	}

	cp->con = pool_open(fd);
	cp->closetime = 0;
	return cp;
}

static POOL_CONNECTION_POOL *new_connection(POOL_CONNECTION_POOL *p)
{
	/* create master connection */
	MASTER_CONNECTION(p) = malloc(sizeof(POOL_CONNECTION_POOL_SLOT));
	if (MASTER_CONNECTION(p) == NULL)
	{
		pool_error("pool_create_cp: malloc() failed");
		return NULL;
	}
	create_cp(MASTER_CONNECTION(p), 0);

			/* initialize Paramter Status save structure */
	if (pool_init_params(&MASTER(p)->params))
	{
		return NULL;
	}
	p->num = 1;	/* number of slots */

	/* create secondary connection */
	if (pool_config.replication_enabled)
	{
		SECONDARY_CONNECTION(p) = malloc(sizeof(POOL_CONNECTION_POOL_SLOT));
		if (SECONDARY_CONNECTION(p) == NULL)
		{
			pool_error("pool_create_cp: malloc() failed");
			return NULL;
		}
		create_cp(SECONDARY_CONNECTION(p), 1);

		/* initialize Paramter Status save structure */
		if (pool_init_params(&MASTER(p)->params))
		{
			return NULL;
		}

		p->num++;	/* number of slots */
	}

	return p;
}
