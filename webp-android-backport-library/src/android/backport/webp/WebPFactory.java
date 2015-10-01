package android.backport.webp;

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;

import java.io.IOException;

/**
 * Factory to encode and decode WebP images into Android Bitmap
 * @author Alexey Pelykh
 */
public final class WebPFactory {

	public static final boolean AVAILABLE;
	static {
		boolean available = false;
		try {
			System.loadLibrary("webpbackport");
			available = true;
		} catch (UnsatisfiedLinkError e) {
			Log.e("webp-backport", "Failed to load native library", e);
		}
		AVAILABLE = available;
	}

	/**
	 * Decodes byte array to bitmap 
	 * @param data Byte array with WebP bitmap data
	 * @param options Options to control decoding. Can be null.
	 * @return Decoded bitmap
	 */
	public static native Bitmap nativeDecodeByteArray(byte[] data, BitmapFactory.Options options);

	/**
	 * Decodes an asset to bitmap
	 * @param am AssetManager to use
	 * @param name asset file name, see {@link AssetManager#openFd(String)}
	 * @param options Options to control decoding. Can be null.
	 * @return Decoded bitmap
	 */
	public static native Bitmap nativeDecodeAsset(AssetManager am, String name, BitmapFactory.Options options) throws IOException;

}
