#include <jni.h>
/* Header for class android_backport_webp_WebPFactory */

#ifndef _Included_android_backport_webp_WebPFactory
#define _Included_android_backport_webp_WebPFactory
#ifdef __cplusplus
extern "C" {
#endif

/*
 * static native Bitmap nativeDecodeByteArray(byte[] data, BitmapFactory.Options options);
 */
JNIEXPORT jobject JNICALL Java_android_backport_webp_WebPFactory_nativeDecodeByteArray
  (JNIEnv *jniEnv, jclass, jbyteArray byteArray, jobject options);

/**
 * static Bitmap nativeDecodeAsset(AssetManager am, String name, BitmapFactory.Options options) throws IOException;
 */
JNIEXPORT jobject JNICALL Java_android_backport_webp_WebPFactory_nativeDecodeAsset
  (JNIEnv *jniEnv, jclass, jobject assetManager, jstring name, jobject options);

#ifdef __cplusplus
}
#endif
#endif
