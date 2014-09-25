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

#ifndef COMPILER_SPECIFIC_H
#define COMPILER_SPECIFIC_H

#ifndef __GNUC__
# error "Unsupported compiler"
#endif /* !defined(__GNUC__) */

#ifdef __GNUC__
# ifndef GCC_VERSION
#  define GCC_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
# endif /* !defined(GCC_VERSION) */

# define MACRO_BEGIN do {
# define MACRO_END } while(0)

# define ATTRIBUTE_UNUSED __attribute__((__unused__))

# if GCC_VERSION >= 3003
#  define ATTRIBUTE_NONNULL(i) __attribute__((__nonnull__(i)))
# else /* GCC_VERSION < 3003 */
#  define ATTRIBUTE_NONNULL(i)
# endif /* GCC_VERSION < 3003 */

# if GCC_VERSION >= 4004
#  define ATTRIBUTE_SENTINEL(i) __attribute__((__sentinel__(i)))
# else /* GCC_VERSION < 4004 */
#  define ATTRIBUTE_SENTINEL(i)
# endif /* GCC_VERSION < 4004 */

# define ATTRIBUTE_PACKED __attribute__((__packed__))

# define ATTRIBUTE_FORMAT(i,j,k) __attribute__((__format__(i,j,k))) ATTRIBUTE_NONNULL(j)
# define ATTRIBUTE_FORMAT_PRINTF __printf__
# define ATTRIBUTE_FORMAT_SCANF __scanf__
#endif /* defined(__GNUC__) */

#endif /* !defined(COMPILER_SPECIFIC_H) */
