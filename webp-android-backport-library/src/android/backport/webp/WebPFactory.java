package android.backport.webp;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

/**
 * Factory to encode and decode WebP images into Android Bitmap
 * @author Alexey Pelykh
 */
public final class WebPFactory {
	static {
		System.loadLibrary("webpbackport");
	}

	/**
	 * Decodes byte array to bitmap 
	 * @param data Byte array with WebP bitmap data
	 * @param options Options to control decoding. Accepts null
	 * @return Decoded bitmap
	 */
	public static native Bitmap nativeDecodeByteArray(byte[] data, BitmapFactory.Options options);
	
}
