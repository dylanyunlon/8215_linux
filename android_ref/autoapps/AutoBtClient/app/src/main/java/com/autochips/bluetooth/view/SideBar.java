package com.autochips.bluetooth.view;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.autochips.bluetooth.R;
import com.autochips.bluetooth.skin.SkinUtils;

public class SideBar extends View {

    private OnTouchingLetterChangedListener onTouchingLetterChangedListener;

    public static String[] b = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "#"};

    private int choose = -1;//
    private Paint paint = new Paint();
    private TextView mTextDialog;
    private int txt_color = 0xffffff;
    private int txt_size = 22;
    private int direction = 0;//0 horizontal     1 vertical
    private int mBackgroundColor = -1;

    // 选中颜色默认
    private int selectColor;

    private Bitmap mTextBackgroundBitmap;

    private int mBitmapWidth;

    private int mBitmapHeight;

    public SideBar(Context context) {
        super(context);
    }


    public SideBar(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    public SideBar(Context context, AttributeSet attrs) {
        super(context, attrs);
//        txt_color = context.getResources().getColor(R.color.tel_search_txt_color_uix);
        txt_color = SkinUtils.getColor(R.color.tel_search_txt_color_uix);
        txt_size = SkinUtils.getInteger(R.integer.side_bar_text_size);
        init(attrs);
    }

    private void init(AttributeSet attrs) {
        TypedArray attributes = getContext().obtainStyledAttributes(attrs, R.styleable.SideBar, 0, 0);
        selectColor = attributes.getColor(R.styleable.SideBar_selectColor, 0xffc60000);
        Drawable drawable = attributes.getDrawable(R.styleable.SideBar_textBgColor);
        if (drawable != null) {
            mTextBackgroundBitmap = drawableToBitmap(drawable);
        }
        txt_size = attributes.getDimensionPixelSize(R.styleable.SideBar_textSize, 22);
        // 获取宽高属性
        mBitmapWidth = attributes.getDimensionPixelSize(R.styleable.SideBar_textBgWidth, ViewGroup.LayoutParams.WRAP_CONTENT);
        mBitmapHeight = attributes.getDimensionPixelSize(R.styleable.SideBar_textBgHeight, ViewGroup.LayoutParams.WRAP_CONTENT);
        attributes.recycle();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        int height = getHeight();//
        int width = getWidth(); //

        float singleHeight = 0x00;
        float singleWidth = 0x00;
        if (direction == 0x00) {
            singleWidth = (width * 1f) / b.length;//
            singleWidth = (width * 1f - singleWidth / 2) / b.length;
        } else {
            singleHeight = (height * 1f) / b.length;//
            singleHeight = (height * 1f - singleHeight / 2) / b.length;
        }
        if (mBackgroundColor != -1) {
            canvas.drawColor(mBackgroundColor);
        }
        // 计算Bitmap的缩放比例并进行缩放
        Bitmap scaledBitmap = null;
        if (mTextBackgroundBitmap != null) {
            scaledBitmap = Bitmap.createScaledBitmap(mTextBackgroundBitmap, mBitmapWidth, mBitmapHeight, true);
        }
        for (int i = 0; i < b.length; i++) {
            paint.setColor(txt_color);
            paint.setTypeface(Typeface.DEFAULT_BOLD);
            paint.setAntiAlias(true);
            paint.setTextSize(txt_size);

            // 绘制Bitmap作为文本背景
            if (scaledBitmap != null) {
                if (direction == 0x00) {
                    float xPos = singleWidth * i + singleWidth - mBitmapWidth / 2;
                    canvas.drawBitmap(scaledBitmap, xPos - 2, 0, null);

                } else {
                    float xPos = width / 2 - paint.measureText(b[i]) / 2;
                    float yPos = singleHeight * i + singleHeight;
                    canvas.drawBitmap(scaledBitmap, xPos, yPos, null);
                }
            }


            // 绘制字符
            if (i == choose) {
                paint.setColor(selectColor);
            }
            if (direction == 0x00) {
                float yPos = height / 2 - paint.measureText(b[i]) / 2;
                float xPos = singleWidth * i + singleWidth;
                canvas.drawText(b[i], xPos - 8, 24, paint);
            } else {
                float xPos = width / 2 - paint.measureText(b[i]) / 2;
                float yPos = singleHeight * i + singleHeight;
                canvas.drawText(b[i], xPos, yPos, paint);
            }
            paint.reset();
        }

    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent event) {
        getParent().requestDisallowInterceptTouchEvent(true);
        final int action = event.getAction();
        final float x = event.getX();//
        final float y = event.getY();
        final int oldChoose = choose;
        final OnTouchingLetterChangedListener listener = onTouchingLetterChangedListener;
        int c = -1;
        if (direction == 0x00) {
            c = (int) (x / getWidth() * b.length);
        } else {
            c = (int) (y / getHeight() * b.length);
        }

        switch (action) {
            case MotionEvent.ACTION_UP:
                //setBackgroundResource(R.drawable.side_bg);
                //choose = -1;//
                invalidate();
                if (mTextDialog != null) {
                    mTextDialog.setVisibility(View.INVISIBLE);
                }
                break;

            default:
                //setBackgroundResource(R.drawable.side_bg);
                if (oldChoose != c) {
                    if (c >= 0 && c < b.length) {
                        if (listener != null) {
                            listener.onTouchingLetterChanged(b[c]);
                        }
                        if (mTextDialog != null) {
                            mTextDialog.setText(b[c]);
                            mTextDialog.setVisibility(View.VISIBLE);
                        }

                        choose = c;
                        invalidate();
                    }
                }

                break;
        }
        return true;
    }

    /**
     * @param onTouchingLetterChangedListener
     */
    public void setOnTouchingLetterChangedListener(OnTouchingLetterChangedListener onTouchingLetterChangedListener) {
        this.onTouchingLetterChangedListener = onTouchingLetterChangedListener;
    }

    /**
     * @author coder
     */
    public interface OnTouchingLetterChangedListener {
        public void onTouchingLetterChanged(String s);
    }


    public void setChoose(int pos) {
        if (pos >= 0 && pos < b.length) {
            this.choose = pos;
        } else {
            this.choose = -1;
        }
        invalidate();
    }

    public void setTextView(TextView mTextDialog) {
        this.mTextDialog = mTextDialog;
    }

    public void setDirection(int direction) {
        this.direction = direction;
    }

    public void setShowBackground(int color) {
        this.mBackgroundColor = color;
    }

    public void setTextSize(int txt_size) {
        this.txt_size = txt_size;
    }

    private Bitmap drawableToBitmap(Drawable drawable) {
        if (drawable instanceof BitmapDrawable) {
            return ((BitmapDrawable) drawable).getBitmap();
        }

        int width = drawable.getIntrinsicWidth();
        int height = drawable.getIntrinsicHeight();
        Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bitmap);
        drawable.setBounds(0, 0, canvas.getWidth(), canvas.getHeight());
        drawable.draw(canvas);

        return bitmap;
    }

    private int dpToPx(int dp) {
        float density = getContext().getResources().getDisplayMetrics().density;
        return Math.round(dp * density);
    }


}