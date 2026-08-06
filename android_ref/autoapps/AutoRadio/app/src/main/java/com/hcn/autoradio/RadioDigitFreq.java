package com.hcn.autoradio;

public class RadioDigitFreq {

    public static int DigitFreqImg(char ch) {

        int R_ID = R.drawable.radio_num_point;

        switch (ch) {
            case '0':
                R_ID = R.drawable.radio_num_0;
                break;
            case '1':
                R_ID = R.drawable.radio_num_1;
                break;
            case '2':
                R_ID = R.drawable.radio_num_2;
                break;
            case '3':
                R_ID = R.drawable.radio_num_3;
                break;
            case '4':
                R_ID = R.drawable.radio_num_4;
                break;
            case '5':
                R_ID = R.drawable.radio_num_5;
                break;
            case '6':
                R_ID = R.drawable.radio_num_6;
                break;
            case '7':
                R_ID = R.drawable.radio_num_7;
                break;
            case '8':
                R_ID = R.drawable.radio_num_8;
                break;
            case '9':
                R_ID = R.drawable.radio_num_9;
                break;
            case '.':
                R_ID = R.drawable.radio_num_point;
                break;
            default:
                break;
        }

        return R_ID;
    }
}
