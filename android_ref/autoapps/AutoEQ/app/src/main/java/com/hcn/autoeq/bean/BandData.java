package com.hcn.autoeq.bean;


import com.hcn.autoeq.util.ConstantFyDsp;

import java.util.ArrayList;
import java.util.List;

public class BandData implements ConstantFyDsp {

    public static List<Band> initCustomBands() {
        List<Band> bands = new ArrayList<>();
        bands.add(new Band(0, 0, DEF_Q_VALUE, 20, 0, 0));
        bands.add(new Band(1, 0, DEF_Q_VALUE, 25, 0, 0));
        bands.add(new Band(2, 0, DEF_Q_VALUE, 32, 0, 0));
        bands.add(new Band(3, 0, DEF_Q_VALUE, 40, 0, 0));
        bands.add(new Band(4, 0, DEF_Q_VALUE, 50, 0, 0));
        bands.add(new Band(5, 0, DEF_Q_VALUE, 63, 0, 0));
        bands.add(new Band(6, 0, DEF_Q_VALUE, 80, 0, 0));
        bands.add(new Band(7, 0, DEF_Q_VALUE, 100, 0, 0));

        bands.add(new Band(8, 0, DEF_Q_VALUE, 125, 0, 0));
        bands.add(new Band(9, 0, DEF_Q_VALUE, 160, 0, 0));
        bands.add(new Band(10, 0, DEF_Q_VALUE, 200, 0, 0));
        bands.add(new Band(11, 0, DEF_Q_VALUE, 250, 0, 0));
        bands.add(new Band(12, 0, DEF_Q_VALUE, 315, 0, 0));
        bands.add(new Band(13, 0, DEF_Q_VALUE, 400, 0, 0));
        bands.add(new Band(14, 0, DEF_Q_VALUE, 500, 0, 0));
        bands.add(new Band(15, 0, DEF_Q_VALUE, 630, 0, 0));

        bands.add(new Band(16, 0, DEF_Q_VALUE, 800, 0, 0));
        bands.add(new Band(17, 0, DEF_Q_VALUE, 1000, 0, 0));
        bands.add(new Band(18, 0, DEF_Q_VALUE, 1250, 0, 0));
        bands.add(new Band(19, 0, DEF_Q_VALUE, 1600, 0, 0));
        bands.add(new Band(20, 0, DEF_Q_VALUE, 2000, 0, 0));
        bands.add(new Band(21, 0, DEF_Q_VALUE, 2500, 0, 0));
        bands.add(new Band(22, 0, DEF_Q_VALUE, 3150, 0, 0));
        bands.add(new Band(23, 0, DEF_Q_VALUE, 4000, 0, 0));

        bands.add(new Band(24, 0, DEF_Q_VALUE, 5000, 0, 0));
        bands.add(new Band(25, 0, DEF_Q_VALUE, 6300, 0, 0));
        bands.add(new Band(26, 0, DEF_Q_VALUE, 8000, 0, 0));
        bands.add(new Band(27, 0, DEF_Q_VALUE, 10000, 0, 0));
        bands.add(new Band(28, 0, DEF_Q_VALUE, 12500, 0, 0));
        bands.add(new Band(29, 0, DEF_Q_VALUE, 16000, 0, 0));
        bands.add(new Band(30, 0, DEF_Q_VALUE, 18000, 0, 0));
        bands.add(new Band(31, 0, DEF_Q_VALUE, 20000, 0, 0));
        return bands;
    }

    public static List<Band> initStandardBands() {
        List<Band> bands = new ArrayList<>();
        bands.add(new Band(0, 0, DEF_Q_VALUE, 20, 0, 0));
        bands.add(new Band(1, 0, DEF_Q_VALUE, 25, 0, 0));
        bands.add(new Band(2, 0, DEF_Q_VALUE, 32, 0, 0));
        bands.add(new Band(3, 0, DEF_Q_VALUE, 40, 0, 0));
        bands.add(new Band(4, 0, DEF_Q_VALUE, 50, 0, 0));
        bands.add(new Band(5, 0, DEF_Q_VALUE, 63, 0, 0));
        bands.add(new Band(6, 0, DEF_Q_VALUE, 80, 0, 0));
        bands.add(new Band(7, 0, DEF_Q_VALUE, 100, 0, 0));

        bands.add(new Band(8, 0, DEF_Q_VALUE, 125, 0, 0));
        bands.add(new Band(9, 0, DEF_Q_VALUE, 160, 0, 0));
        bands.add(new Band(10, 0, DEF_Q_VALUE, 200, 0, 0));
        bands.add(new Band(11, 0, DEF_Q_VALUE, 250, 0, 0));
        bands.add(new Band(12, 0, DEF_Q_VALUE, 315, 0, 0));
        bands.add(new Band(13, 0, DEF_Q_VALUE, 400, 0, 0));
        bands.add(new Band(14, 0, DEF_Q_VALUE, 500, 0, 0));
        bands.add(new Band(15, 0, DEF_Q_VALUE, 630, 0, 0));

        bands.add(new Band(16, 0, DEF_Q_VALUE, 800, 0, 0));
        bands.add(new Band(17, 0, DEF_Q_VALUE, 1000, 0, 0));
        bands.add(new Band(18, 0, DEF_Q_VALUE, 1250, 0, 0));
        bands.add(new Band(19, 0, DEF_Q_VALUE, 1600, 0, 0));
        bands.add(new Band(20, 0, DEF_Q_VALUE, 2000, 0, 0));
        bands.add(new Band(21, 0, DEF_Q_VALUE, 2500, 0, 0));
        bands.add(new Band(22, 0, DEF_Q_VALUE, 3150, 0, 0));
        bands.add(new Band(23, 0, DEF_Q_VALUE, 4000, 0, 0));

        bands.add(new Band(24, 0, DEF_Q_VALUE, 5000, 0, 0));
        bands.add(new Band(25, 0, DEF_Q_VALUE, 6300, 0, 0));
        bands.add(new Band(26, 0, DEF_Q_VALUE, 8000, 0, 0));
        bands.add(new Band(27, 0, DEF_Q_VALUE, 10000, 0, 0));
        bands.add(new Band(28, 0, DEF_Q_VALUE, 12500, 0, 0));
        bands.add(new Band(29, 0, DEF_Q_VALUE, 16000, 0, 0));
        bands.add(new Band(30, 0, DEF_Q_VALUE, 18000, 0, 0));
        bands.add(new Band(31, 0, DEF_Q_VALUE, 20000, 0, 0));
        return bands;
    }

    public static List<Band> initNewsBands() {
        List<Band> bands = new ArrayList<>();
        bands.add(new Band(0, -2, DEF_Q_VALUE, 20, 0, 0));
        bands.add(new Band(1, 0, DEF_Q_VALUE, 25, 0, 0));
        bands.add(new Band(2, 2, DEF_Q_VALUE, 32, 0, 0));
        bands.add(new Band(3, 0, DEF_Q_VALUE, 40, 0, 0));
        bands.add(new Band(4, 2, DEF_Q_VALUE, 50, 0, 0));
        bands.add(new Band(5, 4, DEF_Q_VALUE, 63, 0, 0));
        bands.add(new Band(6, 6, DEF_Q_VALUE, 80, 0, 0));
        bands.add(new Band(7, 8, DEF_Q_VALUE, 100, 0, 0));

        bands.add(new Band(8, 6, DEF_Q_VALUE, 125, 0, 0));
        bands.add(new Band(9, 4, DEF_Q_VALUE, 160, 0, 0));
        bands.add(new Band(10, 6, DEF_Q_VALUE, 200, 0, 0));
        bands.add(new Band(11, 8, DEF_Q_VALUE, 250, 0, 0));
        bands.add(new Band(12, 6, DEF_Q_VALUE, 315, 0, 0));
        bands.add(new Band(13, 6, DEF_Q_VALUE, 400, 0, 0));
        bands.add(new Band(14, 4, DEF_Q_VALUE, 500, 0, 0));
        bands.add(new Band(15, 6, DEF_Q_VALUE, 630, 0, 0));

        bands.add(new Band(16, 2, DEF_Q_VALUE, 800, 0, 0));
        bands.add(new Band(17, 0, DEF_Q_VALUE, 1000, 0, 0));
        bands.add(new Band(18, 0, DEF_Q_VALUE, 1250, 0, 0));
        bands.add(new Band(19, 0, DEF_Q_VALUE, 1600, 0, 0));
        bands.add(new Band(20, 2, DEF_Q_VALUE, 2000, 0, 0));
        bands.add(new Band(21, 0, DEF_Q_VALUE, 2500, 0, 0));
        bands.add(new Band(22, 0, DEF_Q_VALUE, 3150, 0, 0));
        bands.add(new Band(23, 0, DEF_Q_VALUE, 4000, 0, 0));

        bands.add(new Band(24, 2, DEF_Q_VALUE, 5000, 0, 0));
        bands.add(new Band(25, 0, DEF_Q_VALUE, 6300, 0, 0));
        bands.add(new Band(26, 0, DEF_Q_VALUE, 8000, 0, 0));
        bands.add(new Band(27, 2, DEF_Q_VALUE, 10000, 0, 0));
        bands.add(new Band(28, 2, DEF_Q_VALUE, 12500, 0, 0));
        bands.add(new Band(29, 2, DEF_Q_VALUE, 16000, 0, 0));
        bands.add(new Band(30, 0, DEF_Q_VALUE, 18000, 0, 0));
        bands.add(new Band(31, 2, DEF_Q_VALUE, 20000, 0, 0));
        return bands;
    }

    public static List<Band> initJazzBands() {
        List<Band> bands = new ArrayList<>();
        bands.add(new Band(0, 4, DEF_Q_VALUE, 20, 0, 0));
        bands.add(new Band(1, 6, DEF_Q_VALUE, 25, 0, 0));
        bands.add(new Band(2, 2, DEF_Q_VALUE, 32, 0, 0));
        bands.add(new Band(3, 2, DEF_Q_VALUE, 40, 0, 0));
        bands.add(new Band(4, 0, DEF_Q_VALUE, 50, 0, 0));
        bands.add(new Band(5, -4, DEF_Q_VALUE, 63, 0, 0));
        bands.add(new Band(6, -2, DEF_Q_VALUE, 80, 0, 0));
        bands.add(new Band(7, -4, DEF_Q_VALUE, 100, 0, 0));

        bands.add(new Band(8, -4, DEF_Q_VALUE, 125, 0, 0));
        bands.add(new Band(9, -2, DEF_Q_VALUE, 160, 0, 0));
        bands.add(new Band(10, -4, DEF_Q_VALUE, 200, 0, 0));
        bands.add(new Band(11, -2, DEF_Q_VALUE, 250, 0, 0));
        bands.add(new Band(12, -4, DEF_Q_VALUE, 315, 0, 0));
        bands.add(new Band(13, -4, DEF_Q_VALUE, 400, 0, 0));
        bands.add(new Band(14, -8, DEF_Q_VALUE, 500, 0, 0));
        bands.add(new Band(15, -10, DEF_Q_VALUE, 630, 0, 0));

        bands.add(new Band(16, -6, DEF_Q_VALUE, 800, 0, 0));
        bands.add(new Band(17, 0, DEF_Q_VALUE, 1000, 0, 0));
        bands.add(new Band(18, 0, DEF_Q_VALUE, 1250, 0, 0));
        bands.add(new Band(19, -6, DEF_Q_VALUE, 1600, 0, 0));
        bands.add(new Band(20, -4, DEF_Q_VALUE, 2000, 0, 0));
        bands.add(new Band(21, -6, DEF_Q_VALUE, 2500, 0, 0));
        bands.add(new Band(22, -4, DEF_Q_VALUE, 3150, 0, 0));
        bands.add(new Band(23, -6, DEF_Q_VALUE, 4000, 0, 0));

        bands.add(new Band(24, 0, DEF_Q_VALUE, 5000, 0, 0));
        bands.add(new Band(25, 2, DEF_Q_VALUE, 6300, 0, 0));
        bands.add(new Band(26, 0, DEF_Q_VALUE, 8000, 0, 0));
        bands.add(new Band(27, 0, DEF_Q_VALUE, 10000, 0, 0));
        bands.add(new Band(28, 2, DEF_Q_VALUE, 12500, 0, 0));
        bands.add(new Band(29, 2, DEF_Q_VALUE, 16000, 0, 0));
        bands.add(new Band(30, 2, DEF_Q_VALUE, 18000, 0, 0));
        bands.add(new Band(31, 0, DEF_Q_VALUE, 20000, 0, 0));
        return bands;
    }

    public static List<Band> initCityBands() {
        List<Band> bands = new ArrayList<>();
        bands.add(new Band(0, -4, DEF_Q_VALUE, 20, 0, 0));
        bands.add(new Band(1, -4, DEF_Q_VALUE, 25, 0, 0));
        bands.add(new Band(2, -2, DEF_Q_VALUE, 32, 0, 0));
        bands.add(new Band(3, 0, DEF_Q_VALUE, 40, 0, 0));
        bands.add(new Band(4, 2, DEF_Q_VALUE, 50, 0, 0));
        bands.add(new Band(5, 4, DEF_Q_VALUE, 63, 0, 0));
        bands.add(new Band(6, 4, DEF_Q_VALUE, 80, 0, 0));
        bands.add(new Band(7, 2, DEF_Q_VALUE, 100, 0, 0));

        bands.add(new Band(8, 4, DEF_Q_VALUE, 125, 0, 0));
        bands.add(new Band(9, 6, DEF_Q_VALUE, 160, 0, 0));
        bands.add(new Band(10, 4, DEF_Q_VALUE, 200, 0, 0));
        bands.add(new Band(11, 6, DEF_Q_VALUE, 250, 0, 0));
        bands.add(new Band(12, 6, DEF_Q_VALUE, 315, 0, 0));
        bands.add(new Band(13, 8, DEF_Q_VALUE, 400, 0, 0));
        bands.add(new Band(14, 6, DEF_Q_VALUE, 500, 0, 0));
        bands.add(new Band(15, 4, DEF_Q_VALUE, 630, 0, 0));

        bands.add(new Band(16, 2, DEF_Q_VALUE, 800, 0, 0));
        bands.add(new Band(17, 2, DEF_Q_VALUE, 1000, 0, 0));
        bands.add(new Band(18, 2, DEF_Q_VALUE, 1250, 0, 0));
        bands.add(new Band(19, 4, DEF_Q_VALUE, 1600, 0, 0));
        bands.add(new Band(20, 6, DEF_Q_VALUE, 2000, 0, 0));
        bands.add(new Band(21, 2, DEF_Q_VALUE, 2500, 0, 0));
        bands.add(new Band(22, 0, DEF_Q_VALUE, 3150, 0, 0));
        bands.add(new Band(23, 2, DEF_Q_VALUE, 4000, 0, 0));

        bands.add(new Band(24, 2, DEF_Q_VALUE, 5000, 0, 0));
        bands.add(new Band(25, 0, DEF_Q_VALUE, 6300, 0, 0));
        bands.add(new Band(26, 0, DEF_Q_VALUE, 8000, 0, 0));
        bands.add(new Band(27, 0, DEF_Q_VALUE, 10000, 0, 0));
        bands.add(new Band(28, 0, DEF_Q_VALUE, 12500, 0, 0));
        bands.add(new Band(29, 2, DEF_Q_VALUE, 16000, 0, 0));
        bands.add(new Band(30, 0, DEF_Q_VALUE, 18000, 0, 0));
        bands.add(new Band(31, 2, DEF_Q_VALUE, 20000, 0, 0));
        return bands;
    }

    public static List<Band> initPopBands() {
        List<Band> bands = new ArrayList<>();
        bands.add(new Band(0, -2, DEF_Q_VALUE, 20, 0, 0));
        bands.add(new Band(1, 0, DEF_Q_VALUE, 25, 0, 0));
        bands.add(new Band(2, 2, DEF_Q_VALUE, 32, 0, 0));
        bands.add(new Band(3, 4, DEF_Q_VALUE, 40, 0, 0));
        bands.add(new Band(4, 6, DEF_Q_VALUE, 50, 0, 0));
        bands.add(new Band(5, 8, DEF_Q_VALUE, 63, 0, 0));
        bands.add(new Band(6, 6, DEF_Q_VALUE, 80, 0, 0));
        bands.add(new Band(7, 8, DEF_Q_VALUE, 100, 0, 0));

        bands.add(new Band(8, 8, DEF_Q_VALUE, 125, 0, 0));
        bands.add(new Band(9, 8, DEF_Q_VALUE, 160, 0, 0));
        bands.add(new Band(10, 6, DEF_Q_VALUE, 200, 0, 0));
        bands.add(new Band(11, 6, DEF_Q_VALUE, 250, 0, 0));
        bands.add(new Band(12, 6, DEF_Q_VALUE, 315, 0, 0));
        bands.add(new Band(13, 4, DEF_Q_VALUE, 400, 0, 0));
        bands.add(new Band(14, 0, DEF_Q_VALUE, 500, 0, 0));
        bands.add(new Band(15, -4, DEF_Q_VALUE, 630, 0, 0));

        bands.add(new Band(16, -4, DEF_Q_VALUE, 800, 0, 0));
        bands.add(new Band(17, -2, DEF_Q_VALUE, 1000, 0, 0));
        bands.add(new Band(18, -2, DEF_Q_VALUE, 1250, 0, 0));
        bands.add(new Band(19, -4, DEF_Q_VALUE, 1600, 0, 0));
        bands.add(new Band(20, -2, DEF_Q_VALUE, 2000, 0, 0));
        bands.add(new Band(21, -4, DEF_Q_VALUE, 2500, 0, 0));
        bands.add(new Band(22, -2, DEF_Q_VALUE, 3150, 0, 0));
        bands.add(new Band(23, -4, DEF_Q_VALUE, 4000, 0, 0));

        bands.add(new Band(24, -2, DEF_Q_VALUE, 5000, 0, 0));
        bands.add(new Band(25, 0, DEF_Q_VALUE, 6300, 0, 0));
        bands.add(new Band(26, 0, DEF_Q_VALUE, 8000, 0, 0));
        bands.add(new Band(27, -2, DEF_Q_VALUE, 10000, 0, 0));
        bands.add(new Band(28, -2, DEF_Q_VALUE, 12500, 0, 0));
        bands.add(new Band(29, 0, DEF_Q_VALUE, 16000, 0, 0));
        bands.add(new Band(30, 0, DEF_Q_VALUE, 18000, 0, 0));
        bands.add(new Band(31, 0, DEF_Q_VALUE, 20000, 0, 0));
        return bands;
    }

    public static List<Band> initElectronicBands() {
        List<Band> bands = new ArrayList<>();
        bands.add(new Band(0, 8, DEF_Q_VALUE, 20, 0, 0));
        bands.add(new Band(1, 6, DEF_Q_VALUE, 25, 0, 0));
        bands.add(new Band(2, 6, DEF_Q_VALUE, 32, 0, 0));
        bands.add(new Band(3, 6, DEF_Q_VALUE, 40, 0, 0));
        bands.add(new Band(4, 4, DEF_Q_VALUE, 50, 0, 0));
        bands.add(new Band(5, 0, DEF_Q_VALUE, 63, 0, 0));
        bands.add(new Band(6, -2, DEF_Q_VALUE, 80, 0, 0));
        bands.add(new Band(7, 0, DEF_Q_VALUE, 100, 0, 0));

        bands.add(new Band(8, -2, DEF_Q_VALUE, 125, 0, 0));
        bands.add(new Band(9, -6, DEF_Q_VALUE, 160, 0, 0));
        bands.add(new Band(10, -4, DEF_Q_VALUE, 200, 0, 0));
        bands.add(new Band(11, -6, DEF_Q_VALUE, 250, 0, 0));
        bands.add(new Band(12, -4, DEF_Q_VALUE, 315, 0, 0));
        bands.add(new Band(13, -2, DEF_Q_VALUE, 400, 0, 0));
        bands.add(new Band(14, 0, DEF_Q_VALUE, 500, 0, 0));
        bands.add(new Band(15, 8, DEF_Q_VALUE, 630, 0, 0));

        bands.add(new Band(16, 10, DEF_Q_VALUE, 800, 0, 0));
        bands.add(new Band(17, 10, DEF_Q_VALUE, 1000, 0, 0));
        bands.add(new Band(18, 8, DEF_Q_VALUE, 1250, 0, 0));
        bands.add(new Band(19, 8, DEF_Q_VALUE, 1600, 0, 0));
        bands.add(new Band(20, 8, DEF_Q_VALUE, 2000, 0, 0));
        bands.add(new Band(21, 10, DEF_Q_VALUE, 2500, 0, 0));
        bands.add(new Band(22, 8, DEF_Q_VALUE, 3150, 0, 0));
        bands.add(new Band(23, 10, DEF_Q_VALUE, 4000, 0, 0));

        bands.add(new Band(24, 10, DEF_Q_VALUE, 5000, 0, 0));
        bands.add(new Band(25, 10, DEF_Q_VALUE, 6300, 0, 0));
        bands.add(new Band(26, 10, DEF_Q_VALUE, 8000, 0, 0));
        bands.add(new Band(27, 8, DEF_Q_VALUE, 10000, 0, 0));
        bands.add(new Band(28, 8, DEF_Q_VALUE, 12500, 0, 0));
        bands.add(new Band(29, 8, DEF_Q_VALUE, 16000, 0, 0));
        bands.add(new Band(30, 8, DEF_Q_VALUE, 18000, 0, 0));
        bands.add(new Band(31, 10, DEF_Q_VALUE, 20000, 0, 0));
        return bands;
    }

    public static List<Band> initClassicsBands() {
        List<Band> bands = new ArrayList<>();
        bands.add(new Band(0, 0, DEF_Q_VALUE, 20, 0, 0));
        bands.add(new Band(1, 0, DEF_Q_VALUE, 25, 0, 0));
        bands.add(new Band(2, -2, DEF_Q_VALUE, 32, 0, 0));
        bands.add(new Band(3, -2, DEF_Q_VALUE, 40, 0, 0));
        bands.add(new Band(4, 0, DEF_Q_VALUE, 50, 0, 0));
        bands.add(new Band(5, 0, DEF_Q_VALUE, 63, 0, 0));
        bands.add(new Band(6, -2, DEF_Q_VALUE, 80, 0, 0));
        bands.add(new Band(7, -2, DEF_Q_VALUE, 100, 0, 0));

        bands.add(new Band(8, -2, DEF_Q_VALUE, 125, 0, 0));
        bands.add(new Band(9, 0, DEF_Q_VALUE, 160, 0, 0));
        bands.add(new Band(10, 0, DEF_Q_VALUE, 200, 0, 0));
        bands.add(new Band(11, 0, DEF_Q_VALUE, 250, 0, 0));
        bands.add(new Band(12, 0, DEF_Q_VALUE, 315, 0, 0));
        bands.add(new Band(13, 0, DEF_Q_VALUE, 400, 0, 0));
        bands.add(new Band(14, 0, DEF_Q_VALUE, 500, 0, 0));
        bands.add(new Band(15, -10, DEF_Q_VALUE, 630, 0, 0));

        bands.add(new Band(16, -8, DEF_Q_VALUE, 800, 0, 0));
        bands.add(new Band(17, -8, DEF_Q_VALUE, 1000, 0, 0));
        bands.add(new Band(18, -10, DEF_Q_VALUE, 1250, 0, 0));
        bands.add(new Band(19, -12, DEF_Q_VALUE, 1600, 0, 0));
        bands.add(new Band(20, -12, DEF_Q_VALUE, 2000, 0, 0));
        bands.add(new Band(21, -8, DEF_Q_VALUE, 2500, 0, 0));
        bands.add(new Band(22, -6, DEF_Q_VALUE, 3150, 0, 0));
        bands.add(new Band(23, -6, DEF_Q_VALUE, 4000, 0, 0));

        bands.add(new Band(24, -8, DEF_Q_VALUE, 5000, 0, 0));
        bands.add(new Band(25, -8, DEF_Q_VALUE, 6300, 0, 0));
        bands.add(new Band(26, -8, DEF_Q_VALUE, 8000, 0, 0));
        bands.add(new Band(27, -8, DEF_Q_VALUE, 10000, 0, 0));
        bands.add(new Band(28, -10, DEF_Q_VALUE, 12500, 0, 0));
        bands.add(new Band(29, -10, DEF_Q_VALUE, 16000, 0, 0));
        bands.add(new Band(30, -10, DEF_Q_VALUE, 18000, 0, 0));
        bands.add(new Band(31, -10, DEF_Q_VALUE, 20000, 0, 0));
        return bands;
    }

    public static List<Band> initMovieBands() {
        List<Band> bands = new ArrayList<>();
        bands.add(new Band(0, 10, DEF_Q_VALUE, 20, 0, 0));
        bands.add(new Band(1, 6, DEF_Q_VALUE, 25, 0, 0));
        bands.add(new Band(2, 8, DEF_Q_VALUE, 32, 0, 0));
        bands.add(new Band(3, 10, DEF_Q_VALUE, 40, 0, 0));
        bands.add(new Band(4, 8, DEF_Q_VALUE, 50, 0, 0));
        bands.add(new Band(5, 6, DEF_Q_VALUE, 63, 0, 0));
        bands.add(new Band(6, 4, DEF_Q_VALUE, 80, 0, 0));
        bands.add(new Band(7, 6, DEF_Q_VALUE, 100, 0, 0));

        bands.add(new Band(8, 4, DEF_Q_VALUE, 125, 0, 0));
        bands.add(new Band(9, 6, DEF_Q_VALUE, 160, 0, 0));
        bands.add(new Band(10, 4, DEF_Q_VALUE, 200, 0, 0));
        bands.add(new Band(11, 0, DEF_Q_VALUE, 250, 0, 0));
        bands.add(new Band(12, 2, DEF_Q_VALUE, 315, 0, 0));
        bands.add(new Band(13, -6, DEF_Q_VALUE, 400, 0, 0));
        bands.add(new Band(14, -4, DEF_Q_VALUE, 500, 0, 0));
        bands.add(new Band(15, -4, DEF_Q_VALUE, 630, 0, 0));

        bands.add(new Band(16, 0, DEF_Q_VALUE, 800, 0, 0));
        bands.add(new Band(17, 0, DEF_Q_VALUE, 1000, 0, 0));
        bands.add(new Band(18, -4, DEF_Q_VALUE, 1250, 0, 0));
        bands.add(new Band(19, -4, DEF_Q_VALUE, 1600, 0, 0));
        bands.add(new Band(20, -4, DEF_Q_VALUE, 2000, 0, 0));
        bands.add(new Band(21, -2, DEF_Q_VALUE, 2500, 0, 0));
        bands.add(new Band(22, -4, DEF_Q_VALUE, 3150, 0, 0));
        bands.add(new Band(23, 0, DEF_Q_VALUE, 4000, 0, 0));

        bands.add(new Band(24, 2, DEF_Q_VALUE, 5000, 0, 0));
        bands.add(new Band(25, 0, DEF_Q_VALUE, 6300, 0, 0));
        bands.add(new Band(26, 2, DEF_Q_VALUE, 8000, 0, 0));
        bands.add(new Band(27, 0, DEF_Q_VALUE, 10000, 0, 0));
        bands.add(new Band(28, 2, DEF_Q_VALUE, 12500, 0, 0));
        bands.add(new Band(29, 0, DEF_Q_VALUE, 16000, 0, 0));
        bands.add(new Band(30, 2, DEF_Q_VALUE, 18000, 0, 0));
        bands.add(new Band(31, 2, DEF_Q_VALUE, 20000, 0, 0));
        return bands;
    }

    public static List<Band> initRockBands() {
        List<Band> bands = new ArrayList<>();
        bands.add(new Band(0, 4, DEF_Q_VALUE, 20, 0, 0));
        bands.add(new Band(1, 2, DEF_Q_VALUE, 25, 0, 0));
        bands.add(new Band(2, 4, DEF_Q_VALUE, 32, 0, 0));
        bands.add(new Band(3, 0, DEF_Q_VALUE, 40, 0, 0));
        bands.add(new Band(4, 2, DEF_Q_VALUE, 50, 0, 0));
        bands.add(new Band(5, -8, DEF_Q_VALUE, 63, 0, 0));
        bands.add(new Band(6, -6, DEF_Q_VALUE, 80, 0, 0));
        bands.add(new Band(7, -8, DEF_Q_VALUE, 100, 0, 0));

        bands.add(new Band(8, -4, DEF_Q_VALUE, 125, 0, 0));
        bands.add(new Band(9, -8, DEF_Q_VALUE, 160, 0, 0));
        bands.add(new Band(10, -4, DEF_Q_VALUE, 200, 0, 0));
        bands.add(new Band(11, -6, DEF_Q_VALUE, 250, 0, 0));
        bands.add(new Band(12, -4, DEF_Q_VALUE, 315, 0, 0));
        bands.add(new Band(13, 0, DEF_Q_VALUE, 400, 0, 0));
        bands.add(new Band(14, 4, DEF_Q_VALUE, 500, 0, 0));
        bands.add(new Band(15, 6, DEF_Q_VALUE, 630, 0, 0));

        bands.add(new Band(16, 4, DEF_Q_VALUE, 800, 0, 0));
        bands.add(new Band(17, 2, DEF_Q_VALUE, 1000, 0, 0));
        bands.add(new Band(18, 8, DEF_Q_VALUE, 1250, 0, 0));
        bands.add(new Band(19, 8, DEF_Q_VALUE, 1600, 0, 0));
        bands.add(new Band(20, 6, DEF_Q_VALUE, 2000, 0, 0));
        bands.add(new Band(21, 4, DEF_Q_VALUE, 2500, 0, 0));
        bands.add(new Band(22, 2, DEF_Q_VALUE, 3150, 0, 0));
        bands.add(new Band(23, 8, DEF_Q_VALUE, 4000, 0, 0));

        bands.add(new Band(24, 6, DEF_Q_VALUE, 5000, 0, 0));
        bands.add(new Band(25, 4, DEF_Q_VALUE, 6300, 0, 0));
        bands.add(new Band(26, 4, DEF_Q_VALUE, 8000, 0, 0));
        bands.add(new Band(27, 6, DEF_Q_VALUE, 10000, 0, 0));
        bands.add(new Band(28, 6, DEF_Q_VALUE, 12500, 0, 0));
        bands.add(new Band(29, 4, DEF_Q_VALUE, 16000, 0, 0));
        bands.add(new Band(30, 8, DEF_Q_VALUE, 18000, 0, 0));
        bands.add(new Band(31, 4, DEF_Q_VALUE, 20000, 0, 0));
        return bands;
    }

    public static List<Band> initTechnoBands() {
        List<Band> bands = new ArrayList<>();
        bands.add(new Band(0, 4, DEF_Q_VALUE, 20, 0, 0));
        bands.add(new Band(1, 2, DEF_Q_VALUE, 25, 0, 0));
        bands.add(new Band(2, 4, DEF_Q_VALUE, 32, 0, 0));
        bands.add(new Band(3, 2, DEF_Q_VALUE, 40, 0, 0));
        bands.add(new Band(4, 2, DEF_Q_VALUE, 50, 0, 0));
        bands.add(new Band(5, 0, DEF_Q_VALUE, 63, 0, 0));
        bands.add(new Band(6, 0, DEF_Q_VALUE, 80, 0, 0));
        bands.add(new Band(7, 0, DEF_Q_VALUE, 100, 0, 0));

        bands.add(new Band(8, 0, DEF_Q_VALUE, 125, 0, 0));
        bands.add(new Band(9, -2, DEF_Q_VALUE, 160, 0, 0));
        bands.add(new Band(10, 0, DEF_Q_VALUE, 200, 0, 0));
        bands.add(new Band(11, -2, DEF_Q_VALUE, 250, 0, 0));
        bands.add(new Band(12, 0, DEF_Q_VALUE, 315, 0, 0));
        bands.add(new Band(13, 2, DEF_Q_VALUE, 400, 0, 0));
        bands.add(new Band(14, 4, DEF_Q_VALUE, 500, 0, 0));
        bands.add(new Band(15, 8, DEF_Q_VALUE, 630, 0, 0));

        bands.add(new Band(16, 10, DEF_Q_VALUE, 800, 0, 0));
        bands.add(new Band(17, 10, DEF_Q_VALUE, 1000, 0, 0));
        bands.add(new Band(18, 12, DEF_Q_VALUE, 1250, 0, 0));
        bands.add(new Band(19, 8, DEF_Q_VALUE, 1600, 0, 0));
        bands.add(new Band(20, 8, DEF_Q_VALUE, 2000, 0, 0));
        bands.add(new Band(21, 10, DEF_Q_VALUE, 2500, 0, 0));
        bands.add(new Band(22, 8, DEF_Q_VALUE, 3150, 0, 0));
        bands.add(new Band(23, 8, DEF_Q_VALUE, 4000, 0, 0));

        bands.add(new Band(24, 8, DEF_Q_VALUE, 5000, 0, 0));
        bands.add(new Band(25, 10, DEF_Q_VALUE, 6300, 0, 0));
        bands.add(new Band(26, 10, DEF_Q_VALUE, 8000, 0, 0));
        bands.add(new Band(27, 10, DEF_Q_VALUE, 10000, 0, 0));
        bands.add(new Band(28, 12, DEF_Q_VALUE, 12500, 0, 0));
        bands.add(new Band(29, 12, DEF_Q_VALUE, 16000, 0, 0));
        bands.add(new Band(30, 10, DEF_Q_VALUE, 18000, 0, 0));
        bands.add(new Band(31, 12, DEF_Q_VALUE, 20000, 0, 0));
        return bands;
    }
}
