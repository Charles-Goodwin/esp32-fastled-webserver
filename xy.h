/*  
 *  The Neopixel matrix we are dealing with is somewhat irregular and
 *  so I have built this function to return the serial
 *  position of the neopixel based on the X,Y coordinates
 *  provided.
 *  
 *  The matrix is in the shape of a bridge; 
 *  4 long lengths on iether side and 11 short lengths across the top
 *  
 *        S******************
 *        *******************
 *        ****           ****
 *        ****           ****
 *        0***           ****]
 *  
 *  S denotes where the neopixel strip begins (position 1)
 *  0 denotes the coords (0,0)
 *  
 *  The number of neopixels for each length are defined by:
 *    LEDNUM_SHORT and
 *    LEDNUM_LONG
 *    
 *  The sequence of neopixelas follows a boustrophedon path running up and down the matrix  
 *  
 *  A response of -1 is returned for any coordinate that does not have a coresponding neopixel   
 *  
 */
 
      
int XY (uint8_t x, uint8_t y) {
  //Declare how many pixels are in each length
  const uint8_t LEDNUM_SHORT = 10;
  const uint8_t LEDNUM_LONG  = 40;

  const int EVEN_DISP = (LEDNUM_LONG - LEDNUM_SHORT) * 4;
  const int ODD_DISP = ((LEDNUM_LONG - LEDNUM_SHORT) * 4) + LEDNUM_SHORT;
  const int SHORTFALL = (LEDNUM_LONG - LEDNUM_SHORT) * 11;

  int s;
 
  if (x >=19) { return -1;};
  if ((x>3) && (x<15)) { // handle short strips
    if (y >= LEDNUM_SHORT) { s = -1;}  //// return -1 if out of bounds
    else {
      if (x % 2 == 0) {   // handle even
        s = (LEDNUM_SHORT * x) - y + EVEN_DISP; // even
      }
      else{
        s = (LEDNUM_SHORT * x) + ODD_DISP - y; //odd
      }
    }
  }
  else {  // handle long strips
    if (y >= LEDNUM_LONG) {s = -1;}  // return -1 if out of bounds
    else {
      if (x % 2 == 0) {  // even
        s = ((x+1) * LEDNUM_LONG) - y;
      }
      else {             //odd
        s = (x * LEDNUM_LONG) + y;
      };
      // compensate for short strips
      if (x > 14) {
        s -= SHORTFALL;   // i.e. those neopixels that are missing
      }
    }
  }
  return s;


}
