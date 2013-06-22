/* serializer.h
 *
 * Copyright (C) 2008, 2013  David Munger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General License for more details.
 *
 * You should have received a copy of the GNU General License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author:
 * 	David Munger <mungerd@gmail.com>
 */

#ifndef __vncbox_SERIALIZER_H
#define __vncbox_SERIALIZER_H

#include <glibmm.h>
#include <list>
#include <map>

#include <typeinfo>

class SerializerError : public Glib::Exception
{
public:
    enum Code {
        IO_ERROR,
    };

    const Code code;
    const Glib::ustring message;

    SerializerError(Code error_code, const Glib::ustring& error_message)
        : code(error_code), message(error_message) {}

    virtual ~SerializerError() throw() {}

    virtual Glib::ustring what() const { return message; }
};


class Serializer
{
public:
    Serializer(Glib::RefPtr<Glib::IOChannel> iochannel);
    void set_iochannel(Glib::RefPtr<Glib::IOChannel> iochannel);

    template <typename T>
    void read(T& value);

    void read(std::string& str);
    void read(Glib::ustring& str);

    template <typename T>
    void read(std::list<T>& list, bool clear = true);

    template <typename T1, typename T2>
    void read(std::map<T1, T2>& map, bool clear = true);

    template <typename T>
    const T read();

    template <typename T>
    void write(const T& value);

    void write(const std::string& str);
    void write(const Glib::ustring& str);

    template <typename T>
    void write(const std::list<T>& list);

    template <typename T1, typename T2>
    void write(const std::map<T1, T2>& map);

private:
    Glib::RefPtr<Glib::IOChannel> m_iochannel;
    std::string m_encoding;
};


/* read functions */

template <typename T>
void
Serializer::read(T& value)
{
    gsize bytes_read;

    m_iochannel->set_encoding();
    Glib::IOStatus status = m_iochannel->read((char*) &value, sizeof(value), bytes_read);
    m_iochannel->set_encoding(m_encoding);
    if (status != Glib::IO_STATUS_NORMAL)
        throw SerializerError(SerializerError::IO_ERROR,
            "error reading value of type " + std::string(typeid(T).name()));
}

template <typename T>
void
Serializer::read(std::list<T>& list, bool clear)
{
    typename std::list<T>::size_type size;
    read(size);

    if (clear) list.clear();

    while (size--) {
        T val;
        read(val);
        list.push_back(val);
    }
}

template <typename T1, typename T2>
void
Serializer::read(std::map<T1, T2>& map, bool clear)
{
    typename std::map<T1, T2>::size_type size;
    read(size);

    if (clear) map.clear();

    while (size--) {
        T1 key;
        T2 val;
        read(key);
        read(val);
        map[key] = val;
    }
}

template <typename T>
const T
Serializer::read()
{
    T val;
    read(val);
    return val;
}

/* write functions */

template <typename T>
void
Serializer::write(const T& value)
{
    gsize bytes_written;

    m_iochannel->set_encoding();
    Glib::IOStatus status = m_iochannel->write((const char*) &value, sizeof(value), bytes_written);
    m_iochannel->set_encoding(m_encoding);

    if (status != Glib::IO_STATUS_NORMAL)
        throw SerializerError(SerializerError::IO_ERROR,
            "error writing value of type " + std::string(typeid(T).name()));
}

template <typename T>
void
Serializer::write(const std::list<T>& list)
{
    typename std::list<T>::size_type size = list.size();
    write(size);

    for (typename std::list<T>::const_iterator it = list.begin(); it != list.end(); it++)
        write(*it);
}

template <typename T1, typename T2>
void
Serializer::write(const std::map<T1, T2>& map)
{
    typename std::map<T1, T2>::size_type size = map.size();
    write(size);

    for (typename std::map<T1, T2>::const_iterator it = map.begin(); it != map.end(); it++) {
        write(it->first);
        write(it->second);
    }
}

#endif // __vncbox_SERIALIZER_H
