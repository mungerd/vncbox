/* randomport.cc
 *
 * Copyright (C) 2008, 2013  David Munger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author:
 * 	David Munger <mungerd@gmail.com>
 */


#include "randomport.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <limits.h>

/* returns an available random Internet port number */

unsigned short
get_random_inet_port()
{
    struct addrinfo hints;
    struct addrinfo *info;

    // get hostname
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, sizeof(hostname));

    // find free address
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;

    int status = getaddrinfo(hostname, NULL, &hints, &info);
    if (status != 0)
        throw RandomPortError(std::string("getaddrinfo(): ") + gai_strerror(status));

    int sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (sock == -1)
        throw RandomPortError(std::string("socket(): ") + strerror(errno));

    if (bind(sock, info->ai_addr, info->ai_addrlen) != 0)
        throw RandomPortError(std::string("bind(): ") + strerror(errno));

    freeaddrinfo(info);

    struct sockaddr_in sin;
    socklen_t namelen = sizeof(sin);

    if (getsockname(sock, (struct sockaddr *) &sin, &namelen) != 0)
        throw RandomPortError(std::string("getsockname(): ") + strerror(errno));

    close(sock);

    return ntohs(sin.sin_port);
}
