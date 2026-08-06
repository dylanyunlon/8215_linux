package com.hcn.autoeq.view.rca;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PointF;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

import com.hcn.autoeq.R;

import java.util.ArrayList;
import java.util.List;

public class RCALineView extends View {
    private String TAG = RCALineView.class.getSimpleName();
    private Paint mPaintRed;
    private Paint mPaintGray;
    private Paint mPaintGrayText;

    //用来触摸屏时控制贝塞尔曲线步进变化的
    private float xTouchDX = 0;
    private float yTouchDY = 0;
    private float xTouchOld = 0;
    private float yTouchOld = 0;
    /**
     * 二倍程规则中13个分隔坐标
     */
    public int FLAG_POINT_COUNT = 13;
    /**
     * 30等份中二倍程规则13个分隔坐标
     */
    private float[][] beiSaiErBasicPoints;
    /**
     * xy轴变化时需要微调的点,1表示要控制，0表示不控制
     */
    public int[] flagCtrlStatePoints;
    private String bsr_music = "";
    private Bitmap mBitmap_music = null;
    /**
     * 获取贝塞尔曲线上的点数
     */
    private int mBsrCountPoint = 13;
    /**
     * 表示单独控制x轴变化
     */
    public static int FLAG_CTRL_X = 0;
    /**
     * 表示单独控制y轴变化
     */
    public static int FLAG_CTRL_Y = 1;
    /**
     * 单独控制x轴或者y轴变化
     */
    public int mXYCtrl = FLAG_CTRL_X;

    /**
     * 喇叭位置编号
     */
    public static int RCA_LR_0 = 0;
    public static int RCA_LR_1 = 1;
    public static int RCA_LR_2 = 2;
    public static int RCA_LR_3 = 3;

    /**
     * 表示当前曲线控制的喇叭位置
     */
    public int mRCAID = RCA_LR_0;
    /**
     * 频率文字绘制起点
     */
    private float[][] text_point;


    /**
     * 直通线 y轴位置
     */
    private float MIN_Y;
    /**
     * view 宽度
     */
    private float width = 0;
    /**
     * view 高度度
     */
    private float height = 0;
    /**
     * y轴4等分，即高等分数
     */
    private int mYCount = 4;
    /**
     * x轴30等分，即宽等分数
     */
    private int mXCount = 30;
    /**
     * 写字频点
     */
    private boolean drawText = true;
    /**
     * 是否要绘制点，调试用
     */
    private boolean isNeedDrawPoint = false;

    /**
     * 每条线对应一组
     */
    public List<RCAData> mRCAPositionDatas = new ArrayList<RCAData>();
    /**
     * 光滑变量，0<SMOOTHNESS<0.5 ,SMOOTHNESS=0表示表示曲线是一条折线，
     * SMOOTHNESS越大数据点两侧越平坦，一般取0.33，越多弧度越大，越小弧度越小--趋于直线
     */
    private float SMOOTHNESS = 0.33f;
    private float SMOOTHNESS_FIRST_LAST = 0.1f;

    /**
     * 当前曲线
     */
    RCAData mCurrentRCA;

    //2分隔的格子等分规则
    float proportionA1 = 0.59f;
    float proportionA2 = 0.41f;

    //5分隔的格子等分规则
    float proportionB1 = 0.266f;
    float proportionB2 = 0.215f;
    float proportionB3 = 0.196f;
    float proportionB4 = 0.165f;
    float proportionB5 = 0.158f;

    //一个大分隔：占30等份中的3份
    float perXBiger;
    //2分隔线距离
    float proportionA1Value;
    //5分隔线距离
    float proportionB1Value;
    float proportionB2Value;
    float proportionB3Value;
    float proportionB4Value;

    /**
     * 一条曲线对应的数据
     */
    public class RCAData {
        public int id;
        public int colorID;
        public Paint mPaint;
        /**
         * 曲线要经过的点,左边区域
         */
        public List<PointF> pointListLeft = new ArrayList<PointF>();
        /**
         * 曲线要经过的点,右边区域
         */
        public List<PointF> pointListRight = new ArrayList<PointF>();
        /**
         * 曲线要经过的点,左边+右边区域
         */
        public List<PointF> pointList = new ArrayList<PointF>();
        /**
         * 贝塞尔曲线经过 pointList 的点对应的控制点
         */
        public List<PointF> mControlPointList = new ArrayList<PointF>();
        /**
         * 高通频率[20,200000)
         */
        public float highRate = 1250;
        /**
         * 高通斜率[0,90)
         */
        public float highSlope = 0;
        /**
         * 低通频率(20,200000]
         */
        public float lowRate = 4000;
        /**
         * 低通斜率(-90,0]
         */
        public float lowSlope = 0;

        public boolean isHide() {
            return isHide;
        }

        public void setHide(boolean hide) {
            isHide = hide;
        }

        private boolean isHide = false;

        public RCAData(int mID, int mColorID) {
            RCAData.this.id = mID;
            RCAData.this.colorID = mColorID;
            mPaint = new Paint(Paint.ANTI_ALIAS_FLAG); // 设置抗锯齿
            mPaint.setColor(getResources().getColor(mColorID)); // 画笔颜色
            mPaint.setStyle(Paint.Style.STROKE); // 设置填充画笔，只画圆边
            mPaint.setStrokeWidth(3);
        }

        public void clearList() {
            pointListLeft.clear();
            pointListRight.clear();
            pointList.clear();
            mControlPointList.clear();
        }

        public void setColorID(int colorID) {
            this.colorID = colorID;
            mPaint.setColor(getResources().getColor(colorID));
        }

        public int getColorID() {
            return colorID;
        }

        public void reset() {
            highRate = 1250;
            highSlope = 0;
            lowRate = 4000;
            lowSlope = 0;
        }
    }

    public RCALineView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public RCALineView(Context context) {
        super(context);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int wMode = MeasureSpec.getMode(widthMeasureSpec);
        int wSize = MeasureSpec.getSize(widthMeasureSpec);
        int hSize = MeasureSpec.getSize(heightMeasureSpec);
        width = wSize;
        height = hSize;
        float mapF50 = fMapToView(50);
        height = (int) (mapF50 - mapF50 * 0.041);
        setMeasuredDimension((int) width, (int) height);
        /*
        //MeasureSpec.EXACTLY:已经指定大小; MeasureSpec.AT_MOST没有指定大小，上限为父view的小
        if(wMode== MeasureSpec.EXACTLY){
            height=(int)(mapF50-mapF50*0.041);
            setMeasuredDimension((int)width,(int)height);
        }else{
            super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        }*/
//        Log.d(TAG, "onMeasure  "
//                + " EXACTLY=" + MeasureSpec.EXACTLY
//                + " AT_MOST=" + MeasureSpec.AT_MOST
//                + " UNSPECIFIED=" + MeasureSpec.UNSPECIFIED
//                + " wMode=" + wMode
//                + " mapF50=" + mapF50
//                + " wSize=" + wSize
//                + " hSize=" + hSize
//                + " width=" + width
//                + " height=" + height
//        );
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        super.onLayout(changed, left, top, right, bottom);
//        Log.d(TAG, "onLayout width=" + width + "  height=" + height);
        initDefaultPoint();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
//        Log.d(TAG, "onDraw width=" + width + "  height=" + height);
        drawBisectorAndValue(canvas);
        if (drawText) {
            //drawText=false;
            //drawFText(canvas);
        }
        drawPSD(canvas);
        drawBisectorAndValueLeftRightBottom(canvas);
    }


    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int action = event.getAction();
        float x = event.getX();
        float y = event.getY();
        switch (action) {
            case MotionEvent.ACTION_DOWN:
                xTouchOld = x;
                yTouchOld = y;
                xTouchDX = 0;
                yTouchDY = 0;
//                Log.d(TAG, "------------ACTION_UP  x =" + x + "  y =" + y);
                break;
            case MotionEvent.ACTION_MOVE:
                xTouchDX = x - xTouchOld;
                yTouchDY = y - yTouchOld;
                xTouchOld = x;
                yTouchOld = y;
                break;
            case MotionEvent.ACTION_UP:
//                Log.d(TAG, "------------ACTION_UP");
                break;
            case MotionEvent.ACTION_CANCEL:
//                Log.d(TAG, "------------ACTION_CANCEL");
                break;
            default:
                break;
        }

        invalidate();
        return true;
    }

    private void init() {
        mPaintRed = new Paint(Paint.ANTI_ALIAS_FLAG); // 设置抗锯齿
        mPaintRed.setColor(getResources().getColor(R.color.redColor)); // 画笔颜色
        mPaintRed.setStyle(Paint.Style.FILL); // 设置填充画笔，只画圆边
        mPaintRed.setStrokeWidth(6);


        mPaintGray = new Paint(Paint.ANTI_ALIAS_FLAG); // 设置抗锯齿
        mPaintGray.setColor(getResources().getColor(R.color.grayColor)); // 画笔颜色
        mPaintGray.setStyle(Paint.Style.STROKE); // 设置填充画笔，只画圆边
        mPaintGray.setStrokeWidth(1.5f);

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
        if (flagCtrlStatePoints == null) {
            flagCtrlStatePoints = new int[FLAG_POINT_COUNT];
        }

        mRCAPositionDatas.add(new RCAData(RCA_LR_0, R.color.warnColor));
        mRCAPositionDatas.add(new RCAData(RCA_LR_1, R.color.greenColor));
        mRCAPositionDatas.add(new RCAData(RCA_LR_2, R.color.blueColor));
        mRCAPositionDatas.add(new RCAData(RCA_LR_3, R.color.pupleColor));
        mCurrentRCA = getRCAPositionData(mRCAID);

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
//            Log.d(TAG, "initMusicDefaultPoint  i=" + i
//                    + " zb=" + zb
//                    + " yushu=" + yushu
//                    + "  x=" + beiSaiErBasicPoints[i][0]);

            beiSaiErBasicPoints[i][1] = 0;
            if (i == FLAG_POINT_COUNT - 1) {
                text_point[i][0] = beiSaiErBasicPoints[i][0] - 40;
            } else if (i == 0) {
                text_point[i][0] = beiSaiErBasicPoints[i][0];
            } else {
                text_point[i][0] = beiSaiErBasicPoints[i][0] - 20;
            }
            text_point[i][1] = height - 4;
        }

        perXBiger = width / mXCount * 3f;
        //计算2分格线距离
        proportionA1Value = perXBiger * proportionA1;
        //计算5分格线距离
        proportionB1Value = perXBiger * proportionB1;
        proportionB2Value = perXBiger * (proportionB1 + proportionB2);
        proportionB3Value = perXBiger * (proportionB1 + proportionB2 + proportionB3);
        proportionB4Value = perXBiger * (proportionB1 + proportionB2 + proportionB3 + proportionB4);

//        Log.i(TAG, "initDefaultPoint "
//                + " mBsrCountPoint=" + mBsrCountPoint
//                + " FLAG_POINT_COUNT=" + FLAG_POINT_COUNT
//                + " MIN_Y=" + MIN_Y
//                + " stepX=" + stepX
//                + " stepY=" + stepY
//
//
//                + " perXBiger=" + perXBiger
//                + " proportionA1Value=" + proportionA1Value
//                + " proportionB1Value=" + proportionB1Value
//                + " proportionB2Value=" + proportionB2Value
//                + " proportionB3Value=" + proportionB3Value
//                + " proportionB4Value=" + proportionB4Value
//        );

    }


    /**
     * 获取高通（左边），低通（右边）的点，后续根据这些点绘制曲线，
     * 并且这条曲线经过这些点，后续还要根据具体情况，对列表的点添加删除调整，以便绘制圆滑的曲线
     *
     * @param canvas
     * @param w1     MIN_Y*0.3 + MIN_Y
     * @param f1     高通/低通当前的频率
     * @param n      类似角度，<0低通；>0高通； 0不进来
     */
    private void makePSDLinePoints(Canvas canvas, RCAData mRCAPositionData, float w1, float f1, double n) {

        if (mRCAPositionData == null) {
//            Log.d(TAG, "drawPSDLine bad arg"
//                    + " w1=" + w1
//                    + " f1=" + f1
//                    + " n=" + n
//                    + " mRCAID=" + mRCAID
//            );
            return;
        }
        int countPoint = 0;
        float rX = 0;
        float rY = 0;
        //中间点
        float rXFromF1 = 0;
        float rYFromW1 = 0;

        rX = fMapToView(f1);//考虑  30 分 --倍程转化
        rY = (float) wMapToView(w1);
        rXFromF1 = rX;
        rYFromW1 = rY;
//        Log.i(TAG, "------------drawPSDLine"
//                + " w1=" + w1
//                + " f1=" + f1
//                + " n=" + n
//        );

        Path path = new Path();
        if (n < 0) {//降谱--右边区域（f，20Hz）
            double radians = 0;
            int dx = 0;
            double tn = Math.abs(n);
            if (tn > 6) {
                //不要这些上交点了,太近了不利于圆弧的绘制，但是左边是直通的时候需要
            } else {
                //上交点,这里分频斜率是6的时候不要映射了，直接用分频斜率n
                PointF mPointF = getTopPoint(n, f1);
                mRCAPositionData.pointList.add(mPointF);//上交点
                mRCAPositionData.pointListRight.add(mPointF);//上交点
                countPoint++;

                path.moveTo(mPointF.x, mPointF.y);// 此点为起点
                drawPointWithFlag(canvas, mPointF.x, mPointF.y, mPaintRed);

//                Log.d(TAG, "drawPSDLine 上交点"
//                        + " countPoint=" + countPoint
//                        + " mPointF.x=" + mPointF.x
//                        + " mPointF.y=" + mPointF.y
//                );
            }

            //中间点
            mRCAPositionData.pointList.add(new PointF(rXFromF1, rYFromW1));
            mRCAPositionData.pointListRight.add(new PointF(rXFromF1, rYFromW1));
            countPoint++;
            path.lineTo(rXFromF1, rYFromW1);
            drawPointWithFlag(canvas, rXFromF1, rYFromW1, mPaintRed);
//            Log.d(TAG, "drawPSDLine 中交点 "
//                    + " countPoint=" + countPoint
//                    + " rXFromF1=" + rXFromF1
//                    + " rYFromW1=" + rYFromW1
//            );

            //计算下交点--mapAngleByFnRight 用界外交点时对过角度
            double angle = mapAngleByFnRight(f1, n);
            radians = Math.toRadians(angle);//要计算tan先将角度转为弧度PI,180度为1个PI
            dx = (int) ((MIN_Y * 2 + MIN_Y * 0.7) / Math.tan(radians));
            dx = Math.abs(dx);
            if (dx > (width - rXFromF1)) {
                /*
                 //用view界内的点
                int dy=(int)((width - rXFromF1)*Math.tan(radians));
                dy=Math.abs(dy);
                rX=width;
                rY=rYFromW1+ dy;
                */
                //用view界外的点--mapAngleByFnRight
                rX = dx + rXFromF1;
                rY = height;
            } else {
                /*
                //用view界内的点
                rX=dx;
                rY=height;
                */
                //用view界外的点--用mapAngleByFnRight
                int dy = (int) ((width - rXFromF1) * Math.tan(radians));
                dy = Math.abs(dy);
                rX = width;
                rY = rYFromW1 + dy;
            }

            //下交点
            mRCAPositionData.pointList.add(new PointF(rX, rY));//下交点
            mRCAPositionData.pointListRight.add(new PointF(rX, rY));//下交点
            countPoint++;
            path.lineTo(rX, rY);
            drawPointWithFlag(canvas, rX, rY, mPaintRed);
//            Log.d(TAG, "drawPSDLine 下交点"
//                    + " countPoint=" + countPoint
//                    + " dx=" + dx
//                    + " rX=" + rX
//                    + " rY=" + rY
//            );
        } else if (n > 0) {//升谱--左边区域（20，f）
            double radians = 0;
            int dx = 0;
            if (n > 6) {
                //不要这些上交点了,太近了不利于圆弧的绘制，但是右边边是直通的时候需要
            } else {
                //这里分频斜率是6的时候不要映射了，直接用分频斜率n
                PointF mPointF = getTopPoint(n, f1);
                mRCAPositionData.pointList.add(mPointF);//上交点
                mRCAPositionData.pointListLeft.add(mPointF); //上交点
                path.moveTo(mPointF.x, mPointF.y);//此点为起点
                drawPointWithFlag(canvas, mPointF.x, mPointF.y, mPaintRed);
                countPoint++;
//                Log.d(TAG, "drawPSDLine 上交点"
//                        + " countPoint=" + countPoint
//                        + " mPointF.x=" + mPointF.x
//                        + " mPointF.y=" + mPointF.y
//                );
            }

            //中间点--注意在上交点前
            mRCAPositionData.pointList.add(0, new PointF(rXFromF1, rYFromW1));
            mRCAPositionData.pointListLeft.add(0, new PointF(rXFromF1, rYFromW1));
            countPoint++;
            path.lineTo(rXFromF1, rYFromW1);
            drawPointWithFlag(canvas, rXFromF1, rYFromW1, mPaintRed);
//            Log.d(TAG, "drawPSDLine 中交点 "
//                    + " countPoint=" + countPoint
//                    + " rXFromF1=" + rXFromF1
//                    + " rYFromW1=" + rYFromW1
//            );

            //0~90--根据频率和分频斜率映射一个角度，根据样机图片图形状态获取的,mapAngleByFnLeft部分条件对过使用界内交点
            double angle = mapAngleByFnLeft(f1, n);
            //直角三角形勾股定理,计算下交点
            radians = Math.toRadians(Math.abs(angle));//要计算tan先将角度PI,180度为1个PI
            dx = (int) ((MIN_Y * 2 + MIN_Y * 0.7) / Math.tan(radians));
            if (dx > (rXFromF1)) {
                 /*
                //用view界内的点--用mapAngleByFnLeft
                rX=0;
                rY=(int)(rYFromW1+ (rXFromF1)*Math.tan(radians));
                 */
                //用view界外的点
                rX = rXFromF1 - dx;
                rY = height;
            } else {
                /*
                //用view界内的点--会影响映射角的设定--界内和界外--映射角取值有小幅度差异--用mapAngleByFnLeft
                rX=rXFromF1-dx;
                rY=height;
                */
                //用view界外的点--会影响映射角的设定
                rX = 0;
                rY = (int) (rYFromW1 + (rXFromF1) * Math.tan(radians));

            }

            //下交点--注意在中交点前
            mRCAPositionData.pointList.add(0, new PointF(rX, rY));//下交点
            mRCAPositionData.pointListLeft.add(0, new PointF(rX, rY));//下交点
            countPoint++;
            path.lineTo(rX, rY);
            drawPointWithFlag(canvas, rX, rY, mPaintRed);
//            Log.d(TAG, "drawPSDLine 下交点"
//                    + " countPoint=" + countPoint
//                    + " dx=" + dx
//                    + " rX=" + rX
//                    + " rY=" + rY
//            );
        }
        //canvas.drawPath(path, mRCAPositionData.mPaint);
    }

    /**
     * 思考--斜率可以根据：6--4。12--4*0.5，24--4*0.5*0.5 。。。。。。
     * 获取上交点
     *
     * @param n  分频斜率 6,12,24,36,48
     * @param f1 即当前频率映射在view 的x轴的位置
     * @return
     */
    private PointF getTopPoint(double n, float f1) {
        //考虑30等份--倍程转化--映射在view 的x轴的位置
        int rXFromF1 = (int) fMapToView(f1);
        double radians = 0;
        float dx = 0;
        PointF mPointF;
        float rX = 0;
        float rY = 0;
        rY = MIN_Y;
        //计算上交点，n为负数，radians也为负数，dx也为负数
        //tan需要的是弧度参数，先将角度转化为弧度（180度为1个PI），这里不要映射了，直接用分频斜率n
        //radians = Math.toRadians(n);
        radians = Math.toRadians(n / 2);//斜一点会圆滑一些,值越小，越斜
        dx = (float) ((MIN_Y * 0.3) / Math.tan(radians));
        rX = rXFromF1 + dx;
        mPointF = new PointF(rX, rY);
        return mPointF;
    }


    /**
     * 将频率映射到 2倍程规则的界面上
     *
     * @param f 频率
     * @return view上的坐标点
     */
    private float fMapToView(float f) {
        float xView = 0;
        float perViewW = width / mXCount;
        if (f > 50) {
            if (f > 50000) {
                if (f > 40000) {
                    xView = perViewW * 43 + ((f - 400000) / 100000 * perViewW);
                } else if (f > 20000) {
                    xView = perViewW * 40 + ((f - 200000) / 200000 * perViewW * 3);
                } else if (f > 10000) {
                    xView = perViewW * 37 + ((f - 100000) / 100000 * perViewW * 3);
                } else {
                    xView = perViewW * 34 + ((f - 50000) / 50000 * perViewW * 3);
                }
            } else if (f > 5000) {
                if (f > 40000) {
                    xView = perViewW * 33 + ((f - 40000) / 10000 * perViewW);
                } else if (f > 20000) {
                    xView = perViewW * 30 + ((f - 20000) / 20000 * perViewW * 3);
                } else if (f > 10000) {
                    xView = perViewW * 27 + ((f - 10000) / 10000 * perViewW * 3);
                } else {
                    xView = perViewW * 24 + ((f - 5000) / 5000 * perViewW * 3);
                }
            } else if (f > 500) {
                if (f > 4000) {
                    xView = perViewW * 23 + ((f - 4000) / 1000 * perViewW);
                } else if (f > 2000) {
                    xView = perViewW * 20 + ((f - 2000) / 2000 * perViewW * 3);
                } else if (f > 1000) {
                    xView = perViewW * 17 + ((f - 1000) / 1000 * perViewW * 3);
                } else {
                    xView = perViewW * 14 + ((f - 500) / 500 * perViewW * 3);
                }

            } else {
                if (f > 400) {
                    xView = perViewW * 13 + ((f - 400) / 100 * perViewW);
                } else if (f > 200) {
                    xView = perViewW * 10 + ((f - 200) / 200 * perViewW * 3);
                } else if (f > 100) {
                    xView = perViewW * 7 + ((f - 100) / 100 * perViewW * 3);
                } else {
                    xView = perViewW * 4 + ((f - 50) / 50 * perViewW * 3);
                }
            }
        } else {
            if (f > 40) {
                xView = perViewW * 3 + ((f - 40) / 10) * perViewW;
            } else {
                xView = ((f - 20) / 20) * perViewW * 3;
            }
        }

        return xView;
    }

    /**
     * 将Y轴映射 功率谱密度 界面上
     */
    private double wMapToView(double w) {
        double wR = w + MIN_Y;
        return wR;
    }

    /**
     * 根据曲线id获取曲线对象
     *
     * @param id
     * @return
     */
    public RCAData getRCAPositionData(int id) {
        for (int i = 0; i < mRCAPositionDatas.size(); i++) {
            if (mRCAPositionDatas.get(i).id == id) {
                return mRCAPositionDatas.get(i);
            }
        }
        return null;
    }
    //--------psd end

    /**
     * 经过多点的曲线
     * 根据曲线要经过的点计算对应贝塞尔曲线控制点
     *
     * @param mRCAPositionData
     * @return
     */
    private boolean calculateControlPoint(RCAData mRCAPositionData) {
        int size = mRCAPositionData.pointList.size();
        if (size <= 1) {
            Log.d(TAG, "calculateControlPoint return");
            return false;
        }
        PointF point;
        for (int i = 0; i < size; i++) {
            point = mRCAPositionData.pointList.get(i);
            if (i == 0) {
                //第一项---可以考虑弧度问题-SMOOTHNESS 大小
                //添加后控制点
                PointF nextPoint = mRCAPositionData.pointList.get(i + 1);
                float controlX = point.x + (nextPoint.x - point.x) * SMOOTHNESS_FIRST_LAST;
                float controlY = point.y;
                mRCAPositionData.mControlPointList.add(new PointF(controlX, controlY));
            } else if (i == size - 1) {
                //最后一项---可以考虑弧度问题-SMOOTHNESS 大小
                //添加前控制点
                PointF lastPoint = mRCAPositionData.pointList.get(i - 1);
                float controlX = point.x - (point.x - lastPoint.x) * SMOOTHNESS_FIRST_LAST;
                float controlY = point.y;
                mRCAPositionData.mControlPointList.add(new PointF(controlX, controlY));
            } else {
                //中间项
                PointF lastPoint = mRCAPositionData.pointList.get(i - 1);
                PointF nextPoint = mRCAPositionData.pointList.get(i + 1);
                float k = (nextPoint.y - lastPoint.y) / (nextPoint.x - lastPoint.x);
                float b = point.y - k * point.x;
                //添加前控制点
                float lastControlX = point.x - (point.x - lastPoint.x) * SMOOTHNESS;
                float lastControlY = k * lastControlX + b;
                mRCAPositionData.mControlPointList.add(new PointF(lastControlX, lastControlY));
                //添加后控制点
                float nextControlX = point.x + (nextPoint.x - point.x) * SMOOTHNESS;
                if (nextPoint.y == lastPoint.y && nextPoint.y == MIN_Y) {
                    nextControlX = point.x + (nextPoint.x - point.x) * SMOOTHNESS_FIRST_LAST;
                }
                float nextControlY = k * nextControlX + b;
                mRCAPositionData.mControlPointList.add(new PointF(nextControlX, nextControlY));
            }
        }
        return true;
    }

    /**
     * 绘制经过多点的曲线
     *
     * @param canvas
     */
    private void drawPointWithLine(Canvas canvas, RCAData mRCAPositionData) {
        boolean result = calculateControlPoint(mRCAPositionData);
        if (result) {
            Path path = new Path();
            path.moveTo(mRCAPositionData.pointList.get(0).x, mRCAPositionData.pointList.get(0).y);
            for (int i = 0; i < (mRCAPositionData.pointList.size() * 2) - 2; i += 2) {
                PointF leftControlPoint = mRCAPositionData.mControlPointList.get(i);
                PointF rightControlPoint = mRCAPositionData.mControlPointList.get(i + 1);
                PointF rightPoint = mRCAPositionData.pointList.get(i / 2 + 1);
                path.cubicTo(leftControlPoint.x, leftControlPoint.y,
                        rightControlPoint.x, rightControlPoint.y,
                        rightPoint.x, rightPoint.y);
            }
            canvas.drawPath(path, mRCAPositionData.mPaint);
        }
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
        for (int i = 0; i < FLAG_POINT_COUNT; i++) {
            if (i == 0) {
                f = 20;
            } else {
                zb = i / 4;
                yushu = i % 4;
                if (yushu == 1) {
                    f = 40 * Math.pow(10 * 1.0, zb * 1.0);//10的zb次方
                } else if (yushu == 2) {
                    f = 50 * Math.pow(10 * 1.0, zb * 1.0);
                } else if (yushu == 3) {
                    f = 100 * Math.pow(10 * 1.0, zb * 1.0);
                } else if (yushu == 0) {
                    f = 200 * Math.pow(10 * 1.0, (zb - 1) * 1.0);
                }
            }
            if (f >= 1000) {
                txt = String.valueOf((f / 1000)) + "kHz";
            } else {
                txt = String.valueOf((int) f) + "Hz";
            }
            //Log.i(TAG, "drawFText  i=" + i + " txt=" + txt);
            //绘制频率
            canvas.drawText(txt, text_point[i][0], text_point[i][1], mPaintGrayText);

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

        //Y轴方向绘制等分点数字标识
        for (int i = 0; i < 5; i++) {
            if (i == 0) {
                canvas.drawText(String.valueOf(10 - i * 10), 0, MIN_Y * i + 10, mPaintGrayText);
            } else if (i == 4) {
                canvas.drawText(String.valueOf(10 - i * 10), 0, MIN_Y * i - 4, mPaintGrayText);
            } else {
                canvas.drawText(String.valueOf(10 - i * 10), 0, MIN_Y * i, mPaintGrayText);
            }
        }

    }

    /**
     * 绘制等分线
     *
     * @param canvas
     */
    private void drawBisectorAndValue(Canvas canvas) {
        //Y轴方向绘制等分线
        for (int i = 0; i < 5; i++) {
            if (i == 0) {
                //处理首根线有一半显示不全--y轴往下偏移一个像素
                canvas.drawLine(0, MIN_Y * i + 1, width, MIN_Y * i + 1, mPaintGray);
            } else if (i == 4) {
                //处理尾部线有一半显示不全--y轴往上偏移一个像素
                canvas.drawLine(0, MIN_Y * i - 1, width, MIN_Y * i - 1, mPaintGray);
            } else {
                canvas.drawLine(0, MIN_Y * i, width, MIN_Y * i, mPaintGray);
            }
        }

        //x轴方向绘制分格线
        for (int i = 0; i < beiSaiErBasicPoints.length; i++) {
            //30格4种类型区域 13个分隔线
            if (i == 0) {
                //处理首根线有一半显示不全--x轴往前偏移一个像素
                canvas.drawLine(beiSaiErBasicPoints[i][0] + 1, 0, beiSaiErBasicPoints[i][0] + 1, height, mPaintGray);
            } else if (i == beiSaiErBasicPoints.length - 1) {
                //处理尾部线有一半显示不全--x轴往回偏移一个像素
                canvas.drawLine(beiSaiErBasicPoints[i][0] - 1, 0, beiSaiErBasicPoints[i][0] - 1, height, mPaintGray);
            } else {
                canvas.drawLine(beiSaiErBasicPoints[i][0], 0, beiSaiErBasicPoints[i][0], height, mPaintGray);
            }


            if (i == 0) {//第0个刻度线开始20Hz处开始
                //2分隔
                canvas.drawLine(beiSaiErBasicPoints[i][0] + proportionA1Value, 0, beiSaiErBasicPoints[i][0] + proportionA1Value, height, mPaintGray);
            } else {//第1个刻度线开始40Hz处开始，规律二倍程，4*(10^n)，5*(10^n)
                int zb = i / 4;
                int yushu = i % 4;
                if (yushu == 1) {
                    //1分隔
                } else if (yushu == 2) {
                    //5分隔
                    canvas.drawLine(beiSaiErBasicPoints[i][0] + proportionB1Value, 0, beiSaiErBasicPoints[i][0] + proportionB1Value, height, mPaintGray);
                    canvas.drawLine(beiSaiErBasicPoints[i][0] + proportionB2Value, 0, beiSaiErBasicPoints[i][0] + proportionB2Value, height, mPaintGray);
                    canvas.drawLine(beiSaiErBasicPoints[i][0] + proportionB3Value, 0, beiSaiErBasicPoints[i][0] + proportionB3Value, height, mPaintGray);
                    canvas.drawLine(beiSaiErBasicPoints[i][0] + proportionB4Value, 0, beiSaiErBasicPoints[i][0] + proportionB4Value, height, mPaintGray);

                } else if (yushu == 3) {
                    //1分隔
                } else if (yushu == 0) {
                    //2分隔
                    canvas.drawLine(beiSaiErBasicPoints[i][0] + proportionA1Value, 0, beiSaiErBasicPoints[i][0] + proportionA1Value, height, mPaintGray);
                }
            }
        }
    }


    private void drawBisectorAndValueLeftRightBottom(Canvas canvas) {
        //Y轴方向绘制等分线
        for (int i = 0; i < 5; i++) {
            if (i == 0) {
                //处理首根线有一半显示不全--y轴往下偏移一个像素
                //canvas.drawLine(0, MIN_Y*i+1,width, MIN_Y*i+1, mPaintGray);
            } else if (i == 4) {
                //处理尾部线有一半显示不全--y轴往上偏移一个像素
                canvas.drawLine(0, MIN_Y * i - 1, width, MIN_Y * i - 1, mPaintGray);
            } else {
                //canvas.drawLine(0, MIN_Y*i,width, MIN_Y*i, mPaintGray);
            }
        }

        //x轴方向绘制分格线
        for (int i = 0; i < beiSaiErBasicPoints.length; i++) {
            //30格4种类型区域 13个分隔线
            if (i == 0) {
                //处理首根线有一半显示不全--x轴往前偏移一个像素
                canvas.drawLine(beiSaiErBasicPoints[i][0] + 1, 0, beiSaiErBasicPoints[i][0] + 1, height, mPaintGray);
            } else if (i == beiSaiErBasicPoints.length - 1) {
                //处理尾部线有一半显示不全--x轴往回偏移一个像素
                canvas.drawLine(beiSaiErBasicPoints[i][0] - 1, 0, beiSaiErBasicPoints[i][0] - 1, height, mPaintGray);
            } else {
                //canvas.drawLine(beiSaiErBasicPoints[i][0], 0, beiSaiErBasicPoints[i][0], height, mPaintGray);
            }
        }
    }

    /**
     * 打印曲线经过的点相关坐标
     *
     * @param mRCAPositionData
     */
    private void logList(RCAData mRCAPositionData) {
        float x = 0;
        float y = 0;
        int size = mRCAPositionData.pointList.size();
//        Log.i(TAG, "----------------drawPSDW pointList size=" + size);
        for (int i = 0; i < size; i++) {
            x = mRCAPositionData.pointList.get(i).x;
            y = mRCAPositionData.pointList.get(i).y;
//            Log.i(TAG, "drawPSDW pointList"
//                    + " i=" + i
//                    + " x=" + x
//                    + " y=" + y
//            );
        }
        size = mRCAPositionData.pointListLeft.size();
//        Log.i(TAG, "----------------drawPSDW pointListLeft size=" + size);
        for (int i = 0; i < mRCAPositionData.pointListLeft.size(); i++) {
            x = mRCAPositionData.pointListLeft.get(i).x;
            y = mRCAPositionData.pointListLeft.get(i).y;
//            Log.i(TAG, "drawPSDW pointListLeft"
//                    + " i=" + i
//                    + " x=" + x
//                    + " y=" + y
//            );
        }

        size = mRCAPositionData.pointListRight.size();
//        Log.i(TAG, "----------------drawPSDW pointListRight size=" + size);
        for (int i = 0; i < mRCAPositionData.pointListRight.size(); i++) {
            x = mRCAPositionData.pointListRight.get(i).x;
            y = mRCAPositionData.pointListRight.get(i).y;
//            Log.i(TAG, "drawPSDW pointListRight"
//                    + " i=" + i
//                    + " x=" + x
//                    + " y=" + y
//            );
        }
    }

    /**
     * 绘制点，即曲线经过的点，可开启关闭，方便调试用
     *
     * @param canvas
     * @param x
     * @param y
     * @param paint
     */
    private void drawPointWithFlag(Canvas canvas, float x, float y, Paint paint) {
        if (isNeedDrawPoint && canvas != null) {
            canvas.drawPoint(x, y, mPaintRed);
        } else {
//            Log.d(TAG, "drawPointWithFlag not do  isNeedDrawPoint=" + isNeedDrawPoint);
        }
    }


    /**
     * 根据多个点绘制线条
     *
     * @param canvas
     * @param mRCAPositionData
     */
    private void drawPSDWithPoints(Canvas canvas, RCAData mRCAPositionData) {
        float fLeft = mRCAPositionData.highRate;
        double nLeft = mRCAPositionData.highSlope;
        float fRight = mRCAPositionData.lowRate;
        double nRight = mRCAPositionData.lowSlope;
        float wLeft = (MIN_Y * 0.3f);
        float wRight = (MIN_Y * 0.3f);

        float xL = fMapToView(fLeft);
        float xR = fMapToView(fRight);


        if (fLeft >= fRight
                || nLeft < 0
                || nRight > 0) {
//            Log.w(TAG, "drawPSDWithPoints a bad arg return"
//                    + " fLeft=" + fLeft
//                    + " nLeft=" + nLeft
//                    + " fRight=" + fRight
//                    + " nRight=" + nRight
//                    + " mRCAPosition=" + mRCAID
//            );
            return;
        }
        if (mRCAPositionData == null) {
//            Log.i(TAG, "drawPSDWithPoints b bad arg return"
//                    + " wLeft=" + wLeft
//                    + " fLeft=" + fLeft
//                    + " nLeft=" + nLeft
//                    + " wRight=" + wRight
//                    + " fRight=" + fRight
//                    + " nRight=" + nRight
//                    + " mRCAPosition=" + mRCAID
//            );
            return;
        }
//        Log.i(TAG, "------------drawPSDWithPoints "
//                + " wLeft=" + wLeft
//                + " fLeft=" + fLeft
//                + " nLeft=" + nLeft
//                + " wRight=" + wRight
//                + " fRight=" + fRight
//                + " nRight=" + nRight
//                + " mRCAPosition=" + mRCAID
//                + " xL=" + xL + " xR=" + xR
//        );
        mRCAPositionData.clearList();
        if (nLeft == 0 && nRight == 0) {
            //高通直通+低通直通
            if (xL == 0) {
                //高通是直通，并且在20Hz，只加一个点
                mRCAPositionData.pointList.add(new PointF(0, MIN_Y));
                mRCAPositionData.pointListLeft.add(new PointF(0, MIN_Y));
                drawPointWithFlag(canvas, 0, MIN_Y, mPaintRed);
//                Log.i(TAG, "drawPSDWithPoints A add pointListLeft"
//                        + " x=0"
//                        + " MIN_Y=" + MIN_Y
//                );
            } else {
                //高通是直通，>20Hz 位置，加3个点，但是，要考虑，低通的上焦点距离太近的时候，适当去掉x，甚至x/2这两个点
                mRCAPositionData.pointList.add(new PointF(0, MIN_Y));
                mRCAPositionData.pointListLeft.add(new PointF(0, MIN_Y));
                drawPointWithFlag(canvas, 0, MIN_Y, mPaintRed);
//                Log.i(TAG, "drawPSDWithPoints p1 add pointListLeft"
//                        + " x=0"
//                        + " MIN_Y=" + MIN_Y
//                );
                float tx = xL / 2;
                mRCAPositionData.pointList.add(new PointF(tx, MIN_Y));
                mRCAPositionData.pointListLeft.add(new PointF(tx, MIN_Y));
                drawPointWithFlag(canvas, tx, MIN_Y, mPaintRed);
//                Log.i(TAG, "drawPSDWithPoints p2 add pointListLeft"
//                        + " x=" + tx
//                        + " MIN_Y=" + MIN_Y
//                );

                mRCAPositionData.pointList.add(new PointF(xL, MIN_Y));
                mRCAPositionData.pointListLeft.add(new PointF(xL, MIN_Y));
                drawPointWithFlag(canvas, xL, MIN_Y, mPaintRed);
//                Log.i(TAG, "drawPSDWithPoints p3 add pointListLeft"
//                        + " xL=" + xL
//                        + " MIN_Y=" + MIN_Y
//                );
            }

            if (xR == width) {
                if (xL == 0) {
                    mRCAPositionData.pointList.add(new PointF(xR / 2, MIN_Y));
                    mRCAPositionData.pointListRight.add(new PointF(xR / 2, MIN_Y));
                    drawPointWithFlag(canvas, xR / 2, MIN_Y, mPaintRed);
//                    Log.i(TAG, "drawPSDWithPoints A1 add pointListRight"
//                            + " xR/2=" + xR / 2
//                            + " MIN_Y=" + MIN_Y
//                    );
                }
                mRCAPositionData.pointList.add(new PointF(xR, MIN_Y));
                mRCAPositionData.pointListRight.add(new PointF(xR, MIN_Y));
                drawPointWithFlag(canvas, xR, MIN_Y, mPaintRed);
//                Log.i(TAG, "drawPSDWithPoints A2 add pointListRight"
//                        + " xR=" + xR
//                        + " MIN_Y=" + MIN_Y
//                );
            } else {
                mRCAPositionData.pointList.add(new PointF(xR, MIN_Y));
                mRCAPositionData.pointListRight.add(new PointF(xR, MIN_Y));
                drawPointWithFlag(canvas, xR, MIN_Y, mPaintRed);
//                Log.i(TAG, "drawPSDWithPoints p1 add pointListRight"
//                        + " x=" + xR
//                        + " MIN_Y=" + MIN_Y
//                );

                float tx = xR + (width - xR) / 2;
                mRCAPositionData.pointList.add(new PointF(tx, MIN_Y));
                mRCAPositionData.pointListRight.add(new PointF(tx, MIN_Y));
                drawPointWithFlag(canvas, tx, MIN_Y, mPaintRed);
//                Log.i(TAG, "drawPSDWithPoints p2 add pointListRight"
//                        + " tx=" + tx
//                        + " MIN_Y=" + MIN_Y
//                );
                mRCAPositionData.pointList.add(new PointF(width, MIN_Y));
                mRCAPositionData.pointListRight.add(new PointF(width, MIN_Y));
                drawPointWithFlag(canvas, width, MIN_Y, mPaintRed);
//                Log.i(TAG, "drawPSDWithPoints p3 add pointListRight"
//                        + " width=" + width
//                        + " MIN_Y=" + MIN_Y
//                );
            }
        } else if (nLeft != 0 && nRight == 0) {
            //高通非直通+低通直通
            makePSDLinePoints(canvas, mRCAPositionData, wLeft, fLeft, nLeft);
            /*如果右边低通直通，并且左边斜率大于6的，makePSDLinePoints没有计算上交点的，
              要补充上交点，防止上供，注意要在makePSDLinePoints 之后调用，保证点在列表中低到高*/
            //列表中无上交点，考虑需要补充上交点
            double angle = nLeft;
            PointF mPointF = getTopPoint(angle, fLeft);
            if (mPointF.x < xR) {
                /*
                if(mPointF.x+(width/30*1.5)<xR){//两点间大于1.5个单元格（view宽度等分30个单元格）
                    //取高通上交点 ，并且不取低通中交点
                    mRCAPositionData.pointList.add(mPointF);
                    mRCAPositionData.pointListLeft.add(mPointF);
                    drawPointWithFlag(canvas,mPointF.x, mPointF.y, mPaintRed);
                    Log.i(TAG, "drawPSDWithPoints add TopPoint pointListLeft"
                            + " x=" + mPointF.x
                            + " y=" + mPointF.y
                    );
                }else{
                    //取低通中交点,并且不取高通上交点
                    mRCAPositionData.pointList.add(new PointF(xR, MIN_Y));
                    mRCAPositionData.pointListRight.add(new PointF(xR, MIN_Y));
                    drawPointWithFlag(canvas,xR, MIN_Y, mPaintRed);
                    Log.i(TAG, "drawPSDWithPoints p1 add pointListRight"
                            + " xR=" + xR
                            + " MIN_Y=" + MIN_Y
                    );
                }
                */
                if (nLeft > 6) {
                    //取高通上交点
                    mRCAPositionData.pointList.add(mPointF);
                    mRCAPositionData.pointListLeft.add(mPointF);
                    drawPointWithFlag(canvas, mPointF.x, mPointF.y, mPaintRed);
//                    Log.i(TAG, "drawPSDWithPoints add TopPoint pointListLeft"
//                            + " x=" + mPointF.x
//                            + " y=" + mPointF.y
//                    );
                } else {
                    //makePSDLinePoints 已取高通上交点
                }

                //取低通中交点
                if (xR != width) {
                    mRCAPositionData.pointList.add(new PointF(xR, MIN_Y));
                    mRCAPositionData.pointListRight.add(new PointF(xR, MIN_Y));
                    drawPointWithFlag(canvas, xR, MIN_Y, mPaintRed);
//                    Log.i(TAG, "drawPSDWithPoints low mid add pointListRight"
//                            + " xR=" + xR
//                            + " MIN_Y=" + MIN_Y);
                }
                mRCAPositionData.pointList.add(new PointF(width, MIN_Y));
                mRCAPositionData.pointListRight.add(new PointF(width, MIN_Y));
                drawPointWithFlag(canvas, width, MIN_Y, mPaintRed);
//                Log.i(TAG, "drawPSDWithPoints low end add pointListRight"
//                        + " width=" + width
//                        + " MIN_Y=" + MIN_Y
//                );
            } else if (mPointF.x < width) {
                if (nLeft > 6) {
                    //取高通上交点 ，并且不取低通中交点
                    mRCAPositionData.pointList.add(mPointF);
                    mRCAPositionData.pointListLeft.add(mPointF);
                    drawPointWithFlag(canvas, mPointF.x, mPointF.y, mPaintRed);
//                    Log.i(TAG, "drawPSDWithPoints a add  TopPoint pointListLeft"
//                            + " x=" + mPointF.x
//                            + " y=" + mPointF.y
//                    );
                } else {
                    //makePSDLinePoints 已取高通上交点
                }
                //取低通末尾交点,并且不取高通上交点
                mRCAPositionData.pointList.add(new PointF(width, MIN_Y));
                mRCAPositionData.pointListRight.add(new PointF(width, MIN_Y));
                drawPointWithFlag(canvas, width, MIN_Y, mPaintRed);
//                Log.i(TAG, "drawPSDWithPoints b add pointListRight"
//                        + " width=" + width
//                        + " MIN_Y=" + MIN_Y
//                );
            } else {
                if (nLeft > 6) {
                    //取高通上交点   mPointF.x >= width
                    mRCAPositionData.pointList.add(mPointF);
                    mRCAPositionData.pointListLeft.add(mPointF);
                    drawPointWithFlag(canvas, mPointF.x, mPointF.y, mPaintRed);
//                    Log.i(TAG, "drawPSDWithPoints c add TopPoint pointListLeft"
//                            + " x=" + mPointF.x
//                            + " y=" + mPointF.y
//                    );
                } else {
                    //makePSDLinePoints 已取高通上交点
                }
            }

        } else if (nLeft == 0 && nRight != 0) {
            //高通直通+低通非直通
            double angle = nRight;
            //低通上交点
            PointF mPointF = getTopPoint(angle, fRight);
            if (xL == 0) {
                if (mPointF.x <= xL) {
                    //低通上交点<0,并且高通当前频率是20Hz(最左边)，高通的点都不取,低通取上中下交点
                } else {
                    //高通是直通，并且在20Hz，并且低通上交点大于高通中交点，高通只加一个点
                    mRCAPositionData.pointList.add(new PointF(0, MIN_Y));
                    mRCAPositionData.pointListLeft.add(new PointF(0, MIN_Y));
                    drawPointWithFlag(canvas, 0, MIN_Y, mPaintRed);
//                    Log.i(TAG, "drawPSDWithPoints d add pointListLeft"
//                            + " x=0"
//                            + " MIN_Y=" + MIN_Y
//                    );
                }

                if (nRight < -6) {
                    //makePSDLinePoints 中不记算上交点，补充低通上交点
                    mRCAPositionData.pointList.add(mPointF);
                    mRCAPositionData.pointListRight.add(mPointF);
                    drawPointWithFlag(canvas, mPointF.x, mPointF.y, mPaintRed);
//                    Log.i(TAG, "drawPSDWithPoints e add pointListRight"
//                            + " mPointF.x=" + mPointF.x
//                            + " mPointF.y=" + mPointF.y
//                    );
                    makePSDLinePoints(canvas, mRCAPositionData, wRight, fRight, nRight);
                } else {
                    //已有低通上交点
                    makePSDLinePoints(canvas, mRCAPositionData, wRight, fRight, nRight);
                }
            } else {
                if (mPointF.x <= xL) {
                    if (mPointF.x > 0) {
                        //高通只有一个点
                        mRCAPositionData.pointList.add(new PointF(0, MIN_Y));
                        mRCAPositionData.pointListLeft.add(new PointF(0, MIN_Y));
                        drawPointWithFlag(canvas, 0, MIN_Y, mPaintRed);
//                        Log.i(TAG, "drawPSDWithPoints f add pointListLeft"
//                                + " x=0"
//                                + " MIN_Y=" + MIN_Y
//                        );
                    } else {
                        //高通的点都不取,低通取上中下交点
                    }
                } else {
                    //高通只取2个点
                    mRCAPositionData.pointList.add(new PointF(0, MIN_Y));
                    mRCAPositionData.pointListLeft.add(new PointF(0, MIN_Y));
                    drawPointWithFlag(canvas, 0, MIN_Y, mPaintRed);
//                    Log.i(TAG, "drawPSDWithPoints g add pointListLeft"
//                            + " x=0"
//                            + " MIN_Y=" + MIN_Y
//                    );
                    //不在0上才加，避免重复
                    if (xL != 0) {
                        mRCAPositionData.pointList.add(new PointF(xL, MIN_Y));
                        mRCAPositionData.pointListLeft.add(new PointF(xL, MIN_Y));
                        drawPointWithFlag(canvas, xL, MIN_Y, mPaintRed);
//                        Log.i(TAG, "drawPSDWithPoints h add pointListLeft"
//                                + " xL=" + xL
//                                + " MIN_Y=" + MIN_Y
//                        );
                    }
                }

                if (nRight < -6) {
                    //补充低通上交点
                    mRCAPositionData.pointList.add(new PointF(mPointF.x, MIN_Y));
                    mRCAPositionData.pointListRight.add(new PointF(mPointF.x, MIN_Y));
                    drawPointWithFlag(canvas, mPointF.x, MIN_Y, mPaintRed);
//                    Log.i(TAG, "drawPSDWithPoints i add pointListRight"
//                            + " mPointF.x=" + mPointF.x
//                            + " MIN_Y=" + MIN_Y
//                    );
                    makePSDLinePoints(canvas, mRCAPositionData, wRight, fRight, nRight);
                } else {
                    //已有低通上交点
                    makePSDLinePoints(canvas, mRCAPositionData, wRight, fRight, nRight);
                }
            }

        } else if (nLeft != 0 && nRight != 0) {
            //高通非直通+低通非直通
            makePSDLinePoints(canvas, mRCAPositionData, wLeft, fLeft, nLeft);

            /*左边斜率大于6的，makePSDLinePoints没有计算上交点的，
            要补充上交点，防止上供，注意要在makePSDLinePoints 之后调用，保证点在列表中低到高*/
            if (fRight > fLeft * 4 && nLeft > 6) {//右边--低通直通
                double angle = nLeft;
                PointF mPointF = getTopPoint(angle, fLeft);
                mRCAPositionData.pointList.add(mPointF);
                mRCAPositionData.pointListLeft.add(mPointF);
                drawPointWithFlag(canvas, mPointF.x, mPointF.y, mPaintRed);
//                Log.i(TAG, "drawPSDWithPoints j add TopPoint pointListLeft"
//                        + " x=" + mPointF.x
//                        + " y=" + mPointF.y
//                );
            }

            //高通有上交点的情况,检查去掉高通的上交点
            if (nLeft <= 6) {
                if (fLeft * 2 >= fRight) {
                    int size = mRCAPositionData.pointList.size();
                    int size1 = mRCAPositionData.pointListLeft.size();
                    if (size > 0) {
                        PointF mP = mRCAPositionData.pointList.remove(size - 1);
//                        Log.i(TAG, "drawPSDWithPoints k remove for too near pointList"
//                                + " size_pointList=" + size
//                                + " size_pointListLeft=" + size1
//                                + " x=" + mP.x
//                                + " y=" + mP.y
//                        );
                    }
                    if (size1 > 0) {
                        PointF mP1 = mRCAPositionData.pointListLeft.remove(size1 - 1);
//                        Log.i(TAG, "drawPSDWithPoints l remove for too near pointListLeft"
//                                + " size_pointList=" + size
//                                + " size_pointListLeft=" + size1
//                                + " x=" + mP1.x
//                                + " y=" + mP1.y
//                        );
                    }

                    //取相交的点
                    PointF mPointF = getPointWithTwoLine(xL, xR, nLeft, nRight);

                    mRCAPositionData.pointList.add(mPointF);
                    mRCAPositionData.pointListLeft.add(mPointF);
                    drawPointWithFlag(canvas, mPointF.x, mPointF.y, mPaintRed);
//                    Log.i(TAG, "drawPSDWithPoints kl add mid Point pointListLeft"
//                            + " mPointF.x=" + mPointF.x
//                            + " mPointF.y=" + mPointF.y
//                    );


                }
            }

            /*注意要在 makePSDLinePoints 之前调用，保证点在列表中低到高 ，补充上交点*/
            if ((fLeft * 4 <= fRight)
                    || (fLeft * 2.5 <= fRight && (nLeft > 12 || nRight < -12))//二倍程
                    || (fLeft * 2 < fRight && nLeft > 12 && nRight < -12)//二倍程
                    || (fLeft * 2 <= fRight && ((nLeft > 12 || nRight < -12) || (nLeft <= 12 && nRight < -12) || (nLeft > 12 && nRight >= -12)))//二倍程
                    || (fLeft * 1.5 <= fRight && ((nLeft >= 24 && nRight <= -24) || (nLeft >= 12 && nRight < -12) || (nLeft > 12 && nRight <= -12)))//倍程+斜率
            ) {
                PointF mPointFLeftTop = null;
                PointF mPointFRightTop = null;
                //makePSDLinePoints 中处理了>6 没有上交点,这里根据条件补充高通通上交点，防止上供
                if (nLeft > 6) {
                    mPointFLeftTop = getTopPoint(nLeft, fLeft);
                }

                //makePSDLinePoints 中处理了<-6 没有上交点,这里根据条件补充低通上交点，防止上供
                if (nRight < -6) {
                    mPointFRightTop = getTopPoint(nRight, fRight);
                }

                if (mPointFLeftTop != null && mPointFRightTop != null) {
                    if (mPointFLeftTop.x > mPointFRightTop.x && mPointFRightTop.x > 0) {
//                        Log.i(TAG, "drawPSDWithPoints m not add TopPoint"
//                                + " mPointFLeftTop.x=" + mPointFLeftTop.x
//                                + " mPointFLeftTop.y=" + mPointFLeftTop.y
//                                + " mPointFRightTop.x=" + mPointFRightTop.x
//                                + " mPointFRightTop.y=" + mPointFRightTop.y
//                        );
                        //if((fLeft*2==fRight && ((nLeft>12|| nRight<-12) || (nLeft<=12 && nRight<-12) || (nLeft>12&& nRight>=-12)))){
                        //取相交的点
                        PointF mPointF = getPointWithTwoLine(xL, xR, nLeft, nRight);

                        mRCAPositionData.pointList.add(mPointF);
                        mRCAPositionData.pointListLeft.add(mPointF);
                        drawPointWithFlag(canvas, mPointF.x, mPointF.y, mPaintRed);
//                        Log.i(TAG, "drawPSDWithPoints add mid Point pointListLeft"
//                                + " mPointF.x=" + mPointF.x
//                                + " mPointF.y=" + mPointF.y
//                        );
                        //}

                    } else {
                        mRCAPositionData.pointList.add(mPointFLeftTop);
                        mRCAPositionData.pointListLeft.add(mPointFLeftTop);
                        drawPointWithFlag(canvas, mPointFLeftTop.x, mPointFLeftTop.y, mPaintRed);
//                        Log.i(TAG, "drawPSDWithPoints o add TopPoint pointListLeft"
//                                + " x=" + mPointFLeftTop.x
//                                + " y=" + mPointFLeftTop.y
//                        );
                        mRCAPositionData.pointList.add(mPointFRightTop);
                        mRCAPositionData.pointListRight.add(mPointFRightTop);
                        drawPointWithFlag(canvas, mPointFRightTop.x, mPointFRightTop.y, mPaintRed);
//                        Log.i(TAG, "drawPSDWithPoints p add TopPoint pointListRight"
//                                + " x=" + mPointFRightTop.x
//                                + " y=" + mPointFRightTop.y
//                        );
                    }
                } else if (mPointFLeftTop != null && mPointFRightTop == null) {
                    mRCAPositionData.pointList.add(mPointFLeftTop);
                    mRCAPositionData.pointListLeft.add(mPointFLeftTop);
                    drawPointWithFlag(canvas, mPointFLeftTop.x, mPointFLeftTop.y, mPaintRed);
//                    Log.i(TAG, "drawPSDWithPoints q add TopPoint pointListLeft"
//                            + " x=" + mPointFLeftTop.x
//                            + " y=" + mPointFLeftTop.y
//                    );
                } else if (mPointFLeftTop == null && mPointFRightTop != null) {
                    mRCAPositionData.pointList.add(mPointFRightTop);
                    mRCAPositionData.pointListRight.add(mPointFRightTop);
                    drawPointWithFlag(canvas, mPointFRightTop.x, mPointFRightTop.y, mPaintRed);
//                    Log.i(TAG, "drawPSDWithPoints f add TopPoint pointListRight"
//                            + " x=" + mPointFRightTop.x
//                            + " y=" + mPointFRightTop.y
//                    );
                }

            } else {
                //补充一个点，防止上供
                if ((nLeft >= 36 && nRight <= -36 && fLeft == 16000 && fRight == 20000)
                        || (nLeft == 48 && nRight <= -12 && fLeft == 16000 && fRight == 20000)
                ) {
                    PointF mPointFLeftTop = getTopPoint(nLeft, fLeft);
                    mRCAPositionData.pointList.add(mPointFLeftTop);
                    mRCAPositionData.pointListLeft.add(mPointFLeftTop);
                    drawPointWithFlag(canvas, mPointFLeftTop.x, mPointFLeftTop.y, mPaintRed);
//                    Log.i(TAG, "drawPSDWithPoints e add TopPoint pointListLeft"
//                            + " x=" + mPointFLeftTop.x
//                            + " y=" + mPointFLeftTop.y
//                    );
                }


            }


            makePSDLinePoints(canvas, mRCAPositionData, wRight, fRight, nRight);


            //两个太近的点，都去掉
            //两个太近的点，去掉一个
            //去掉交叉的2个点
            int sizeLeft = mRCAPositionData.pointListLeft.size();
            int sizeRight = mRCAPositionData.pointListRight.size();
            if (sizeLeft > 0 && sizeRight > 0) {
                PointF mPointLeftLastOne = mRCAPositionData.pointListLeft.get(sizeLeft - 1);
                PointF mPointRightFirstOne = mRCAPositionData.pointListRight.get(0);
//                Log.i(TAG, "drawPSDWithPoints "
//                        + " mPointLeftLastOne.x=" + mPointLeftLastOne.x
//                        + " mPointLeftLastOne.y=" + mPointLeftLastOne.y
//                        + " mPointRightFirstOne.x=" + mPointRightFirstOne.x
//                        + " mPointRightFirstOne.y=" + mPointRightFirstOne.y
//                        + " sizeLeft=" + sizeLeft
//                        + " sizeRight=" + sizeRight
//                );
                //去掉交叉的2个点上交点
                if (mPointLeftLastOne.x > mPointRightFirstOne.x) {
                    mRCAPositionData.pointList.clear();
                    //高通最后一个点在起始位置20Hz处不用移除了
                    if (mPointLeftLastOne.x == 0) {
                        Log.i(TAG, "drawPSDWithPoints f no rm pointListLeft");
                        //细调做下在y轴上偏移对mPointLeftLastOne.y
                        if (mPointRightFirstOne.x < 0) {
                        }
                    } else {
                        if (nLeft > 6) {
                            //上面有可能存在根据条件添加了高通上交点
//                            Log.i(TAG, "drawPSDWithPoints g no remove pointListLeft");
                        } else {
                            //低通通是直通，并且高通通频点很近20KHz，计算得的高通上交点x>width,把低通(右边)的点全部清空
                            if (mPointLeftLastOne.x > width && nRight == 0) {
                                int size = mRCAPositionData.pointListRight.size();
//                                Log.i(TAG, "drawPSDWithPoints clear pointListRight size=" + size);
                                mRCAPositionData.pointListRight.clear();
                            } else {
                                if (mRCAPositionData.pointListLeft.size() > 0) {
                                    PointF mP = mRCAPositionData.pointListLeft.remove(sizeLeft - 1);
//                                    Log.i(TAG, "drawPSDWithPoints remove pointListLeft"
//                                            + " mP.x=" + mP.x
//                                            + " mP.y=" + mP.y
//                                    );
                                } else {
                                }
                            }
                        }
                    }
                    if (nRight < -6) {
                        //没有上交点---注意上面可能添加了
//                        Log.i(TAG, "drawPSDWithPoints no remove pointListRight");
                        if (mPointRightFirstOne.x > 0 && fLeft * 2 < fRight) {

                        }
                    } else {
                        //高通是直通，并且低通频点很近20Hz，计算得的低通上交点x<0,把高通的点全部清空
                        if (mPointRightFirstOne.x < 0 && nLeft == 0) {
                            int size = mRCAPositionData.pointListLeft.size();
//                            Log.i(TAG, "drawPSDWithPoints clear pointListLeft size=" + size);
                            mRCAPositionData.pointListLeft.clear();
                        } else {
                            if (mRCAPositionData.pointListRight.size() > 0) {
                                PointF mP = mRCAPositionData.pointListRight.remove(0);
//                                Log.i(TAG, "drawPSDWithPoints remove pointListRight"
//                                        + " mP.x=" + mP.x
//                                        + " mP.y=" + mP.y
//                                );
                            }

                        }
                    }
                    if (!mRCAPositionData.pointListLeft.isEmpty()) {
                        mRCAPositionData.pointList.addAll(mRCAPositionData.pointListLeft);
                    } else {
//                        Log.i(TAG, "drawPSDWithPoints pointListLeft isEmpty");
                    }
                    if (!mRCAPositionData.pointListRight.isEmpty()) {
                        mRCAPositionData.pointList.addAll(mRCAPositionData.pointListRight);
                    } else {
//                        Log.i(TAG, "drawPSDWithPoints pointListRight isEmpty");
                    }
                }

                //有上交点，没有交叉，并且很近,去掉其中一个上交点
                else if ((mPointRightFirstOne.x - mPointLeftLastOne.x) < (width / 30)) {
                    mRCAPositionData.pointList.clear();
                    if (nLeft <= 6 && nRight >= -6) {
                        if (nLeft == 0 && nRight == 0) {
//                            Log.i(TAG, "drawPSDWithPoints do nothing all zero");
                        } else {
                            //高低通都有上交点,去掉一个上焦点
                            PointF mP = mRCAPositionData.pointListLeft.remove(sizeLeft - 1);
                            //mRCAPositionData.pointListRight.remove(0);
//                            Log.i(TAG, "drawPSDWithPoints too near remove pointListLeft"
//                                    + " mP.x=" + mP.x
//                                    + " mP.y=" + mP.y
//                            );
                        }
                    }
                    if (!mRCAPositionData.pointListLeft.isEmpty()) {
                        mRCAPositionData.pointList.addAll(mRCAPositionData.pointListLeft);
                    } else {
//                        Log.i(TAG, "drawPSDWithPoints e pointListLeft isEmpty");
                    }
                    if (!mRCAPositionData.pointListRight.isEmpty()) {
                        mRCAPositionData.pointList.addAll(mRCAPositionData.pointListRight);
                    } else {
//                        Log.i(TAG, "drawPSDWithPoints e pointListRight isEmpty");
                    }
                } else if ((mPointRightFirstOne.x - mPointLeftLastOne.x) >= (width / 30)
                        && mPointRightFirstOne.y == MIN_Y
                        && mPointLeftLastOne.y == MIN_Y
                ) {
                    //高通低通都有上交点,两交点直线会上供，加一个交点，防止上供
                    mRCAPositionData.pointList.clear();
                    if (!mRCAPositionData.pointListLeft.isEmpty()) {
                        mRCAPositionData.pointList.addAll(mRCAPositionData.pointListLeft);
                    } else {
//                        Log.i(TAG, "drawPSDWithPoints e pointListLeft isEmpty");
                    }

                    //直线中间添加一个点，防止上供
                    float midX = (mPointRightFirstOne.x - mPointLeftLastOne.x) / 2 + mPointLeftLastOne.x;
                    mRCAPositionData.pointList.add(new PointF(midX, MIN_Y));
                    drawPointWithFlag(canvas, midX, MIN_Y, mPaintRed);
//                    Log.i(TAG, "drawPSDWithPoints e add X=" + midX + " Y=" + MIN_Y);

                    if (!mRCAPositionData.pointListRight.isEmpty()) {
                        mRCAPositionData.pointList.addAll(mRCAPositionData.pointListRight);
                    } else {
//                        Log.i(TAG, "drawPSDWithPoints e pointListRight isEmpty");
                    }
                }


            }
        }

        int size = mRCAPositionData.pointList.size();
        if (size > 2) {
            //绘制曲线
            drawPointWithLine(canvas, mRCAPositionData);
        } else {
//            Log.i(TAG, "drawPSDWithThreePoints not drawPointWithLine   size=" + size);
        }

        logList(mRCAPositionData);
    }

    /**
     * 上斜线的交点
     *
     * @param xL
     * @param xR
     * @param nLeft
     * @param nRight
     * @return
     */
    private PointF getPointWithTwoLine(float xL, float xR, double nLeft, double nRight) {
        float D = xR - xL;
        //要计算tan先将角度PI,180度为1个PI--注意上交点用的角度规则和下交点的不一样--查看getTopPoint
        double radians = Math.toRadians(Math.abs(nLeft / 2));
        double tA = Math.tan(radians);
        //要计算tan先将角度PI,180度为1个PI--注意上交点用的角度规则和下交点的不一样--查看getTopPoint
        radians = Math.toRadians(Math.abs(nRight / 2));
        double tB = Math.tan(radians);
        double a = D / (tA / tB + 1);
        double b = tA * a;
        float xC = (float) (xL + a);
        float yC = (float) (MIN_Y + (MIN_Y * 0.3 - b));
        return new PointF(xC, yC);
    }

    /**
     * 下交点取界内点时使用的角配置，注意界内界外取点配置的角参数有小幅差异--要细调
     * 左边升频高通专用！？样机升频高通（左边）和降频低通（右边）的斜率不一样--下交点取界内或者界外有影响！！！
     * 可微调这个映射角度，调整曲线倾斜状态； 再对比样机的对应参数的图片效果图，最终确认角度配置
     * <p>
     * 根据频率和分频斜率映射一个角度，根据样机图片图形状态获取的
     *
     * @param f1 频率
     * @param n  分频斜率，正数（左边高通）/负数（右边低通）
     * @return
     */
    private double mapAngleByFnLeft(float f1, double n) {
        double angle = n;
        double tpN = Math.abs(n);
        if (f1 >= 16000) {
            if (tpN == 6) {
                angle = 11.6;
            } else if (tpN == 12) {
                angle = 27;
            } else if (tpN == 24) {
                angle = 41.5;
            } else if (tpN == 36) {
                angle = 53.5;
            } else if (tpN == 48) {
                angle = 60;
            }
        } else if (f1 >= 12500) {
            if (tpN == 6) {
                angle = 10.6;
            } else if (tpN == 12) {
                angle = 22.8;
            } else if (tpN == 24) {
                angle = 34.5;
            } else if (tpN == 36) {
                angle = 44.5;
            } else if (tpN == 48) {
                angle = 53;
            }
        } else if (f1 >= 10000) {//下交点使用使用界外点
            if (tpN == 6) {
                angle = 10.1;
            } else if (tpN == 12) {
                angle = 21;
            } else if (tpN == 24) {
                angle = 30;
            } else if (tpN == 36) {
                angle = 38.5;
            } else if (tpN == 48) {
                angle = 46;
            }
        } else if (f1 >= 5000) {//下交点使用使用界外点
            if (tpN == 6) {
                angle = 9.5;
            } else if (tpN == 12) {
                angle = 19.1;
            } else if (tpN == 24) {
                angle = 27;
            } else if (tpN == 36) {
                angle = 34.3;
            } else if (tpN == 48) {
                angle = 40;
            }
        } else if (f1 >= 4000) {//下交点使用使用界外点
            if (tpN == 6) {
                angle = 9.5;
            } else if (tpN == 12) {
                angle = 19;
            } else if (tpN == 24) {
                angle = 27;
            } else if (tpN == 36) {
                angle = 34;
            } else if (tpN == 48) {
                angle = 41;
            }
        } else if (f1 >= 2000) {//下交点使用使用界外点
            if (tpN == 6) {
                angle = 9.5;
            } else if (tpN == 12) {
                angle = 18.5;
            } else if (tpN == 24) {
                angle = 27;
            } else if (tpN == 36) {
                angle = 34;
            } else if (tpN == 48) {
                angle = 40;
            }
        } else if (f1 >= 1000) {//下交点使用使用界外点
            if (tpN == 6) {
                angle = 9.5;
            } else if (tpN == 12) {
                angle = 18.5;
            } else if (tpN == 24) {
                angle = 26.5;
            } else if (tpN == 36) {
                angle = 34;
            } else if (tpN == 48) {
                angle = 40;
            }
        } else if (f1 >= 500) {//下交点使用使用界外点
            if (tpN == 6) {
                angle = 9;
            } else if (tpN == 12) {
                angle = 18.5;
            } else if (tpN == 24) {
                angle = 26.5;
            } else if (tpN == 36) {
                angle = 34;
            } else if (tpN == 48) {
                angle = 40;
            }
        } else if (f1 >= 400) {//下交点使用使用界外点
            if (tpN == 6) {
                angle = 9;
            } else if (tpN == 12) {
                angle = 18.7;
            } else if (tpN == 24) {
                angle = 26.5;
            } else if (tpN == 36) {
                angle = 34;
            } else if (tpN == 48) {
                angle = 40;
            }
        } else if (f1 >= 200) {//下交点使用使用界外点
            if (tpN == 6) {
                angle = 9;
            } else if (tpN == 12) {
                angle = 18.5;
            } else if (tpN == 24) {
                angle = 26.5;
            } else if (tpN == 36) {
                angle = 34;
            } else if (tpN == 48) {
                angle = 40;
            }
        } else if (f1 >= 100) {//下交点使用使用界外点
            if (tpN == 6) {
                angle = 8.2;
            } else if (tpN == 12) {
                angle = 18;
            } else if (tpN == 24) {
                angle = 26;
            } else if (tpN == 36) {
                angle = 34;
            } else if (tpN == 48) {
                angle = 40;
            }
        } else if (f1 >= 50) {//下交点使用使用界外点
            if (tpN == 6) {
                angle = 7;
            } else if (tpN == 12) {
                angle = 17;
            } else if (tpN == 24) {
                angle = 25.5;
            } else if (tpN == 36) {
                angle = 34.5;
            } else if (tpN == 48) {
                angle = 40;
            }
        } else if (f1 >= 40) {//下交点使用使用界外点
            if (tpN == 6) {
                angle = 7;
            } else if (tpN == 12) {
                angle = 16;
            } else if (tpN == 24) {
                angle = 24;
            } else if (tpN == 36) {
                angle = 33;
            } else if (tpN == 48) {
                angle = 40;
            }
        } else {//下交点使用使用界外点
            if (tpN == 6) {
                angle = 7;
            } else if (tpN == 12) {
                angle = 15;
            } else if (tpN == 24) {
                angle = 22;
            } else if (tpN == 36) {
                angle = 28;
            } else if (tpN == 48) {
                angle = 38;
            }
        }

        return angle;
    }


    /**
     * 下交点取界外点时使用的角配置--注意界内界外取点配置的角参数有小幅差异--要细调
     * 右边降频低通专用！？样机升频高通（左边）和降频低通（右边）的斜率不一样--下交点取界内或者界外有影响！！！
     * 可微调这个映射角度，调整曲线倾斜状态； 再对比样机的对应参数的图片效果图，最终确认角度配置
     *
     * @param f1 频率
     * @param n  分频斜率，正数（左边高通）/负数（右边低通）
     * @return
     */
    private double mapAngleByFnRight(float f1, double n) {
        double angle = n;
        double tpN = Math.abs(n);

        if (f1 >= 16000) {
            if (tpN == 6) {
                angle = 20;
            } else if (tpN == 12) {
                angle = 38;
            } else if (tpN == 24) {
                angle = 53;
            } else if (tpN == 36) {
                angle = 61;
            } else if (tpN == 48) {
                angle = 65;
            }
        } else if (f1 >= 12500) {
            if (tpN == 6) {
                angle = 20;
            } else if (tpN == 12) {
                angle = 37;
            } else if (tpN == 24) {
                angle = 49;
            } else if (tpN == 36) {
                angle = 53;
            } else if (tpN == 48) {
                angle = 55;
            }
        } else if (f1 >= 10000) {
            if (tpN == 6) {
                angle = 18;
            } else if (tpN == 12) {
                angle = 36;
            } else if (tpN == 24) {
                angle = 43;
            } else if (tpN == 36) {
                angle = 48;
            } else if (tpN == 48) {
                angle = 53;
            }
        } else if (f1 >= 5000) {//ok
            if (tpN == 6) {
                angle = 15.5;
            } else if (tpN == 12) {
                angle = 25;
            } else if (tpN == 24) {
                angle = 31;
            } else if (tpN == 36) {
                angle = 37;
            } else if (tpN == 48) {
                angle = 42;
            }
        } else if (f1 >= 4000) {
            if (tpN == 6) {
                angle = 13.6;
            } else if (tpN == 12) {
                angle = 24;
            } else if (tpN == 24) {
                angle = 29;
            } else if (tpN == 36) {
                angle = 36;
            } else if (tpN == 48) {
                angle = 42;
            }
        } else if (f1 >= 2000) {
            if (tpN == 6) {
                angle = 13.6;
            } else if (tpN == 12) {
                angle = 19.8;
            } else if (tpN == 24) {
                angle = 27;
            } else if (tpN == 36) {
                angle = 34;
            } else if (tpN == 48) {
                angle = 40;
            }
        } else if (f1 >= 1000) {
            if (tpN == 6) {
                angle = 11.5;
            } else if (tpN == 12) {
                angle = 18.5;
            } else if (tpN == 24) {
                angle = 27;
            } else if (tpN == 36) {
                angle = 34;
            } else if (tpN == 48) {
                angle = 40;
            }
        } else if (f1 >= 500) {
            if (tpN == 6) {
                angle = 10;
            } else if (tpN == 12) {
                angle = 18.5;
            } else if (tpN == 24) {
                angle = 27;
            } else if (tpN == 36) {
                angle = 34;
            } else if (tpN == 48) {
                angle = 40;
            }
        } else if (f1 >= 400) {
            if (tpN == 6) {
                angle = 10;
            } else if (tpN == 12) {
                angle = 18.5;
            } else if (tpN == 24) {
                angle = 27;
            } else if (tpN == 36) {
                angle = 34;
            } else if (tpN == 48) {
                angle = 40;
            }
        } else if (f1 >= 200) {
            if (tpN == 6) {
                angle = 9.6;
            } else if (tpN == 12) {
                angle = 18.5;
            } else if (tpN == 24) {
                angle = 27;
            } else if (tpN == 36) {
                angle = 34;
            } else if (tpN == 48) {
                angle = 40;
            }
        } else if (f1 >= 100) {
            if (tpN == 6) {
                angle = 9.5;
            } else if (tpN == 12) {
                angle = 18.5;
            } else if (tpN == 24) {
                angle = 27;
            } else if (tpN == 36) {
                angle = 34;
            } else if (tpN == 48) {
                angle = 40;
            }
        } else if (f1 >= 50) {
            if (tpN == 6) {
                angle = 9.5;
            } else if (tpN == 12) {
                angle = 18.5;
            } else if (tpN == 24) {
                angle = 27;
            } else if (tpN == 36) {
                angle = 34;
            } else if (tpN == 48) {
                angle = 40;
            }
        } else {
            if (tpN == 6) {
                angle = 9.5;
            } else if (tpN == 12) {
                angle = 18.5;
            } else if (tpN == 24) {
                angle = 27;
            } else if (tpN == 36) {
                angle = 34;
            } else if (tpN == 48) {
                angle = 40;
            }
        }

        return angle;
    }

    /**
     * 绘制高通+低通曲线
     *
     * @param canvas
     */
    private void drawPSD(Canvas canvas) {
//        Log.w(TAG, "drawPSD mRCAID=" + mRCAID
//                + " highRate=" + mCurrentRCA.highRate
//                + " highSlope=" + mCurrentRCA.highSlope
//                + " lowSRate=" + mCurrentRCA.lowRate
//                + " lowSlope=" + mCurrentRCA.lowSlope
//        );

        for (int i = 0; i < mRCAPositionDatas.size(); i++) {
            RCAData mRCAData = mRCAPositionDatas.get(i);
            if (mRCAData.id == mRCAID) {
//                Log.w(TAG, "drawPSD just draw last mRCAID=" + mRCAID);
            } else {
                if (!mRCAData.isHide) {
                    drawPSDWithPoints(canvas, mRCAData);
                }
            }
        }
        if (!mCurrentRCA.isHide) {
            drawPSDWithPoints(canvas, mCurrentRCA);
        }

    }

    /**
     * 设置当前控制曲线的高通频率
     *
     * @param idRCA 曲线id
     * @param rate  高通频率
     * @return 设置成功返回true，否则为false
     */
    public boolean setHighRate(int idRCA, int rate) {
        RCAData mRCAPositionData = getRCAPositionData(idRCA);
        if (mRCAPositionData != null) {
            if (rate >= mRCAPositionData.lowRate) {
//                Log.w(TAG, "setHighRate bad rate idRCA=" + idRCA
//                        + " rate=" + rate
//                        + " lowRate=" + mRCAPositionData.lowRate
//                );
                return false;
            }
            mRCAPositionData.highRate = rate;
//            Log.w(TAG, "setHighRate idRCA=" + idRCA
//                    + " rate=" + rate
//            );
            return true;
        }
        return false;
    }

    /**
     * 设置当前控制曲线的高通斜率
     *
     * @param idRCA 曲线id
     * @param slope 斜率
     * @return 设置成功返回true，否则为false
     */
    public boolean setHighSlope(int idRCA, int slope) {
        RCAData mRCAPositionData = getRCAPositionData(idRCA);
        if (mRCAPositionData != null) {
            mRCAPositionData.highSlope = slope;
//            Log.w(TAG, "setHighSlope idRCA=" + idRCA
//                    + " slope=" + slope
//            );
            return true;
        }
        return false;
    }

    /**
     * 设置当前控制曲线的低通频率
     *
     * @param idRCA 曲线id
     * @param rate  频率
     * @return 设置成功返回true，否则为false
     */
    public boolean setLowRate(int idRCA, int rate) {
        RCAData mRCAPositionData = getRCAPositionData(idRCA);
        if (mRCAPositionData != null) {
            if (rate <= mRCAPositionData.highRate) {
//                Log.w(TAG, "setLowRate bad rate idRCA=" + idRCA
//                        + " rate=" + rate
//                        + " highRate=" + mRCAPositionData.highRate
//                );
                return false;
            }
            mRCAPositionData.lowRate = rate;
//            Log.w(TAG, "setLowRate idRCA=" + idRCA
//                    + " rate=" + rate
//            );
            return true;
        }
        return false;
    }

    /**
     * 设置当前控制曲线的低通斜率
     *
     * @param idRCA 曲线id
     * @param slope 斜率
     * @return 设置成功返回true，否则为false
     */
    public boolean setLowSlope(int idRCA, int slope) {
        RCAData mRCAPositionData = getRCAPositionData(idRCA);
        if (mRCAPositionData != null) {
            if (slope > 0) {
                slope = 0 - slope;
//                Log.i(TAG, "setLowSlope   idRCA=" + idRCA
//                        + " just to <0  slope=" + slope
//                );
            }
            mRCAPositionData.lowSlope = slope;
//            Log.w(TAG, "setLowSlope idRCA=" + idRCA
//                    + " slope=" + slope
//            );
            return true;
        }
        return false;
    }

    /**
     * 设置曲线的高通/低通的频率和斜率
     *
     * @param idRCA     曲线id
     * @param highRate  高通频率
     * @param highSlope 高通斜率
     * @param lowRate   低通频率
     * @param lowSlope  低通斜率
     * @return 设置成功返回true，否则为false
     */
    public boolean setRateSlope(int idRCA, float highRate, float highSlope, float lowRate, float lowSlope) {
        if (highRate >= lowRate) {
//            Log.w(TAG, "setRateSlope return bad data idRCA=" + idRCA
//                    + " highRate=" + highRate
//                    + " lowRate=" + lowRate
//                    + " highSlope=" + highSlope
//                    + " lowSlope=" + lowSlope
//            );
            return false;
        }

        if (lowSlope > 0) {
            lowSlope = 0 - lowSlope;
//            Log.i(TAG, "setRateSlope   idRCA=" + idRCA
//                    + " just to <0  lowSlope=" + lowSlope
//            );
        }
        RCAData mRCAPositionData = getRCAPositionData(idRCA);
        if (mRCAPositionData != null) {
            mRCAPositionData.highRate = highRate;
            mRCAPositionData.highSlope = highSlope;
            mRCAPositionData.lowRate = lowRate;
            mRCAPositionData.lowSlope = lowSlope;
//            Log.w(TAG, "setRateSlope idRCA=" + idRCA
//                    + " highRate=" + highRate
//                    + " lowRate=" + lowRate
//                    + " highSlope=" + highSlope
//                    + " lowSlope=" + lowSlope
//            );
            return true;
        }
        return false;
    }

    /**
     * 设置当前控制曲线
     *
     * @param mRCAData
     */
    public void setCurrentRCA(RCAData mRCAData) {
        if (mRCAData != null) {
            RCAData mRCAPositionData = getRCAPositionData(mRCAData.id);
            if (mRCAPositionData == null) {
//                Log.w(TAG, "setCurrentRCA not in mRCAPositionDatas bad mRCAData.id=" + mRCAData.id);
                return;
            }
            mRCAID = mRCAData.id;
            mCurrentRCA = mRCAData;
//            Log.w(TAG, "setCurrentRCA id=" + mRCAData.id);
        } else {
//            Log.w(TAG, "setCurrentRCA do nothing bad data mRCAData=" + mRCAData);
        }
    }

    /**
     * 设置当前控制曲线
     *
     * @param mFLAG RCA_CENTER_LR = 0;RCA_CENTER_FRONT_LR = 1;RCA_CENTER_REAR_LR = 2;RCA_CENTER_SUB_SW_LR = 3;
     */
    public void setCurrentRCA(int mFLAG) {
        xTouchDX = 0;
        yTouchDY = 0;
        RCAData mRCAPositionData = getRCAPositionData(mFLAG);
        if (mRCAPositionData == null) {
//            Log.w(TAG, "setRCAPosition not in mRCAPositionDatas bad    mFLAG=" + mFLAG);
            return;
        }
        mCurrentRCA = mRCAPositionData;
//        Log.i(TAG, "setRCAPosition bad    mFLAG=" + mFLAG
//                + " highRate=" + mCurrentRCA.highRate
//                + " highSlope=" + mCurrentRCA.highSlope
//                + " lowRate=" + mCurrentRCA.lowRate
//                + " lowSlope=" + mCurrentRCA.lowSlope
//        );
        this.mRCAID = mFLAG;
    }

    /**
     * 重置数据,将曲线还原为直线
     */
    public void resetRCAData() {
        xTouchDX = 0;
        yTouchDY = 0;
        if (mRCAID == RCA_LR_0) {
            initDefaultPoint();
        }
        for (int i = 0; i < mRCAPositionDatas.size(); i++) {
            mRCAPositionDatas.get(i).reset();
        }
        /*
        mRCAPositionDatas.clear();
        mRCAPositionDatas.add(new RCAData(RCA_LR_0,R.color.warnColor));
        mRCAPositionDatas.add(new RCAData(RCA_LR_1,R.color.greenColor));
        mRCAPositionDatas.add(new RCAData(RCA_LR_2,R.color.blueColor));
        mRCAPositionDatas.add(new RCAData(RCA_LR_3,R.color.pupleColor));
        mRCAID = RCA_LR_0;
        mCurrentRCA=getRCAPositionData(mRCAID);
        */
//        Log.i(TAG, "setRCAPosition bad    mRCAID=" + mRCAID
//                + " mCurrentRCA.id=" + mCurrentRCA.id
//        );

    }

    /**
     * 获取当前控制曲线id
     *
     * @return
     */
    public int getCurrentRCAID() {
        return mRCAID;
    }

    /**
     * 获取当前控制曲线
     *
     * @return
     */
    public RCAData getCurrentRCA() {
        return getRCAPositionData(mRCAID);
    }

    /**
     * 添加曲线
     *
     * @param mRCAData
     */
    private void addRCAData(RCAData mRCAData) {
        if (mRCAData != null) {
            RCAData mRCAPositionData = getRCAPositionData(mRCAData.id);
            if (mRCAPositionData == null) {
//                Log.w(TAG, "addRCAData  mRCAData.id=" + mRCAData.id
//                        + " mRCAID=" + mRCAID);
                mRCAPositionDatas.add(mRCAData);
                return;
            } else {
//                Log.w(TAG, "addRCAData  donothing is had in list  mRCAData.id=" + mRCAData.id
//                        + " mRCAID=" + mRCAID);
            }
        }
    }

    /**
     * 移除曲线指定曲线
     *
     * @param mRCAData
     */
    private void rmRCAData(RCAData mRCAData) {
        if (mRCAData != null) {
            RCAData mRCAPositionData = getRCAPositionData(mRCAData.id);
            if (mRCAPositionData != null) {
//                Log.w(TAG, "rmRCAData  mRCAData.id=" + mRCAData.id + " mRCAID=" + mRCAID);
                mRCAPositionDatas.remove(mRCAPositionData);
            } else {
//                Log.w(TAG, "rmRCAData  donothing is no in list  mRCAData.id=" + mRCAData.id);
            }
        }
    }

    /**
     * 移除所有曲线
     */
    private void rmAllRCAData() {
        mRCAPositionDatas.clear();
    }


}
