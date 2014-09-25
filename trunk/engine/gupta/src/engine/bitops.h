#ifndef BITOPS_H
#define BITOPS_H

#define BIT_SET(bitset, index) ((bitset) |= 1 << (index))
#define BIT_IS_SET(bitset, index) (((bitset) & (1 << (index))) != 0)
/* For more emphasis, one can use BIT_IS_NOT_SET instead of !BIT_IS_SET. */
#define BIT_IS_NOT_SET(bitset, index) (!BIT_IS_SET((bitset), (index)))

#define BIT_IS_ANY_SET(bitset, mask) (((bitset) & (mask)) != 0)

#define BITS_ARE_ALL_CLEAR(bitset, mask) (((bitset) & (mask)) == 0)
#define BITS_ARE_ALL_SET(bitset, mask) (((bitset) & (mask)) == (mask))

#endif /* !defined(BITOPS_H) */
