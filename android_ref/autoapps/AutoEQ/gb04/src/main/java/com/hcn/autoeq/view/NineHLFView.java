package com.hcn.autoeq.view;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

import com.hcn.autoeq.R;
import com.hcn_library.util.SkinUtils;

public class NineHLFView extends View {
    private static final String TAG = NineHLFView.class.getSimpleName();
    private Paint paint;
    private Paint paintDebug;
    private Paint paintDebugStartEnd;
    private Path path;
    private TouchLineMoveInterface touchLineMoveInterface;
    private static final int HLF_HORIZONTAL_PADDING = (int) SkinUtils.getDimension(R.dimen.x60); // 设置padding的地方无法设置顶点坐标，不参与频率比例计算。
    private static final int HLF_TOP_PADDING = (int) SkinUtils.getDimension(R.dimen.x60);
    private float startX;
    private float startY;
    private float endX;
    private float endY;
    private float controlX1;
    private float controlY1;
    private float controlX2;
    private float controlY2;
    private float topX;
    private float topY;
    private float controlX3;
    private float controlY3;
    private float controlX4;
    private float controlY4;
    private int leftLimit;
    private int rightLimit;
    private float minY;
    private float maxY;
    private int freqLow;
    private int freqHigh;
    private int freqLeftLimit = 20;
    private int freqRightLimit = 20000;
    private int freqLeftLimitCommon = 20;
    private int freqRightLimitCommon = 20000;
    private float slopeRateH = 6.0f;
    private float slopeRateL = 6.0f;
    private boolean isDebug = false;

    public interface TouchLineMoveInterface {
        void onLineMove(float f, float f2);

        void onLineMoveStop(float f, float f2);

        void onStartMoveLine();
    }

    public NineHLFView(Context context) {
        super(context);
        init();
    }

    public NineHLFView(Context context, AttributeSet attributeSet) {
        super(context, attributeSet);
        init();
    }

    public NineHLFView(Context context, AttributeSet attributeSet, int i) {
        super(context, attributeSet, i);
        init();
    }

    private void init() {
        path = new Path();
        paint = new Paint();
        paint.setColor(SkinUtils.getColor(R.color.nine_curve_line_color));
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(SkinUtils.getDimension(R.dimen.y4));
        paint.setAntiAlias(true);
        if (isDebug) {
            paintDebug = new Paint();
            paintDebug.setColor(SkinUtils.getColor(R.color.nine_red_color));
            paintDebug.setStyle(Paint.Style.STROKE);
            paintDebug.setStrokeWidth(2.0f);
            paintDebug.setAntiAlias(true);
            paintDebugStartEnd = new Paint();
            paintDebugStartEnd.setColor(SkinUtils.getColor(R.color.nine_debug_line_color));
            paintDebugStartEnd.setStyle(Paint.Style.STROKE);
            paintDebugStartEnd.setStrokeWidth(2.0f);
            paintDebugStartEnd.setAntiAlias(true);
        }
        Log.d(TAG, "init");
    }

    @Override
    protected void onMeasure(int i, int i2) {
        super.onMeasure(i, i2);
        int measuredWidth = MeasureSpec.getSize(i);
        int measuredHeight = MeasureSpec.getSize(i2);

        leftLimit = frequencyToX(freqLeftLimit);
        rightLimit = frequencyToX(freqRightLimit);
        minY = HLF_TOP_PADDING;
        maxY = measuredHeight;
        Log.d(TAG, "onMeasure leftLimit: " + leftLimit + " rightLimit: " + rightLimit + " width: " + measuredWidth + " freqLeftLimit: " + freqLeftLimit + " freqRightLimit: " + freqRightLimit + " minY: " + minY + " maxY: " + maxY);
        initPointData();
    }

    private void initPointData() {
        Log.d(TAG, "开始初始化点数据");
        // 设置 controlY2 和 controlY3
        controlY2 = minY;
        controlY3 = minY;
        Log.d(TAG, "设置 controlY2 和 controlY3 为 minY，controlY2: " + controlY2 + ", controlY3: " + controlY3);
        Log.d(TAG, "leftLimit: " + leftLimit + ", rightLimit: " + rightLimit);

        // 限制控制点范围
        Log.d(TAG, "开始限制控制点范围");
        if (controlX3 > rightLimit) {
            controlX3 = rightLimit;
            Log.d(TAG, "controlX3 超出右边界，调整为 rightLimit，controlX3: " + controlX3);
        }
        if (controlX2 < leftLimit) {
            controlX2 = leftLimit;
            Log.d(TAG, "controlX2 超出左边界，调整为 leftLimit，controlX2: " + controlX2);
        }
        if (controlX2 > controlX3) {
            float temp = controlX2;
            controlX2 = controlX3;
            controlX3 = temp;
            Log.d(TAG, "controlX2 大于 controlX3，交换两者值，controlX2: " + controlX2 + ", controlX3: " + controlX3);
        }

        // 根据left斜率算出start点和控制点2的线性关系，判断start点和坐标轴的交点是在x轴上还是y轴上
        startX = leftLimit - HLF_HORIZONTAL_PADDING;
        float tempStartY = (maxY - controlY2) - slopeRateH * (controlX2 - startX);
        if (tempStartY < 0) {
            // 交点在x轴上，计算新的startX
            startY = maxY - 0;
            startX = controlX2 + (controlY2 - maxY) / (slopeRateH);
            Log.d(TAG, "adjust before，startX: " + startX + ", startY: " + startY);
            if (startX > rightLimit) {
                startX = rightLimit;
            }
            if (startX < leftLimit - HLF_HORIZONTAL_PADDING) {
                startX = leftLimit - HLF_HORIZONTAL_PADDING;
            }
            Log.d(TAG, "adjust after，startX: " + startX + ", startY: " + startY);
        } else {
            startY = maxY - tempStartY;
            if (startY < minY) {
                startY = minY;
            }
        }
        Log.d(TAG, "根据左斜率计算起始点，startX: " + startX + ", startY: " + startY);

        // 根据right斜率算出end点和控制点3的线性关系，判断end点和坐标轴的交点是在x轴上还是y轴上
        endX = rightLimit + HLF_HORIZONTAL_PADDING;
        float tempEndY = (maxY - controlY3) + slopeRateL * (endX - controlX3);
        if (tempEndY < 0) {
            // 交点在x轴上，计算新的endX
            endY = maxY - 0;
            endX = controlX3 + (controlY3 - maxY) / (slopeRateL);
            Log.d(TAG, "adjust before，endX: " + endX + ", endY: " + endY);
            if (endX > rightLimit + HLF_HORIZONTAL_PADDING) {
                endX = rightLimit + HLF_HORIZONTAL_PADDING;
            }
            if (endX < leftLimit - HLF_HORIZONTAL_PADDING) {
                endX = leftLimit - HLF_HORIZONTAL_PADDING;
            }
            Log.d(TAG, "adjust after，endX: " + endX + ", endY: " + endY);
        } else {
            endY = maxY - tempEndY;
            if (endY < minY) {
                endY = minY;
            }
        }
        Log.d(TAG, "根据右斜率计算结束点，endX: " + endX + ", endY: " + endY);

        // 计算其他控制点和顶点
        controlX1 = (startX + controlX2) / 2.0f;
        controlX4 = (endX + controlX3) / 2.0f;
        controlY1 = (startY + controlY2) / 2.0f;
        controlY4 = (endY + controlY3) / 2.0f;
        topX = (controlX2 + controlX3) / 2.0f;
        topY = minY;
        Log.d(TAG, "计算其他控制点和顶点：" +
                "controlX1: " + controlX1 + ", controlY1: " + controlY1 +
                ", controlX2: " + controlX2 + ", controlY2: " + controlY2 +
                ", topX: " + topX + ", topY: " + topY +
                ", controlX3: " + controlX3 + ", controlY3: " + controlY3 +
                ", controlX4: " + controlX4 + ", controlY4: " + controlY4);

        Log.d(TAG, "初始化点数据完成，width: " + getMeasuredWidth() + " height: " + getMeasuredHeight() +
                " startX: " + startX + " controlX1: " + controlX1 + " controlX2: " + controlX2 +
                " topX: " + topX + " controlX3: " + controlX3 + " controlX4: " + controlX4 +
                " endX: " + endX);
        Log.d(TAG, "初始化点数据完成，width: " + getMeasuredWidth() + " height: " + getMeasuredHeight() +
                " startY: " + startY + " controlY1: " + controlY1 + " controlY2: " + controlY2 +
                " topY: " + topY + " controlY3: " + controlY3 + " controlY4: " + controlY4 +
                " endY: " + endY);

    }

    private float parseIndexToSlope(float slopeIndex) {
        float resultSlope = 6;
        switch ((int) slopeIndex) {
            case 0:
                resultSlope = 0;
                break;
            case 1:
                resultSlope = 0.25f;
                break;
            case 2:
                resultSlope = 0.3f;
                break;
            case 3:
                resultSlope = 0.35f;
                break;
            case 4:
                resultSlope = 0.4f;
                break;
            case 5:
                resultSlope = 0.45f;
                break;
            case 6:
                resultSlope = 0.5f;
                break;
        }
        return resultSlope;
    }

    public void setSlopeH(float slopeIndex) {
        slopeRateH = parseIndexToSlope(slopeIndex);
    }

    public void setSlopeL(float slopeIndex) {
        slopeRateL = -parseIndexToSlope(slopeIndex);
    }

    public void setFreqHigh(int f) {
        freqHigh = f;
        controlX2 = frequencyToX(f);
        Log.d(TAG, "setFreqHigh，freqHigh: " + freqHigh + ", controlX2: " + controlX2);
    }

    public void setFreqLow(int f) {
        freqLow = f;
        controlX3 = frequencyToX(f);
        Log.d(TAG, "setFreqLow，freqLow: " + freqLow + ", controlX3: " + controlX3);
    }

    public void setFreqLimit(int f, int f2) {
        freqLeftLimit = f;
        freqRightLimit = f2;
        leftLimit = frequencyToX(f);
        rightLimit = frequencyToX(f2);
    }

    public void reDraw() {
        invalidate();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        path.reset();
        initPointData();
        float adjustStartX = startX;
        float adjustEndX = endX;
        if (slopeRateH == 0.0f) {
            adjustStartX = leftLimit - HLF_HORIZONTAL_PADDING;
        }
        if (slopeRateL == 0.0f) {
            adjustEndX = rightLimit + HLF_HORIZONTAL_PADDING;
        }
        path.moveTo(adjustStartX, startY);
        path.cubicTo(controlX1, controlY1, controlX2, controlY2, topX, topY);
        path.cubicTo(controlX3, controlY3, controlX4, controlY4, adjustEndX, endY);
        canvas.drawPath(path, paint);
        if (isDebug) {
//            paintDebug.setColor(SkinUtils.getColor(R.color.nine_red_color));
//            canvas.drawLine(controlX1, measuredHeight(), controlX1, 0.0f, paintDebug);
//            canvas.drawLine(controlX2, measuredHeight(), controlX2, 0.0f, paintDebug);
//            canvas.drawLine(topX, measuredHeight(), topX, 0.0f, paintDebug);
//            paintDebug.setColor(SkinUtils.getColor(R.color.nine_curve_line_color));
//            canvas.drawLine(controlX3, measuredHeight(), controlX3, 0.0f, paintDebug);
//            canvas.drawLine(controlX4, measuredHeight(), controlX4, 0.0f, paintDebug);
//            canvas.drawLine(startX, measuredHeight(), startX, 0.0f, paintDebugStartEnd);
//            canvas.drawLine(endX, measuredHeight(), endX, 0.0f, paintDebugStartEnd);
            onMyDraw(canvas);
        }
        Log.d(TAG, "onDraw，width: " + getMeasuredWidth() + " height: " + getMeasuredHeight() +
                " startX: " + startX + " controlX1: " + controlX1 + " controlX2: " + controlX2 +
                " topX: " + topX + " controlX3: " + controlX3 + " controlX4: " + controlX4 +
                " endX: " + endX);
        Log.d(TAG, "onDraw，width: " + getMeasuredWidth() + " height: " + getMeasuredHeight() +
                " startY: " + startY + " controlY1: " + controlY1 + " controlY2: " + controlY2 +
                " topY: " + topY + " controlY3: " + controlY3 + " controlY4: " + controlY4 +
                " endY: " + endY);
    }

    protected void onMyDraw(Canvas canvas) {
        int width = measuredWidth() - HLF_HORIZONTAL_PADDING * 2;
        int height = measuredHeight();

        int intervalWidth = width / 3;
        // 绘制 20 - 200 区间
        drawInterval(canvas, 20, 200, 0 + HLF_HORIZONTAL_PADDING, intervalWidth, height);
        // 绘制 200 - 2000 区间
        drawInterval(canvas, 200, 2000, intervalWidth + HLF_HORIZONTAL_PADDING, intervalWidth, height);
        // 绘制 2000 - 20000 区间
        drawInterval(canvas, 2000, 20000, 2 * intervalWidth + HLF_HORIZONTAL_PADDING, intervalWidth, height);
    }

    private void drawInterval(Canvas canvas, int minFreq, int maxFreq, int startX, int width, int height) {
        double logMinFrequency = Math.log10(minFreq);
        double logMaxFrequency = Math.log10(maxFreq);
        double logFrequencyRange = logMaxFrequency - logMinFrequency;

        int step;
        int endFrequency;
        if (minFreq <= 100) {
            step = 10;
            endFrequency = 100;
        } else if (minFreq <= 1000) {
            step = 100;
            endFrequency = 1000;
        } else {
            step = 1000;
            endFrequency = 10000;
        }

        // 在需要绘制竖线的子区间内绘制竖线和标签
        for (int frequency = minFreq; frequency <= Math.min(maxFreq, endFrequency); frequency += step) {
            double logFrequency = Math.log10(frequency);
            float x = (float) ((logFrequency - logMinFrequency) / logFrequencyRange * width) + startX;

            canvas.drawLine(x, 0, x, height, paint);
//            canvas.drawText(frequency + "Hz", x, height - 10, paintDebug);
            Log.d(TAG, "Frequency: " + frequency + "Hz, X coordinate: " + x);
        }

        // 绘制区间最大值对应的竖线和标签
        double logMax = Math.log10(maxFreq);
        float xMax = (float) ((logMax - logMinFrequency) / logFrequencyRange * width) + startX;
        canvas.drawLine(xMax, 0, xMax, height, paint);
        // canvas.drawText(maxFreq + "Hz", xMax, height - 10, paintDebug);
        Log.d(TAG, "Frequency: " + maxFreq + "Hz, X coordinate: " + xMax);
    }

    public int frequencyToX(int frequency) {
        //Log.d(TAG, "开始将频率 " + frequency + " 转换为横坐标");
        // 步骤 1: 确定最小频率、最大频率、宽度和起始横坐标
        int minFreq = freqLeftLimitCommon;
        int maxFreq = freqRightLimitCommon;
        int width = measuredWidth() - HLF_HORIZONTAL_PADDING * 2; // 有效区域需去除padding
        int paddingStart = HLF_HORIZONTAL_PADDING;
//        Log.d(TAG, "最小频率 minFreq: " + minFreq);
//        Log.d(TAG, "最大频率 maxFreq: " + maxFreq);
//        Log.d(TAG, "视图宽度 width: " + width);
//        Log.d(TAG, "起始横坐标 startX: " + startX);

        // 步骤 2: 计算最小频率和最大频率的对数
        double logMinFrequency = Math.log10(minFreq);
        double logMaxFrequency = Math.log10(maxFreq);
        //Log.d(TAG, "最小频率的对数 logMinFrequency: " + logMinFrequency);
        //Log.d(TAG, "最大频率的对数 logMaxFrequency: " + logMaxFrequency);

        // 步骤 3: 计算频率对数范围
        double logFrequencyRange = logMaxFrequency - logMinFrequency;
        //Log.d(TAG, "频率对数范围 logFrequencyRange: " + logFrequencyRange);

        // 步骤 4: 计算输入频率的对数
        double logFrequency = Math.log10(frequency);
        //Log.d(TAG, "输入频率 " + frequency + " 的对数 logFrequency: " + logFrequency);

        // 步骤 5: 计算最终的横坐标
        int x = (int) (((logFrequency - logMinFrequency) / logFrequencyRange * width) + paddingStart);
        //Log.d(TAG, "计算得到的横坐标 x: " + x);

        Log.d(TAG, "频率 " + frequency + " 转换为横坐标完成，结果为: " + x);
        return x;
    }

    public int xToFrequency(float x) {
        int minFreq = (int) freqLeftLimitCommon;
        int maxFreq = (int) freqRightLimitCommon;
        int width = measuredWidth() - HLF_HORIZONTAL_PADDING * 2; // 有效区域需去除padding
        int paddingStart = HLF_HORIZONTAL_PADDING;
        if (x < startX) {
            return minFreq;
        } else if (x > startX + width) {
            return maxFreq;
        }
        double logMinFrequency = Math.log10(minFreq);
        double logMaxFrequency = Math.log10(maxFreq);
        double logFrequencyRange = logMaxFrequency - logMinFrequency;
        double relativeX = (x - paddingStart) / (float) width;
        double logFrequency = relativeX * logFrequencyRange + logMinFrequency;
        return (int) Math.pow(10, logFrequency);
    }

    private float actionDownX;
    @Override
    public boolean onTouchEvent(MotionEvent motionEvent) {
        int action = motionEvent.getAction();
        float x = motionEvent.getX();
        float y = motionEvent.getY();
        if (action == MotionEvent.ACTION_DOWN) {
            actionDownX = x;
            Log.d(TAG, "------------ACTION_DOWN  x = " + x + "  y = " + y);
        } else if (action == MotionEvent.ACTION_UP) {
            if (Math.abs(x - controlX2) > Math.abs(x - controlX3)) {
                controlX3 = x;
            } else if (Math.abs(x - controlX2) < Math.abs(x - controlX3)) {
                controlX2 = x;
            } else if (x > controlX3) {
                controlX3 = x;
            } else {
                controlX2 = x;
            }
            if (controlX2 < leftLimit) {
                controlX2 = leftLimit;
            }
            if (controlX3 > rightLimit) {
                controlX3 = rightLimit;
            }
            freqHigh = xToFrequency(controlX2);
            freqLow = xToFrequency(controlX3);
            touchLineMoveInterface.onLineMoveStop(freqHigh, freqLow);
            Log.d(TAG, "------------ACTION_UP, x = " + x + "  y = " + y + " controlX2= " + controlX2 + " controlX3= " + controlX3 + " freqHigh = " + freqHigh + " freqLow: " + freqLow);
        } else if (action == MotionEvent.ACTION_MOVE) {
            if (false) {
                Log.d(TAG, "ACTION_MOVE, x = " + x + "  y = " + y + " controlX2= " + controlX2 + " controlX3= " + controlX3);
            } else {
                int freq = xToFrequency(x);
                Log.d(TAG, "test xToFrequency freq = " + freq + " pair with x: " + x);
                float testX = frequencyToX(freq);
                Log.d(TAG, "test frequencyToX testX = " + testX + " pair with freq: " + freq);
                if (Math.abs(x - controlX2) > Math.abs(x - controlX3)) {
                    controlX3 = x;
                } else if (Math.abs(x - controlX2) < Math.abs(x - controlX3)) {
                    controlX2 = x;
                } else if (x > controlX3) {
                    controlX3 = x;
                } else {
                    controlX2 = x;
                }
                if (controlX2 < leftLimit) {
                    controlX2 = leftLimit;
                }
                if (controlX3 > rightLimit) {
                    controlX3 = rightLimit;
                }
                freqHigh = xToFrequency(controlX2);
                freqLow = xToFrequency(controlX3);
                touchLineMoveInterface.onLineMove(freqHigh, freqLow);
                Log.d(TAG, "------------ACTION_MOVE, x = " + x + "  y = " + y + " controlX2= " + controlX2 + " controlX3= " + controlX3 + " freqHigh = " + freqHigh + " freqLow: " + freqLow);
            }
        } else if (action == MotionEvent.ACTION_CANCEL) {
            Log.d(TAG, "------------ACTION_CANCEL");
        }
        invalidate();
        return true;
    }

    public void clickHighFreq(boolean z) {
        float step = (measuredWidth() - HLF_HORIZONTAL_PADDING * 2) / 100;
        float newX = controlX2 + (z ? step : -step);
        if (newX < leftLimit) {
            newX = leftLimit;
        }
        if (newX > controlX3) {
            newX = controlX3;
        }
        controlX2 = newX;
        freqHigh = xToFrequency(controlX2);
        if (!z && (freqHigh - 1) <= freqRightLimit && (freqHigh - 1) >= freqLeftLimit) {
            freqHigh--;
        }
        if (z && (freqHigh + 1) <= freqRightLimit && (freqHigh + 1) >= freqLeftLimit) {
            freqHigh++;
        }
        if (freqHigh > freqLow) {
            freqHigh = freqLow;
        }
        touchLineMoveInterface.onLineMove(freqHigh, freqLow);
        touchLineMoveInterface.onLineMoveStop(freqHigh, freqLow);
        invalidate();
    }

    public void clickLowFreq(boolean z) {
        float step = (measuredWidth() - HLF_HORIZONTAL_PADDING * 2) / 100;
        float newX = controlX3 + (z ? step : -step);
        if (newX > rightLimit) {
            newX = rightLimit;
        }
        if (newX < controlX2) {
            newX = controlX2;
        }
        controlX3 = newX;
        freqLow = xToFrequency(controlX3);
        if (!z && (freqLow - 1) >= freqLeftLimit && (freqLow - 1) <= freqRightLimit) {
            freqLow--;
        }
        if (z && (freqLow + 1) >= freqLeftLimit && (freqLow + 1) <= freqRightLimit) {
            freqLow++;
        }
        if (freqLow < freqHigh) {
            freqLow = freqHigh;
        }
        touchLineMoveInterface.onLineMove(freqHigh, freqLow);
        touchLineMoveInterface.onLineMoveStop(freqHigh, freqLow);
        invalidate();
    }

    public void setTouchLineMoveInterface(TouchLineMoveInterface touchLineMoveInterface) {
        this.touchLineMoveInterface = touchLineMoveInterface;
    }

    private int measuredWidth() {
        return (getMeasuredWidth() > 0) ? getMeasuredWidth() : (int) SkinUtils.getDimension(R.dimen.x1580);
    }

    private int measuredHeight() {
        return (getMeasuredHeight() > 0) ? getMeasuredHeight() : (int) SkinUtils.getDimension(R.dimen.y260);
    }
}