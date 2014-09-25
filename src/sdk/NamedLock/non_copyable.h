/*
    Written by Jelle Geerts (jellegeerts@gmail.com).

    To the extent possible under law, the author(s) have dedicated all
    copyright and related and neighboring rights to this software to
    the public domain worldwide. This software is distributed without
    any warranty.

    You should have received a copy of the CC0 Public Domain Dedication
    along with this software.
    If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef NON_COPYABLE_H
#define NON_COPYABLE_H

class non_copyable
{
protected:
#if __cplusplus >= 201103 // C++11 or later.
    non_copyable() = default;

    // Defining a protected destructor prevents this situation:
    //     object * o = new object; // Assume object derives from non_copyable.
    //     non_copyable * m = o;
    //     delete m; // ~object() will not be called.
    ~non_copyable() = default;

    non_copyable(const non_copyable &) = delete;
    non_copyable & operator=(const non_copyable &) = delete;
#else /* __cplusplus < 201103 */
    non_copyable() {};
    ~non_copyable() {};

private:
    // These are private so user can't access them, thereby providing
    // compile-time errors if such usage does occur.
    non_copyable(const non_copyable &);
    non_copyable & operator=(const non_copyable &);
#endif /* __cplusplus < 201103 */
};

#endif
