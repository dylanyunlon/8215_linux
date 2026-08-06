package com.hcn.bluetooth.bean;


public class CacheData {
   private String  TAG="CacheData";
   private static volatile CacheData mHModule = null;
   public static CacheData getInstance() {
      if (mHModule == null) {
         synchronized (CacheData.class) {
            if (mHModule == null) {
               mHModule = new CacheData();
            }
         }
      }
      return mHModule;
   }

   /**AA是否连接*/
   private boolean isAndroidAutoMode=false;

   public boolean isAndroidAutoMode() {
      return isAndroidAutoMode;
   }

   public void setAndroidAutoMode(boolean androidAutoMode) {
      isAndroidAutoMode = androidAutoMode;
   }

}
