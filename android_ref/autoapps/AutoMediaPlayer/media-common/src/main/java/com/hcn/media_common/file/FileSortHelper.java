/*
 * Copyright (c) 2010-2011, The MiCode Open Source Community (www.micode.net)
 *
 * This file is part of FileExplorer.
 *
 * FileExplorer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FileExplorer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SwiFTP.  If not, see <http://www.gnu.org/licenses/>.
 */

package com.hcn.media_common.file;

import com.hcn.mediaservice.data.MusicInfo;

import java.util.Comparator;
import java.util.HashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * 文件排序辅助器
 *
 * @author 86158
 */
@SuppressWarnings("rawtypes")
public class FileSortHelper {
    public static final String SORT_NAME = "name";
    public static final String SORT_INDEX = "index";

    private String mSort;
    private final Pattern mPattern = Pattern.compile("^\\d+");
    private final HashMap<String, Comparator<MusicInfo>> mComparatorList = new HashMap<>();

    public FileSortHelper() {
        mSort = SORT_NAME;

        // [按名字排序]
        Comparator<MusicInfo> cmpName = new FileComparator() {
            @Override
            public int doCompare(MusicInfo object1, MusicInfo object2) {
                return object1.mFileName.compareToIgnoreCase(object2.mFileName);
            }
        };

        // [需要考虑歌曲都是数字的情况]
        Comparator<MusicInfo> cmpIndex = new FileComparator() {

            @Override
            public int doCompare(MusicInfo info1, MusicInfo info2) {
                Matcher matcher1 = mPattern.matcher(info1.mFileName);
                Matcher matcher2 = mPattern.matcher(info2.mFileName);

                boolean bFind1 = matcher1.find();
                boolean bFind2 = matcher2.find();

                // [需要考虑歌曲都是数字的情况]
                if (bFind1 && bFind2) {
                    int num1 = 0, num2 = 0;

                    try {
                        num1 = Integer.parseInt(matcher1.group());
                    } catch (Exception e) {
                        num1 = Integer.MAX_VALUE;
                    }

                    try {
                        num2 = Integer.parseInt(matcher2.group());
                    } catch (Exception e) {
                        num2 = Integer.MAX_VALUE;
                    }

                    if (num1 == num2) {
                        return info1.mFileName.compareToIgnoreCase(info2.mFileName);
                    } else {
                        return num1 - num2;
                    }
                } else if (bFind1) {
                    return -1;
                } else if (bFind2) {
                    return 1;
                } else {
                    return info1.mFileName.compareToIgnoreCase(info2.mFileName);
                }
            }
        };

        mComparatorList.put(SORT_NAME, cmpName);
        mComparatorList.put(SORT_INDEX, cmpIndex);
    }

    public void setSortMethod(String sortMode) {
        mSort = sortMode;
    }

    public String getSortMethod() {
        return mSort;
    }

    public Comparator<MusicInfo> getComparator() {
        return mComparatorList.get(mSort);
    }

    /** 文件名字比较器 **/
    private abstract static class FileComparator implements Comparator<MusicInfo> {

        @Override
        public int compare(MusicInfo object1, MusicInfo object2) {
            if (object1.mIndex == -1 && object2.mIndex >= 0) {
                return -1;
            } else if (object2.mIndex == -1 && object1.mIndex >= 0) {
                return 1;
            } else {
                return doCompare(object1, object2);
            }
        }

        /**
         * 按规则比较2个对象
         *
         * @param object1 对象1
         * @param object2 对象2
         * @return
         */
        protected abstract int doCompare(MusicInfo object1, MusicInfo object2);
    }
}
