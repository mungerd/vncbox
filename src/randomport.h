/* randomport.h
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


#ifndef __vncbox_RANDOMPORT_H
#define __vncbox_RANDOMPORT_H

#include <string>
#include <exception>

class RandomPortError : public std::exception {
public:
    const std::string message;

    RandomPortError(const std::string& message_)
        : message(message_) {}

    virtual ~RandomPortError() throw() {}

    virtual const char* what() const throw()
        { return message.c_str(); }
};


/* returns an available random Internet port number */

unsigned short get_random_inet_port();

#endif // __vncbox_RANDOMPORT_H
