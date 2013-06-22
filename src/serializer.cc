/* serializer.cc
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

#include "serializer.h"

using Glib::ustring;

Serializer::Serializer(Glib::RefPtr<Glib::IOChannel> iochannel)
{
    set_iochannel(iochannel);
}

void
Serializer::set_iochannel(Glib::RefPtr<Glib::IOChannel> iochannel)
{
    m_iochannel = iochannel;
    m_encoding = m_iochannel->get_encoding();
}

/* read functions */

void
Serializer::read(std::string& str)
{
    std::string::size_type size;
    read(size);

    ustring ustr;
    Glib::IOStatus status = m_iochannel->read(ustr, size);
    str = (std::string) ustr;
    if (status != Glib::IO_STATUS_NORMAL)
        throw SerializerError(SerializerError::IO_ERROR, "error reading std::string");
}

void
Serializer::read(ustring& str)
{
    ustring::size_type size;
    read(size);

    Glib::IOStatus status = m_iochannel->read(str, size);
    if (status != Glib::IO_STATUS_NORMAL)
        throw SerializerError(SerializerError::IO_ERROR, "error reading ustring");
}

/* write functions */

void
Serializer::write(const std::string& str)
{
    std::string::size_type size = str.size();
    write(size);

    Glib::IOStatus status = m_iochannel->write(str);
    if (status != Glib::IO_STATUS_NORMAL)
        throw SerializerError(SerializerError::IO_ERROR, "error writing string");
}

void
Serializer::write(const ustring& str)
{
    ustring::size_type size = str.size();
    write(size);

    Glib::IOStatus status = m_iochannel->write(str);
    if (status != Glib::IO_STATUS_NORMAL)
        throw SerializerError(SerializerError::IO_ERROR, "error writing ustring");
}
