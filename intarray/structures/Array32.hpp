#include "Common.hpp"

#define VECTORIZE 1
#define WRITE_VECTOR 1

using namespace std;

namespace array32 {
  static __m128i shuffle_mask32[16]; // precomputed dictionary
  static int getBitV(int value, int position) {
    return ( ( value & (1 << position) ) >> position);
  }
  static void prepare_shuffling_dictionary32() {
    for(int i = 0; i < 16; i++) {
      int counter = 0;
      char permutation[16];
      memset(permutation, 0xFF, sizeof(permutation));
      for(char b = 0; b < 4; b++) {
        if(getBitV(i, b)) {
          permutation[counter++] = 4*b;
          permutation[counter++] = 4*b + 1;
          permutation[counter++] = 4*b + 2;
          permutation[counter++] = 4*b + 3;
        }
      }
      __m128i mask = _mm_loadu_si128((const __m128i*)permutation);
      shuffle_mask32[i] = mask;
    }
  }
  inline size_t intersect(unsigned int *C, const unsigned int *A, const unsigned int *B, size_t s_a, size_t s_b) {
    size_t count = 0;
    size_t i_a = 0, i_b = 0;

    // trim lengths to be a multiple of 4
    #ifdef VECTORIZE
    size_t st_a = (s_a / 4) * 4;
    size_t st_b = (s_b / 4) * 4;
    while(i_a < st_a && i_b < st_b) {
      //[ load segments of four 32-bit elements
      __m128i v_a = _mm_loadu_si128((__m128i*)&A[i_a]);
      __m128i v_b = _mm_loadu_si128((__m128i*)&B[i_b]);
      //]

      //[ move pointers
      unsigned int a_max = _mm_extract_epi32(v_a, 3);
      unsigned int b_max = _mm_extract_epi32(v_b, 3);
      i_a += (a_max <= b_max) * 4;
      i_b += (a_max >= b_max) * 4;
      //]

      //[ compute mask of common elements
      unsigned int cyclic_shift = _MM_SHUFFLE(0,3,2,1);
      __m128i cmp_mask1 = _mm_cmpeq_epi32(v_a, v_b);    // pairwise comparison
      v_b = _mm_shuffle_epi32(v_b, cyclic_shift);       // shuffling
      __m128i cmp_mask2 = _mm_cmpeq_epi32(v_a, v_b);    // again...
      v_b = _mm_shuffle_epi32(v_b, cyclic_shift);
      __m128i cmp_mask3 = _mm_cmpeq_epi32(v_a, v_b);    // and again...
      v_b = _mm_shuffle_epi32(v_b, cyclic_shift);
      __m128i cmp_mask4 = _mm_cmpeq_epi32(v_a, v_b);    // and again.
      __m128i cmp_mask = _mm_or_si128(
              _mm_or_si128(cmp_mask1, cmp_mask2),
              _mm_or_si128(cmp_mask3, cmp_mask4)
      ); // OR-ing of comparison masks
      // convert the 128-bit mask to the 4-bit mask
      unsigned int mask = _mm_movemask_ps((__m128)cmp_mask);
      //]
      
      //[ copy out common elements
      #if WRITE_VECTOR == 1
      __m128i p = _mm_shuffle_epi8(v_a, shuffle_mask32[mask]);
      _mm_storeu_si128((__m128i*)&C[count], p);
      #endif

      count += _mm_popcnt_u32(mask); // a number of elements is a weight of the mask
      //]
    }
    #endif

    // intersect the tail using scalar intersection
    bool notFinished = i_a < s_a  && i_b < s_b;
    while(notFinished){
      while(notFinished && B[i_b] < A[i_a]){
        ++i_b;
        notFinished = i_b < s_b;
      }
      if(notFinished && A[i_a] == B[i_b]){
        #if WRITE_VECTOR == 1
        C[count] = A[i_a];
        #endif

        ++count;
      }
      ++i_a;
      notFinished = notFinished && i_a < s_a;
    }

    return count;
  }
  
  template<typename T> 
  inline T foreach(T (*function)(unsigned int,unsigned int),unsigned int col,unsigned short *data, size_t length){
    T result = (T) 0;
    unsigned int *actual_data = (unsigned int*) data;
    for(size_t i = 0; i < (length/2); i++){
      result += function(col,actual_data[i]);
    }
    return result;
  }

  inline void print_data(unsigned short *data,size_t length){
    unsigned int *actual_data = (unsigned int*) data;
    cout << "LEN: " << length << endl;
    for(size_t i = 0; i < (length/2); i++){
      cout << " Data: " << actual_data[i] << endl;
    }
  }
  inline size_t preprocess(unsigned short *r, size_t index, unsigned int *data, size_t length){
    prepare_shuffling_dictionary32();
    std::copy(data,data+length,(unsigned int*)(r+index));
    return index+length*2;
	}


	template<typename T> 
	T reduce(unsigned short *data, size_t length,T (*function)(T,T), T zero){
		unsigned int *actual_data = (unsigned int*) data;
		T result = zero;
    #pragma omp parallel for schedule(static,150) reduction(+:result)   
		for(size_t i = 0; i < length; i++){
			result = function(result,actual_data[i]);
		}	
		return result;
	}
} 