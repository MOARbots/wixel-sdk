#ifndef FAKE_DATA_H
/** Make faked data
 */
int fi = 0;
int num_id = 23;

XDATA int16 fdata[] = {
  4, 100, 100,  0,
  2,  5,  5, 10,
  2,  5,  5, 20,
  2,  5,  5, 35,
  2,  5,  5, 45,
  2,  6,  6, 45,
  2,  7,  7, 45,
  2,  9,  9, 45,
  4, 100, 100,  0,
  2,  9,  9, 55,
  2,  9,  9, 45,
  2,  20, 20, 45,
  2,  40, 40, 45,
  2,  50, 50, 35,
  2,  50, 50, 40,
  2,  50, 50, 43,
  2,  60, 60, 43,
  2,  70, 70, 63,
  2,  70, 70, 53,
  2,  70, 70, 47,
  2,  90, 90, 47,
  2,  98, 99, 47,
  2,  99, 99, 45,
};
XDATA int16 fdata2[] = {
  4, 30, 30,  0,
  3, 50, 30,  0,
  2,  5,  5, 10,
  2,  5,  5, 20,
  2,  5,  5, 35,
  4, 30, 30,  0,
  3, 50, 30,  0,
  2,  6,  6, 45,
  2,  9,  9, 45,
  4, 30, 30,  0,
  3, 50, 30,  0,
  4, 30, 30,  0,
  2, 10, 10, 45,
  4, 30, 30,  0,
  2,  8,  8, 45,
  2,  20,  20, 45,
  2,  30,  30, 45,
  2,  30,  30, 60,
  2,  30,  30, 80,
  2,  30,  30, 90,
  2,  35,  30, 90,
  2,  45,  30, 90,
  2,  50,  30, 90,
};

int ReadFakeData(int16* id, int16* x, int16* y, int16* r) {
  // return if not typein
  if (!g_cbk_f || fi>num_id) {
    *id=0; *x=0; *y=0; *r=0;
    return 0;
  }
  *id = fdata[fi*4+0];
  *x  = fdata[fi*4+1];
  *y  = fdata[fi*4+2];
  *r  = fdata[fi*4+3];

  SendBackStr3("GetFake IdXY", fi, *x, *y);

  fi++;
  g_cbk_f = 0;
  return 1;
}

void ReadFakeData2(int16* id, int16* x, int16* y, int16* r) {
  *id = g_my_id;
  *x = rand()%640;
  *y = rand()%480;
  *r = rand()%360;
}

#endif
