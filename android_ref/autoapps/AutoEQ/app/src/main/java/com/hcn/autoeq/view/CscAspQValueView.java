package com.hcn.autoeq.view;

import android.animation.ValueAnimator;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.Point;
import android.graphics.Shader;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.view.animation.LinearInterpolator;

import androidx.annotation.Nullable;

import com.hcn.autoeq.R;
import com.hcn.autoeq.util.EqUtils;
import com.hcn.autoeq.util.SkinUtils;

import java.util.ArrayList;
import java.util.List;

/**
 * Q值曲线图
 */
public class CscAspQValueView extends View {
    private static final String TAG = CscAspQValueView.class.getSimpleName();
    private Context mContext;

    /**
     * 画笔和路径
     */
    private Paint mPaint;
    private Path mPath;

    /**
     * 填充颜色（临时，后面优化）
     */
    private int colorStart;
    private int colorAfterStart;
    private int colorMiddle;
    private int colorBeforeEnd;
    private int colorEnd;
    private LinearGradient backGradient;

    /**
     * 控件尺寸
     */
    private float widthView;
    private float highView;

    private static final int SIZE_POINT = 7;


    //控件区域最小格子宽度
    private float cellWidth;
    //控件区域最小格子高度
    private float cellHigh;
    //控件区域高，最小格子数量
    private static final int CELL_HIGH_NUMBER = 10;
    //控件区域宽，最小格子数量
    private static final int CELL_WIDTH_NUMBER = 30;

    private Point startp = new Point();
    private Point endp = new Point();
    private Point p3 = new Point();
    private Point p4 = new Point();

    private boolean initPointDate = true;

    /**
     * 判断存在存储数据
     */
    private boolean exitDate = false;

    //动画目标坐标位置（算格子数）
    private final float[][] defaultPoints = new float[][]{
            {0, 5}, {5, 1}, {10, 5},
            {15, 1}, {20, 5},
            {25, 1}, {30, 5}
    };
    //进场坐标位置（算格子数）
    private final float[][] enterPoints = new float[][]{
            {0, 1}, {5, 1}, {10, 1}, {15, 1}, {20, 1}, {25, 1}, {30, 1}
    };

    //预期动画坐标位置（算格子数）
    private float[][] animTargetPoints = new float[SIZE_POINT][2];
    //真实坐标位置（算格子数）
    private float[][] realPoints = new float[SIZE_POINT][2];


    //实际坐标位置（x，y坐标点）
    private Point[] realPointsValue;

    private int mValueLow;
    private int mValueMiddle;
    private int mValueHigh;

    /**
     * 通用动画的步进值，因为不同分辨率的格子大小不同的，所以需要根据格子大小来计算步进值；
     */
    private float animSteep = 0.01f;


    /**
     * 开场动画的步进值，因为不同分辨率的格子大小不同的，所以需要根据格子大小来计算步进值；
     */
    private float enterAnimSteep = 0.1f;

    public CscAspQValueView(Context context) {
        super(context);
        mContext = context;
        initUtils();
    }

    public CscAspQValueView(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
        initUtils();

    }

    /**
     * 初始化数据
     */
    public void initQValueDate(int lowQValue, int middleQValue, int highQValue) {
        mValueLow = lowQValue;
        mValueMiddle = middleQValue;
        mValueHigh = highQValue;
        //实际坐标数据
        initRealPoint();
    }

    /**
     * 初始化绘画工具
     */
    private void initUtils() {
        mPaint = new Paint();
        mPaint.setStrokeWidth(1);
        mPaint.setAntiAlias(true);
        mPaint.setStyle(Paint.Style.FILL);
        mPaint.setDither(true);
        mPath = new Path();

        colorStart = SkinUtils.getColor(R.color.csc_asp_q_value_view_linear_color_1);
        colorAfterStart = SkinUtils.getColor(R.color.csc_asp_q_value_view_linear_color_2);
        colorMiddle = SkinUtils.getColor(R.color.csc_asp_q_value_view_linear_color_3);
        colorBeforeEnd = SkinUtils.getColor(R.color.csc_asp_q_value_view_linear_color_4);
        colorEnd = SkinUtils.getColor(R.color.csc_asp_q_value_view_linear_color_5);

    }


    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        //获取控件宽高
        widthView = (float) getMeasuredWidth();
        highView = (float) getMeasuredHeight();
        //控件区域最小格子宽度
        cellWidth = widthView / CELL_WIDTH_NUMBER;
        cellHigh = highView / CELL_HIGH_NUMBER;
        //动画步进值
        animSteep = cellHigh / 2000;
        enterAnimSteep = cellHigh / 200;
    }


    /**
     * 初始化添加真实数据
     *
     * @return
     */
    private void initRealPoint() {
        //第一次的进入，使用进场数据
        for (int i = 0; i < SIZE_POINT; i++) {
            realPoints[i][0] = enterPoints[i][0];
            realPoints[i][1] = enterPoints[i][1];
        }
        enterAnim();
    }


    /**
     * 根据传入的数据，确定绘制的点
     *
     * @return
     */
    private Point[] initPoint() {
        Point[] points = new Point[SIZE_POINT];
        for (int i = 0; i < SIZE_POINT; i++) {
            int x = (int) (realPoints[i][0] * cellWidth);
            int y = (int) (realPoints[i][1] * cellHigh);
            points[i] = new Point(x, y);
        }
        return points;
    }


    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        super.onSizeChanged(w, h, oldw, oldh);
        if ("mcc602_asp".equals(EqUtils.getSkinName())) {
            // 渐变角度（度），设置为 253 - 90 度
            float angle = 163;
            // 将角度转换为弧度
            float radians = (float) Math.toRadians(angle);

            // 计算起始点和结束点
            float centerX = w / 2f;
            float centerY = h / 2f;
            float radius = (float) Math.sqrt(centerX * centerX + centerY * centerY);
            float startX = (float) (centerX + radius * Math.cos(radians));
            float startY = (float) (centerY + radius * Math.sin(radians));
            float endX = (float) (centerX - radius * Math.cos(radians));
            float endY = (float) (centerY - radius * Math.sin(radians));

            backGradient = new LinearGradient(startX, startY, endX, endY,
                    new int[]{colorStart, colorAfterStart, colorMiddle, colorBeforeEnd, colorEnd},
                    new float[]{0.1f, 0.3f, 0.5f, 0.7f, 0.88f}, Shader.TileMode.CLAMP);
        } else {
            //控制颜色
            backGradient = new LinearGradient(0, 0, w, 0, new int[]{colorStart, colorAfterStart, colorMiddle, colorBeforeEnd, colorEnd}, null, Shader.TileMode.CLAMP);
        }
    }


    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        //设置渐变色
        mPaint.setShader(backGradient);
        realPointsValue = initPoint();
        //绘制曲线-曲线图
        drawScrollLine(canvas);
    }


    /**
     * 绘制曲线-曲线图
     *
     * @param canvas
     */
    private void drawScrollLine(Canvas canvas) {
        //上半部分
        for (int i = 0; i < SIZE_POINT - 1; i++) {
            startp.x = realPointsValue[i].x;
            startp.y = realPointsValue[i].y;
            endp.y = realPointsValue[i + 1].y;
            endp.x = realPointsValue[i + 1].x;
            //中间控制点，先下p3后上p4，
            int wt = (startp.x + endp.x) / 2;
            p3.y = startp.y;
            p3.x = wt;
            p4.y = endp.y;
            p4.x = wt;
            if (i == 0) {
                mPath.moveTo(startp.x, startp.y);
            }
            mPath.cubicTo(p3.x, p3.y, p4.x, p4.y, endp.x, endp.y);
        }

        //下半部分(做了一个小循环，首尾连接)
        for (int i = SIZE_POINT - 1; i > 0; i--) {
            startp.x = realPointsValue[i].x;
            startp.y = realPointsValue[i].y;
            endp.y = (int) (highView - realPointsValue[i - 1].y);
            endp.x = realPointsValue[i - 1].x;
            //中间控制点，先下p5后上p6，
            int wt = (startp.x + endp.x) / 2;
            p3.y = (int) (highView - startp.y);
            p3.x = wt;
            p4.y = endp.y;
            p4.x = wt;
            //连接上半部分
            if (i == (SIZE_POINT - 1)) {
                mPath.lineTo(startp.x, (int) (highView - startp.y));
            }
            mPath.cubicTo(p3.x, p3.y, p4.x, p4.y, endp.x, endp.y);
        }
        //闭环
        mPath.close();
        canvas.drawPath(mPath, mPaint);
        mPath.reset();
    }


    /**
     * q值控制；通过改变三个q值，并且更新动画的目标点；
     */
    public void qValueControl(int valueLow, int valueMiddle, int valueHigh) {
        mValueLow = valueLow;
        mValueMiddle = valueMiddle;
        mValueHigh = valueHigh;
        if (valueLow == 0) {
            animTargetPoints[0][1] = defaultPoints[0][1];
            animTargetPoints[2][1] = defaultPoints[2][1];
        } else {
            animTargetPoints[0][1] = defaultPoints[0][1] - (0.1f * (float) Math.pow(2, valueLow));
            animTargetPoints[2][1] = defaultPoints[2][1] - (0.1f * (float) Math.pow(2, Math.min(valueLow, valueMiddle)));
        }

        if (valueMiddle == 0) {
            animTargetPoints[2][1] = defaultPoints[2][1];
            animTargetPoints[4][1] = defaultPoints[4][1];
        } else {
            animTargetPoints[2][1] = defaultPoints[2][1] - (0.1f * (float) Math.pow(2, Math.min(valueLow, valueMiddle)));
            animTargetPoints[4][1] = defaultPoints[4][1] - (0.1f * (float) Math.pow(2, valueMiddle));
        }

        if (valueHigh == 0) {
            animTargetPoints[4][1] = defaultPoints[4][1];
        } else {
            animTargetPoints[4][1] = defaultPoints[4][1] - (valueMiddle < valueHigh ? 0f : 0.5f);
        }
        animTargetPoints[6][1] = defaultPoints[6][1] - (0.5f * valueHigh);

        animTargetPoints[2][0] = defaultPoints[2][0] + (valueLow > valueMiddle ? 0.2f : valueLow < valueMiddle ? -0.2f : 0f);
        animTargetPoints[4][0] = defaultPoints[4][0] + (valueMiddle > valueHigh ? 0.2f : valueMiddle < valueHigh ? -0.2f : 0f);

    }


    /**
     * q值动画
     */
    public void qValueAnim(float animSteep, long durationTime) {
        //如果动画时间很短，既可直接更新，不需要动画效果
        if (durationTime <= 0) {
            for (int i = 0; i < SIZE_POINT; i++) {
                realPoints[i][1] = animTargetPoints[i][1];
                realPoints[i][0] = animTargetPoints[i][0];
            }
            postInvalidate();
            return;
        }

        ValueAnimator valueAnimator = ValueAnimator.ofFloat(0f, 300f);
        valueAnimator.setInterpolator(new LinearInterpolator());
        valueAnimator.setDuration(durationTime);
        valueAnimator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator animation) {
                for (int i = 0; i < SIZE_POINT; i++) {
                    //一旦达到目标值，就跳过
                    if (animTargetPoints[i][1] == realPoints[i][1]) {
                        continue;
                    }
                    //根据不同的情况，决定步进方向；
                    if (realPoints[i][1] < animTargetPoints[i][1]) {
                        if (realPoints[i][1] < animTargetPoints[i][1]) {
                            realPoints[i][1] += animSteep;
                        }
                        //如果超出目标,强行纠正，并且跳过
                        if (realPoints[i][1] >= animTargetPoints[i][1]) {
                            realPoints[i][1] = animTargetPoints[i][1];
                            continue;
                        }
                    } else {
                        if (realPoints[i][1] > animTargetPoints[i][1]) {
                            realPoints[i][1] -= animSteep;
                        }
                        //如果超出目标,强行纠正，并且跳过
                        if (realPoints[i][1] <= animTargetPoints[i][1]) {
                            realPoints[i][1] = animTargetPoints[i][1];
                            continue;
                        }
                    }
                }
                for (int i = 0; i < SIZE_POINT; i++) {
                    //一旦达到目标值，就跳过
                    if (animTargetPoints[i][0] == realPoints[i][0]) {
                        continue;
                    }
                    //根据不同的情况，决定步进方向；
                    if (realPoints[i][0] < animTargetPoints[i][0]) {
                        if (realPoints[i][0] < animTargetPoints[i][0]) {
                            realPoints[i][0] += animSteep;
                        }
                        //如果超出目标,强行纠正，并且跳过
                        if (realPoints[i][0] >= animTargetPoints[i][0]) {
                            realPoints[i][0] = animTargetPoints[i][0];
                            continue;
                        }
                    } else {
                        if (realPoints[i][0] > animTargetPoints[i][0]) {
                            realPoints[i][0] -= animSteep;
                        }
                        //如果超出目标,强行纠正，并且跳过
                        if (realPoints[i][0] <= animTargetPoints[i][0]) {
                            realPoints[i][0] = animTargetPoints[i][0];
                            continue;
                        }
                    }
                }
                postInvalidate();

            }
        });
        valueAnimator.start();
    }

    /**
     * 进场动画
     */
    public void enterAnim() {
        for (int i = 0; i < SIZE_POINT; i++) {
            animTargetPoints[i][0] = defaultPoints[i][0];
            animTargetPoints[i][1] = defaultPoints[i][1];
        }
        if (mValueLow != 0 || mValueMiddle != 0 || mValueHigh != 0) {
            qValueControl(mValueLow, mValueMiddle, mValueHigh);
        }
        qValueAnim(getEnterAnimSteep(), 3000);
    }

    public float getAnimSteep() {
        return animSteep;
    }

    public float getEnterAnimSteep() {
        return enterAnimSteep;
    }

}
