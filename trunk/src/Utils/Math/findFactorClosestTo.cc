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

#ifdef TEST_FIND_FACTOR_CLOSEST_TO

#ifdef NDEBUG
# error "Compile me without NDEBUG defined."
#endif /* defined(NDEBUG) */

#include <cassert>
#include <cmath>

int findFactorClosestTo(int, int, bool = true);

#endif /* defined(TEST_FIND_FACTOR_CLOSEST_TO) */

/*
 * Finds a factor of 'value' closest to the value 'target', rounding up or down
 * depending on the value of the 'roundUp' parameter.
 *
 * Returns -1 if and only if 'value' or 'target' is lower than or equal to zero
 * or if 'value' is a prime number. Otherwise, the factor closest to 'target'
 * is returned.
 */
int findFactorClosestTo(int target, int value, bool roundUp /* = true */)
{
    if (value <= 0 || target < 0)
        return -1;

    // Any negative integer would do here, since it's further away from
    // 'target' than all possible factors as the factors have to be positive.
    int bestCandidate = -1;

    int squareRoot = int(sqrt(value));
    for (int i = 2; i <= squareRoot; ++i)
    {
        if (value % i == 0)
        {
            int deltaI = target - i;
            int deltaB = target - bestCandidate;
            bool needToRound = deltaI + deltaB == 0;

            if (needToRound || (deltaI < 0 ? deltaI : -deltaI) >= (deltaB < 0 ? deltaB : -deltaB))
                bestCandidate = (!needToRound || roundUp) ? i : bestCandidate;
        }
    }

    return bestCandidate;
}

#ifdef TEST_FIND_FACTOR_CLOSEST_TO
int main(void)
{
    assert(findFactorClosestTo(0, 0) == -1);
    assert(findFactorClosestTo(-1, 1) == -1);
    assert(findFactorClosestTo(5, 60, false) == 5);
    assert(findFactorClosestTo(5, 60, true) == 5);

    // Test the rounding code paths. First, 2 will be the best candidate since
    // it is a factor of 80. Then, 4 is also found to be a factor of 80, and
    // because 3 - 4 = -1 and 3 - 2 = 1, needToRound flips to 'true', so that
    // even though -1 (deltaI) >= -1 (-deltaB) == false, we evaluate whether we
    // have a new best candidate. Also, both values for 'roundUp' are tested.
    assert(findFactorClosestTo(3, 80, false) == 2);
    assert(findFactorClosestTo(3, 80, true) == 4);

    return 0;
}
#endif /* defined(TEST_FIND_FACTOR_CLOSEST_TO) */

} /* namespace Math */
} /* namespace Utils */
