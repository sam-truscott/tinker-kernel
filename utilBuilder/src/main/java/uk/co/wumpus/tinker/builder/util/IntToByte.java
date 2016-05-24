package uk.co.wumpus.tinker.builder.util;

public final class IntToByte {

	private IntToByte() {
		/* hidden */
	}

	private static byte[] intToByteArrayBigEndian(int value) {
	    return new byte[] {
	            (byte)(value >>> 24),
	            (byte)(value >>> 16),
	            (byte)(value >>> 8),
	            (byte)value};
	}
	    
	private static byte[] intToByteArrayLittleEndian(int value) {
	    return new byte[] {
	            (byte)(value),
	            (byte)(value >>> 8),
	            (byte)(value >>> 16),
	            (byte)(value >>> 24)};
	}
	
	public static byte[] intToByteArray(final int value, final Endian e) {
		if (e == null) {
			throw new IllegalArgumentException("Endianess is null");
		}
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
