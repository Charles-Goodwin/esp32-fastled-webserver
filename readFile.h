#include <SPIFFS.h>
 
    

// Functions to read a CSV text file one field at a time.
//
//#include <limits.h>
//#include <SPI.h>

// next line for SD.h
//#include <SD.h>

// next two lines for SdFat
//#include <SdFat.h>
//SdFat SD;

//#define CS_PIN SS

#define INT8_MIN 0
#define INT8_MAX 255



/*
 * Read a file one field at a time.
 *
 * file - File to read.
 *
 * str - Character array for the field.
 *
 * size - Size of str array.
 *
 * delim - csv delimiter.
 *
 * return - negative value for failure.
 *          delimiter, '\n' or zero(EOF) for success.           
 */
int csvReadText(File* file, char* str, size_t size, char delim) {
  char ch;
  int rtn;
  size_t n = 0;
  while (true) {
    // check for EOF
    if (!file->available()) {
      rtn = -1;
      break;
    }
    ch = file->read();
    
    // Delete CR.
    if (ch == '\r') {
      continue;
    }
    if (ch == delim || ch == '\n') {
      rtn = ch;
      break;
    }
    if ((n+1) >= size) {
      // string too long
      rtn = -2;
      n--;
      break;
    }
    str[n++] = ch;
  }
  str[n] = '\0';
  return rtn;
}
//------------------------------------------------------------------------------
int csvReadInt32(File* file, int32_t* num, char delim) {
  char buf[20];
  char* ptr;
  int rtn = csvReadText(file, buf, sizeof(buf), delim);
  if (rtn < 0) return rtn;
  *num = strtol(buf, &ptr, 10);
  if (buf == ptr) return -3;
  while(isspace(*ptr)) ptr++;
  return *ptr == 0 ? rtn : -4;
}
//------------------------------------------------------------------------------
int csvReadInt16(File* file, int16_t* num, char delim) {
  int32_t tmp;
  int rtn = csvReadInt32(file, &tmp, delim);
  if (rtn < 0) return rtn;
  if (tmp < INT_MIN || tmp > INT_MAX) return -5;
  *num = tmp;
  return rtn;
}
//------------------------------------------------------------------------------
int csvReadInt8(File* file, uint8_t* num, char delim) {
  int32_t tmp;
  int rtn = csvReadInt32(file, &tmp, delim);
  if (rtn < 0) return rtn;
  if (tmp < INT8_MIN || tmp > INT8_MAX) return -5;
  *num = tmp;
  return rtn;
}
//------------------------------------------------------------------------------
int csvReadUint32(File* file, uint32_t* num, char delim) {
  char buf[20];
  char* ptr;
  int rtn = csvReadText(file, buf, sizeof(buf), delim);
  if (rtn < 0) return rtn;
  *num = strtoul(buf, &ptr, 10);
  if (buf == ptr) return -3;
  while(isspace(*ptr)) ptr++;
  return *ptr == 0 ? rtn : -4;
}
//------------------------------------------------------------------------------
int csvReadUint16(File* file, uint16_t* num, char delim) {
  uint32_t tmp;
  int rtn = csvReadUint32(file, &tmp, delim);
  if (rtn < 0) return rtn;
  if (tmp > UINT_MAX) return -5;
  *num = tmp;
  return rtn;
}
//------------------------------------------------------------------------------
int csvReadDouble(File* file, double* num, char delim) {
  char buf[20];
  char* ptr;
  int rtn = csvReadText(file, buf, sizeof(buf), delim);
  if (rtn < 0) return rtn;
  *num = strtod(buf, &ptr);
  if (buf == ptr) return -3;
  while(isspace(*ptr)) ptr++;
  return *ptr == 0 ? rtn : -4;
}
//------------------------------------------------------------------------------
int csvReadFloat(File* file, float* num, char delim) {
  double tmp;
  int rtn = csvReadDouble(file, &tmp, delim);
  if (rtn < 0)return rtn;
  // could test for too large.
  *num = tmp;
  return rtn;
}

//------------------------------------------------------------------------------
int FetchColorsFromFile(char fileName[], uint8_t* strip) {
 
  Serial.begin(115200);
 
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return -1;
  }
  
  File file = SPIFFS.open(fileName);
  if(!file){
    Serial.println("Failed to open file for reading");
    return -1;
  }

  char delimiter;
  for ( int i = 0; i < NUM_LEDS; i++) {
    if (file.available()) {
      delimiter = csvReadInt8(&file, &strip[i], ',');
      if (delimiter != ',' || delimiter != '\n') {
        Serial.println("read error");
        int ch;
        int nr = 0;
        // print part of file after error.
        while ((ch = file.read()) > 0 && nr++ < 100) {
          Serial.write(ch);
        }
        return -2;
      }          
    }
  }
  file.close();
}
