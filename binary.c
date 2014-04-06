#include "global.h"

/* Given a sorted array with n elements, we search for who using binary search.
   We return a position where found, or -1 if not there
*/
int binary(int n, int* a, int who) {
	int left = 0; 
	int right = n-1;
	int mid;
	while (left <= right) {
		mid = left + (right-left)/2;
		if (who < a[mid])
			right = mid - 1;
		else if (who > a[mid])
			left = mid + 1;
		else
			return mid;
	}
	return left;
}
