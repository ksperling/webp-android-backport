#include "android_backport_webp_WebPFactory.h"
#include "android_backport_webp.h"

#include <stdio.h>
#include <string.h>
#include <android/bitmap.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <webp/decode.h>

#ifdef __cplusplus
extern "C" {
#endif


static jobject WebPFactory_decodeBuffer
  (JNIEnv *jniEnv, const uint8_t *buffer, size_t length, jobject options)
{
	// Validate image
	int bitmapWidth = 0;
	int bitmapHeight = 0;
	if (!WebPGetInfo(buffer, length, &bitmapWidth, &bitmapHeight))
	{
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Invalid WebP format");
		return NULL;
	}

	// Check if size is all what we were requested to do
	if(options && jniEnv->GetBooleanField(options, jrefs::android::graphics::BitmapFactory->Options.inJustDecodeBounds) == JNI_TRUE)
	{
		jniEnv->SetIntField(options, jrefs::android::graphics::BitmapFactory->Options.outWidth, bitmapWidth);
		jniEnv->SetIntField(options, jrefs::android::graphics::BitmapFactory->Options.outHeight, bitmapHeight);
		return NULL;
	}

	// Initialize decoder config and configure scaling if requested
	WebPDecoderConfig config;
	if (!WebPInitDecoderConfig(&config))
	{
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Unable to init WebP decoder config");
		return NULL;
	}

	if (options)
	{
		jint inSampleSize = jniEnv->GetIntField(options, jrefs::android::graphics::BitmapFactory->Options.inSampleSize);
		if (inSampleSize > 1)
		{
			config.options.use_scaling = 1;
			config.options.scaled_width = bitmapWidth /= inSampleSize;
			config.options.scaled_height = bitmapHeight /= inSampleSize;
		}
	}

	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Decoding into %dx%d bitmap", bitmapWidth, bitmapHeight);

	// Create bitmap
	jobject value__ARGB_8888 = jniEnv->GetStaticObjectField(jrefs::android::graphics::Bitmap->Config.jclassRef, jrefs::android::graphics::Bitmap->Config.ARGB_8888);
	jobject outputBitmap = jniEnv->CallStaticObjectMethod(jrefs::android::graphics::Bitmap->jclassRef, jrefs::android::graphics::Bitmap->createBitmap,
		(jint)bitmapWidth, (jint)bitmapHeight,
		value__ARGB_8888);
	if (!outputBitmap)
	{
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to allocate Bitmap");
		return NULL;
	}

	// Get information about bitmap passed
	AndroidBitmapInfo bitmapInfo;
	if (AndroidBitmap_getInfo(jniEnv, outputBitmap, &bitmapInfo) != ANDROID_BITMAP_RESULT_SUCCESS)
	{
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to get Bitmap information");
		return NULL;
	}

	// Lock pixels
	void *bitmapPixels = 0;
	if (AndroidBitmap_lockPixels(jniEnv, outputBitmap, &bitmapPixels) != ANDROID_BITMAP_RESULT_SUCCESS)
	{
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to lock Bitmap pixels");
		return NULL;
	}

	// Decode to ARGB
	config.output.colorspace = MODE_RGBA;
	config.output.u.RGBA.rgba = (uint8_t*)bitmapPixels;
	config.output.u.RGBA.stride = bitmapInfo.stride;
	config.output.u.RGBA.size = bitmapInfo.height * bitmapInfo.stride;
	config.output.is_external_memory = 1;
	if (WebPDecode(buffer, length, &config) != VP8_STATUS_OK)
	{
		AndroidBitmap_unlockPixels(jniEnv, outputBitmap);
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to decode WebP pixel data");
		return NULL;
	}

	// Unlock pixels
	if (AndroidBitmap_unlockPixels(jniEnv, outputBitmap) != ANDROID_BITMAP_RESULT_SUCCESS)
	{
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to unlock Bitmap pixels");
		return NULL;
	}

	return outputBitmap;
}

/*
 * static native Bitmap nativeDecodeByteArray(byte[] data, BitmapFactory.Options options);
 */
JNIEXPORT jobject JNICALL Java_android_backport_webp_WebPFactory_nativeDecodeByteArray
  (JNIEnv *jniEnv, jclass, jbyteArray byteArray, jobject options)
{
	if (!byteArray)
	{
		jniEnv->ThrowNew(jrefs::java::lang::NullPointerException->jclassRef, "buffer is null");
		return NULL;
	}

	jbyte *buffer = jniEnv->GetByteArrayElements(byteArray, NULL);
	if (!buffer)
	{
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to lock byte[] buffer");
		return NULL;
	}
	size_t length = jniEnv->GetArrayLength(byteArray);
	jobject outputBitmap = WebPFactory_decodeBuffer(jniEnv, (const uint8_t *) buffer, length, options);

	jniEnv->ReleaseByteArrayElements(byteArray, buffer, JNI_ABORT);
	return outputBitmap;
}

/**
 * static Bitmap nativeDecodeAsset(AssetManager am, String name, BitmapFactory.Options options) throws IOException;
 */
JNIEXPORT jobject JNICALL Java_android_backport_webp_WebPFactory_nativeDecodeAsset
  (JNIEnv *jniEnv, jclass, jobject assetManager, jstring name, jobject options)
{
	if (!assetManager) {
		jniEnv->ThrowNew(jrefs::java::lang::NullPointerException->jclassRef, "assetManager is null");
		return NULL;
	}
	if (!name) {
		jniEnv->ThrowNew(jrefs::java::lang::NullPointerException->jclassRef, "name is null");
		return NULL;
	}

	AAssetManager *aam = AAssetManager_fromJava(jniEnv, assetManager);
	if (!aam)
	{
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to access AssetManager");
		return NULL;
	}

	const char *namechars = jniEnv->GetStringUTFChars(name, NULL);
	AAsset *asset = AAssetManager_open(aam, namechars, AASSET_MODE_BUFFER);
	if (!asset)
	{
		jniEnv->ThrowNew(jrefs::java::io::FileNotFoundException->jclassRef, namechars);
		jniEnv->ReleaseStringUTFChars(name, namechars);
		return NULL;
	}

	const void *buffer = AAsset_getBuffer(asset);
	if (!buffer)
	{
		jniEnv->ReleaseStringUTFChars(name, namechars);
		AAsset_close(asset);
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to get asset buffer");
		return NULL;
	}

	size_t length = AAsset_getLength(asset);
	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Decoding asset '%s' (%d bytes)", namechars, length);
	jobject outputBitmap = WebPFactory_decodeBuffer(jniEnv, (const uint8_t *)buffer, length, options);

	jniEnv->ReleaseStringUTFChars(name, namechars);
	AAsset_close(asset);
	return outputBitmap;
}

#ifdef __cplusplus
}
#endif