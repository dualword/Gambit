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

namespace Utils
{
namespace Math
{

bool isPrime(int n)
{
    // If this number equals 1, it is by definition not a prime.
    if (n == 1)
        return false;

    // If this is an even number, it is by definition not a prime.
    if ((n & 1) == 0)
        return false;

    for (int i = 2; i < n - 1; ++i)
    {
        if (n % i == 0)
            return false;
    }

    return true;
}

} /* namespace Math */
} /* namespace Utils */
