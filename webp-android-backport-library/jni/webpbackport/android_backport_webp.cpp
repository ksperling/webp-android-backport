#include "android_backport_webp.h"

#include <jni.h>
#include <android/log.h>

const char* const LOG_TAG = "android.backport.webp:native";

static jclass FindClassGlobal(JNIEnv* jniEnv, const char* name)
{
	jclass local = jniEnv->FindClass(name);
	if (jniEnv->ExceptionCheck()) return 0;
	return (jclass) jniEnv->NewGlobalRef(local);
}

namespace jrefs {

namespace java {
namespace lang {

jclass_NullPointerException::jclass_NullPointerException(JNIEnv* jniEnv)
{
	jclassRef = FindClassGlobal(jniEnv, "java/lang/NullPointerException");
}
jclass_NullPointerException* NullPointerException = 0;

jclass_IllegalArgumentException::jclass_IllegalArgumentException(JNIEnv* jniEnv)
{
	jclassRef = FindClassGlobal(jniEnv, "java/lang/IllegalArgumentException");
}
jclass_IllegalArgumentException* IllegalArgumentException = 0;

jclass_RuntimeException::jclass_RuntimeException(JNIEnv* jniEnv)
{
	jclassRef = FindClassGlobal(jniEnv, "java/lang/RuntimeException");
}
jclass_RuntimeException* RuntimeException = 0;

} // namespace lang
} // namespace java

namespace android {
namespace graphics {

jclass_Bitmap* Bitmap = 0;
jclass_Bitmap::jclass_Bitmap(JNIEnv* jniEnv)
	: Config(jniEnv)
{
	jclassRef = FindClassGlobal(jniEnv, "android/graphics/Bitmap");
	if (jniEnv->ExceptionCheck()) return;
	createBitmap = jniEnv->GetStaticMethodID(jclassRef,
		"createBitmap",
		"(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
}

jclass_Bitmap::jclass_Config::jclass_Config(JNIEnv* jniEnv)
{
	jclassRef = FindClassGlobal(jniEnv, "android/graphics/Bitmap$Config");
	if (jniEnv->ExceptionCheck()) return;
	ARGB_8888 = jniEnv->GetStaticFieldID(jclassRef,
		"ARGB_8888",
		"Landroid/graphics/Bitmap$Config;");
}

jclass_BitmapFactory* BitmapFactory = 0;
jclass_BitmapFactory::jclass_BitmapFactory(JNIEnv* jniEnv)
	: Options(jniEnv)
{
}

jclass_BitmapFactory::jclass_Options::jclass_Options(JNIEnv* jniEnv)
{
	jclassRef = FindClassGlobal(jniEnv, "android/graphics/BitmapFactory$Options");
	if (jniEnv->ExceptionCheck()) return;
	inJustDecodeBounds = jniEnv->GetFieldID(jclassRef,
		"inJustDecodeBounds",
		"Z");
	if (jniEnv->ExceptionCheck()) return;
	outHeight = jniEnv->GetFieldID(jclassRef,
		"outHeight",
		"I");
	if (jniEnv->ExceptionCheck()) return;
	outWidth = jniEnv->GetFieldID(jclassRef,
		"outWidth",
		"I");
	if (jniEnv->ExceptionCheck()) return;
	inSampleSize = jniEnv->GetFieldID(jclassRef,
		"inSampleSize",
		"I");
}

} // namespace graphics
} // namespace android
} // namespace jrefs

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNICALL JNI_OnLoad
  (JavaVM *vm, void *reserved)
{
	JNIEnv *jniEnv = 0;
	if (vm->GetEnv((void **)&jniEnv, JNI_VERSION_1_6) != JNI_OK) return JNI_ERR;
	
	// Load Java classes
	jrefs::java::lang::IllegalArgumentException = new jrefs::java::lang::jclass_IllegalArgumentException(jniEnv);
	if (jniEnv->ExceptionCheck()) return JNI_ERR;
	jrefs::java::lang::NullPointerException = new jrefs::java::lang::jclass_NullPointerException(jniEnv);
	if (jniEnv->ExceptionCheck()) return JNI_ERR;
	jrefs::java::lang::RuntimeException = new jrefs::java::lang::jclass_RuntimeException(jniEnv);
	if (jniEnv->ExceptionCheck()) return JNI_ERR;
	jrefs::android::graphics::Bitmap = new jrefs::android::graphics::jclass_Bitmap(jniEnv);
	if (jniEnv->ExceptionCheck()) return JNI_ERR;
	jrefs::android::graphics::BitmapFactory = new jrefs::android::graphics::jclass_BitmapFactory(jniEnv);
	if (jniEnv->ExceptionCheck()) return JNI_ERR;

	return JNI_VERSION_1_6;
}

#ifdef __cplusplus
}
#endif