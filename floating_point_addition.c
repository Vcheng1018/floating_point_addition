#include <stdio.h>
#include <stdlib.h>


#define FOUR_BIT_BASK 15 

// Defined as macros the expressions for extracting the
// sign, exponent, and fraction fields of a 32-bit
// floating point number.

// bit 31, shifted all the way to the right.

#define SIGN(x) ((x) >> 31)

// bits 23 through 30, shifted right by 23 bits

#define EXP(x) (((x) >> 23) & 0xFF)

// bits 0 through 22 (the rightmost 23 bits)

#define FRAC(x) ((x) & 0x7FFFFF)


// This function performs a floating point addition without
// using the built-in floating point addition -- instead, it only
// uses integer addition and subtraction. It does so
// by extracting on the sign, exponent, and fraction of the
// operands and performing operations using those to
// compute the sign, exponent, and operand of the result.

float float_add(float f, float g)
{

  // The values stored in f and g
  // as 32-bit unsigned numbers. The
  // sign, exponent, and fraction fields from f
  // and g, are extracted using the SIGN, EXP, and FRAC macros
  
  unsigned int *pf = (unsigned int *)&f;
  unsigned int sign_f = SIGN(*pf);
  

  unsigned int *pg = (unsigned int *)&g;
  unsigned int sign_g =  SIGN(*pg);

  
  unsigned int exp_f =  EXP(*pf);
  unsigned int exp_g =  EXP(*pg);

  unsigned int frac_f = FRAC(*pf);
  unsigned int frac_g = FRAC(*pg);

  // Handle the special case where f is zero (i.e.
  // both the exp_f and frac_f are zero), 
  // in which case the value of g should be returned immediately.
  if (exp_f == 0 && frac_f == 0){
      return g;
  }
  
  
  // Do the same for g (i.e. check if g is zero).
  if (exp_g == 0 && frac_g == 0){
    return f;
  }
  

  // In order to perform the multiplication, the implicit
  // leading 1 in the mantissa for f and g must be made
  // explicit. That is, the mantissa for f should contain
  // a 1 in the bit 23 position, followed by the bits of frac_f.
  // The same is true for the mantissa of g.

  unsigned int mantissa_f = frac_f | 0x800000;
  unsigned int mantissa_g = frac_g | 0x800000;

  // Before performing any addition, the two numbers must have the
  // same exponent. The mantissa of the number with the smaller
  // exponent is shifted right by the difference in the
  // exponents, and the smaller exponent is set to the larger exponent.
  // If f has a smaller exponent than g, shift mantissa_f
  // the right by (exp_g - exp_f) bits and set exp_f to exp_g.

    
  if (exp_f < exp_g){
    mantissa_f = mantissa_f >> (exp_g - exp_f);
    exp_f = exp_g;
  }
  else if (exp_g < exp_f){
    mantissa_g = mantissa_g >> (exp_f - exp_g);
    exp_g = exp_f;
  }


  // The exponent of the result (before normalization) is
  // now the same as exponent_f (which is the same as
  // exponent_g).
  
  // This will hold the sign of the result.

  unsigned int sign_res;

  // This will hold the mantissa of the result.
  
  unsigned int mantissa_res;

  // This holds the exponent of the result.
  
  unsigned int exp_res = exp_f;

  // If  sign_f and sign_g are the same, i.e. they are both
  // 0 (positive) or 1 (negative), then:
  //    -- the sign of the result is the same sign_f and sign_g
  //    -- the mantissa of the result is just the sum of mantissa_f and
  //       mantissa_g.
  //    -- since the sum of the two mantissas may cause a carry into
  //       bit 24 of the result, the result may need to be renormalized.
  //       That is, if bit 24 of the result mantissa is 1, then the
  //       result mantissa should be shifted to the right by 1 and the
  //       exponent of the result should be incremented by 1.

  if (sign_f == sign_g) {
      sign_res = sign_f;
      mantissa_res = mantissa_f + mantissa_g;
      if(((mantissa_res >> 24) && 1) == 1){
        mantissa_res = mantissa_res >> 1;
        exp_res++;
      }
   

  }

  else {

    // Otherwise, namely if sign_f and sign_g are different (i.e. one
    // number was positive and one negative), then:
    //    -- the sign of the result is the sign of the number with the larger
    //       mantissa (since the numbers have the same exponent at this point).
    //    -- the mantissa of the result should be the result of subtracting
    //       the smaller mantissa from the larger mantissa.
    //
    //       For example, if mantissa_f > mantissa_g, then the sign of the result is
    //       set to sign_f and the mantissa of the result is set to
    //       (mantissa_f - mantissa_g).
    // 
    //    -- If the resulting mantissa is 0, then the entire result is 0 and 
    //       the function should just return 0.0.
    //    -- Otherwise, the resulting mantissa may be small enough that it has to be
    //       renormalized to have a 1 in the bit 23 position. Do this in
    //       a loop, shifting the result mantissa to the left by 1 bit and subtracting
    //       1 from the result exponent, until the mantissa has a 1 in the
    //       bit 23 position.
        if(mantissa_f > mantissa_g){
          sign_res = sign_f;
          mantissa_res = (mantissa_f - mantissa_g);
        }
        else if (mantissa_g > mantissa_f){
          sign_res = sign_g;
          mantissa_res = (mantissa_g - mantissa_f);
        }

        else if (mantissa_g == mantissa_f){
          sign_res = sign_f;
          mantissa_res = (mantissa_f - mantissa_g);
        }
        
        if (mantissa_res == 0){
          return 0.0;
        }
        else{
          while (((mantissa_res >> 23) & 1) != 1){
            mantissa_res = mantissa_res << 1;
            exp_res --;
          }
        }

  }
  
  //  -- the sign bit of the result, shifting into the sign position
  //  -- the lowest 8 bits of the exponent, shifted into exponent position
  //  -- the lowest 23 bits of the mantissa (i.e. removing the 1 in bit 23 position,
  //     since it is implicit)

  unsigned int result = 0;
  result  =  result | (sign_res << 31);
  unsigned int exp_final = exp_res & 0xFF;
  result = result | (exp_final << 23);
  unsigned int mantissa_final = mantissa_res & 0x7FFFFF;
  result = result | (mantissa_final);
  
  float *fpx = (float *)&result;
  float final_float = *fpx;
  // Return the computed result (which is an unsigned int) as a floating point number.
  // Be sure that the compiler does not perform a conversion (see the hint sheet).
  return final_float;
  
}

    


  
  

int main()
{


  float f, g;

  printf("Enter two floating point numbers (to add) > ");
  scanf("%f", &f);
  scanf("%f", &g);

  printf("Computed %f + %f = %f\n", f, g, float_add(f,g));
  printf("Checking, answer should be %f\n", f+g);

  
}