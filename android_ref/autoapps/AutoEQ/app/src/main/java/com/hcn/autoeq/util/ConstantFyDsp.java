package com.hcn.autoeq.util;

import com.hcn.autoeq.bean.FyDspBandMode;
import com.hcn.autoeq.bean.FyDspOutputMode;

public interface ConstantFyDsp {

    FyDspBandMode DEF_BAND_MODE = FyDspBandMode.STANDARD; // 默认 band 模式

    int DEF_Q_VALUE = 50; // q值默认值（8163平台、飞音客户、陈贵峰要求）
    int DEF_FREQ_MIN = 20;
    int DEF_FREQ_MAX = 18000;

    int DEF_BASS_BOOST_FREQ = 45; // bass boost freq 默认值(2023.12.16:实际值是90-600，但显示45-300)

    int DEF_GAIN_PROGRESS_MAX = EqUtils.getDspGainMax(); // 增益进度条的最大值，不是底层增益值。比如最大值24时，一般会/2，正负值就是±12，

    int DEF_LOUDNESS_DISABLE = 0; // loudness 默认关闭（8163平台、飞音客户、陈贵峰要求）

    // attenuate
    int DEF_ATTENUATE = 0; // balance 衰减的默认值

    FyDspOutputMode DEF_OUTPUT_MODE = FyDspOutputMode.WAY6; // 默认输出模式
}
