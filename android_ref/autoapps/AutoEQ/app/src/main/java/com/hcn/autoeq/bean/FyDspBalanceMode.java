package com.hcn.autoeq.bean;

public enum FyDspBalanceMode {
    MAIN("MAIN", 3, 3), CO("CO", 11, 3), REAR("REAR", 7, 11), WHOLE("WHOLE", 7, 7),
    LEFT("LEFT", 0, 7), CENTER("CENTER", 7, 7), RIGHT("RIGHT", 14, 7);

    private String name;
    private int x, y;

    FyDspBalanceMode(String name, int x, int y) {
        this.name = name;
        this.x = x;
        this.y = y;
    }

    public String getName() {
        return name;
    }

    public int getX() {
        return x;
    }

    public int getY() {
        return y;
    }
}
