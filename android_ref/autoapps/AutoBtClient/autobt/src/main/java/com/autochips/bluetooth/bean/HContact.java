package com.autochips.bluetooth.bean;

import android.text.TextUtils;

public class HContact implements Comparable<HContact> {
    String name;
    String phone;
    long time;
    String label;

    public long getTime() {
        return time;
    }

    public void setTime(long time) {
        this.time = time;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getPhone() {
        return phone;
    }

    public void setPhone(String phone) {
        this.phone = phone;
    }

    public String getLabel() {
        return label;
    }

    public void setLabel(String label) {
        this.label = label;
    }


    @Override
    public int compareTo(HContact o) {
        if(TextUtils.isEmpty(this.label)){
            if(TextUtils.isEmpty(o.label)){
                return 0;
            }else{
                return -1;
            }
        }else{
            if(TextUtils.isEmpty(o.label)){
                return 1;
            }
        }
        char cur = this.label.charAt(0);
        char obj = o.label.charAt(0);
        if(cur == obj){
            return 0;
        }

        if(cur == '#'){
            if(obj != '#'){
                return 1;
            }
        }else{
            if(obj == '#'){
                return -1;
            }
        }

        if(cur > obj){
            return 1;
        }else{
            return -1;
        }
    }
}
