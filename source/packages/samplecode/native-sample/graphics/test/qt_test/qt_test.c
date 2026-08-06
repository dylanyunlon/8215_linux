#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <AtcDisplaySettings.h>

int main(int argc, char **argv)
{

    int rotate = 0;

    unsigned char rotate_value = 0;
    if(GetRotateValue(&rotate_value) == 0) {
        printf("Get rotate value success:%d \n",(int)rotate_value);
        switch((int)rotate_value)
        {
          case 0:
              rotate = 0;
              break;
          case 1:
              rotate = -90;
              break;
          case 2:
              rotate = 180;
              break;
          case 3:
              rotate = 90;
              break;
          default:
              rotate = 0;
              break;
        }
    } else {
        printf("Get rotate value failed, use defaulted value: %d \n", rotate);
    }

    char rotate_str[16];
    snprintf(rotate_str, sizeof(rotate_str), "%d", rotate);
    setenv("QT_QPA_EGLFS_ROTATION", rotate_str, 1);

    system("/usr/bin/basicdrawing");

    return 0;
}
