package com.hcn.autoradio.collection;

import static com.hcn.autoradio.data.RadioData.BAND_AM_1;
import static com.hcn.autoradio.data.RadioData.BAND_FM_1;
import static com.hcn.autoradio.data.RadioData.BAND_FM_2;
import static com.hcn.autoradio.data.RadioData.BAND_FM_3;

import android.util.Log;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

public class FMCollectionFunction {
    private static final String TAG = "RadioLogic_CollFunction";
    /**
     *     数据缓存,方便取数据，而不用每次读操作database,启动应用时同步数据库的数据
     */
    final double THRESHOLD = 0.0001;

    private Comparator<String> mComparator = new Comparator<String>() {
        @Override
        public int compare(String o1, String o2) {
            float f1 = Float.parseFloat(o1);
            float f2 = Float.parseFloat(o2);
            if (Math.abs(f1 - f2) < THRESHOLD) {
                return 0;
            }else if ((f1 - f2) > 0) {
                return  1;
            }else {
                return  -1;
            }
        }
    };
    /**
     * 对KEY排序，不对值排序，目的为了兼容后面不带POS的用法
     */
    TreeMap<String, Integer> mFM1Map = new TreeMap<String, Integer>(mComparator);
    TreeMap<String, Integer> mFM2Map = new TreeMap<String, Integer>(mComparator);
    TreeMap<String, Integer> mFM3Map = new TreeMap<String, Integer>(mComparator);
    TreeMap<String, Integer> mAM1Map = new TreeMap<String, Integer>(mComparator);

    private static FMCollectionFunction mInstance = null;

    public static FMCollectionFunction getInstance() {
        if (mInstance == null) {
            mInstance = new FMCollectionFunction();
        }
        return mInstance;
    }

    private FMCollectionFunction() {

    }

    public void init() {
        getAllCollectListInfo();
    }

    private synchronized void getAllCollectListInfo() {
        List<FMCollectListInfo> collectListInfo = FMCollectListDao.queryCollectListAllInfos();
        if (collectListInfo.size() > 0) {
            mAM1Map.clear();
            mFM1Map.clear();
            mFM2Map.clear();
            mFM3Map.clear();
            for (FMCollectListInfo fmCollectListInfo : collectListInfo) {
                Log.v(TAG, " ######db: " + fmCollectListInfo.getColumnBand() + " " + fmCollectListInfo.getColumnFreq() + " " + fmCollectListInfo.getColumnPos());
                TreeMap<String, Integer> map = null;
                int band = fmCollectListInfo.getColumnBand();
                if (band == BAND_FM_1) {
                    map = mFM1Map;
                } else if (band == BAND_FM_2) {
                    map = mFM2Map;
                } else if (band == BAND_FM_3) {
                    map = mFM3Map;
                } else if (band == BAND_AM_1) {
                    map = mAM1Map;
                }

                if (map != null) {
                    map.put(fmCollectListInfo.getColumnFreq(),
                            fmCollectListInfo.getColumnPos());
                }
            }
        } else {
            Log.v(TAG, "########### no database data ");
        }
    }

    public boolean isFreqCollected(int band, int pos) {
        boolean ret = false;
        TreeMap<String, Integer> map = null;
        if (band == BAND_FM_1) {
            map = mFM1Map;
        } else if (band == BAND_FM_2) {
            map = mFM2Map;
        } else if (band == BAND_FM_3) {
            map = mFM3Map;
        } else if (band == BAND_AM_1) {
            map = mAM1Map;
        }

        if (map != null) {
            if (map.containsValue(pos)) {
                ret = true;
            }
        }
        return ret;
    }

    public boolean isFreqCollected(int band, String freq) {
        boolean ret = false;
        TreeMap<String, Integer> map = null;
        if (band == BAND_FM_1) {
            map = mFM1Map;
        } else if (band == BAND_FM_2) {
            map = mFM2Map;
        } else if (band == BAND_FM_3) {
            map = mFM3Map;
        } else if (band == BAND_AM_1) {
            map = mAM1Map;
        }

        if (map != null) {
            if (map.containsKey(freq)) {
                ret = true;
            }
        }
        return ret;
    }

    public boolean collectFreq(int band, String freq, int pos) {
        if (isFreqCollected(band, freq)) {
            Log.v(TAG, " the freq is have collected");
            return false;
        }

        Log.v(TAG, " collectFreq(...) band = " + band + " freq = " + freq + " pos = " + pos);

        boolean ret = false;
        TreeMap<String, Integer> map = null;
        if (band == BAND_FM_1) {
            map = mFM1Map;
        } else if (band == BAND_FM_2) {
            map = mFM2Map;
        } else if (band == BAND_FM_3) {
            map = mFM3Map;
        } else if (band == BAND_AM_1) {
            map = mAM1Map;
        }

        synchronized (this) {
            if (map != null) {
                if (FMCollectListDao.addCollectInfo(band, freq, pos)) {
                    map.put(freq, pos);
                    ret = true;
                }
            }
        }
        return ret;
    }

    public boolean collectFreq(int band, String freq) {
        if (isFreqCollected(band, freq)) {
            Log.v(TAG, " the freq is have collected");
            return false;
        }

        Log.v(TAG, " collectFreq(..) band = " + band + " freq = " + freq + " pos = -1");

        boolean ret = false;
        TreeMap<String, Integer> map = null;
        if (band == BAND_FM_1) {
            map = mFM1Map;
        } else if (band == BAND_FM_2) {
            map = mFM2Map;
        } else if (band == BAND_FM_3) {
            map = mFM3Map;
        } else if (band == BAND_AM_1) {
            map = mAM1Map;
        }

        synchronized (this) {
            if (map != null) {
                if (FMCollectListDao.addCollectInfo(band, freq, -1)) {
                    map.put(freq, -1);
                    ret = true;
                }
            }
        }
        return ret;
    }

    public List<String> getCollectedFreqList(int band) {
        List<String> list = new ArrayList<String>();
        TreeMap<String, Integer> map = null;
        if (band == BAND_FM_1) {
            map = mFM1Map;
        } else if (band == BAND_FM_2) {
            map = mFM2Map;
        } else if (band == BAND_FM_3) {
            map = mFM3Map;
        } else if (band == BAND_AM_1) {
            map = mAM1Map;
        }

        if (map != null) {
            list.addAll(map.keySet());
        }
        return list;
    }

    public String getCollectedFreq(int band, int pos) {
        String ret = "";
        TreeMap<String, Integer> map = null;
        if (band == BAND_FM_1) {
            map = mFM1Map;
        } else if (band == BAND_FM_2) {
            map = mFM2Map;
        } else if (band == BAND_FM_3) {
            map = mFM3Map;
        } else if (band == BAND_AM_1) {
            map = mAM1Map;
        }

        if (map != null && pos >= 0) {
            for (Map.Entry<String, Integer> entry : map.entrySet()) {
                if ((int) entry.getValue() == pos) {
                    ret = (String) entry.getKey();
                    break;
                }
            }
        } else {
            Log.v(TAG, " getCollectFreq(band, pos) parma error ");
        }
        return ret;
    }

    public int deleteCollectedFreq(int band, int pos) {
        TreeMap<String, Integer> map = null;
        if (band == BAND_FM_1) {
            map = mFM1Map;
        } else if (band == BAND_FM_2) {
            map = mFM2Map;
        } else if (band == BAND_FM_3) {
            map = mFM3Map;
        } else if (band == BAND_AM_1) {
            map = mAM1Map;
        }

        synchronized (this) {
            if (map != null) {
                String key = null;
                for (Map.Entry<String, Integer> entry : map.entrySet()) {
                    if (entry.getValue() == pos) {
                        key = entry.getKey();
                        break;
                    }
                }

                if (key != null) {
                    map.remove(key);
                }
            }
            return FMCollectListDao.deleteFreqByColumn(band, pos);
        }
    }

    public int deleteCollectedFreq(int band, String freq) {
        TreeMap<String, Integer> map = null;
        if (band == BAND_FM_1) {
            map = mFM1Map;
        } else if (band == BAND_FM_2) {
            map = mFM2Map;
        } else if (band == BAND_FM_3) {
            map = mFM3Map;
        } else if (band == BAND_AM_1) {
            map = mAM1Map;
        }

        synchronized (this) {
            if (map != null && map.containsKey(freq)) {
                map.remove(freq);
            }
            return FMCollectListDao.deleteFreqByColumn(band, freq);
        }
    }

    public int deleteCollectedFreqAll(int band) {
        TreeMap<String, Integer> map = null;
        if (band == BAND_FM_1) {
            map = mFM1Map;
        } else if (band == BAND_FM_2) {
            map = mFM2Map;
        } else if (band == BAND_FM_3) {
            map = mFM3Map;
        } else if (band == BAND_AM_1) {
            map = mAM1Map;
        }

        synchronized (this) {
            if (map != null) {
                map.clear();
            }
            return FMCollectListDao.deleteFreqByColumn(band);
        }
    }

    public int deleteCollectedFreqAll() {
        synchronized (this) {
            mFM1Map.clear();
            mFM2Map.clear();
            mFM3Map.clear();
            mAM1Map.clear();
            return FMCollectListDao.deleteAll();
        }
    }

    public int updateCollectedFreq(int band, int pos, String new_freq) {
        TreeMap<String, Integer> map = null;
        if (band == BAND_FM_1) {
            map = mFM1Map;
        } else if (band == BAND_FM_2) {
            map = mFM2Map;
        } else if (band == BAND_FM_3) {
            map = mFM3Map;
        } else if (band == BAND_AM_1) {
            map = mAM1Map;
        }

        synchronized (this) {
            assert map != null;
            if (!map.containsValue(pos)) {
                return 0;
            }

            FMCollectListDao.updateCollectedInfo(band, pos, new_freq);
            String key = null;
            for (Map.Entry<String, Integer> entry : map.entrySet()) {
                if (entry.getValue() == pos) {
                    key = entry.getKey();
                    break;
                }
            }

            if (key != null) {
                map.remove(key);
            }

            map.put(new_freq, pos);
        }
        return 0;
    }

    public int updateCollectedFreq(int band, String old_freq, String new_freq) {
        TreeMap<String, Integer> map = null;
        if (band == BAND_FM_1) {
            map = mFM1Map;
        } else if (band == BAND_FM_2) {
            map = mFM2Map;
        } else if (band == BAND_FM_3) {
            map = mFM3Map;
        } else if (band == BAND_AM_1) {
            map = mAM1Map;
        }

        synchronized (this) {
            assert map != null;
            if (!map.containsKey(old_freq)) {
                return 0;
            }

            FMCollectListDao.updateCollectedInfo(band, old_freq, new_freq);
            String key = null;
            int pos = -1;
            for (Map.Entry<String, Integer> entry : map.entrySet()) {
                if (entry.getKey() == old_freq) {
                    key = entry.getKey();
                    pos = entry.getValue();
                    break;
                }
            }

            if (key != null) {
                map.remove(key);
            }

            map.put(new_freq, pos);
        }
        return 0;
    }
}
