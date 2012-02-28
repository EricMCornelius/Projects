/*
 * Utilities.h
 *
 *  Created on: Feb 25, 2012
 *      Author: corneliu
 */

#ifndef UTILITIES_H_
#define UTILITIES_H_

// returns log2 of val, or 0 if non-integral result
inline constexpr size_t integral_log2(const size_t val, const size_t depth = 0) {
  return (val == 1) ? depth : (val % 2) ? 0 : integral_log2(val / 2, depth + 1);
}

#endif /* UTILITIES_H_ */
