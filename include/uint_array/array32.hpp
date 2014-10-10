#include "common.hpp"

namespace array32 {
  inline void print_sse_register(__m128i reg){
    cout << "reg[0]: " << _mm_extract_epi32(reg,0) << endl;
    cout << "reg[1]: " << _mm_extract_epi32(reg,1) << endl;
    cout << "reg[2]: " << _mm_extract_epi32(reg,2) << endl;
    cout << "reg[3]: " << _mm_extract_epi32(reg,3) << endl << endl;   
  }
  static char max = char(0xff);
  static __m128i shuffle_mask32[16] = {        
    _mm_set_epi8(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0), //0
    _mm_set_epi8(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0), //1
    _mm_set_epi8(15,14,13,12,11,10,9,8,7,6,5,4,7,6,5,4), //2
    _mm_set_epi8(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0), //3
    _mm_set_epi8(15,14,13,12,11,10,9,8,7,6,5,4,11,10,9,8), //4
    _mm_set_epi8(15,14,13,12,11,10,9,8,11,10,9,8,3,2,1,0), //5
    _mm_set_epi8(15,14,13,12,11,10,9,8,11,10,9,8,7,6,5,4), //6
    _mm_set_epi8(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0), //7
    _mm_set_epi8(15,14,13,12,11,10,9,8,7,6,5,4,15,14,13,12), //8
    _mm_set_epi8(15,14,13,12,11,10,9,8,15,14,13,12,3,2,1,0), //9
    _mm_set_epi8(15,14,13,12,11,10,9,8,15,14,13,12,7,6,5,4), //10
    _mm_set_epi8(15,14,13,12,15,14,13,12,7,6,5,4,3,2,1,0), //11
    _mm_set_epi8(15,14,13,12,11,10,9,8,15,14,13,12,11,10,9,8), //12
    _mm_set_epi8(15,14,13,12,15,14,13,12,11,10,9,8,3,2,1,0), //13
    _mm_set_epi8(15,14,13,12,15,14,13,12,11,10,9,8,7,6,5,4), //14
    _mm_set_epi8(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0), //15
  }; 
    //128 = 0xf0
  static __m128i shuffle_union_mask32 = _mm_set_epi8(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
 
  //128 = 0xf0
  static __m128i shuffle_difference_mask32_a[16] = {        
    _mm_set_epi8(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0), //0
    _mm_set_epi8(max,max,max,max,15,14,13,12,11,10,9,8,7,6,5,4), //1
    _mm_set_epi8(max,max,max,max,11,10,9,8,7,6,5,4,3,2,1,0), //2
    _mm_set_epi8(max,max,max,max,max,max,max,max,15,14,13,12,11,10,9,8), //3
    _mm_set_epi8(max,max,max,max,15,14,13,12,7,6,5,4,3,2,1,0), //4
    _mm_set_epi8(max,max,max,max,max,max,max,max,15,14,13,12,7,6,5,4), //5 0101
    _mm_set_epi8(max,max,max,max,max,max,max,max,15,14,13,12,3,2,1,0), //6 0110
    _mm_set_epi8(max,max,max,max,max,max,max,max,max,max,max,max,15,14,13,12), //7
    _mm_set_epi8(max,max,max,max,15,14,13,12,7,6,5,4,3,2,1,0), //8 1000
    _mm_set_epi8(max,max,max,max,max,max,max,max,11,10,9,8,7,6,5,4), //9 1001
    _mm_set_epi8(max,max,max,max,max,max,max,max,11,10,9,8,3,2,1,0), //10 1010
    _mm_set_epi8(max,max,max,max,max,max,max,max,max,max,max,max,11,10,9,8), //11 1011
    _mm_set_epi8(max,max,max,max,max,max,max,max,7,6,5,4,3,2,1,0), //12 1100
    _mm_set_epi8(max,max,max,max,max,max,max,max,max,max,max,max,7,6,5,4), //13 1101
    _mm_set_epi8(max,max,max,max,max,max,max,max,max,max,max,max,3,2,1,0), //14
    _mm_set_epi8(max,max,max,max,max,max,max,max,max,max,max,max,max,max,max,max), //15
  }; 
  //128 = 0xf0
  static __m128i shuffle_difference_mask32_b[16] = {        
    _mm_set_epi8(3,2,1,0,7,6,5,4,11,10,9,8,15,14,13,12), //0
    _mm_set_epi8(7,6,5,4,11,10,9,8,15,14,13,12,max,max,max,max), //1
    _mm_set_epi8(3,2,1,0,7,6,5,4,15,14,13,12,max,max,max,max), //2
    _mm_set_epi8(11,10,9,8,15,14,13,12,max,max,max,max,max,max,max,max), //3
    _mm_set_epi8(3,2,1,0,7,6,5,4,15,14,13,12,max,max,max,max), //4
    _mm_set_epi8(7,6,5,4,15,14,13,12,max,max,max,max,max,max,max,max), //5 0101
    _mm_set_epi8(3,2,1,0,15,14,13,12,max,max,max,max,max,max,max,max), //6 0110
    _mm_set_epi8(15,14,13,12,max,max,max,max,max,max,max,max,max,max,max,max), //7
    _mm_set_epi8(3,2,1,0,7,6,5,4,11,10,9,8,max,max,max,max), //8 1000
    _mm_set_epi8(7,6,5,4,11,10,9,8,max,max,max,max,max,max,max,max), //9 1001
    _mm_set_epi8(3,2,1,0,11,10,9,8,max,max,max,max,max,max,max,max), //10 1010
    _mm_set_epi8(11,10,9,8,max,max,max,max,max,max,max,max,max,max,max,max), //11 1011
    _mm_set_epi8(3,2,1,0,7,6,5,4,max,max,max,max,max,max,max,max), //12 1100
    _mm_set_epi8(7,6,5,4,max,max,max,max,max,max,max,max,max,max,max,max), //13 1101
    _mm_set_epi8(3,2,1,0,max,max,max,max,max,max,max,max,max,max,max,max), //14
    _mm_set_epi8(max,max,max,max,max,max,max,max,max,max,max,max,max,max,max,max), //15
  };
  static __m128i setinel_mask_a[16] = {        
    _mm_set_epi8(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0), //0
    _mm_set_epi8(max,max,max,max,0,0,0,0,0,0,0,0,0,0,0,0), //1
    _mm_set_epi8(max,max,max,max,0,0,0,0,0,0,0,0,0,0,0,0),//2
    _mm_set_epi8(max,max,max,max,max,max,max,max,0,0,0,0,0,0,0,0), //3
    _mm_set_epi8(max,max,max,max,max,max,max,max,max,max,max,max,0,0,0,0), //4
    _mm_set_epi8(max,max,max,max,max,max,max,max,0,0,0,0,0,0,0,0), //5
    _mm_set_epi8(max,max,max,max,max,max,max,max,0,0,0,0,0,0,0,0), //6
    _mm_set_epi8(max,max,max,max,max,max,max,max,max,max,max,max,0,0,0,0), //7
    _mm_set_epi8(max,max,max,max,0,0,0,0,0,0,0,0,0,0,0,0), //8
    _mm_set_epi8(max,max,max,max,max,max,max,max,0,0,0,0,0,0,0,0),//9
    _mm_set_epi8(max,max,max,max,max,max,max,max,0,0,0,0,0,0,0,0),  //10
    _mm_set_epi8(max,max,max,max,max,max,max,max,max,max,max,max,0,0,0,0),//11
    _mm_set_epi8(max,max,max,max,max,max,max,max,0,0,0,0,0,0,0,0),  //12
    _mm_set_epi8(max,max,max,max,max,max,max,max,max,max,max,max,0,0,0,0), //13
    _mm_set_epi8(max,max,max,max,max,max,max,max,max,max,max,max,0,0,0,0), //14
    _mm_set_epi8(max,max,max,max,max,max,max,max,max,max,max,max,max,max,max,max), //15
  }; 
  static __m128i setinel_mask_b[16] = {        
    _mm_set_epi8(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0), //0
    _mm_set_epi8(0,0,0,0,0,0,0,0,0,0,0,0,max,max,max,max), //1
    _mm_set_epi8(0,0,0,0,0,0,0,0,0,0,0,0,max,max,max,max),//2
    _mm_set_epi8(0,0,0,0,0,0,0,0,max,max,max,max,max,max,max,max), //3
    _mm_set_epi8(0,0,0,0,0,0,0,0,max,max,max,max,max,max,max,max), //4
    _mm_set_epi8(0,0,0,0,max,max,max,max,max,max,max,max,max,max,max,max), //5
    _mm_set_epi8(0,0,0,0,max,max,max,max,max,max,max,max,max,max,max,max), //6
    _mm_set_epi8(0,0,0,0,max,max,max,max,max,max,max,max,max,max,max,max), //7
    _mm_set_epi8(max,max,max,max,max,max,max,max,max,max,max,max,max,max,max,max), //8
    _mm_set_epi8(max,max,max,max,max,max,max,max,max,max,max,max,max,max,max,max), //9
    _mm_set_epi8(max,max,max,max,max,max,max,max,max,max,max,max,max,max,max,max), //10
    _mm_set_epi8(max,max,max,max,max,max,max,max,max,max,max,max,max,max,max,max), //11
    _mm_set_epi8(max,max,max,max,max,max,max,max,max,max,max,max,max,max,max,max), //12
    _mm_set_epi8(max,max,max,max,max,max,max,max,max,max,max,max,max,max,max,max), //13
    _mm_set_epi8(max,max,max,max,max,max,max,max,max,max,max,max,max,max,max,max), //14
    _mm_set_epi8(max,max,max,max,max,max,max,max,max,max,max,max,max,max,max,max), //15
  }; 

  inline size_t intersect(unsigned int *C, const unsigned int *A, const unsigned int *B, size_t s_a, size_t s_b) {
    size_t count = 0;
    size_t i_a = 0, i_b = 0;

    // trim lengths to be a multiple of 4
    #if VECTORIZE == 1
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
      unsigned int right_cyclic_shift = _MM_SHUFFLE(0,3,2,1);
      __m128i cmp_mask1 = _mm_cmpeq_epi32(v_a, v_b);    // pairwise comparison
      v_b = _mm_shuffle_epi32(v_b, right_cyclic_shift);       // shuffling
      __m128i cmp_mask2 = _mm_cmpeq_epi32(v_a, v_b);    // again...
      v_b = _mm_shuffle_epi32(v_b, right_cyclic_shift);
      __m128i cmp_mask3 = _mm_cmpeq_epi32(v_a, v_b);    // and again...
      v_b = _mm_shuffle_epi32(v_b, right_cyclic_shift);
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
      //cout << "mask: " << mask << endl;
      __m128i p = _mm_shuffle_epi8(v_a, shuffle_mask32[mask]);
      _mm_storeu_si128((__m128i*)&C[count], p);
      //cout << "C[" << count << "]: " << C[count] << endl;

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

    #if WRITE_VECTOR == 0
    C = C;
    #endif
    
    return count;
  }
  inline size_t set_union(unsigned int *C, const unsigned int *A, const unsigned int *B, size_t s_a, size_t s_b) {
    size_t count = 0;
    size_t i_a = 0, i_b = 0;
    size_t next_i_a = 0, next_i_b = 0;
    // trim lengths to be a multiple of 4
    #if VECTORIZE == 1
    size_t st_a = (s_a / 4) * 4;
    size_t st_b = (s_b / 4) * 4;
    unsigned int num_hit = 0;
    __m128i v_a;
    __m128i v_b;
    unsigned int a_max = 0;
    unsigned int b_max = 0;
    if(i_a < st_a && i_b < st_b){
      v_a = _mm_loadu_si128((__m128i*)&A[i_a]);
      v_b = _mm_loadu_si128((__m128i*)&B[i_b]);
      a_max = _mm_extract_epi32(v_a, 3);
      b_max = _mm_extract_epi32(v_b, 3);
    }
    while(i_a < st_a && i_b < st_b) {
      next_i_a = i_a + 4;
      next_i_b = i_b + 4;

      cout << "printing data" << endl;
      print_sse_register(v_a);
      print_sse_register(v_b);

      //[ compute mask of common elements
      unsigned int right_cyclic_shift = _MM_SHUFFLE(0,3,2,1);
      
      __m128i cmp_a_mask0 = _mm_cmpeq_epi32(v_a, v_b);    // pairwise comparison
      v_b = _mm_shuffle_epi32(v_b, right_cyclic_shift);       // shuffling

      __m128i cmp_a_mask1 = _mm_cmpeq_epi32(v_a, v_b);    // again...
      v_b = _mm_shuffle_epi32(v_b, right_cyclic_shift);

      __m128i cmp_a_mask2 = _mm_cmpeq_epi32(v_a, v_b);    // and again...
      v_b = _mm_shuffle_epi32(v_b, right_cyclic_shift);

      __m128i cmp_a_mask3 = _mm_cmpeq_epi32(v_a, v_b);    // and again.
      v_b = _mm_shuffle_epi32(v_b, right_cyclic_shift);

      __m128i cmp_a_mask = _mm_or_si128(
              _mm_or_si128(cmp_a_mask0, cmp_a_mask1),
              _mm_or_si128(cmp_a_mask2, cmp_a_mask3)
      ); // OR-ing of comparison masks
      
      // convert the 128-bit mask to the 4-bit mask
      unsigned int mask_a = _mm_movemask_ps((__m128)cmp_a_mask);
      
      num_hit = _mm_popcnt_u32(mask_a);
      //[ copy out common elements
      #if WRITE_VECTOR == 1
      __m128i p_a = _mm_or_si128(setinel_mask_a[mask_a],_mm_shuffle_epi8(v_a, shuffle_difference_mask32_a[mask_a]));  
      __m128i p_b = v_b;
     
      __m128i l_1 = _mm_min_epu32(p_a,p_b);
      __m128i h_1 = _mm_max_epu32(p_a,p_b);      

      unsigned int left_cyclic_shift = _MM_SHUFFLE(2,1,0,3);

      // x x x v = l1 (v = valid data, x = tbd)
      // v x x x = h1 
      __m128i l_2 = _mm_min_epu32(_mm_shuffle_epi32(h_1,left_cyclic_shift),l_1);
      __m128i h_2 = _mm_max_epu32(h_1,_mm_shuffle_epi32(l_1,right_cyclic_shift));

      // x x v v = l1 (v = valid data, x = tbd)
      // v v x x = h2 

      left_cyclic_shift = _MM_SHUFFLE(1,0,3,2);
      right_cyclic_shift = left_cyclic_shift;
      __m128i l_3 = _mm_min_epu32(_mm_shuffle_epi32(h_2, left_cyclic_shift),l_2);
      __m128i h_3 = _mm_max_epu32(h_2,_mm_shuffle_epi32(l_2,right_cyclic_shift));

      left_cyclic_shift = _MM_SHUFFLE(0,3,2,1);
      right_cyclic_shift = _MM_SHUFFLE(2,1,0,3);

      __m128i l_4 = _mm_min_epu32(_mm_shuffle_epi32(h_3, left_cyclic_shift),l_3);
      __m128i h_4 = _mm_max_epu32(h_3,_mm_shuffle_epi32(l_3,right_cyclic_shift));
      _mm_storeu_si128((__m128i*)&C[count], l_4);
      _mm_storeu_si128((__m128i*)&C[count+INTS_PER_REG], h_4);

      cout <<"Num hit: " << num_hit << endl;
      print_sse_register(l_4);
      print_sse_register(h_4);

      #endif

      unsigned int new_a_max = a_max;
      cout << "count: " << count << endl;
      cout << "maxes: " << a_max << " " << b_max << " " << i_a << " " << next_i_a << endl;
      if(a_max <= b_max){
        i_a = next_i_a;
        v_a = _mm_loadu_si128((__m128i*)&A[i_a]);
        new_a_max = _mm_extract_epi32(v_a, 3);
        v_b = _mm_loadu_si128((__m128i*)&C[count+(INTS_PER_REG-num_hit)]);
      }
      if(a_max >= b_max){
        i_b = next_i_b;
        v_b = _mm_loadu_si128((__m128i*)&B[i_b]);
        b_max = _mm_extract_epi32(v_b, 3);
        v_a = _mm_loadu_si128((__m128i*)&C[count+(INTS_PER_REG-num_hit)]);
      }
      a_max = new_a_max;
      count += INTS_PER_REG-num_hit;// a number of elements is a weight of the mask
      cout << "count: " << count << endl;
    }
    #endif

    if(a_max < b_max && i_b < s_b){
      bool a = i_b < s_b && C[count] > B[i_b];
      bool b = (i_b+1) < s_b && C[count+1] > B[i_b+1];
      bool c = (i_b+2) < s_b && C[count+2] > B[i_b+2];
      bool d = (i_b+3) < s_b && C[count+3] > B[i_b+3];

      size_t incr = a*1 + b*1 + c*1 + d*1;
      cout << "incr: " << incr << endl;
      count += incr;
      i_b += incr;
    }
    if(a_max > b_max && i_a < s_a){
      bool a = i_a < s_a && C[count] > A[i_a];
      bool b = (i_a+1) < s_a && C[count+1] > A[i_a+1];
      bool c = (i_a+2) < s_a && C[count+2] > A[i_a+2];
      bool d = (i_a+3) < s_a && C[count+3] > A[i_a+3];

      size_t incr = a*1 + b*1 + c*1 + d*1;
      cout << "incr: " << incr << " " << _mm_extract_epi32(v_a,0) << endl;
      count += incr;
      i_a += incr;
    }

    // intersect the tail using scalar intersection
    bool notFinished = i_a < s_a  && i_b < s_b;
    while(notFinished){
      while(notFinished && B[i_b] < A[i_a]){
        #if WRITE_VECTOR == 1
        C[count] = B[i_b];
        #endif
        ++count;
        ++i_b;
        notFinished = i_b < s_b;
      }
      cout << A[i_a] << endl;
      #if WRITE_VECTOR == 1
      C[count] = A[i_a];
      #endif
      ++count;
      ++i_a;
      notFinished = notFinished && i_a < s_a;
    }
    while(i_a < s_a){
      #if WRITE_VECTOR == 1
      C[count] = A[i_a];
      #endif
      ++count;
      ++i_a;
    }
    while(i_b < s_b){
      #if WRITE_VECTOR == 1
      C[count] = B[i_b];
      #endif
      ++count;
      ++i_b;
    }

    #if WRITE_VECTOR == 0
    C = C;
    #endif

    return count;
  }
  inline size_t difference(unsigned int *C, const unsigned int *A, const unsigned int *B, size_t s_a, size_t s_b) {
    size_t count = 0;
    size_t i_a = 0, i_b = 0;
    size_t next_i_a = 0, next_i_b = 0;

    // trim lengths to be a multiple of 4
    #if VECTORIZE == 1
    size_t st_a = (s_a / 4) * 4;
    size_t st_b = (s_b / 4) * 4;
    unsigned int num_hit = 0;
    __m128i v_a;
    __m128i v_b;
    unsigned int a_max = 0;
    unsigned int b_max = 0;
    if(i_a < st_a && i_b < st_b){
      v_a = _mm_loadu_si128((__m128i*)&A[i_a]);
      v_b = _mm_loadu_si128((__m128i*)&B[i_b]);
      a_max = _mm_extract_epi32(v_a, 3);
      b_max = _mm_extract_epi32(v_b, 3);
    }
    while(i_a < st_a && i_b < st_b) {
      //[ load segments of four 32-bit elements
      next_i_a = i_a+4;
      next_i_b = i_b+4;      

      //[ compute mask of common elements
      unsigned int right_cyclic_shift = _MM_SHUFFLE(0,3,2,1);     

      __m128i cmp_a_mask0 = _mm_cmpeq_epi32(v_a, v_b);    // pairwise comparison
      v_b = _mm_shuffle_epi32(v_b, right_cyclic_shift);       // shuffling

      __m128i cmp_a_mask1 = _mm_cmpeq_epi32(v_a, v_b);    // again...
      v_b = _mm_shuffle_epi32(v_b, right_cyclic_shift);

      __m128i cmp_a_mask2 = _mm_cmpeq_epi32(v_a, v_b);    // and again...
      v_b = _mm_shuffle_epi32(v_b, right_cyclic_shift);

      __m128i cmp_a_mask3 = _mm_cmpeq_epi32(v_a, v_b);    // and again.
      v_b = _mm_shuffle_epi32(v_b, right_cyclic_shift);

      __m128i cmp_a_mask = _mm_or_si128(
              _mm_or_si128(cmp_a_mask0, cmp_a_mask1),
              _mm_or_si128(cmp_a_mask2, cmp_a_mask3)
      ); // OR-ing of comparison masks

      // convert the 128-bit mask to the 4-bit mask
      unsigned int mask_a = _mm_movemask_ps((__m128)cmp_a_mask);
      
      num_hit += _mm_popcnt_u32(mask_a);
      //[ copy out common elements
      #if WRITE_VECTOR == 1
      __m128i p_a = _mm_shuffle_epi8(v_a, shuffle_difference_mask32_a[mask_a]);
      #endif

      unsigned int new_a_max = a_max;
      if(a_max <= b_max){
        _mm_storeu_si128((__m128i*)&C[count], p_a);
        count += INTS_PER_REG-num_hit;
        num_hit = 0;
        i_a = next_i_a;
        v_a = _mm_loadu_si128((__m128i*)&A[i_a]);
        new_a_max = _mm_extract_epi32(v_a, 3);
      }
      if(a_max >= b_max){
        i_b = next_i_b;
        v_b = _mm_loadu_si128((__m128i*)&B[i_b]);
        b_max = _mm_extract_epi32(v_b, 3);    
      }
      a_max = new_a_max;
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
      if(notFinished && A[i_a] != B[i_b]){
        #if WRITE_VECTOR == 1
        C[count] = A[i_a];
        #endif
        ++count;
      }
      ++i_a;
      notFinished = notFinished && i_a < s_a;
    }
    while(i_a < s_a){
      C[count++] = A[i_a++];
    }

    #if WRITE_VECTOR == 0
    C = C;
    #endif
    
    return count;
  }
  template<typename T> 
  inline T sum_decoded(T (*function)(unsigned int,unsigned int,unsigned int*),unsigned int col,unsigned int *data, size_t length,unsigned int *outputA){
    T result = (T) 0;
    for(size_t i = 0; i < length; i++){
      result += function(col,data[i],outputA);
    }
    return result;
  }
  template<typename T> 
  inline T sum(unsigned int *data, size_t length, T *old_data, unsigned int *lengths){
    T result = 0.0;
    for(size_t i = 0; i < length; i++){
      result += old_data[data[i]]/lengths[data[i]];
    } 
    return result;
  }
  inline void print_data(unsigned int *data,size_t length,ofstream &file){
    //cout << "LEN: " << length << endl;
    for(size_t i = 0; i < length; i++){
      file << " Data: " << data[i] << endl;
    }
  }
  inline size_t preprocess(unsigned int *r, unsigned int *data, size_t length){
    std::copy(data,data+length,r);
    return length*4;
	}
} 