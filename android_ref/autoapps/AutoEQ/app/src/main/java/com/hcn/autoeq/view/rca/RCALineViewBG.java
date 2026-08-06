package com.hcn.autoeq.view.rca;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

import com.hcn.autoeq.R;

public class RCALineViewBG extends View {
    private String TAG = "RCALineViewBG";
    private Paint mPaintGrayText;
    /**
     * 直通线 y轴位置
     */
    private float MIN_Y;
    /**
     * 贝塞尔曲线基点坐标(x,y)
     */
    private float[][] beiSaiErBasicPoints;
    /**
     * view曲线控件的宽度 宽度
     */
    private float width = 1200;
    /**
     * view曲线控件的 高度度
     */
    private float height = 153;
    /**
     * y轴4等分，即高等分数
     */
    private int mYCount = 4;
    /**
     * x轴30等分，即宽等分数
     */
    private int mXCount = 30;
    /**
     * 二倍程规则中13个分隔坐标
     */
    public int FLAG_POINT_COUNT = 13;

    /**
     * 频率文字绘制起点
     */
    private float[][] text_point;

    public RCALineViewBG(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public RCALineViewBG(Context context) {
        super(context);
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        super.onLayout(changed, left, top, right, bottom);
        Log.d(TAG, "onLayout width=" + width + "  height=" + height);
        initDefaultPoint();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        drawFText(canvas);
    }

    private void init() {
        mPaintGrayText = new Paint(Paint.ANTI_ALIAS_FLAG); // 设置抗锯齿
        mPaintGrayText.setColor(getResources().getColor(R.color.grayColor)); // 画笔颜色
        mPaintGrayText.setStyle(Paint.Style.FILL); // 设置填充画笔，只画圆边
        mPaintGrayText.setStrokeWidth(1f);

        if (beiSaiErBasicPoints == null) {
            beiSaiErBasicPoints = new float[FLAG_POINT_COUNT][2];
        }
        if (text_point == null) {
            text_point = new float[FLAG_POINT_COUNT][2];
        }
    }

    private void initDefaultPoint() {
        MIN_Y = height / 4;
        float stepY = height / mYCount;
        float stepX = width / mXCount;
        int zb = 0;
        int yushu = 0;

        /*view等分30格--基点初始值在二倍程规则中*/
        for (int i = 0; i < FLAG_POINT_COUNT; i++) {
            if (i == 0) {
                //第0个刻度线开始20Hz处开始
                beiSaiErBasicPoints[i][0] = 0;
            } else {
                //第1个刻度线开始40Hz处开始，规律二倍程，4*(10^n)，5*(10^n)
                zb = i / 4;
                yushu = i % 4;
                if (yushu == 1) {
                    beiSaiErBasicPoints[i][0] = stepX * 3 + stepX * zb * 10;//30格中，每10格一个周期
                } else if (yushu == 2) {
                    beiSaiErBasicPoints[i][0] = stepX * 4 + stepX * zb * 10;
                } else if (yushu == 3) {
                    beiSaiErBasicPoints[i][0] = stepX * 7 + stepX * zb * 10;
                } else if (yushu == 0) {
                    beiSaiErBasicPoints[i][0] = stepX * 10 + stepX * (zb - 1) * 10;//注意这里要减一
                }
            }
            Log.d(TAG, "initMusicDefaultPoint  i=" + i
                    + " zb=" + zb
                    + " yushu=" + yushu
                    + "  x=" + beiSaiErBasicPoints[i][0]);

            beiSaiErBasicPoints[i][1] = 0;
            if (i == FLAG_POINT_COUNT - 1) {
                text_point[i][0] = beiSaiErBasicPoints[i][0];
            } else if (i == 0) {
                text_point[i][0] = beiSaiErBasicPoints[i][0];
            } else {
                text_point[i][0] = beiSaiErBasicPoints[i][0];
            }
            text_point[i][1] = height;
        }

        Log.i(TAG, "initDefaultPoint "
                + " FLAG_POINT_COUNT=" + FLAG_POINT_COUNT
                + " MIN_Y=" + MIN_Y
                + " stepX=" + stepX
                + " stepY=" + stepY
        );

    }

    /**
     * 绘制值
     *
     * @param canvas
     */
    private void drawFText(Canvas canvas) {
        String txt = "";
        double f = 20;
        //2倍程规则，从40Hz开始每4个位置一个周期，zb表示：zb组4个位置
        int zb = 0;
        int yushu = 0;
        //x轴向右偏移
        int dxRate = 0;
        //y轴向下偏移
        int dyRate = 0;
        for (int i = 0; i < FLAG_POINT_COUNT; i++) {
            dxRate = 20;
            dyRate = 45;
            if (i == 0) {
                f = 20;
            } else {
                zb = i / 4;
                yushu = i % 4;
                if (yushu == 1) {
                    f = 40 * Math.pow(10 * 1.0, zb * 1.0);//10的zb次方
                    continue;
                } else if (yushu == 2) {
                    f = 50 * Math.pow(10 * 1.0, zb * 1.0);
                } else if (yushu == 3) {
                    f = 100 * Math.pow(10 * 1.0, zb * 1.0);
                } else if (yushu == 0) {
                    f = 200 * Math.pow(10 * 1.0, (zb - 1) * 1.0);
                }
            }

            if (f >= 1000) {
                //txt = String.valueOf((f / 1000)) + "kHz";
                txt = String.valueOf((f / 1000)).replace(".0", "") + "K";
            } else {
                //txt = String.valueOf((int) f) + "Hz";
                txt = String.valueOf((int) f);
            }
            //Log.i(TAG, "drawFText  i=" + i + " txt=" + txt);
            //绘制频率
            if (FLAG_POINT_COUNT - 1 == i) {
                dxRate = 12;
            } else if (0 == i) {
                dxRate = 25;
            }
            canvas.drawText(txt, text_point[i][0] + dxRate, text_point[i][1] + dyRate, mPaintGrayText);

            /*
            //绘制view (x,y)坐标点 数值
            if(i==FLAG_POINT_COUNT-1){
                canvas.drawText(String.valueOf((int)text_point[i][0]+40),text_point[i][0]+10,20,mPaintGrayText);
            }else if(i==0){
                canvas.drawText(String.valueOf((int)text_point[i][0]),text_point[i][0],20,mPaintGrayText);
            }
            else{
                canvas.drawText(String.valueOf((int)text_point[i][0]+20),text_point[i][0]+10,20,mPaintGrayText);
            }*/

        }

        //x轴向右偏移
        int dxDB = 0;
        //y轴向下偏移
        int dyDB = 0;
        //Y轴方向绘制等分点数字标识
        for (int i = 0; i < 5; i++) {
            dxDB = 10;
            dyDB = 35;
            if (i == 0) {
                dxDB = dxDB + 3;
                dyDB = dyDB + 5;
            } else if (i == 1) {
                dxDB = dxDB + 10;
            } else if (i == 4) {
                dyDB = dyDB - 5;
            } else {
            }
            canvas.drawText(String.valueOf(10 - i * 10), dxDB, MIN_Y * i + dyDB, mPaintGrayText);
        }

    }

}
