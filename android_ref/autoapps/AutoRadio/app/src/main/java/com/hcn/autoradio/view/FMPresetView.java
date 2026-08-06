package com.hcn.autoradio.view;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.view.View;
import android.widget.RadioButton;

import com.hcn.autoradio.R;
import com.hcn.autoradio.ScreenSpec;
import com.hcn.autoradio.data.RadioData;
import com.hcn.autoradio.skin.SkinID;
import com.hcn.autoradio.skin.SkinUtils;
import com.hcn.autoradio.skin.ThemeID;

public class FMPresetView extends RadioButton {

    private boolean mIsAimed = false;
    private Drawable mDrawable = null;

    private String mIndex;
    private String mFreqUnit;

    private Paint mIndexPaint;

    public FMPresetView(Context context) {
        this(context, null);
    }

    public FMPresetView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public FMPresetView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        mIndexPaint = new Paint();
    }

    public boolean isAimed() {
        return mIsAimed;
    }

    public void setAimed(boolean isAimed) {

        if (mIsAimed != isAimed) {
            mIsAimed = isAimed;

            if (mIsAimed) {
                mDrawable = getBackground();
                setBackgroundResource(SkinUtils.getId(R.drawable.radio_favitem_add));
            } else {
                setBackgroundDrawable(mDrawable);
            }
        }
    }

    public void setIndex(String index) {
        mIndex = index;
    }

    public String getFreqUnit() {
        return mFreqUnit;
    }

    public void setFreqUnit(String Unit) {
        mFreqUnit = Unit;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (mIndex != null) {
            mIndexPaint.reset();
            mIndexPaint.setAntiAlias(true);

            mIndexPaint.setColor(SkinUtils.getColor(R.color.preset_index_color));
            if (ScreenSpec.getScreenStatus() != ScreenSpec.FULL_SCREEN) {
                mIndexPaint.setTextSize(
                        getResources().getDimension(SkinUtils.getId(R.dimen.PresetViewIndexSmallTextSize)));
            } else {
                mIndexPaint.setTextSize(
                        getResources().getDimension(SkinUtils.getId(R.dimen.PresetViewIndexTextSize)));
            }

            drawDigitFreqView(canvas);
        }
        super.onDraw(canvas);
    }

    private void drawDigitFreqView(Canvas canvas) {
        if (SkinUtils.useSkinPackage()) {
            drawDigitFreqViewForSkin(canvas);
        } else {
            drawDigitFreqViewForMcc(canvas);
        }
    }

    /**
     * 绘制皮肤包UI的预存频点
     * @param canvas
     */
    private void drawDigitFreqViewForSkin(Canvas canvas) {
        Paint.FontMetrics fontMetrics = mIndexPaint.getFontMetrics();
        float distance = (fontMetrics.bottom - fontMetrics.top) / 2 - fontMetrics.bottom;
        float baseline = getHeight() * 0.5f + distance;

        switch (SkinUtils.getCurrentSkinID()) {
            case SkinID.SKIN_ZA01:
            case SkinID.SKIN_ZA03:
            case SkinID.SKIN_ZA04:
            case SkinID.SKIN_ZA05:
            case SkinID.SKIN_ZA33:
            case SkinID.SKIN_GB01:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.FULL_SCREEN) {
                    int width = getWidth();
                    int xOffsetIndex;
                    int xOffsetFreqUnit;

                    if (getResources().getConfiguration().getLayoutDirection() == View.LAYOUT_DIRECTION_RTL) {
                        xOffsetIndex = (int) (width * 0.65f);
                        xOffsetFreqUnit = (int) (width * 0.2f - mIndexPaint.measureText(getFreqUnit()));
                    } else {
                        xOffsetIndex = (int) (width * 0.3f);
                        xOffsetFreqUnit = (int) (width * 0.9f - mIndexPaint.measureText(getFreqUnit()));
                    }

                    // 绘制文本
                    canvas.drawText(mIndex, xOffsetIndex, baseline, mIndexPaint);

                    if (getFreqUnit() != null) {
                        canvas.drawText(getFreqUnit(), xOffsetFreqUnit, baseline, mIndexPaint);
                    }
                }
                break;
            case SkinID.SKIN_ZA09:
            case SkinID.SKIN_ZA10:
            case SkinID.SKIN_ZA12:
            case SkinID.SKIN_ZA36:
            case SkinID.SKIN_ZA37:
            case SkinID.SKIN_ZA39:
            case SkinID.SKIN_SA48:
            case SkinID.SKIN_XT144:
            case SkinID.SKIN_XT510:
            case SkinID.SKIN_RK01:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.FULL_SCREEN) {
                    canvas.drawText(mIndex, getWidth() * 0.1f, baseline, mIndexPaint);
                    if (getFreqUnit() != null) {
                        int mFreqUnitX = (int) (getWidth() * 0.9f - mIndexPaint.measureText(mFreqUnit));
                        canvas.drawText(getFreqUnit(), mFreqUnitX, baseline, mIndexPaint);
                    }
                }
                break;
            case SkinID.SKIN_SA82:
            case SkinID.SKIN_SA85:
            case SkinID.SKIN_SA87:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.FULL_SCREEN) {
                    canvas.drawText(mIndex, getWidth() * 0.09f, baseline, mIndexPaint);
                }
                break;
            case SkinID.SKIN_SA133:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.FULL_SCREEN) {
                    canvas.drawText(mIndex, getWidth() * 0.15f, baseline, mIndexPaint);
                }
                break;
            case SkinID.SKIN_SA143:
                baseline = getHeight() * 0.55f + distance;
                if (isChecked()) {
                    mIndexPaint.setColor(SkinUtils.getColor(R.color.preset_text_color_p));
                } else {
                    mIndexPaint.setColor(SkinUtils.getColor(R.color.preset_index_color));
                }
                if (ScreenSpec.getScreenStatus() == ScreenSpec.FULL_SCREEN) {
                    canvas.drawText(mIndex, getWidth() * 0.08f, baseline, mIndexPaint);
                }
                if (getFreqUnit() != null) {
                    int mFreqUnitX = (int) (getWidth() * 0.92f - mIndexPaint.measureText(mFreqUnit));
                    canvas.drawText(getFreqUnit(), mFreqUnitX, baseline, mIndexPaint);
                }
                break;
            case SkinID.SKIN_SA155:
                if (getFreqUnit() != null) {
                    int mFreqUnitX = (int) (getWidth() * 0.67f - mIndexPaint.measureText(mFreqUnit));
                    baseline = getHeight() * 0.7f + distance;
                    canvas.drawText(getFreqUnit(), mFreqUnitX, baseline, mIndexPaint);
                }
                break;
            case SkinID.SKIN_DZ16:
            case SkinID.SKIN_XT366:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.FULL_SCREEN) {
                    if (isChecked()) {
                        setText("");
                    } else {
                        if (getFreqUnit() != null) {
                            int mFreqUnitX = (int) (getWidth() * 0.65f - mIndexPaint.measureText(mFreqUnit));
                            canvas.drawText(getFreqUnit(), mFreqUnitX, baseline, mIndexPaint);
                        }
                    }
                }
                break;
            case SkinID.SKIN_DZ17:
                if (getFreqUnit() != null) {
                    int mFreqUnitX = (int) (getWidth() * 0.92f - mIndexPaint.measureText(mFreqUnit));
                    canvas.drawText(getFreqUnit(), mFreqUnitX, getHeight() * 0.97f, mIndexPaint);
                }
                if (ScreenSpec.getScreenStatus() == ScreenSpec.FULL_SCREEN) {
                    canvas.drawText(mIndex, getWidth() * 0.12f, getHeight() * 0.97f, mIndexPaint);
                }
                break;
            case SkinID.SKIN_XT554:
                if (isChecked()) {
                    setText("");
                } else {
                    baseline = getHeight() - 125;
                    canvas.drawText(mIndex, getWidth() * 0.23f, baseline, mIndexPaint);}
                break;
            case SkinID.SKIN_N91:
                if (isChecked()) {
                    mIndexPaint.setColor(SkinUtils.getColor(R.color.preset_text_color_p));
                } else {
                    mIndexPaint.setColor(SkinUtils.getColor(R.color.preset_index_color));
                }
                if (getFreqUnit() != null) {
                    int mFreqUnitX = (int) (getWidth() * 0.91f - mIndexPaint.measureText(mFreqUnit));
                    canvas.drawText(getFreqUnit(), mFreqUnitX, getHeight() * 0.90f, mIndexPaint);
                }
                if (ScreenSpec.getScreenStatus() == ScreenSpec.FULL_SCREEN) {
                    canvas.drawText(mIndex, getWidth() * 0.12f, getHeight() * 0.22f, mIndexPaint);
                }
                break;
            case SkinID.SKIN_NONE:
            default:
                mIndexPaint.setShadowLayer(1.0f, 1.0f, 2.0f, Color.BLACK);
                canvas.drawText(mIndex, (int) ((getWidth() - mIndexPaint.measureText(mIndex)) * 0.5), (int) (getHeight() * 0.33), mIndexPaint);
                break;
        }

    }

    /**
     * 绘制原MCC UI的预存频点
     * @param canvas
     */
    private void drawDigitFreqViewForMcc(Canvas canvas) {
        Paint.FontMetrics fontMetrics = mIndexPaint.getFontMetrics();
        float distance = (fontMetrics.bottom - fontMetrics.top) / 2 - fontMetrics.bottom;
        float baseline = getHeight() * 0.5f + distance;

        switch (RadioData.E_THEME_GOD) {
            case ThemeID.E_THEME_ID_153:
                if (getLayoutDirection() == LAYOUT_DIRECTION_RTL) {
                    int xPos = (int) ((getWidth() * 0.83 - mIndexPaint.measureText(mIndex) * 0.5));
                    canvas.drawText(mIndex, xPos, baseline, mIndexPaint);
                } else {
                    int xPos = (int) ((getWidth() * 0.33 - mIndexPaint.measureText(mIndex)) * 0.5);
                    canvas.drawText(mIndex, xPos, baseline, mIndexPaint);
                }
                break;
            case ThemeID.E_THEME_ID_203:
            case ThemeID.E_THEME_ID_403:
                canvas.drawText(mIndex, getWidth() * 0.05f, baseline, mIndexPaint);
                break;
            case ThemeID.E_THEME_ID_405:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.FULL_SCREEN) {
                    canvas.drawText(mIndex, getWidth() * 0.05f, baseline, mIndexPaint);
                }
                break;
            case ThemeID.E_THEME_ID_205:
                baseline = getHeight() * 0.55f + distance;
                canvas.drawText(mIndex, getWidth() * 0.09f, baseline, mIndexPaint);
                break;
            case ThemeID.E_THEME_ID_209:
                float xPos = 10f;
                float yPos = 25f;
                canvas.drawText(mIndex, xPos, yPos, mIndexPaint);
                if (getFreqUnit() != null) {
                    float mFreqUnitX = (getWidth() - mIndexPaint.measureText(mFreqUnit)) * 0.5f;
                    canvas.drawText(getFreqUnit(), mFreqUnitX, getHeight() * 0.9f, mIndexPaint);
                }
                break;
            case ThemeID.E_THEME_ID_400:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.FULL_SCREEN) {
                    if (RadioData.E_THEME_SUB == 21 || RadioData.E_THEME_SUB == 30) {
                        baseline = getHeight() * 0.2f + distance;
                        canvas.drawText(mIndex, getWidth() * 0.10f, baseline, mIndexPaint);
                    } else if (RadioData.E_THEME_SUB == 39) {
                        baseline = getHeight() * 0.24f + distance;
                        canvas.drawText(mIndex, getWidth() * 0.62f, baseline, mIndexPaint);
                    } else if (RadioData.E_THEME_SUB == 23) {
                        canvas.drawText(mIndex, getWidth() * 0.09f, baseline, mIndexPaint);
                    } else if (RadioData.E_THEME_SUB == 25) {
                        canvas.drawText(mIndex, getWidth() * 0.09f, baseline, mIndexPaint);
                        if (getFreqUnit() != null) {
                            int mFreqUnitX = (int) (getWidth() * 0.9f - mIndexPaint.measureText(mFreqUnit));
                            canvas.drawText(getFreqUnit(), mFreqUnitX, baseline, mIndexPaint);
                        }
                    } else if (RadioData.E_THEME_SUB == 27) {
                        mIndexPaint.setTextSize(
                                getResources().getDimension(SkinUtils.getId(R.dimen.PresetViewIndexTextSize)));
                        if (isChecked()) {
                            mIndexPaint.setColor(SkinUtils.getColor(R.color.preset_text_color_p));
                        }
                        baseline = getHeight() * 0.2f + distance;
                        canvas.drawText(mIndex, getWidth() * 0.41f, baseline, mIndexPaint);
                        if (getFreqUnit() != null) {
                            float mFreqUnitX = (getWidth() - mIndexPaint.measureText(mFreqUnit)) * 0.5f;
                            canvas.drawText(getFreqUnit(), mFreqUnitX, getHeight() * 0.9f, mIndexPaint);
                        }
                    } else if (RadioData.E_THEME_SUB == 28) {
                        mIndexPaint.setTextSize(
                                getResources().getDimension(SkinUtils.getId(R.dimen.PresetViewIndexTextSize)));
                        canvas.drawText(mIndex, getWidth() * 0.78f, getHeight() * 0.26f, mIndexPaint);
                    } else {
                        canvas.drawText(mIndex, getWidth() * 0.15f, baseline, mIndexPaint);
                        if (getFreqUnit() != null) {
                            int mFreqUnitX = (int) (getWidth() * 0.9f - mIndexPaint.measureText(
                                    mFreqUnit));
                            canvas.drawText(getFreqUnit(), mFreqUnitX, baseline, mIndexPaint);
                        }
                    }
                }
                break;
            case ThemeID.E_THEME_ID_404:
            case ThemeID.E_THEME_ID_408:
            case ThemeID.E_THEME_ID_409:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.FULL_SCREEN) {
                    canvas.drawText(mIndex, getWidth() * 0.15f, baseline, mIndexPaint);
                    if (getFreqUnit() != null) {
                        int mFreqUnitX = (int) (getWidth() * 0.9f - mIndexPaint.measureText(
                                mFreqUnit));
                        canvas.drawText(getFreqUnit(), mFreqUnitX, baseline, mIndexPaint);
                    }
                }
                break;
            case ThemeID.E_THEME_ID_401:
                canvas.drawText(mIndex, (int) ((40 - mIndexPaint.measureText(mIndex)) * 0.5), 35, mIndexPaint);
                break;
            default:
                mIndexPaint.setShadowLayer(1.0f, 1.0f, 2.0f, Color.BLACK);
                canvas.drawText(mIndex, (int) ((getWidth() - mIndexPaint.measureText(mIndex)) * 0.5), (int) (getHeight() * 0.33), mIndexPaint);
                break;
        }
    }
}
