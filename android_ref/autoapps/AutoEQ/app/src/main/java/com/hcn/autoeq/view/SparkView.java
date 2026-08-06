package com.hcn.autoeq.view;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.BlurMaskFilter;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PixelFormat;
import android.graphics.Point;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

import com.hcn.autoeq.R;
import com.hcn.autoeq.util.SkinUtils;

import java.util.LinkedList;
import java.util.Queue;
import java.util.Random;

/**
 * 礼花效果控件，控件本身进行visible/gone 使得刷新线程工作和停止
 * @author xcj
 */
public class SparkView extends SurfaceView implements SurfaceHolder.Callback, Runnable {
    private String TAG = "SparkView";
    //----------绘制礼花 start
    private SurfaceHolder mHolder;
    private Canvas mCanvas;
    private boolean isRun;
    /**当前触摸点X，Y坐标*/
    private float X, Y;
    /**控件宽高*/
    public static int WIDTH, HEIGHT;
    /**画笔对象*/
    private Paint mSparkPaint;
    /**花火半径*/
    private float radius = 0;
    /**火花喷射距离*/
    private float mDistance = 0;
    /**当前喷射距离*/
    private float mCurDistance = 0;
    /**火花半径*/
    private static final float SPARK_RADIUS = 2.0F;
    /**火花外侧阴影大小*/
    private static final float BLUR_SIZE = 5.0F;
    /**每帧礼花轨迹变化速度 2 ，1.5 ，1*/
    private static final float PER_SPEED_SEC = 2.0F;
    /**礼花数组最大值200,150,120*/
    private static final int COUNT_SPARK =120;
    /**随机数*/
    private Random mRandom = new Random();
    /**火花的起始点，终点，塞贝儿曲线拐点1，塞贝儿曲线拐点2*/
    private Point start, end, c1, c2;
    /**计算塞贝儿曲线的当前点 */
    private Point bezierPoint;
    /**是否是激活状态*/
    private boolean isActive = false;
    /**for clear*/
    Paint paintClear;
//----------绘制礼花 end


//----------绘制线条 start
    /**拖尾画笔*/
    private Paint mPaintLine;
    /**是否按下*/
    private boolean fingerBegin = false;
    /**当前touch位置X*/
    private float fingerCurrentX = 0;
    /**当前touch位置Y*/
    private float fingerCurrentY = 0;
    /**保存轨迹X*/
    private Queue<Float> fingerX = new LinkedList<Float>();
    /**保存轨迹Y*/
    private Queue<Float> fingerY = new LinkedList<Float>();
    private Path mPath = new Path();
    /**轨迹点在列表中最大个数，太长时在阶乘运算是有数据溢出的风险（>long）,可取10,12，15,20,*/
    private int MAX_POINT_FOR_LINE =9;
    /**true：down 事件时校验不绘制拖影线；false:非down事件不处理*/
    private boolean stopDrawLineForDown=false;
    /**拖影线宽度*/
    private  int strokeWidthLine=18;
    /**拖影线透明度*/
    private  int alphaLine=150;
    /**拖影线样式1*/
    private int LINE_STYLE1=1;
    /**拖影线样式2*/
    private int LINE_STYLE2=2;
    /**拖影线样式，有LINE_STYLE1和LINE_STYLE2两种样式*/
    private int lineStyle=LINE_STYLE2;
//----------绘制线条 end

    /**特效触摸下需要touch事件的底层view*/
    private View viewer;
    private Thread mThread;
    private final  static  int MSG_DELAY_START=1;

    public SparkView(Context context) {
        super(context);
    }

    public SparkView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context);
        initLine(context);
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Log.i(TAG, "surfaceChanged width=" + width + " height=" + height);
    }
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.i(TAG, "surfaceCreated  ");
        mH.removeMessages(MSG_DELAY_START);
        mH.sendEmptyMessageDelayed(MSG_DELAY_START,100);
    }
    @Override
    public void surfaceDestroyed(SurfaceHolder argholder0) {
        Log.i(TAG, "surfaceDestroyed  ");
        stopDraw();
    }

    @SuppressLint("ClickableViewAccessibility")
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        switch (event.getPointerCount()) {
            //单点触摸
            case 1:
                switch (event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        Log.i(TAG, "onTouchEvent ACTION_DOWN");
                        isActive = true;
                        //FOR drawLineStyle1/drawLineStyle2
                        stopDrawLineForDown=true;
                        fingerY.clear();
                        fingerX.clear();

                        X = event.getX();
                        Y = event.getY();

                        //FOR drawLineStyle1/drawLineStyle2
                        fingerCurrentX = event.getX();
                        fingerCurrentY = event.getY();
                        //FOR drawLineStyle2
                        mPath.reset();
                        fingerBegin = true;
                        break;
                    case MotionEvent.ACTION_MOVE:
                        stopDrawLineForDown=false;
                        X = event.getX();
                        Y = event.getY();
                        //FOR drawLineStyle1/drawLineStyle2
                        fingerCurrentX = event.getX();
                        fingerCurrentY = event.getY();
                        break;
                    case MotionEvent.ACTION_UP:
                    case MotionEvent.ACTION_CANCEL:
                        Log.i(TAG, "onTouchEvent ACTION_UP");
                        isActive = false;

                        //FOR LINE
                        fingerBegin = false;
                        break;
                    default:
                        break;
                }
                break;
        }
        if(viewer!=null){
            //该view消费了event，所以下层的view必须dispatchTouchEvent才能获得事件
            MotionEvent newEvent = MotionEvent.obtain(event);
            viewer.dispatchTouchEvent(newEvent);
        }
        return true;
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        int wSize=MeasureSpec.getSize(widthMeasureSpec);
        int hSize=MeasureSpec.getSize(heightMeasureSpec);
        WIDTH = wSize;
        HEIGHT = hSize;
        setMeasuredDimension(WIDTH,HEIGHT);
    }
    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        super.onLayout(changed, left, top, right, bottom);
        Log.d(TAG, "onLayout width=" + WIDTH + "  height=" + HEIGHT);
    }

    @Override
    public void run() {
        Log.i(TAG, "run start");
        //礼花数组
        int[][] sparks = new int[COUNT_SPARK][10];
        //long startTime = 0;
        //long dxTime = 0;
        while (isRun) {
            if(WIDTH<=0 || mHolder==null){
                Log.i(TAG, "run start  bad WIDTH="+WIDTH +" mHolder=" +mHolder);
                try {
                    Thread.sleep(50);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                continue;
            }
            if(!isRun){
                Log.i(TAG, "SparkView run return isRun=" + isRun);
                return;
            }
            //startTime = System.currentTimeMillis();

            try {
                mCanvas = mHolder.lockCanvas(mHolder.getSurfaceFrame());
                //mCanvas = mHolder.lockCanvas();
                if (mCanvas != null) {
                    synchronized (mHolder) {
                        //清屏
                        mCanvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR);
                        //循环绘制所有礼花
                        for (int[] n : sparks) {
                            if(!isRun){
                                Log.i(TAG, "SparkView run break isRun=" + isRun);
                                mCurDistance = 0;
                                mDistance = 0;
                                radius = 0;
                                break;
                            }
                            n = drawSpark(mCanvas, (int) X, (int) Y, n);
                        }

                        if(!stopDrawLineForDown && isRun){
                            workInRunnable(mCanvas);
                        }else{
                            Log.i(TAG, "run no drawline is dowm stopDrawLineForDown="+stopDrawLineForDown);
                        }
                        //dxTime = System.currentTimeMillis() - startTime;
                        //Log.i("SparkView", "SparkView g run dxTime=" + dxTime+" mCanvas.hashCode="+mCanvas.hashCode());
                        if(isRun){
                            //控制帧数
                            //Thread.sleep(Math.max(0, 30 - dxTime));
                            Thread.sleep(20);
                        }
                    }
                }else{
                    Log.i(TAG, "run bad mCanvas="+mCanvas+" isRun="+isRun);
                    if(isRun) {
                        Thread.sleep(Math.max(0, 50));
                    }
                }

                try {
                    if (mCanvas != null && mHolder!=null) {
                        mHolder.unlockCanvasAndPost(mCanvas);
                    }
                }catch (Exception e){
                    e.printStackTrace();
                    Log.e(TAG, "run unlockCanvasAndPost Exception e="+e);
                }
            } catch (Exception e) {
                e.printStackTrace();
                Log.e(TAG, "run Exception e="+e);
            }

            //清屏
            if(!isRun){
                mCanvas = mHolder.lockCanvas(mHolder.getSurfaceFrame());
                //mCanvas = mHolder.lockCanvas();
                if (mCanvas != null) {
                    Log.i(TAG, "run end while clear drawRect isRun="+isRun);
                    mCanvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR);
                    mCanvas.drawRect(0, 0, WIDTH, HEIGHT, paintClear);
                    try {
                        if ( mHolder!=null) {
                            mHolder.unlockCanvasAndPost(mCanvas);
                        }
                    }catch (Exception e){
                        e.printStackTrace();
                        Log.e(TAG, "run end while unlockCanvasAndPost Exception e="+e);
                    }
                }
            }
        }
        clearWhenUnRun();
        mCurDistance = 0;
        mDistance = 0;
        radius = 0;
        Log.i(TAG, "run end");
    }


//---绘制礼花--start
    /**
     * 初始化礼花参数
     * @param context
     */
    private void init(Context context) {
        initHodler();

        this.mSparkPaint = new Paint();
        //打开抗锯齿
        this.mSparkPaint.setAntiAlias(true);
        //防抖，使得颜色过渡柔和
        this.mSparkPaint.setDither(true);
        /*STROKE：描边；FILL_AND_STROKE：描边并填充；FILL：填充*/
        this.mSparkPaint.setStyle(Paint.Style.FILL);
        //设置外围模糊效果
        this.mSparkPaint.setMaskFilter(new BlurMaskFilter(BLUR_SIZE, BlurMaskFilter.Blur.SOLID));

        start = new Point(0,0);
        end = new Point(0,0);
        c1 = new Point(0,0);
        c2 = new Point(0,0);
        bezierPoint = new Point(0,0);


        paintClear = new Paint();
        paintClear.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.CLEAR));
    }
    private void initHodler(){
        mHolder = this.getHolder();
        if(mHolder==null){
            Log.i(TAG, "initHodler bad mHolder="+mHolder);
        }
        mHolder.addCallback(this);
        //画布透明处理，没有这，默认画布是黑色的
        setZOrderOnTop(true);
        mHolder.setFormat(PixelFormat.TRANSLUCENT);
    }


    /**
     * 调用绘制礼花特效
     * @param canvas
     * @param x
     * @param y
     * @param store
     * @return
     */
    private int[] drawSpark(Canvas canvas, int x, int y, int[] store) {
        this.X = x;
        this.Y = y;
        this.mCurDistance = store[0];
        this.mDistance = store[1];

        //初始化礼花
        if (mCurDistance == mDistance && isActive) {
            mDistance = getRandom(SparkView.HEIGHT / 2, mRandom.nextInt(15)) + 1;
            mCurDistance = 0;
            start.set((int)X,(int)Y);
            //减少new 动作
            updateEndRandomPoint(start.x, start.y, (int) mDistance);
            updateC1RandomPoint(start.x, start.y, mRandom.nextInt(SparkView.HEIGHT / 4));
            updateC2RandomPoint(end.x, end.y, mRandom.nextInt(SparkView.HEIGHT / 4));
        }
        else {
            //恢复礼花路径
            start.set(store[2], store[3]);
            end.set(store[4], store[5]);
            c1.set(store[6], store[7]);
            c2.set(store[8], store[9]);
        }

        //更新礼花路径
        updateSparkPath();
        if(mDistance!=0){
            //计算塞贝儿曲线的当前点
            updateBezierPoint(mCurDistance / mDistance, start, c1, c2, end);
            //设置随机颜色
            mSparkPaint.setColor(Color.argb(255, mRandom.nextInt(128) + 128, mRandom.nextInt(128) + 128, mRandom.nextInt(128) + 128));
            //画礼花
            canvas.drawCircle(bezierPoint.x, bezierPoint.y, radius, mSparkPaint);
        }

        //重置礼花状态
        if (mCurDistance == mDistance) {
            store[0] = 0;
            store[1] = 0;
        } else {
            //保持花火的状态
            store[0] = (int) mCurDistance;
            store[1] = (int) mDistance;
            store[2] = start.x;
            store[3] = start.y;
            store[4] = end.x;
            store[5] = end.y;
            store[6] = c1.x;
            store[7] = c1.y;
            store[8] = c2.x;
            store[9] = c2.y;
        }
        return store;
    }

    /**
     * 更新礼花路径
     */
    private void updateSparkPath() {
        if(!isRun){
            Log.i(TAG, "updateSparkPath  return isRun" + isRun );
            mCurDistance = 0;
            mDistance = 0;
            radius = 0;
            return;
        }
        mCurDistance += PER_SPEED_SEC;
        //前半段
        if (mCurDistance < (mDistance / 2) && (mCurDistance != 0)) {
            radius = SPARK_RADIUS * (mCurDistance / (mDistance / 2));
        }
        //后半段
        else if (mCurDistance > (mDistance / 2) && (mCurDistance < mDistance)) {
            radius = SPARK_RADIUS - SPARK_RADIUS * ((mCurDistance / (mDistance / 2)) - 1);
        }
        //完成
        else if (mCurDistance >= mDistance) {
            mCurDistance = 0;
            mDistance = 0;
            radius = 0;
        }
    }

    /**
     * 根据基准点获取指定范围为半径的随机点
     */
    private Point getRandomPoint(int baseX, int baseY, int r) {
        if (r <= 0) {
            r = 1;
        }
        int x = mRandom.nextInt(r);
        int y = (int) Math.sqrt(r * r - x * x);
        x = baseX + getRandomPNValue(x);
        y = baseY + getRandomPNValue(y);
        return new Point(x, y);
    }

    /**
     * 减少new 动作
     * 根据基准点获取指定范围为半径的随机点
     *计算更新礼花塞贝儿曲线拐点1
     * @param baseX  基准点x
     * @param baseY  基准点y
     * @param r    半径范围
     */
    private void updateC1RandomPoint(int baseX, int baseY, int r) {
        if (r <= 0) {
            r = 1;
        }
        int x = mRandom.nextInt(r);
        int y = (int) Math.sqrt(r * r - x * x);
        x = baseX + getRandomPNValue(x);
        y = baseY + getRandomPNValue(y);
        c1.x=x;
        c1.y=y;
    }
    /**
     * 减少new 动作
     * 根据基准点获取指定范围为半径的随机点
     *计算更新礼花塞贝儿曲线拐点2
     * @param baseX  基准点x
     * @param baseY  基准点y
     * @param r    半径范围
     */
    private void updateC2RandomPoint(int baseX, int baseY, int r) {
        if (r <= 0) {
            r = 1;
        }
        int x = mRandom.nextInt(r);
        int y = (int) Math.sqrt(r * r - x * x);
        x = baseX + getRandomPNValue(x);
        y = baseY + getRandomPNValue(y);
        c2.x=x;
        c2.y=y;
    }
    /**
     * 减少new 动作
     * 根据基准点获取指定范围为半径的随机点
     *计算更新礼花终点
     * @param baseX  基准点x
     * @param baseY  基准点y
     * @param r    半径范围
     */
    private void updateEndRandomPoint(int baseX, int baseY, int r) {
        if (r <= 0) {
            r = 1;
        }
        int x = mRandom.nextInt(r);
        int y = (int) Math.sqrt(r * r - x * x);
        x = baseX + getRandomPNValue(x);
        y = baseY + getRandomPNValue(y);
        end.x=x;
        end.y=y;
    }

    /**
     * 根据range范围，和chance几率。返回一个随机值
     */
    private int getRandom(int range, int chance) {
        int num = 0;
        switch (chance) {
            case 0:
                num = mRandom.nextInt(range);
                break;
            default:
                num = mRandom.nextInt(range / 4);
                break;
        }
        return num;
    }

    /**
     * 获取随机正负数
     */
    private int getRandomPNValue(int value) {
        return mRandom.nextBoolean() ? value : 0 - value;
    }

    /**
     * 计算塞贝儿曲线
     *
     * @param t  时间，范围0-1
     * @param s  起始点
     * @param c1 拐点1
     * @param c2 拐点2
     * @param e  终点
     * @return 塞贝儿曲线在当前时间下的点
     */
    private Point CalculateBezierPoint(float t, Point s, Point c1, Point c2, Point e) {
        float u = 1 - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;

        Point p = new Point((int) (s.x * uuu), (int) (s.y * uuu));
        p.x += 3 * uu * t * c1.x;
        p.y += 3 * uu * t * c1.y;
        p.x += 3 * u * tt * c2.x;
        p.y += 3 * u * tt * c2.y;
        p.x += ttt * e.x;
        p.y += ttt * e.y;

        return p;
    }

    /**
     * 计算塞贝儿曲线,塞贝儿曲线在当前时间下的点 ,减少new动作，特别是高频执行发生的场景
     * @param t  时间，范围0-1
     * @param s  起始点
     * @param c1 拐点1
     * @param c2 拐点2
     * @param e  终点
     */
    private void updateBezierPoint(float t, Point s, Point c1, Point c2, Point e) {
        float u = 1 - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;
        bezierPoint.x=(int) (s.x * uuu);
        bezierPoint.y=(int) (s.y * uuu);
        bezierPoint.x += 3 * uu * t * c1.x;
        bezierPoint.y += 3 * uu * t * c1.y;
        bezierPoint.x += 3 * u * tt * c2.x;
        bezierPoint.y += 3 * u * tt * c2.y;
        bezierPoint.x += ttt * e.x;
        bezierPoint.y += ttt * e.y;
    }



//---绘制礼花--end



//---绘制拖影线--start
    /**
     * 初始化拖影线条变量
     * @param context
     */
    private void initLine(Context context){
        if(lineStyle==LINE_STYLE2){
            strokeWidthLine=10;
        }
        //对画笔初始化
        mPaintLine = new Paint();
        //设置画笔颜色
        mPaintLine.setColor(SkinUtils.getColor(R.color.colorlines));
        //设置画笔宽度
        mPaintLine.setStrokeWidth(strokeWidthLine);
        //设置抗锯齿-边缘位置
        mPaintLine.setAntiAlias(true);
        //防抖，使得颜色线条过渡柔和
        mPaintLine.setDither(true);
        // 设置填充画笔，只画圆边
        mPaintLine.setStyle(Paint.Style.STROKE);
        mPaintLine.setAlpha(alphaLine);
        //设置画笔变为圆滑状
        mPaintLine.setStrokeCap(Paint.Cap.ROUND);
    }

    /**
     *在线程中处理坐标，绘制拖影线条
     * @param canvas
     */
    private void workInRunnable(Canvas canvas){
        if (fingerX.peek() != null) {
            //鼠标不动时不记录坐标
            boolean is_add_mouse = Math.abs(fingerX.peek() - fingerCurrentX) < 0.01;
            if (!is_add_mouse) {
                fingerX.offer(fingerCurrentX);
                fingerY.offer(fingerCurrentY);
            }
            if (fingerX.size() > MAX_POINT_FOR_LINE || is_add_mouse) {
                fingerX.poll();
                fingerY.poll();
            }
        } else if (fingerBegin) {
            fingerX.offer(fingerCurrentX);
            fingerY.offer(fingerCurrentY);
        }

        if(lineStyle==LINE_STYLE2){
            drawLineStyle2(canvas);
        }else{
            drawLineStyle1(canvas);
        }
    }

    /**
     贝塞尔公式调用, 根据touch点集合，生成贝塞尔曲线上的一个点
     * @param theArrayX 触摸轨迹点x列表
     * @param theArrayY 触摸轨迹点y列表
     * @param t   触摸点的索引长度在touch点集合长度中百分比位置，相当于时间百分比[0,1],覆盖整条贝塞尔曲线
     * @return   返回贝塞尔曲线上的一个点
     */
    private float[] bezier(LinkedList<Float> theArrayX,
                           LinkedList<Float> theArrayY,
                           float t) {
        float x = 0;
        float y = 0;
        //控制点数组
        int n = theArrayX.size() - 1;
        int size = theArrayX.size();
        for (int index = 0; index < size; index++) {
            if(stopDrawLineForDown || !isRun){
                Log.i(TAG, "bezier no drawline is dowm " +
                        " stopDrawLineForDown="+stopDrawLineForDown
                        +" index="+index
                        +" size="+size
                );
                return null;
            }
            float itemX = theArrayX.get(index);
            float itemY = theArrayY.get(index);
            if (index == 0) {
                x += itemX * Math.pow((1 - t), n - index) * Math.pow(t, index);
                y += itemY * Math.pow((1 - t), n - index) * Math.pow(t, index);
            } else {
                //factorial为阶乘函数
                x += factorial(n) / factorial(index) / factorial(n - index) * itemX * Math.pow((1 - t), n - index) * Math.pow(t, index);
                y += factorial(n) / factorial(index) / factorial(n - index) * itemY * Math.pow((1 - t), n - index) * Math.pow(t, index);
            }
        }
        return new float[]{x, y};
    }

    /**
     * 阶乘函数
     * @param num
     * @return
     */
    private long factorial(int num) {
        if (num < 0) {
            return -1;
        } else if (num == 0 || num == 1) {
            return 1;
        } else {
            return (num * factorial(num - 1));
        }
    }

    /**
     随机生成rbg颜色值
     * @param t
     * @return
     */
    private int[] rainBow(float t) {
        int red, green, blue;
        if (t < 0.334) {
            red = (int) (255 - t * 3 * 255);
            green = (int) (t * 3 * 255);
            blue = 0;
        } else if (t < 0.667) {
            red = 0;
            green = (int) (255 - (t - 0.334) * 3 * 255);
            blue = (int) ((t - 0.334) * 3 * 255);
        } else {
            red = (int) ((t - 0.667) * 3 * 255);
            green = 0;
            blue = (int) (255 - (t - 0.667) * 3 * 255);
        }
        return new int[]{red, green, blue};
    }

    /**
     * 绘制拖影线条 风格1
     * @param canvas
     */
    private void drawLineStyle1(Canvas canvas){
        int size = fingerX.size();
        if(size>0){
            if(stopDrawLineForDown){
                Log.w(TAG, "drawLineStyle1 no drawline is dowm "
                        +" stopDrawLineForDown="+stopDrawLineForDown
                        +" size="+size
                );
                return;
            }
            if(!checkNeedToDrawLine(size)){
                //Log.w(TAG, "drawLineStyle1 checkNeedToDrawLine false return size=" +size);
                return;
            }

            float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
            for (int i = 0; i < size; i++) {
                if(!isRun){
                    Log.w(TAG, "drawLineStyle1 rerurn isRun="+isRun );
                    clearWhenUnRun();
                    return;
                }
                if(stopDrawLineForDown){
                    Log.w(TAG, "drawLineStyle1 no drawline is dowm "
                            +" stopDrawLineForDown="+stopDrawLineForDown
                            +" size="+size
                            +" i="+i
                    );
                    return;
                }
                float percent = (float) i / size;
                float[] res = bezier((LinkedList) fingerX, (LinkedList) fingerY, percent);
                if(res==null){
                    Log.w(TAG, "drawLineStyle1 res isnull return stopDrawLineForDown="+stopDrawLineForDown );
                    return;
                }
                x1 = res[0];
                y1 = res[1];

                if (i == 0) {
                    x2 = x1;
                    y2 = y1;
                    continue;
                }

                mPaintLine.setStrokeWidth(percent * strokeWidthLine);
                canvas.drawLine(x1, y1, x2, y2, mPaintLine);
                if (i == size - 1) {
                    //连接最后一段与当前touch点
                    canvas.drawLine(x1, y1, fingerCurrentX, fingerCurrentY, mPaintLine);
                }
                x2 = x1;
                y2 = y1;

                /*
                Log.i(TAG, "drawLineStyle1 "
                        +" size="+size
                        +" i="+i
                        +" alphaLine="+alphaLine
                        +" x1=" + x1
                        +" y1=" + y1
                );*/
            }
        }
    }

    /**
     * 绘制拖影线条，风格2
     * @param canvas
     */
    private void drawLineStyle2(Canvas canvas){
        int size = fingerX.size();
        if(size>0){
            if(stopDrawLineForDown){
                Log.w(TAG, "drawLineStyle2 no drawline is dowm "
                        +" stopDrawLineForDown="+stopDrawLineForDown
                        +" size="+size
                );
                return;
            }
            if(!checkNeedToDrawLine(size)){
                Log.w(TAG, "drawLineStyle2 checkNeedToDrawLine false return size=" +size);
                return;
            }

            float x1 = 0, y1 = 0 ;
            mPath.reset();
            for (int i = 0; i < size; i++) {
                if(!isRun){
                    Log.w(TAG, "drawLineStyle1 rerurn isRun="+isRun );
                    clearWhenUnRun();
                    return;
                }

                if(stopDrawLineForDown){
                    Log.w(TAG, "drawLineStyle2 no drawline is dowm "
                            +" stopDrawLineForDown="+stopDrawLineForDown
                            +" size="+size
                            +" i="+i
                    );
                    return;
                }
                float percent = (float) i / size;
                float[] res = bezier((LinkedList) fingerX, (LinkedList) fingerY, percent);
                if(res==null){
                    Log.w(TAG, "drawLineStyle2 res isnull return stopDrawLineForDown="+stopDrawLineForDown );
                    return;
                }
                x1 = res[0];
                y1 = res[1];
                if (i == 0) {
                    mPath.moveTo(x1, y1);
                    continue;
                }
                mPath.lineTo(x1, y1);
                /*
                Log.i(TAG, "drawLineStyle2 "
                        +" size="+size
                        +" i="+i
                        +" x1=" + x1
                        +" y1=" + y1
                );
                */
            }
            canvas.drawPath(mPath, mPaintLine);
        }
    }

    /**
     * 绘制拖影线条时相邻太近的点不再绘制，否则会闪烁
     * @param size
     * @return
     */
    private boolean checkNeedToDrawLine(int size){
        if(!isActive){
            float firstX = (float)((LinkedList) fingerX).get(0);
            float firstY = (float)((LinkedList) fingerY).get(0);
            float lastX = (float)((LinkedList) fingerX).get(size-1);
            float lastY = (float)((LinkedList) fingerY).get(size-1);
            int dX=(int)(lastX-firstX);
            int dY=(int)(lastY-firstY);
            dX=Math.abs(dX);
            dY=Math.abs(dY);
            /*
            Log.i(TAG, "checkNeedToDrawLine "
                    +" size="+size
                    +" dX="+dX
                    +" dY="+dY
            );*/
            if(dX ==0 && dY ==0){
                return false;
            }
        }
        return true;
    }

    //---绘制拖影线--end
    private void clearWhenUnRun(){
        Log.i(TAG, "clearWhenUnRun");
        fingerY.clear();
        fingerX.clear();
        mPath.reset();
    }

    private  Handler mH=new Handler(){
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what){
                case MSG_DELAY_START:
                    initHodler();
                    startDraw();
                    break;
            }
        }
    };

    /**
     * 注意因为touch事件是由mSparkView直接将触摸事件参数原封不动传给 viewer，
     * 所以mSparkView和 viewer 的大小必须一致，否则，在viewer中的触摸事件相对位置有问题
     * @param viewer  特效触摸时需要touch事件的底层view
     */
    public void setViewThatNeedTouch(View viewer) {
        this.viewer = viewer;
    }


    /**
     * surfaceview create 时内部调用
     */
    private void startDraw(){
        isRun=true;
        //clearOne=true;
        if(mThread==null){
            mThread=new Thread(SparkView.this);
            mThread.start();
            mThread.setPriority(Thread.MAX_PRIORITY);
        }
    }

    /**
     * 外部调用，再退出activity界面时调用，及时结束线程刷新，
     * 因为surfaceview在退出界面时系统会自动销毁，销毁的过程有一定的周期，
     * 不能及时停止线程，再进activity时可能会有脏点clear不了
     */
    public void stopDraw(){
        isRun=false;
        mH.removeMessages(MSG_DELAY_START);
        mCurDistance = 0;
        mDistance = 0;
        radius = 0;
        mThread=null;
    }


}
