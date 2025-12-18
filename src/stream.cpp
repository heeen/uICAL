/*############################################################################
# Copyright (c) 2020 Source Simian  :  https://github.com/sourcesimian/uICAL #
############################################################################*/
#include "uICAL/cppstl.h"
#include "uICAL/stream.h"
#include "uICAL/string.h"

namespace uICAL {
    ostream& ostream::operator <<(const ostream& stm) {
        for (string st : stm.strings) {
            this->strings.push_back(st);
        }
        return *this;
    }

    ostream& ostream::operator <<(const char* st) {
        this->strings.push_back(st);
        return *this;
    }

    ostream& ostream::operator <<(const string& st) {
        this->strings.push_back(st);
        return *this;
    }

    ostream& ostream::operator <<(char ch) {
        this->strings.push_back(string::fmt("%c", ch));
        return *this;
    }

    ostream& ostream::operator <<(int i) {
        this->strings.push_back(string::fmt("%d", i));
        return *this;
    }

    ostream& ostream::operator <<(unsigned int i) {
        this->strings.push_back(string::fmt("%u", i));
        return *this;
    }

    ostream& ostream::operator <<(long long int i) {
        this->strings.push_back(string::fmt("%lld", i));
        return *this;
    }

    ostream::operator string() const {
        return this->str();
    }

    bool ostream::empty() const {
        return this->strings.size() == 0;
    }

    void ostream::clear() {
        this->strings.clear();
    }

    string ostream::str() const {
        string ret = "";
        for (string st : this->strings) {
            ret += st;
        }
        return ret;
    }

    #ifdef ARDUINO

        istream_Stream::istream_Stream(Stream& stm)
        : stm(stm)
        {}

        char istream_Stream::peek() const {
            int ch = this->stm.peek();
            return ch;
        }

        char istream_Stream::get() {
            int ch = this->stm.read();
            return ch;
        }

        bool istream_Stream::readuntil(string& st, char delim) {
            // ICS lines can be very long before folding - use larger buffer
            // and loop to handle lines longer than buffer
            static const size_t CHUNK_SIZE = 256;
            char buf[CHUNK_SIZE];
            st = "";

            while (true) {
                size_t bytesRead = this->stm.readBytesUntil(delim, buf, CHUNK_SIZE - 1);
                if (bytesRead == 0) {
                    // Timeout or EOF with no data
                    return !st.isEmpty();
                }

                buf[bytesRead] = 0;
                st += buf;

                // If we read less than buffer size, we hit the delimiter
                if (bytesRead < CHUNK_SIZE - 1) {
                    return true;
                }
                // Buffer was full - continue reading until delimiter
            }
        }

        istream_String::istream_String(const String& st)
        : st(st)
        , pos(0)
        {}

        char istream_String::peek() const {
            return this->st.charAt(this->pos);
        }

        char istream_String::get() {
            return this->st.charAt(this->pos++);
        }

        bool istream_String::readuntil(string& st, char delim) {
            if (this->pos >= this->st.length()) {
                return false;
            }

            size_t index = this->st.indexOf(delim, this->pos);
            if (index == (size_t)-1) {
                st = this->st.substring(this->pos);
                this->pos = this->st.length();
            }
            else {
                st = this->st.substring(this->pos, index);
                this->pos = index + 1;
            }
            return true;
        }

    #else

        istream_stl::istream_stl(std::istream& istm)
        : istm(istm)
        {}

        char istream_stl::peek() const {
            return this->istm.peek();
        }

        char istream_stl::get() {
            return this->istm.get();
        }

        bool istream_stl::readuntil(string& st, char delim) {
            if (std::getline(this->istm, st, delim)) {
                return true;
            }
            return false;
        }

    #endif
}
