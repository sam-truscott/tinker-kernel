package uk.co.wumpus.tinker.builder.util;

public final class IntToByte {

	private IntToByte() {
		/* hidden */
	}

	public static byte[] intToByteArrayBigEndian(int value) {
	    return new byte[] {
	            (byte)(value >>> 24),
	            (byte)(value >>> 16),
	            (byte)(value >>> 8),
	            (byte)value};
	}
	    
	public static byte[] intToByteArrayLittleEndian(int value) {
	    return new byte[] {
	            (byte)(value),
	            (byte)(value >>> 8),
	            (byte)(value >>> 16),
	            (byte)(value >>> 24)};
	}
	
	public static byte[] intToByteArray(final int value, final Endian e) {
		switch (e) {
		case BIG:
			return intToByteArrayBigEndian(value);
		case SMALL:
			return intToByteArrayLittleEndian(value);
		default:
			throw new IllegalArgumentException("Unsupported endianness");
		}
	}
}
