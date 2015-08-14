#include "android_backport_webp_WebPFactory.h"
#include "android_backport_webp.h"

#include <stdio.h>
#include <string.h>
#include <android/bitmap.h>
#include <android/log.h>
#include <webp/decode.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     android_backport_webp_WebPFactory
 * Method:    nativeDecodeByteArray
 * Signature: ([BLandroid/graphics/BitmapFactory/Options;)Landroid/graphics/Bitmap;
 */
JNIEXPORT jobject JNICALL Java_android_backport_webp_WebPFactory_nativeDecodeByteArray
  (JNIEnv *jniEnv, jclass, jbyteArray byteArray, jobject options)
{
	// Check if input is valid
	if(!byteArray)
	{
		jniEnv->ThrowNew(jrefs::java::lang::NullPointerException->jclassRef, "Input buffer can not be null");
		return 0;
	}

	// Lock buffer
	jbyte* inputBuffer = jniEnv->GetByteArrayElements(byteArray, NULL);
	size_t inputBufferLen = jniEnv->GetArrayLength(byteArray);

	// Validate image
	int bitmapWidth = 0;
	int bitmapHeight = 0;
	if(!WebPGetInfo((uint8_t*)inputBuffer, inputBufferLen, &bitmapWidth, &bitmapHeight))
	{
		jniEnv->ReleaseByteArrayElements(byteArray, inputBuffer, JNI_ABORT);
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Invalid WebP format");
		return 0;
	}

	// Check if size is all what we were requested to do
	if(options && jniEnv->GetBooleanField(options, jrefs::android::graphics::BitmapFactory->Options.inJustDecodeBounds) == JNI_TRUE)
	{
		// Set values
		jniEnv->SetIntField(options, jrefs::android::graphics::BitmapFactory->Options.outWidth, bitmapWidth);
		jniEnv->SetIntField(options, jrefs::android::graphics::BitmapFactory->Options.outHeight, bitmapHeight);

		// Unlock buffer
		jniEnv->ReleaseByteArrayElements(byteArray, inputBuffer, JNI_ABORT);

		return 0;
	}

	// Initialize decoder config and configure scaling if requested
	WebPDecoderConfig config;
	if (!WebPInitDecoderConfig(&config))
	{
		jniEnv->ReleaseByteArrayElements(byteArray, inputBuffer, JNI_ABORT);
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Unable to init WebP decoder config");
		return 0;
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

	__android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Decoding into %dx%d bitmap", bitmapWidth, bitmapHeight);

	// Create bitmap
	jobject value__ARGB_8888 = jniEnv->GetStaticObjectField(jrefs::android::graphics::Bitmap->Config.jclassRef, jrefs::android::graphics::Bitmap->Config.ARGB_8888);
	jobject outputBitmap = jniEnv->CallStaticObjectMethod(jrefs::android::graphics::Bitmap->jclassRef, jrefs::android::graphics::Bitmap->createBitmap,
		(jint)bitmapWidth, (jint)bitmapHeight,
		value__ARGB_8888);
	if(!outputBitmap)
	{
		jniEnv->ReleaseByteArrayElements(byteArray, inputBuffer, JNI_ABORT);
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to allocate Bitmap");
		return 0;
	}
	outputBitmap = jniEnv->NewLocalRef(outputBitmap);

	// Get information about bitmap passed
	AndroidBitmapInfo bitmapInfo;
	if(AndroidBitmap_getInfo(jniEnv, outputBitmap, &bitmapInfo) != ANDROID_BITMAP_RESUT_SUCCESS)
	{
		jniEnv->ReleaseByteArrayElements(byteArray, inputBuffer, JNI_ABORT);
		jniEnv->DeleteLocalRef(outputBitmap);
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to get Bitmap information");
		return 0;
	}

	// Lock pixels
	void* bitmapPixels = 0;
	if(AndroidBitmap_lockPixels(jniEnv, outputBitmap, &bitmapPixels) != ANDROID_BITMAP_RESUT_SUCCESS)
	{
		jniEnv->ReleaseByteArrayElements(byteArray, inputBuffer, JNI_ABORT);
		jniEnv->DeleteLocalRef(outputBitmap);
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to lock Bitmap pixels");
		return 0;
	}

	// Decode to ARGB
	config.output.colorspace = MODE_RGBA;
	config.output.u.RGBA.rgba = (uint8_t*)bitmapPixels;
	config.output.u.RGBA.stride = bitmapInfo.stride;
	config.output.u.RGBA.size = bitmapInfo.height * bitmapInfo.stride;
	config.output.is_external_memory = 1;
	if (WebPDecode((uint8_t*)inputBuffer, inputBufferLen, &config) != VP8_STATUS_OK)
	{
		AndroidBitmap_unlockPixels(jniEnv, outputBitmap);
		jniEnv->ReleaseByteArrayElements(byteArray, inputBuffer, JNI_ABORT);
		jniEnv->DeleteLocalRef(outputBitmap);
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to decode WebP pixel data");
		return 0;
	}

	// Unlock pixels
	if(AndroidBitmap_unlockPixels(jniEnv, outputBitmap) != ANDROID_BITMAP_RESUT_SUCCESS)
	{
		jniEnv->ReleaseByteArrayElements(byteArray, inputBuffer, JNI_ABORT);
		jniEnv->DeleteLocalRef(outputBitmap);
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to unlock Bitmap pixels");
		return 0;
	}
	
	// Unlock buffer
	jniEnv->ReleaseByteArrayElements(byteArray, inputBuffer, JNI_ABORT);

	return outputBitmap;
}

#ifdef __cplusplus
}
#endif