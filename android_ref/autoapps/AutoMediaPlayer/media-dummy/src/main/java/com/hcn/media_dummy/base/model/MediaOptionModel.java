package com.hcn.media_dummy.base.model;

/**
 * 配置 ijk option 用;
 * @author 65821
 */
public class MediaOptionModel {

    public static final int VALUE_TYPE_INT = 0;
    public static final int VALUE_TYPE_STRING = 1;

    /**
     * 配置选项的值类型
     * <p> ijk 的 option value 类型
     */
    int valueType = VALUE_TYPE_INT;

    int category;

    int valueInt;

    String name;

    String valueString;

    public MediaOptionModel(int category, String name, int value) {
        super();
        this.category = category;
        this.name = name;
        this.valueInt = value;
        valueType = VALUE_TYPE_INT;
    }

    public MediaOptionModel(int category, String name, String value) {
        super();
        this.category = category;
        this.name = name;
        this.valueString = value;
        valueType = VALUE_TYPE_STRING;
    }

    public int getValueType() {
        return valueType;
    }

    public void setValueType(int valueType) {
        this.valueType = valueType;
    }

    public int getCategory() {
        return category;
    }

    public void setCategory(int category) {
        this.category = category;
    }

    public int getValueInt() {
        return valueInt;
    }

    public void setValueInt(int valueInt) {
        this.valueInt = valueInt;
        valueType = VALUE_TYPE_INT;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getValueString() {
        return valueString;
    }

    public void setValueString(String valueString) {
        this.valueString = valueString;
        valueType = VALUE_TYPE_STRING;
    }
}
